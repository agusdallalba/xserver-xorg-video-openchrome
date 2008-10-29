/*
 * Copyright 2006 Thomas Hellström. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <picturestr.h>
#include "via_3d.h"
#include "via_3d_reg.h"
#include "via_drm.h"

typedef struct
{
    Bool supported;
    CARD32 col0;
    CARD32 col1;
    CARD32 al0;
    CARD32 al1;
} ViaCompositeOperator;

typedef struct
{
    CARD32 pictFormat;
    Bool dstSupported;
    Bool texSupported;
    CARD32 dstFormat;
    CARD32 texFormat;
} Via3DFormat;

static ViaCompositeOperator viaOperatorModes[256];
static Via3DFormat via3DFormats[256];

#define VIA_NUM_3D_OPCODES 19
#define VIA_NUM_3D_FORMATS 15
#define VIA_FMT_HASH(arg) (((((arg) >> 1) + (arg)) >> 8) & 0xFF)

/*
 * Operator, 
 */
static const CARD32 viaOpCodes[VIA_NUM_3D_OPCODES][5] = {
    {PictOpClear, 0x05, 0x45, 0x40, 0x80},
    {PictOpSrc, 0x15, 0x45, 0x50, 0x80},
    {PictOpDst, 0x05, 0x55, 0x40, 0x90},
    {PictOpOver, 0x15, 0x52, 0x50, 0x91},
    {PictOpOverReverse, 
     (HC_HABLFCa_InvOPC |
      HC_HABLCa_Csrc |
      HC_HABLFCa_Adst) >> 4,
     (HC_HABLFCb_InvOPC |
      HC_HABLCb_Cdst |
      HC_HABLFCb_HABLRCb) >> 2, 0x52, 0x90},
    {PictOpIn, 0x03, 0x45, 0x42, 0x80},
    {PictOpInReverse, 0x05, 0x42, 0x40, 0x81},
    {PictOpOut, 0x13, 0x45, 0x52, 0x80},
    {PictOpOutReverse, 0x05, 0x52, 0x40, 0x91},
    {PictOpAtop, 0x03, 0x52, 0x42, 0x91},
    {PictOpAtopReverse, 0x13, 0x42, 0x52, 0x81},
    {PictOpXor, 
     (HC_HABLFCa_InvOPC |
      HC_HABLCa_Csrc |
      HC_HABLFCa_Adst) >> 4,
     (HC_HABLFCb_InvOPC |
      HC_HABLCb_Cdst |
      HC_HABLFCb_Asrc) >> 2, 0x52, 0x91},
    {PictOpAdd, 0x15, 0x55, 0x50, 0x90},
    {PictOpDisjointClear, 0x05, 0x45, 0x40, 0x80},
    {PictOpDisjointSrc, 0x15, 0x45, 0x50, 0x80},
    {PictOpDisjointDst, 0x05, 0x55, 0x40, 0x90},
    {PictOpConjointClear, 0x05, 0x45, 0x40, 0x80},
    {PictOpConjointSrc, 0x15, 0x45, 0x50, 0x80},
    {PictOpConjointDst, 0x05, 0x55, 0x40, 0x90}
};

static const CARD32 viaFormats[VIA_NUM_3D_FORMATS][5] = {
    {PICT_x1r5g5b5, HC_HDBFM_RGB555, HC_HTXnFM_RGB555, 1, 1},
    {PICT_r5g6b5, HC_HDBFM_RGB565, HC_HTXnFM_RGB565, 1, 1},
    {PICT_a4r4g4b4, HC_HDBFM_ARGB4444, HC_HTXnFM_ARGB4444, 1, 1},
    {PICT_a1r5g5b5, HC_HDBFM_ARGB1555, HC_HTXnFM_ARGB1555, 1, 1},
    {PICT_x1b5g5r5, HC_HDBFM_BGR555, HC_HTXnFM_BGR555, 1, 1},
    {PICT_b5g6r5, HC_HDBFM_BGR565, HC_HTXnFM_BGR565, 1, 1},
    {PICT_a4b4g4r4, HC_HDBFM_ABGR4444, HC_HTXnFM_ABGR4444, 1, 1},
    {PICT_a1b5g5r5, HC_HDBFM_ABGR1555, HC_HTXnFM_ABGR1555, 1, 1},
    {PICT_x8r8g8b8, HC_HDBFM_ARGB0888, HC_HTXnFM_ARGB0888, 1, 1},
    {PICT_a8r8g8b8, HC_HDBFM_ARGB8888, HC_HTXnFM_ARGB8888, 1, 1},
    {PICT_x8b8g8r8, HC_HDBFM_ABGR0888, HC_HTXnFM_ABGR0888, 1, 1},
    {PICT_a8b8g8r8, HC_HDBFM_ABGR8888, HC_HTXnFM_ABGR8888, 1, 1},
    {PICT_a8, 0x00, HC_HTXnFM_A8, 0, 1},
    {PICT_a4, 0x00, HC_HTXnFM_A4, 0, 1},
    {PICT_a1, 0x00, HC_HTXnFM_A1, 0, 1}
};

/*
 * Load the binary representation of a float into a CARD32.
 */

static inline CARD32 viafc32(register float x)
{
    register union {
	CARD32 c;
	float f;
    } ufc;

    ufc.f = x;

    return ufc.c;
}

static CARD32
via3DDstFormat(int format)
{
    return via3DFormats[VIA_FMT_HASH(format)].dstFormat;
}

static CARD32
via3DTexFormat(int format)
{
    return via3DFormats[VIA_FMT_HASH(format)].texFormat;
}

static Bool
via3DDstSupported(int format)
{
    Via3DFormat *fm = via3DFormats + VIA_FMT_HASH(format);

    if (fm->pictFormat != format)
        return FALSE;
    return fm->dstSupported;
}

static Bool
via3DTexSupported(int format)
{
    Via3DFormat *fm = via3DFormats + VIA_FMT_HASH(format);

    if (fm->pictFormat != format)
        return FALSE;
    return fm->texSupported;
}

static void
viaSet3DDestination(Via3DState * v3d, struct _WsbmBufferObject *destBuf,
		    CARD32 destDelta, CARD32 pitch, int format)
{
    v3d->drawingDirty = TRUE;  /* Affects planemask format. */
    v3d->destDirty = TRUE;
    v3d->destBuf = destBuf;
    v3d->destDelta = destDelta;
    v3d->destPitch = pitch;
    v3d->destFormat = via3DDstFormat(format);
    v3d->destDepth = (v3d->destFormat < HC_HDBFM_ARGB0888) ? 16 : 32;
}

static void
viaSet3DDrawing(Via3DState * v3d, int rop,
                CARD32 planeMask, CARD32 solidColor, CARD32 solidAlpha)
{
    v3d->drawingDirty = TRUE;
    v3d->rop = rop;
    v3d->planeMask = planeMask;
    v3d->solidColor = solidColor;
    v3d->solidAlpha = solidAlpha;
}

static void
viaSet3DFlags(Via3DState * v3d, int numTextures,
              Bool writeAlpha, Bool writeColor, Bool blend)
{
    v3d->enableDirty = TRUE;
    v3d->blendDirty = TRUE;
    v3d->numTextures = numTextures;
    v3d->writeAlpha = writeAlpha;
    v3d->writeColor = writeColor;
    v3d->blend = blend;
}

static Bool
viaOrder(CARD32 val, CARD32 * shift)
{
    *shift = 0;

    while (val > (1 << *shift))
        (*shift)++;
    return (val == (1 << *shift));
}

static Bool
viaSet3DTexture(Via3DState * v3d, struct _WsbmBufferObject *buf,
		CARD32 delta, int tex, 
                CARD32 pitch, Bool npot, CARD32 width, CARD32 height,
                int format, ViaTextureModes sMode, ViaTextureModes tMode,
                ViaTexBlendingModes blendingMode,
		Bool premult)
{
    ViaTextureUnit *vTex = v3d->tex + tex;
    CARD32 pmFlags;

    vTex->list.buf = buf;
    vTex->list.delta = delta;
    vTex->npot = npot;
    if (!viaOrder(pitch, &vTex->textureLevel0Exp) && !vTex->npot)
        return FALSE;
    vTex->textureLevel0Pitch = pitch;
    if (!viaOrder(width, &vTex->textureLevel0WExp))
        return FALSE;
    if (!viaOrder(height, &vTex->textureLevel0HExp))
        return FALSE;

    if (pitch <= 4) {
        ErrorF("Warning: texture pitch <= 4 !\n");
    }

    vTex->textureFormat = via3DTexFormat(format);

    switch (blendingMode) {
    case via_src:
	vTex->texCsat =  HC_HTXnTBLCsat_MASK |
	    ((PICT_FORMAT_TYPE(format) == PICT_TYPE_A) ? 
	     HC_HTXnTBLCa_0 : HC_HTXnTBLCa_InvTOPC |  HC_HTXnTBLCa_0) |
	    HC_HTXnTBLCb_Tex |
	    HC_HTXnTBLCc_0;
	vTex->texAsat = HC_HTXnTBLAa_InvTOPA |
	    HC_HTXnTBLAa_HTXnTBLRA |
	    (PICT_FORMAT_A(format) ? 
	     HC_HTXnTBLAb_Atex : HC_HTXnTBLAb_Acur) |
	    HC_HTXnTBLAc_HTXnTBLRA;
	vTex->texRCa = 0x00000000;
	vTex->texRAa = 0x00000000;
	vTex->texBColDirty = TRUE;
	break;
    case via_src_onepix_mask: 
    case via_src_onepix_comp_mask:
	pmFlags = (premult) ? 
	    HC_HTXnTBLCa_HTXnTBLRC :  
	HC_HTXnTBLCa_0 | HC_HTXnTBLCa_InvTOPC; 
	vTex->texCsat =  HC_HTXnTBLCsat_MASK | 
	    ((PICT_FORMAT_TYPE(format) == PICT_TYPE_A) ?
	     HC_HTXnTBLCa_0 :  pmFlags) |
	    HC_HTXnTBLCb_Tex |
	    HC_HTXnTBLCc_0;
	vTex->texAsat =  HC_HTXnTBLAsat_MASK |		
	    HC_HTXnTBLAa_HTXnTBLRA |
	    (PICT_FORMAT_A(format) ? 
	     HC_HTXnTBLAb_Atex :
	     HC_HTXnTBLAb_Acur) |
	    HC_HTXnTBLAc_HTXnTBLRA;
	break;
    case via_mask:
	pmFlags = (premult) ? 
	    HC_HTXnTBLCa_Atex :  
	HC_HTXnTBLCa_0 | HC_HTXnTBLCa_InvTOPC; 
	vTex->texCsat =  HC_HTXnTBLCsat_MASK |  
	    pmFlags | 
	    HC_HTXnTBLCb_Cur | 
	    HC_HTXnTBLCc_0;
	vTex->texAsat =  HC_HTXnTBLAsat_MASK | 
	    HC_HTXnTBLAa_Atex |
	    HC_HTXnTBLAb_Acur |
	    HC_HTXnTBLAc_HTXnTBLRA;
	break;
    case via_comp_mask:
	pmFlags = (premult) ? 
	    HC_HTXnTBLCa_Tex :  
	HC_HTXnTBLCa_0 | HC_HTXnTBLCa_InvTOPC;
	vTex->texCsat =  HC_HTXnTBLCsat_MASK |
	    pmFlags |
	    HC_HTXnTBLCb_Cur |
	    HC_HTXnTBLCc_0;
	vTex->texAsat =  HC_HTXnTBLAsat_MASK |  
	    HC_HTXnTBLAa_Atex |  
	    HC_HTXnTBLAb_Acur |
	    HC_HTXnTBLAc_HTXnTBLRA;
	break;
    default:
	return FALSE;
    }

    vTex->textureDirty = TRUE;
    vTex->textureModesS = sMode - via_single;
    vTex->textureModesT = tMode - via_single;

    return TRUE;
}

static void
viaSet3DTexBlendCol(Via3DState * v3d, int tex, Bool component, CARD32 color)
{
    CARD32 alpha;
    ViaTextureUnit *vTex = v3d->tex + tex;

    vTex->texRAa = (color >> 8) & 0x00FF0000;
    if (component) {
        vTex->texRCa = (color & 0x00FFFFFF);
    } else {
        alpha = color >> 24;
        vTex->texRCa = alpha | (alpha << 8) | (alpha << 16) | (alpha << 24);
    }
    vTex->texBColDirty = TRUE;
}

/*
 * Check if the compositing operator is supported and
 * return the corresponding register setting.
 */
static void
viaSet3DCompositeOperator(Via3DState * v3d, CARD8 op)
{
    ViaCompositeOperator *vOp = viaOperatorModes + op;

    v3d->blendDirty = TRUE;
    if (v3d && vOp->supported) {
        v3d->blendCol0 = (vOp->col0 << 4) | HC_HABLCsat_MASK;
        v3d->blendCol1 = vOp->col1 << 2;
        v3d->blendAl0 = (vOp->al0 << 4) |  HC_HABLAsat_MASK;
        v3d->blendAl1 = vOp->al1 << 2;
    }
}

static Bool
via3DOpSupported(CARD8 op)
{
    return viaOperatorModes[op].supported;
}

static void
via3DEmitQuad(Via3DState * v3d, ViaCommandBuffer * cb, int dstX, int dstY,
              int src0X, int src0Y, int src1X, int src1Y, int w, int h)
{
    CARD32 acmd;
    CARD32 bcmd;
    float dx1, dx2, dy1, dy2, sx1[2], sx2[2], sy1[2], sy2[2];
    double scalex, scaley;
    int i, numTex;
    ViaTextureUnit *vTex;
    CARD32 wc32;

    numTex = v3d->numTextures;
    dx1 = dstX;
    dx2 = dstX + w;
    dy1 = dstY;
    dy2 = dstY + h;

    if (numTex) {
        sx1[0] = src0X;
        sx1[1] = src1X;
        sy1[0] = src0Y;
        sy1[1] = src1Y;
        for (i = 0; i < numTex; ++i) {
            vTex = v3d->tex + i;
            scalex = 1. / (double)((1 << vTex->textureLevel0WExp));
            scaley = 1. / (double)((1 << vTex->textureLevel0HExp));
            sx2[i] = sx1[i] + w;
            sy2[i] = sy1[i] + h;
            sx1[i] *= scalex;
            sy1[i] *= scaley;
            sx2[i] *= scalex;
            sy2[i] *= scaley;
        }
    }

    wc32 = viafc32(1.f);

    /*
     * Vertex buffer. Emit two 3-point triangles. The W or Z coordinate
     * is needed for AGP DMA, and the W coordinate is for some obscure
     * reason needed for texture mapping to be done correctly. So emit
     * a w value after the x and y coordinates.
     */

    BEGIN_H2(HC_ParaType_NotTex, 2);
    OUT_RING( 0xCCCCCCCC );
    OUT_RING( 0xDDDDDDDD );
    ADVANCE_RING;

    BEGIN_H2(HC_ParaType_CmdVdata, 16 + numTex * 8);
    bcmd = (HC_HVPMSK_X | HC_HVPMSK_Y | HC_HVPMSK_W);
    if (numTex)
        bcmd |= (HC_HVPMSK_S | HC_HVPMSK_T);

    /*
     * Emit a Tri strip. 
     * New a vertex is the old c vertex.
     * New b vertex is the old b vertex.
     * New c vertex is emitted.
     */

    acmd =  HC_HPMType_Tri | HC_HVCycle_AFP | HC_HVCycle_AC | 
	HC_HVCycle_BB | HC_HVCycle_NewC;

    OUT_RING_SubA(0xEC, bcmd);
    OUT_RING_SubA(0xEE, acmd);

    OUT_RING(viafc32(dx1));
    OUT_RING(viafc32(dy1));
    OUT_RING(wc32);
    for (i = 0; i < numTex; ++i) {
        OUT_RING(viafc32(sx1[i]));
        OUT_RING(viafc32(sy1[i]));
    }

    OUT_RING(viafc32(dx1));
    OUT_RING(viafc32(dy2));
    OUT_RING(wc32);
    for (i = 0; i < numTex; ++i) {
        OUT_RING(viafc32(sx1[i]));
        OUT_RING(viafc32(sy2[i]));
    }

    OUT_RING(viafc32(dx2));
    OUT_RING(viafc32(dy1));
    OUT_RING(wc32);
    for (i = 0; i < numTex; ++i) {
        OUT_RING(viafc32(sx2[i]));
        OUT_RING(viafc32(sy1[i]));
    }

    OUT_RING(viafc32(dx2));
    OUT_RING(viafc32(dy2));
    OUT_RING(wc32);
    for (i = 0; i < numTex; ++i) {
        OUT_RING(viafc32(sx2[i]));
        OUT_RING(viafc32(sy2[i]));
    }

    OUT_RING_SubA(0xEE,
                  acmd | HC_HPLEND_MASK | HC_HPMValidN_MASK | HC_HE3Fire_MASK);
    if (cb->pos & 1) {
	OUT_RING_SubA(0xEE,
		      acmd | HC_HPLEND_MASK | HC_HPMValidN_MASK | 
		      HC_HE3Fire_MASK);
    }
    ADVANCE_RING;
}

static void
via3DEmitState(Via3DState * v3d, ViaCommandBuffer * cb, Bool forceUpload)
{
    int i;
    Bool saveHas3dState;
    ViaTextureUnit *vTex;
    int ret;

    /*
     * Destination buffer location, format and pitch.
     */

    if (forceUpload || v3d->destDirty) {
        v3d->destDirty = FALSE;
        BEGIN_H2(HC_ParaType_NotTex, 3);
	ret = ochr_dest_relocation(cb, v3d->destBuf, 
				   v3d->destDelta,
				   WSBM_PL_FLAG_VRAM,
				   WSBM_PL_MASK_MEM);
        OUT_RING_SubA(HC_SubA_HDBFM, v3d->destFormat |
                      (v3d->destPitch & HC_HDBPit_MASK) | HC_HDBLoc_Local);
	ADVANCE_RING;
	QWORD_PAD_RING;
    }

    if (forceUpload || v3d->blendDirty) {
        v3d->blendDirty = FALSE;
        BEGIN_H2(HC_ParaType_NotTex, 6);
        OUT_RING_SubA(HC_SubA_HABLRFCa, 0x00);
        OUT_RING_SubA(HC_SubA_HABLRFCb, 0x00);
        OUT_RING_SubA(HC_SubA_HABLCsat, v3d->blendCol0);
        OUT_RING_SubA(HC_SubA_HABLCop, v3d->blendCol1);
        OUT_RING_SubA(HC_SubA_HABLAsat, v3d->blendAl0);
        OUT_RING_SubA(HC_SubA_HABLAop, v3d->blendAl1);
	ADVANCE_RING;
	QWORD_PAD_RING;
    }

    if (forceUpload || v3d->drawingDirty) {

        CARD32 planeMaskLo, planeMaskHi;

        v3d->drawingDirty = FALSE;
        BEGIN_H2(HC_ParaType_NotTex, 4);

        /*
         * Raster operation and Planemask.
         */

        if ( /* v3d->destDepth == 16 Bad Docs? */ FALSE) {
            planeMaskLo = (v3d->planeMask & 0x000000FF) << 16;
            planeMaskHi = (v3d->planeMask & 0x0000FF00) >> 8;
        } else {
            planeMaskLo = v3d->planeMask & 0x00FFFFFF;
            planeMaskHi = v3d->planeMask >> 24;
        }

        OUT_RING_SubA(HC_SubA_HROP, ((v3d->rop & 0x0F) << 8) | planeMaskHi);
        OUT_RING_SubA(HC_SubA_HFBBMSKL, planeMaskLo);

        /*
         * Solid shading color and alpha. Pixel center at 
         * floating coordinates (X.5,Y.5).
         */

        OUT_RING_SubA(HC_SubA_HSolidCL,
                      (v3d->solidColor & 0x00FFFFFF) | (0 << 23));
        OUT_RING_SubA(HC_SubA_HPixGC,
                      (((v3d->solidColor & 0xFF000000) >> 16) | (0 << 23)
                       | (v3d->solidAlpha & 0xFF)));
	ADVANCE_RING;
	QWORD_PAD_RING;
    }

    if (1 || v3d->enableDirty ) {
        v3d->enableDirty = FALSE;
        BEGIN_H2(HC_ParaType_NotTex, 1);

        OUT_RING_SubA(HC_SubA_HEnable,
                      ((v3d->writeColor) ? HC_HenCW_MASK : 0) |
                      ((v3d->blend) ? HC_HenABL_MASK : 0) |
                      ((v3d->numTextures) ? HC_HenTXMP_MASK | HC_HenTXCH_MASK: 0) |
                      ((v3d->writeAlpha) ? HC_HenAW_MASK : 0));
	ADVANCE_RING;
	QWORD_PAD_RING;

        if (v3d->numTextures) {
	    CARD32 texSetting = 
		(5 << 8) | /* Max texture fifo */
		(((v3d->numTextures - 1) & 0x01) << HC_HTXNum_SHIFT);
		
            BEGIN_H2((HC_ParaType_Tex | (HC_SubType_TexGeneral << 8)), 2);
	    /*
	     * Clear texture cache.
	     */
            OUT_RING_SubA(HC_SubA_HTXSMD, texSetting | HC_HTXCHCLR_MASK);
            OUT_RING_SubA(HC_SubA_HTXSMD, texSetting);
	    ADVANCE_RING;
        }
    }

    for (i = 0; i < v3d->numTextures; ++i) {
        vTex = v3d->tex + i;

        if (forceUpload || vTex->textureDirty) {
	    CARD32 regTexFM;

            vTex->textureDirty = FALSE;

            BEGIN_H2((HC_ParaType_Tex |
                      (((i == 0) ? HC_SubType_Tex0 : HC_SubType_Tex1) << 8)),
                     14);

	    regTexFM = (HC_SubA_HTXnFM << HC_SubA_SHIFT) | 
		vTex->textureFormat;
	    
	    ret = ochr_tex_relocation(cb, &vTex->list, 0, 0, regTexFM,
				      WSBM_PL_FLAG_VRAM |
				      WSBM_PL_FLAG_TT |
				      VIA_PL_FLAG_AGP,
				      WSBM_PL_MASK_MEM);
	    
            if (vTex->npot) {
                OUT_RING_SubA(HC_SubA_HTXnL0Pit,
                              (vTex->textureLevel0Pitch & HC_HTXnLnPit_MASK) |
                              HC_HTXnEnPit_MASK);
            } else {
                OUT_RING_SubA(HC_SubA_HTXnL0Pit,
                              vTex->textureLevel0Exp << HC_HTXnLnPitE_SHIFT);
            }
            OUT_RING_SubA(HC_SubA_HTXnL0_5WE, vTex->textureLevel0WExp);
            OUT_RING_SubA(HC_SubA_HTXnL0_5HE, vTex->textureLevel0HExp);
            OUT_RING_SubA(HC_SubA_HTXnL0OS, 0x00);
            OUT_RING_SubA(HC_SubA_HTXnTB, 0x00);
            OUT_RING_SubA(HC_SubA_HTXnMPMD,
                          ((((unsigned)vTex->textureModesT) << 19)
                           | (((unsigned)vTex->textureModesS) << 16)));

            OUT_RING_SubA(HC_SubA_HTXnTBLCsat, vTex->texCsat);
            OUT_RING_SubA(HC_SubA_HTXnTBLCop, (0x00 << 22) | (0x00 << 19) |
                          (0x00 << 14) | (0x02 << 11) |
                          (0x00 << 7) | (0x03 << 3) | 0x02);
            OUT_RING_SubA(HC_SubA_HTXnTBLAsat, vTex->texAsat);
            OUT_RING_SubA(HC_SubA_HTXnTBLRFog, 0x00);
            OUT_RING_SubA(HC_SubA_HTXnTBLRFog, 0x00);
	    ADVANCE_RING;
        }
    }

    for (i = 0; i < v3d->numTextures; ++i) {
        vTex = v3d->tex + i;

        if (forceUpload || vTex->texBColDirty) {
            saveHas3dState = cb->has3dState;
            vTex->texBColDirty = FALSE;
            BEGIN_H2((HC_ParaType_Tex |
                      (((i == 0) ? HC_SubType_Tex0 : HC_SubType_Tex1) << 8)),
                     2);
            OUT_RING_SubA(HC_SubA_HTXnTBLRAa, vTex->texRAa);
            OUT_RING_SubA(HC_SubA_HTXnTBLRCa, vTex->texRCa);
	    ADVANCE_RING;
            cb->has3dState = saveHas3dState;
        }
    }
}

/*
 * Cliprect. Considered not important for the DRM 3D State, so restore the
 * has3dState flag afterwards.
 */
static void
via3DEmitClipRect(Via3DState * v3d, ViaCommandBuffer * cb, int x, int y,
                  int w, int h)
{
    Bool saveHas3dState;

    saveHas3dState = cb->has3dState;
    BEGIN_H2(HC_ParaType_NotTex, 2);
    OUT_RING_SubA(HC_SubA_HClipTB, (y << 12) | (y + h));
    OUT_RING_SubA(HC_SubA_HClipLR, (x << 12) | (x + w));
    ADVANCE_RING;
    QWORD_PAD_RING;
    cb->has3dState = saveHas3dState;
}

void
viaInit3DState(Via3DState * v3d)
{
    ViaCompositeOperator *op;
    int i;
    CARD32 tmp, hash;
    Via3DFormat *format;

    v3d->setDestination = viaSet3DDestination;
    v3d->setDrawing = viaSet3DDrawing;
    v3d->setFlags = viaSet3DFlags;
    v3d->setTexture = viaSet3DTexture;
    v3d->setTexBlendCol = viaSet3DTexBlendCol;
    v3d->opSupported = via3DOpSupported;
    v3d->setCompositeOperator = viaSet3DCompositeOperator;
    v3d->emitQuad = via3DEmitQuad;
    v3d->emitState = via3DEmitState;
    v3d->emitClipRect = via3DEmitClipRect;
    v3d->dstSupported = via3DDstSupported;
    v3d->texSupported = via3DTexSupported;

    for (i = 0; i < 256; ++i) {
        viaOperatorModes[i].supported = FALSE;
    }

    for (i = 0; i < VIA_NUM_3D_OPCODES; ++i) {
        op = viaOperatorModes + viaOpCodes[i][0];
        op->supported = TRUE;
        op->col0 = viaOpCodes[i][1];
        op->col1 = viaOpCodes[i][2];
        op->al0 = viaOpCodes[i][3];
        op->al1 = viaOpCodes[i][4];
    }

    for (i = 0; i < 256; ++i) {
        via3DFormats[i].pictFormat = 0x00;
    }
    for (i = 0; i < VIA_NUM_3D_FORMATS; ++i) {
        tmp = viaFormats[i][0];
        hash = VIA_FMT_HASH(tmp);
        format = via3DFormats + hash;
        if (format->pictFormat) {
            ErrorF("BUG: Bad hash function\n");
        }
        format->pictFormat = tmp;
        format->dstSupported = (viaFormats[i][3] != 0x00);
        format->texSupported = (viaFormats[i][4] != 0x00);
        format->dstFormat = viaFormats[i][1];
        format->texFormat = viaFormats[i][2];
    }
}
