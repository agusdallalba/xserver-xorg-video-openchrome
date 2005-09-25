/*
 * Copyright 1998-2003 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2003 S3 Graphics, Inc. All Rights Reserved.
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

#ifndef _VIA_DRI_H_
#define _VIA_DRI_H_ 1

#include "drm.h"
#include "xf86drm.h"
#include "via_drm.h"

#define VIA_MAX_DRAWABLES 256

#define VIA_DRI_VERSION_MAJOR		4
#define VIA_DRI_VERSION_MINOR		1

typedef drm_via_sarea_t VIASAREAPriv;

typedef struct {
    drm_handle_t handle;
    drmSize size;
    drmAddress map;
} viaRegion, *viaRegionPtr;

typedef struct {
    viaRegion regs, agp;
    int deviceID;
    int width;
    int height;
    int mem;
    int bytesPerPixel;
    int priv1;
    int priv2;
    int fbOffset;
    int fbSize;
    Bool drixinerama;
    int backOffset;
    int depthOffset;
    int textureOffset;
    int textureSize;
    int irqEnabled;
    unsigned int scrnX, scrnY;
    int sarea_priv_offset;
    int ringBufActive;
    unsigned int reg_pause_addr;
    int drmVerMajor;
    int drmVerMinor;
    int drmVerPL;
} VIADRIRec, *VIADRIPtr;

typedef struct {
    int dummy;
} VIAConfigPrivRec, *VIAConfigPrivPtr;

typedef struct {
    int dummy;
} VIADRIContextRec, *VIADRIContextPtr;

#ifdef XFree86Server

#include "screenint.h"

Bool VIADRIScreenInit(ScreenPtr pScreen);
void VIADRICloseScreen(ScreenPtr pScreen);
Bool VIADRIFinishScreenInit(ScreenPtr pScreen);
void VIADRIRingBufferCleanup(ScrnInfoPtr pScrn);
Bool VIADRIRingBufferInit(ScrnInfoPtr pScrn);



#endif /* XFree86Server */
#endif /* _VIA_DRI_H_ */
