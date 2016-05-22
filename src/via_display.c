/*
 * Copyright 2015-2016 Kevin Brace
 * Copyright 2005-2016 The OpenChrome Project
 *                     [http://www.freedesktop.org/wiki/Openchrome]
 * Copyright 2004-2005 The Unichrome Project  [unichrome.sf.net]
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "via_driver.h"

/*
 * Controls IGA1 DPMS State.
 */
void
viaIGA1DPMSControl(ScrnInfoPtr pScrn, CARD8 DPMS_Control)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA1DPMSControl.\n"));

    /* 3X5.36[5:4]: DPMS Control */
    ViaCrtcMask(hwp, 0x36, (DPMS_Control << 4) & 0x30, 0x30);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA1DPMSControl.\n"));
}

/*
 * Controls IGA2 display output on or off state.
 */
void
viaIGA2DisplayOutput(ScrnInfoPtr pScrn, Bool OutputEnable)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA2DisplayOutput.\n"));

    /* 3X5.6B[2] - IGA2 Screen Off
     *             0: Screen on
     *             1: Screen off */
    ViaCrtcMask(hwp, 0x6B, OutputEnable ? 0x00 : 0x04, 0x04);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA2DisplayOutput.\n"));
}

/*
 * Controls IGA2 display channel state.
 */
void
viaIGA2DisplayChannel(ScrnInfoPtr pScrn, Bool Channel_State)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA2DisplayChannel.\n"));

    /* 3X5.6A[7]: Second Display Channel Enable
     * 3X5.6A[6]: Second Display Channel Reset (0 for reset) */
    ViaCrtcMask(hwp, 0x6A, 0x00, 0x40);
    ViaCrtcMask(hwp, 0x6A, Channel_State ? 0x80 : 0x00, 0x80);
    ViaCrtcMask(hwp, 0x6A, 0x40, 0x40);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "IGA2 Display Channel: %s\n",
                Channel_State ? "On" : "Off");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA2DisplayChannel.\n"));
}

/*
 * Initial settings for displays.
 */
void
viaDisplayInit(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaDisplayInit.\n"));

    ViaCrtcMask(hwp, 0x6A, 0x00, 0x3D);
    hwp->writeCrtc(hwp, 0x6B, 0x00);
    hwp->writeCrtc(hwp, 0x6C, 0x00);
    hwp->writeCrtc(hwp, 0x79, 0x00);

    /* (IGA1 Timing Plus 2, added in VT3259 A3 or later) */
    if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400)
        ViaCrtcMask(hwp, 0x47, 0x00, 0xC8);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaDisplayInit.\n"));
}

/*
 * Enables simultaneous mode.
 */
void
ViaDisplayEnableSimultaneous(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaDisplayEnableSimultaneous\n"));
    ViaCrtcMask(hwp, 0x6B, 0x08, 0x08);
}

/*
 * Disables simultaneous mode.
 */
void
ViaDisplayDisableSimultaneous(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaDisplayDisableSimultaneous\n"));
    ViaCrtcMask(hwp, 0x6B, 0x00, 0x08);
}

/*
 * Sets the primary or secondary display stream on internal TMDS.
 */
void
ViaDisplaySetStreamOnDFP(ScrnInfoPtr pScrn, Bool primary)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplaySetStreamOnDFP\n"));

    if (primary)
        ViaCrtcMask(hwp, 0x99, 0x00, 0x10);
    else
        ViaCrtcMask(hwp, 0x99, 0x10, 0x10);
}

static void
ViaCRTCSetGraphicsRegisters(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    /* graphics registers */
    hwp->writeGr(hwp, 0x00, 0x00);
    hwp->writeGr(hwp, 0x01, 0x00);
    hwp->writeGr(hwp, 0x02, 0x00);
    hwp->writeGr(hwp, 0x03, 0x00);
    hwp->writeGr(hwp, 0x04, 0x00);
    hwp->writeGr(hwp, 0x05, 0x40);
    hwp->writeGr(hwp, 0x06, 0x05);
    hwp->writeGr(hwp, 0x07, 0x0F);
    hwp->writeGr(hwp, 0x08, 0xFF);

    ViaGrMask(hwp, 0x20, 0, 0xFF);
    ViaGrMask(hwp, 0x21, 0, 0xFF);
    ViaGrMask(hwp, 0x22, 0, 0xFF);
}

static void
ViaCRTCSetAttributeRegisters(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 i;

    /* attribute registers */
    for (i = 0; i <= 0xF; i++) {
        hwp->writeAttr(hwp, i, i);
    }
    hwp->writeAttr(hwp, 0x10, 0x41);
    hwp->writeAttr(hwp, 0x11, 0xFF);
    hwp->writeAttr(hwp, 0x12, 0x0F);
    hwp->writeAttr(hwp, 0x13, 0x00);
    hwp->writeAttr(hwp, 0x14, 0x00);
}

void
VIALoadRgbLut(ScrnInfoPtr pScrn, int start, int numColors, LOCO *colors)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    int i, j;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIALoadRgbLut\n"));

    hwp->enablePalette(hwp);
    hwp->writeDacMask(hwp, 0xFF);

    /* We need the same palette contents for both 16 and 24 bits, but X doesn't
     * play: X's colormap handling is hopelessly intertwined with almost every
     * X subsystem.  So we just space out RGB values over the 256*3. */

    switch (pScrn->bitsPerPixel) {
        case 16:
            for (i = start; i < numColors; i++) {
                hwp->writeDacWriteAddr(hwp, i * 4);
                for (j = 0; j < 4; j++) {
                    hwp->writeDacData(hwp, colors[i / 2].red);
                    hwp->writeDacData(hwp, colors[i].green);
                    hwp->writeDacData(hwp, colors[i / 2].blue);
                }
            }
            break;
        case 8:
        case 24:
        case 32:
            for (i = start; i < numColors; i++) {
                hwp->writeDacWriteAddr(hwp, i);
                hwp->writeDacData(hwp, colors[i].red);
                hwp->writeDacData(hwp, colors[i].green);
                hwp->writeDacData(hwp, colors[i].blue);
            }
            break;
        default:
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                       "Unsupported bitdepth: %d\n", pScrn->bitsPerPixel);
            break;
    }
    hwp->disablePalette(hwp);
}

void
ViaGammaDisable(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    switch (pVia->Chipset) {
        case VIA_CLE266:
        case VIA_KM400:
            ViaSeqMask(hwp, 0x16, 0x00, 0x80);
            break;
        default:
            ViaCrtcMask(hwp, 0x33, 0x00, 0x80);
            break;
    }

    /* Disable gamma on secondary */
    /* This is needed or the hardware will lockup */
    ViaSeqMask(hwp, 0x1A, 0x00, 0x01);
    ViaCrtcMask(hwp, 0x6A, 0x00, 0x02);
    switch (pVia->Chipset) {
        case VIA_CLE266:
        case VIA_KM400:
        case VIA_K8M800:
        case VIA_PM800:
            break;
        default:
            ViaCrtcMask(hwp, 0x6A, 0x00, 0x20);
            break;
    }
}

void
ViaCRTCInit(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    hwp->writeSeq(hwp, 0x10, 0x01); /* unlock extended registers */
    ViaCrtcMask(hwp, 0x47, 0x00, 0x01); /* unlock CRT registers */
    ViaCRTCSetGraphicsRegisters(pScrn);
    ViaCRTCSetAttributeRegisters(pScrn);
}

/*
 * Initialize common IGA (Integrated Graphics Accelerator) registers.
 */
void
viaIGAInitCommon(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 temp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGAInitCommon.\n"));

    /* Unlock VIA Technologies extended VGA registers. */
    /* 3C5.10[0] - Unlock Accessing of I/O Space
     *             0: Disable
     *             1: Enable */
    ViaSeqMask(hwp, 0x10, 0x01, 0x01);

    ViaCRTCSetGraphicsRegisters(pScrn);
    ViaCRTCSetAttributeRegisters(pScrn);

    temp = hwp->readSeq(hwp, 0x15);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR15: 0x%02X\n", temp));

    /* Be careful with 3C5.15[5].
     * It must be set to 1 for correct operation. */
    /* 3C5.15[7]   - 8/6 Bits LUT
     *               0: 6-bit
     *               1: 8-bit
     * 3C5.15[6]   - Text Column Control
     *               0: 80 column
     *               1: 132 column
     * 3C5.15[5]   - Wrap Around Disable
     *               0: Disable (For Mode 0-13)
     *               1: Enable
     * 3C5.15[4]   - Hi Color Mode Select
     *               0: 555
     *               1: 565
     * 3C5.15[3:2] - Display Color Depth Select
     *               00: 8bpp
     *               01: 16bpp
     *               10: 30bpp
     *               11: 32bpp
     * 3C5.15[1]   - Extended Display Mode Enable
     *               0: Disable
     *               1: Enable
     * 3C5.15[0]   - Reserved */
    ViaSeqMask(hwp, 0x15, 0x22, 0x62);

    temp = hwp->readSeq(hwp, 0x1A);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR1A: 0x%02X\n", temp));
    /* 3C5.1A[7]: Read Cache Enable
     *            0 = Disable
     *            1 = Enable
     * 3C5.1A[3]: Extended Mode Memory Access Enable
     *            0 = Disable
     *            1 = Enable */
    ViaSeqMask(hwp, 0x1A, 0x88, 0x88);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGAInitCommon.\n"));
}

void
viaIGA1SetDisplayRegister(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD16 temp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA1SetDisplayRegister.\n"));

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "IGA1 Requested Screen Mode: %s\n", mode->name);

    ViaCrtcMask(hwp, 0x11, 0x00, 0x80); /* modify starting address */
    ViaCrtcMask(hwp, 0x03, 0x80, 0x80); /* enable vertical retrace access */

    /* Set Misc Register */
    temp = 0x23;
    if (mode->Flags & V_NHSYNC)
        temp |= 0x40;
    if (mode->Flags & V_NVSYNC)
        temp |= 0x80;
    temp |= 0x0C; /* Undefined/external clock */
    hwp->writeMiscOut(hwp, temp);

    /* Sequence registers */
    hwp->writeSeq(hwp, 0x00, 0x00);

#if 0
    if (mode->Flags & V_CLKDIV2)
        hwp->writeSeq(hwp, 0x01, 0x09);
    else
#endif
        hwp->writeSeq(hwp, 0x01, 0x01);

    hwp->writeSeq(hwp, 0x02, 0x0F);
    hwp->writeSeq(hwp, 0x03, 0x00);
    hwp->writeSeq(hwp, 0x04, 0x0E);

    ViaSeqMask(hwp, 0x15, 0x02, 0x02);

    /* Set the color depth for IGA1. */
    switch (pScrn->bitsPerPixel) {
    case 8:
        /* Only CLE266.AX use 6bits LUT. */
        if (pVia->Chipset == VIA_CLE266 && pVia->ChipRev < 15) {
            ViaSeqMask(hwp, 0x15, 0x22, 0xFE);
        } else {
            ViaSeqMask(hwp, 0x15, 0xA2, 0xFE);
        }

        break;
    case 16:
        ViaSeqMask(hwp, 0x15, 0xB6, 0xFE);
        break;
    case 24:
    case 32:
        ViaSeqMask(hwp, 0x15, 0xAE, 0xFE);
        break;
    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Unsupported color depth: %d\n",
                    pScrn->bitsPerPixel);
        break;
    }

    switch (pVia->Chipset) {
        case VIA_CX700:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            break;
        default:
            ViaSeqMask(hwp, 0x16, 0x08, 0xBF);
            ViaSeqMask(hwp, 0x17, 0x1F, 0xFF);
            ViaSeqMask(hwp, 0x18, 0x4E, 0xFF);
            ViaSeqMask(hwp, 0x1A, 0x08, 0xF9);
            break;
    }

    /* Linear Mode */
    ViaCrtcMask(hwp, 0x43, 0x00, 0x04);


    /* Set IGA1 horizontal total.*/
    /* Due to IGA1 horizontal total being only 9 bits wide,
     * the adjusted horizontal total needs to be shifted by
     * 3 bit positions to the right.
     * In addition to that, this particular register requires the
     * value to be 5 less than the actual value being written. */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA1 CrtcHTotal: %d\n", mode->CrtcHTotal));
    temp = (mode->CrtcHTotal >> 3) - 5;

    /* 3X5.00[7:0] - Horizontal Total Bits [7:0] */
    hwp->writeCrtc(hwp, 0x00, temp & 0xFF);

    /* 3X5.36[3] - Horizontal Total Bit [8] */
    ViaCrtcMask(hwp, 0x36, temp >> 5, 0x08);


    /* Set IGA1 horizontal display end. */
    /* Due to IGA1 horizontal display end being only 8 bits wide,
     * the adjusted horizontal display end needs to be shifted by
     * 3 bit positions to the right.
     * In addition to that, this particular register requires the
     * value to be 1 less than the actual value being written. */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA1 CrtcHDisplay: %d\n", mode->CrtcHDisplay));
    temp = (mode->CrtcHDisplay >> 3) - 1;

    /* 3X5.01[7:0] - Horizontal Display End Bits [7:0] */
    hwp->writeCrtc(hwp, 0x01, temp & 0xFF);


    /* horizontal blanking start : 2048 */
    /* temp = (mode->CrtcHDisplay >> 3) - 1; */
    temp = (mode->CrtcHBlankStart >> 3) - 1;
    hwp->writeCrtc(hwp, 0x02, temp & 0xFF);
    /* If HblankStart has more bits anywhere, add them here */

    /* horizontal blanking end : start + 1025 */
    /* temp = (mode->CrtcHTotal >> 3) - 1; */
    temp = (mode->CrtcHBlankEnd >> 3) - 1;
    ViaCrtcMask(hwp, 0x03, temp, 0x1F);
    ViaCrtcMask(hwp, 0x05, temp << 2, 0x80);
    ViaCrtcMask(hwp, 0x33, temp >> 1, 0x20);

    /* CrtcHSkew ??? */

    /* horizontal sync start : 4095 */
    temp = mode->CrtcHSyncStart >> 3;
    hwp->writeCrtc(hwp, 0x04, temp & 0xFF);
    ViaCrtcMask(hwp, 0x33, temp >> 4, 0x10);

    /* horizontal sync end : start + 256 */
    temp = mode->CrtcHSyncEnd >> 3;
    ViaCrtcMask(hwp, 0x05, temp, 0x1F);

    /* vertical total : 2049 */
    temp = mode->CrtcVTotal - 2;
    hwp->writeCrtc(hwp, 0x06, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 8, 0x01);
    ViaCrtcMask(hwp, 0x07, temp >> 4, 0x20);
    ViaCrtcMask(hwp, 0x35, temp >> 10, 0x01);

    /* vertical address : 2048 */
    temp = mode->CrtcVDisplay - 1;
    hwp->writeCrtc(hwp, 0x12, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 7, 0x02);
    ViaCrtcMask(hwp, 0x07, temp >> 3, 0x40);
    ViaCrtcMask(hwp, 0x35, temp >> 8, 0x04);

    /* Primary starting address -> 0x00, adjustframe does the rest */
    hwp->writeCrtc(hwp, 0x0C, 0x00);
    hwp->writeCrtc(hwp, 0x0D, 0x00);
    ViaCrtcMask(hwp, 0x48, 0x00, 0x03); /* is this even possible on CLE266A ? */
    hwp->writeCrtc(hwp, 0x34, 0x00);

    /* vertical sync start : 2047 */
    temp = mode->CrtcVSyncStart;
    hwp->writeCrtc(hwp, 0x10, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 6, 0x04);
    ViaCrtcMask(hwp, 0x07, temp >> 2, 0x80);
    ViaCrtcMask(hwp, 0x35, temp >> 9, 0x02);

    /* vertical sync end : start + 16 -- other bits someplace? */
    ViaCrtcMask(hwp, 0x11, mode->CrtcVSyncEnd, 0x0F);

    /* line compare: We are not doing splitscreen so 0x3FFF */
    hwp->writeCrtc(hwp, 0x18, 0xFF);
    ViaCrtcMask(hwp, 0x07, 0x10, 0x10);
    ViaCrtcMask(hwp, 0x09, 0x40, 0x40);
    ViaCrtcMask(hwp, 0x33, 0x06, 0x07);
    ViaCrtcMask(hwp, 0x35, 0x10, 0x10);

    /* zero Maximum scan line */
    ViaCrtcMask(hwp, 0x09, 0x00, 0x1F);
    hwp->writeCrtc(hwp, 0x14, 0x00);

    /* vertical blanking start : 2048 */
    /* temp = mode->CrtcVDisplay - 1; */
    temp = mode->CrtcVBlankStart - 1;
    hwp->writeCrtc(hwp, 0x15, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 5, 0x08);
    ViaCrtcMask(hwp, 0x09, temp >> 4, 0x20);
    ViaCrtcMask(hwp, 0x35, temp >> 7, 0x08);

    /* vertical blanking end : start + 257 */
    /* temp = mode->CrtcVTotal - 1; */
    temp = mode->CrtcVBlankEnd - 1;
    hwp->writeCrtc(hwp, 0x16, temp);

    /* offset */
    temp = (pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) >> 3;
    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
    }
    hwp->writeCrtc(hwp, 0x13, temp & 0xFF);
    ViaCrtcMask(hwp, 0x35, temp >> 3, 0xE0);

    /* fetch count */
    temp = (mode->CrtcHDisplay * (pScrn->bitsPerPixel >> 3)) >> 3;
    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
    }

    hwp->writeSeq(hwp, 0x1C, ((temp >> 1)+1) & 0xFF);
    ViaSeqMask(hwp, 0x1D, temp >> 9, 0x03);

    switch (pVia->Chipset) {
        case VIA_CX700:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            break;
        default:
            /* some leftovers */
            ViaCrtcMask(hwp, 0x32, 0, 0xFF);
            ViaCrtcMask(hwp, 0x33, 0, 0xC8);
            break;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA1SetDisplayRegister.\n"));
}

void
viaIGA1SetFBStartingAddress(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
    drmmode_ptr drmmode = drmmode_crtc->drmmode;
    CARD32 Base;
    CARD8 cr0c, cr0d, cr34, cr48;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA1SetFBStartingAddress.\n"));

    Base = (y * pScrn->displayWidth + x) * (pScrn->bitsPerPixel / 8);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Base Address: 0x%x\n",
                        Base));
    Base = (Base + drmmode->front_bo->offset) >> 1;
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "DRI Base Address: 0x%x\n",
                Base);

    hwp->writeCrtc(hwp, 0x0D, Base & 0xFF);
    hwp->writeCrtc(hwp, 0x0C, (Base & 0xFF00) >> 8);

    /* FIXME The proper starting address for CR48 is 0x1F - Bits[28:24] */
    if (!(pVia->Chipset == VIA_CLE266 && CLE266_REV_IS_AX(pVia->ChipRev))) {
        ViaCrtcMask(hwp, 0x48, Base >> 24, 0x0F);
    }

    /* CR34 are fire bits. Must be written after CR0C, CR0D, and CR48. */
    hwp->writeCrtc(hwp, 0x34, (Base & 0xFF0000) >> 16);

#ifdef HAVE_DEBUG
    cr0d = hwp->readCrtc(hwp, 0x0D);
    cr0c = hwp->readCrtc(hwp, 0x0C);
    cr34 = hwp->readCrtc(hwp, 0x34);
    cr48 = hwp->readCrtc(hwp, 0x48);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "CR0D: 0x%02x\n", cr0d));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "CR0C: 0x%02x\n", cr0c));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "CR34: 0x%02x\n", cr34));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "CR48: 0x%02x\n", cr48));
#endif

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA1SetFBStartingAddress.\n"));
}

/*
 * Checks for limitations imposed by the available VGA timing registers.
 */
static ModeStatus
viaIGA1ModeValid(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA1ModeValid.\n"));

    if (mode->CrtcHTotal > 4100)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHDisplay > 2048)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHBlankStart > 2048)
        return MODE_BAD_HVALUE;

    if ((mode->CrtcHBlankEnd - mode->CrtcHBlankStart) > 1025)
        return MODE_HBLANK_WIDE;

    if (mode->CrtcHSyncStart > 4095)
        return MODE_BAD_HVALUE;

    if ((mode->CrtcHSyncEnd - mode->CrtcHSyncStart) > 256)
        return MODE_HSYNC_WIDE;

    if (mode->CrtcVTotal > 2049)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVDisplay > 2048)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVSyncStart > 2047)
        return MODE_BAD_VVALUE;

    if ((mode->CrtcVSyncEnd - mode->CrtcVSyncStart) > 16)
        return MODE_VSYNC_WIDE;

    if (mode->CrtcVBlankStart > 2048)
        return MODE_BAD_VVALUE;

    if ((mode->CrtcVBlankEnd - mode->CrtcVBlankStart) > 257)
        return MODE_VBLANK_WIDE;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA1ModeValid.\n"));
    return MODE_OK;
}

void
viaIGA1Save(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIARegPtr Regs = &pVia->SavedReg;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA1Save.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Saving sequencer registers.\n"));

    /* Unlock extended registers. */
    hwp->writeSeq(hwp, 0x10, 0x01);

    Regs->SR14 = hwp->readSeq(hwp, 0x14);
    Regs->SR15 = hwp->readSeq(hwp, 0x15);
    Regs->SR16 = hwp->readSeq(hwp, 0x16);
    Regs->SR17 = hwp->readSeq(hwp, 0x17);
    Regs->SR18 = hwp->readSeq(hwp, 0x18);
    Regs->SR19 = hwp->readSeq(hwp, 0x19);

    /* PCI Bus Control */
    Regs->SR1A = hwp->readSeq(hwp, 0x1A);

    Regs->SR1B = hwp->readSeq(hwp, 0x1B);
    Regs->SR1C = hwp->readSeq(hwp, 0x1C);
    Regs->SR1D = hwp->readSeq(hwp, 0x1D);
    Regs->SR1E = hwp->readSeq(hwp, 0x1E);
    Regs->SR1F = hwp->readSeq(hwp, 0x1F);

    Regs->SR22 = hwp->readSeq(hwp, 0x22);

    /* Registers 3C5.23 through 3C5.25 are not used by Chrome9.
     * Registers 3C5.27 through 3C5.29 are not used by Chrome9. */
    switch (pVia->Chipset) {
    case VIA_CLE266:
    case VIA_KM400:
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
        Regs->SR23 = hwp->readSeq(hwp, 0x23);
        Regs->SR24 = hwp->readSeq(hwp, 0x24);
        Regs->SR25 = hwp->readSeq(hwp, 0x25);

        Regs->SR27 = hwp->readSeq(hwp, 0x27);
        Regs->SR28 = hwp->readSeq(hwp, 0x28);
        Regs->SR29 = hwp->readSeq(hwp, 0x29);
        break;
    default:
        break;
    }

    Regs->SR26 = hwp->readSeq(hwp, 0x26);

    Regs->SR2A = hwp->readSeq(hwp, 0x2A);
    Regs->SR2B = hwp->readSeq(hwp, 0x2B);
    Regs->SR2D = hwp->readSeq(hwp, 0x2D);
    Regs->SR2E = hwp->readSeq(hwp, 0x2E);

    /* Save PCI Configuration Memory Base Shadow 0 and 1.
     * These registers are available only in UniChrome, UniChrome Pro,
     * and UniChrome Pro II. */
    switch (pVia->Chipset) {
    case VIA_CLE266:
    case VIA_KM400:
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
        Regs->SR2F = hwp->readSeq(hwp, 0x2F);
        Regs->SR30 = hwp->readSeq(hwp, 0x30);
        break;
    default:
        break;
    }

    /* Save PLL settings and several miscellaneous registers.
     * For UniChrome, register 3C5.44 through 3C5.4B are saved.
     * For UniChrome Pro and Chrome9, register 3C5.44 through 3C5.4C
     * are saved. */
    Regs->SR44 = hwp->readSeq(hwp, 0x44);
    Regs->SR45 = hwp->readSeq(hwp, 0x45);
    Regs->SR46 = hwp->readSeq(hwp, 0x46);
    Regs->SR47 = hwp->readSeq(hwp, 0x47);
    Regs->SR48 = hwp->readSeq(hwp, 0x48);
    Regs->SR49 = hwp->readSeq(hwp, 0x49);
    Regs->SR4A = hwp->readSeq(hwp, 0x4A);
    Regs->SR4B = hwp->readSeq(hwp, 0x4B);

    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

        Regs->SR4C = hwp->readSeq(hwp, 0x4C);

    /* Save register 3C5.4D.
     * According to CX700 (UniChrome Pro II) documentation, this register
     * is called Dual Channel Memory Control.
     * According to VX800 / VX855 / VX900 (Chrome 9 HC3 / HCM / HD)
     * documentations, this register is called Preemptive Arbiter Control.
     * It is likely that this register is also supported in UniChrome Pro. */
        Regs->SR4D = hwp->readSeq(hwp, 0x4D);

        Regs->SR4E = hwp->readSeq(hwp, 0x4E);
        Regs->SR4F = hwp->readSeq(hwp, 0x4F);
        break;
    default:
        break;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Finished saving sequencer registers.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Saving IGA1 registers.\n"));

    /* UniChrome Pro or later */
    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

        /* Display Fetch Blocking Control */
        Regs->CR30 = hwp->readCrtc(hwp, 0x30);

        /* Half Line Position */
        Regs->CR31 = hwp->readCrtc(hwp, 0x31);
        break;
    default:
        break;
    }

    Regs->CR32 = hwp->readCrtc(hwp, 0x32);
    Regs->CR33 = hwp->readCrtc(hwp, 0x33);
    Regs->CR35 = hwp->readCrtc(hwp, 0x35);
    Regs->CR36 = hwp->readCrtc(hwp, 0x36);

    /* UniChrome Pro or later */
    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

        /* DAC Control Register */
        Regs->CR37 = hwp->readCrtc(hwp, 0x37);
        break;
    default:
        break;
    }

    Regs->CR38 = hwp->readCrtc(hwp, 0x38);
    Regs->CR39 = hwp->readCrtc(hwp, 0x39);
    Regs->CR3A = hwp->readCrtc(hwp, 0x3A);
    Regs->CR3B = hwp->readCrtc(hwp, 0x3B);
    Regs->CR3C = hwp->readCrtc(hwp, 0x3C);
    Regs->CR3D = hwp->readCrtc(hwp, 0x3D);
    Regs->CR3E = hwp->readCrtc(hwp, 0x3E);
    Regs->CR3F = hwp->readCrtc(hwp, 0x3F);

    Regs->CR40 = hwp->readCrtc(hwp, 0x40);

    /* UniChrome Pro or later */
    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

        Regs->CR43 = hwp->readCrtc(hwp, 0x43);
        Regs->CR45 = hwp->readCrtc(hwp, 0x45);
        break;
    default:
        break;
    }

    Regs->CR46 = hwp->readCrtc(hwp, 0x46);
    Regs->CR47 = hwp->readCrtc(hwp, 0x47);

    /* Starting Address */
    /* Start Address High */
    Regs->CR0C = hwp->readCrtc(hwp, 0x0C);

    /* Start Address Low */
    Regs->CR0D = hwp->readCrtc(hwp, 0x0D);

    /* UniChrome Pro or later */
    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

        /* Starting Address Overflow[28:24] */
        Regs->CR48 = hwp->readCrtc(hwp, 0x48);
        break;
    default:
        break;
    }

    /* Starting Address Overflow[23:16] */
    Regs->CR34 = hwp->readCrtc(hwp, 0x34);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Finished saving IGA1 registers.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA1Save.\n"));
}

void
viaIGA1Restore(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIARegPtr Regs = &pVia->SavedReg;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA1Restore.\n"));

    /* Unlock extended registers. */
    hwp->writeSeq(hwp, 0x10, 0x01);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Restoring sequencer registers.\n"));

    hwp->writeSeq(hwp, 0x14, Regs->SR14);
    hwp->writeSeq(hwp, 0x15, Regs->SR15);
    hwp->writeSeq(hwp, 0x16, Regs->SR16);
    hwp->writeSeq(hwp, 0x17, Regs->SR17);
    hwp->writeSeq(hwp, 0x18, Regs->SR18);
    hwp->writeSeq(hwp, 0x19, Regs->SR19);

    /* PCI Bus Control */
    hwp->writeSeq(hwp, 0x1A, Regs->SR1A);

    hwp->writeSeq(hwp, 0x1B, Regs->SR1B);
    hwp->writeSeq(hwp, 0x1C, Regs->SR1C);
    hwp->writeSeq(hwp, 0x1D, Regs->SR1D);
    hwp->writeSeq(hwp, 0x1E, Regs->SR1E);
    hwp->writeSeq(hwp, 0x1F, Regs->SR1F);

    hwp->writeSeq(hwp, 0x22, Regs->SR22);

    /* Registers 3C5.23 through 3C5.25 are not used by Chrome9.
     * Registers 3C5.27 through 3C5.29 are not used by Chrome9. */
    switch (pVia->Chipset) {
    case VIA_CLE266:
    case VIA_KM400:
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:

        hwp->writeSeq(hwp, 0x23, Regs->SR23);
        hwp->writeSeq(hwp, 0x24, Regs->SR24);
        hwp->writeSeq(hwp, 0x25, Regs->SR25);

        hwp->writeSeq(hwp, 0x27, Regs->SR27);
        hwp->writeSeq(hwp, 0x28, Regs->SR28);
        hwp->writeSeq(hwp, 0x29, Regs->SR29);
        break;
    default:
        break;
    }

    hwp->writeSeq(hwp, 0x26, Regs->SR26);

    hwp->writeSeq(hwp, 0x2A, Regs->SR2A);
    hwp->writeSeq(hwp, 0x2B, Regs->SR2B);

    hwp->writeSeq(hwp, 0x2D, Regs->SR2D);
    hwp->writeSeq(hwp, 0x2E, Regs->SR2E);

    /* Restore PCI Configuration Memory Base Shadow 0 and 1.
     * These registers are available only in UniChrome, UniChrome Pro,
     * and UniChrome Pro II. */
    switch (pVia->Chipset) {
    case VIA_CLE266:
    case VIA_KM400:
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:

        hwp->writeSeq(hwp, 0x2F, Regs->SR2F);
        hwp->writeSeq(hwp, 0x30, Regs->SR30);
        break;
    default:
        break;
    }

    /* Restore PLL settings and several miscellaneous registers.
     * For UniChrome, register 3C5.44 through 3C5.4B are restored.
     * For UniChrome Pro and Chrome 9, register 3C5.44 through 3C5.4C
     * are restored. */
    switch (pVia->Chipset) {
    case VIA_CLE266:
    case VIA_KM400:
        /* Engine Clock (ECK) PLL settings */
        hwp->writeSeq(hwp, 0x48, Regs->SR48);
        hwp->writeSeq(hwp, 0x49, Regs->SR49);

        /* Memory Clock (MCK) PLL settings */
        hwp->writeSeq(hwp, 0x4a, Regs->SR4A);
        hwp->writeSeq(hwp, 0x4b, Regs->SR4B);

        /* Primary Display Clock (VCK) PLL settings */
        hwp->writeSeq(hwp, 0x46, Regs->SR46);
        hwp->writeSeq(hwp, 0x47, Regs->SR47);

        /* Secondary Display Clock (LCDCK) PLL settings */
        hwp->writeSeq(hwp, 0x44, Regs->SR44);
        hwp->writeSeq(hwp, 0x45, Regs->SR45);
        break;
    default:
        /* Engine Clock (ECK) PLL settings */
        hwp->writeSeq(hwp, 0x47, Regs->SR47);
        hwp->writeSeq(hwp, 0x48, Regs->SR48);
        hwp->writeSeq(hwp, 0x49, Regs->SR49);

        /* Reset ECK PLL. */
        hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) | 0x01); /* Set SR40[0] to 1 */
        hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) & (~0x01)); /* Set SR40[0] to 0 */


        /* Primary Display Clock (VCK) PLL settings */
        hwp->writeSeq(hwp, 0x44, Regs->SR44);
        hwp->writeSeq(hwp, 0x45, Regs->SR45);
        hwp->writeSeq(hwp, 0x46, Regs->SR46);

        /* Reset VCK PLL. */
        hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) | 0x02); /* Set SR40[1] to 1 */
        hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) & (~0x02)); /* Set SR40[1] to 0 */


        /* Secondary Display Clock (LCDCK) PLL settings */
        hwp->writeSeq(hwp, 0x4A, Regs->SR4A);
        hwp->writeSeq(hwp, 0x4B, Regs->SR4B);
        hwp->writeSeq(hwp, 0x4C, Regs->SR4C);

        /* Reset LCDCK PLL. */
        hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) | 0x04); /* Set SR40[2] to 1 */
        hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) & (~0x04)); /* Set SR40[2] to 0 */
        break;
    }

    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

    /* Restore register 3C5.4D.
     * According to CX700 (UniChrome Pro II) documentation, this register
     * is called Dual Channel Memory Control.
     * According to VX800 / VX855 / VX900 (Chrome 9 HC3 / HCM / HD)
     * documentations, this register is called Preemptive Arbiter Control.
     * It is likely that this register is also supported in UniChrome Pro. */
        hwp->writeSeq(hwp, 0x4D, Regs->SR4D);

        hwp->writeSeq(hwp, 0x4E, Regs->SR4E);
        hwp->writeSeq(hwp, 0x4F, Regs->SR4F);
        break;
    default:
        break;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Finished restoring sequencer registers.\n"));

/* Reset dot clocks. */
    ViaSeqMask(hwp, 0x40, 0x06, 0x06);
    ViaSeqMask(hwp, 0x40, 0x00, 0x06);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Restoring IGA1 registers.\n"));

/* UniChrome Pro or later */
    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

        /* Display Fetch Blocking Control */
        hwp->writeCrtc(hwp, 0x30, Regs->CR30);

        /* Half Line Position */
        hwp->writeCrtc(hwp, 0x31, Regs->CR31);
        break;
    default:
        break;
    }

    /* Restore CRTC controller extended registers. */
    /* Mode Control */
    hwp->writeCrtc(hwp, 0x32, Regs->CR32);

    /* HSYNCH Adjuster */
    hwp->writeCrtc(hwp, 0x33, Regs->CR33);

    /* Extended Overflow */
    hwp->writeCrtc(hwp, 0x35, Regs->CR35);

    /* Power Management 3 (Monitor Control) */
    hwp->writeCrtc(hwp, 0x36, Regs->CR36);

/* UniChrome Pro or later */
    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

        /* DAC control Register */
        hwp->writeCrtc(hwp, 0x37, Regs->CR37);
        break;
    default:
        break;
    }

    hwp->writeCrtc(hwp, 0x38, Regs->CR38);
    hwp->writeCrtc(hwp, 0x39, Regs->CR39);
    hwp->writeCrtc(hwp, 0x3A, Regs->CR3A);
    hwp->writeCrtc(hwp, 0x3B, Regs->CR3B);
    hwp->writeCrtc(hwp, 0x3C, Regs->CR3C);
    hwp->writeCrtc(hwp, 0x3D, Regs->CR3D);
    hwp->writeCrtc(hwp, 0x3E, Regs->CR3E);
    hwp->writeCrtc(hwp, 0x3F, Regs->CR3F);

    hwp->writeCrtc(hwp, 0x40, Regs->CR40);

    /* UniChrome Pro or later */
    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

        hwp->writeCrtc(hwp, 0x43, Regs->CR43);
        hwp->writeCrtc(hwp, 0x45, Regs->CR45);
        break;
    default:
        break;
    }

    hwp->writeCrtc(hwp, 0x46, Regs->CR46);
    hwp->writeCrtc(hwp, 0x47, Regs->CR47);

    /* Starting Address */
    /* Start Address High */
    hwp->writeCrtc(hwp, 0x0C, Regs->CR0C);

    /* Start Address Low */
    hwp->writeCrtc(hwp, 0x0D, Regs->CR0D);

    /* UniChrome Pro or later */
    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:

        /* Starting Address Overflow[28:24] */
        hwp->writeCrtc(hwp, 0x48, Regs->CR48);
        break;
    default:
        break;
    }

    /* CR34 is fire bits. Must be written after CR0C, CR0D, and CR48.
     * Starting Address Overflow[23:16] */
    hwp->writeCrtc(hwp, 0x34, Regs->CR34);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Finished restoring IGA1 registers.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA1Restore.\n"));
}

void
viaIGA2SetFBStartingAddress(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
    drmmode_ptr drmmode = drmmode_crtc->drmmode;
    CARD32 Base, tmp;
    CARD8 cr62, cr63, cr64, cra3;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA2SetFBStartingAddress.\n"));

    Base = (y * pScrn->displayWidth + x) * (pScrn->bitsPerPixel / 8);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Base Address: 0x%x\n",
                        Base));
    Base = (Base + drmmode->front_bo->offset) >> 3;
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "DRI Base Address: 0x%x\n",
                Base);

    tmp = hwp->readCrtc(hwp, 0x62) & 0x01;
    tmp |= (Base & 0x7F) << 1;
    hwp->writeCrtc(hwp, 0x62, tmp);

    hwp->writeCrtc(hwp, 0x63, (Base & 0x7F80) >> 7);
    hwp->writeCrtc(hwp, 0x64, (Base & 0x7F8000) >> 15);
    hwp->writeCrtc(hwp, 0xA3, (Base & 0x03800000) >> 23);

#ifdef HAVE_DEBUG
    cr62 = hwp->readCrtc(hwp, 0x62);
    cr63 = hwp->readCrtc(hwp, 0x63);
    cr64 = hwp->readCrtc(hwp, 0x64);
    cra3 = hwp->readCrtc(hwp, 0xA3);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "CR62: 0x%02x\n", cr62));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "CR63: 0x%02x\n", cr63));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "CR64: 0x%02x\n", cr64));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "CRA3: 0x%02x\n", cra3));
#endif

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA2SetFBStartingAddress.\n"));
}

void
viaIGA2SetDisplayRegister(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD16 temp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA2SetDisplayRegister.\n"));
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Requested Screen Mode: %s\n", mode->name);

    /* Set the color depth for IGA2. */
    switch (pScrn->bitsPerPixel) {
        case 8:
            ViaCrtcMask(hwp, 0x67, 0x00, 0xC0);
            break;
        case 16:
            ViaCrtcMask(hwp, 0x67, 0x40, 0xC0);
            break;
        case 24:
        case 32:
            ViaCrtcMask(hwp, 0x67, 0xC0, 0xC0);
            break;
        default:
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "Unsupported color depth: %d\n",
                        pScrn->bitsPerPixel);
            break;
    }


    /* LVDS Channel 1 and 2 should be controlled by PMS
     * (Power Management Status). */
    ViaSeqMask(hwp, 0x2A, 0x0F, 0x0F);

    /* 3X5.99[3:0] appears to be a register to adjust an LCD panel
     * (the official name of the register is unknown). */
    if (pVia->Chipset == VIA_P4M900) {
        ViaCrtcMask(hwp, 0x99, 0x08, 0x0F);
    }

    /* IGA2 for DFP Low. */
    ViaCrtcMask(hwp, 0x99, 0x10, 0x10);

    /* Use IGA2 for DVP1 Data Source Selection 0. */
    ViaCrtcMask(hwp, 0x9B, 0x10, 0x10);

    /* Linear Mode */
    ViaCrtcMask(hwp, 0x62, 0x00, 0x01);


    /* Set IGA2 horizontal total pixels.*/
    /* Horizontal Total Period: 4096 - 1 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcHTotal: %d\n", mode->CrtcHTotal));
    temp = mode->CrtcHTotal - 1;

    /* 3X5.50[7:0]: Horizontal Total Period Bits[7:0] */
    hwp->writeCrtc(hwp, 0x50, temp & 0xFF);

    /* 3X5.55[3:0]: Horizontal Total Period Bits[11:8] */
    ViaCrtcMask(hwp, 0x55, (temp >> 8) & 0x0F, 0x0F);


    /* Set IGA2 horizontal display end position. */
    /* Horizontal Active Data Period: 2048 - 1 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcHDisplay: %d\n", mode->CrtcHDisplay));
    temp = mode->CrtcHDisplay - 1;

    /* 3X5.51[7:0]: Horizontal Active Data Period Bits[7:0] */
    hwp->writeCrtc(hwp, 0x51, temp & 0xFF);

    /* 3X5.55[6:4]: Horizontal Active Data Period Bits[10:8] */
    ViaCrtcMask(hwp, 0x55, (temp >> 4) & 0x70, 0x70);


    /* Set IGA2 horizontal blank start. */
    /* Horizontal Blanking Start: 2048 - 1 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcHBlankStart: %d\n", mode->CrtcHBlankStart));
    temp = mode->CrtcHBlankStart - 1;

    /* 3X5.52[7:0]: Horizontal Blanking Start Bits[7:0] */
    hwp->writeCrtc(hwp, 0x52, temp & 0xFF);

    /* 3X5.54[2:0]: Horizontal Blanking Start Bits[10:8] */
    ViaCrtcMask(hwp, 0x54, (temp >> 8) & 0x07, 0x07);


    /* Set IGA2 horizontal blank end. */
    /* Horizontal Blanking End: 4096 - 1 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcHBlankEnd: %d\n", mode->CrtcHBlankEnd));
    temp = mode->CrtcHBlankEnd - 1;

    /* 3X5.53[7:0]: Horizontal Blanking End Bits[7:0] */
    hwp->writeCrtc(hwp, 0x53, temp & 0xFF);

    /* 3X5.54[5:3]: Horizontal Blanking End Bits[10:8] */
    ViaCrtcMask(hwp, 0x54, (temp >> 5) & 0x38, 0x38);

    /* 3X5.5D[6]: Horizontal Blanking End Bits[11] */
    ViaCrtcMask(hwp, 0x5D, (temp >> 5) & 0x40, 0x40);


    /* Set IGA2 horizontal synchronization start. */
    /* Horizontal Retrace Start: 2047 (max, UniChrome),
     *                           4095 (max, UniChrome Pro and Chrome9) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcHSyncStart: %d\n", mode->CrtcHSyncStart));
    temp = mode->CrtcHSyncStart;

    /* 3X5.56[7:0]: Horizontal Retrace Start Bits[7:0] */
    hwp->writeCrtc(hwp, 0x56, temp & 0xFF);

    /* 3X5.54[7:6]: Horizontal Retrace Start Bits[9:8] */
    ViaCrtcMask(hwp, 0x54, (temp >> 2) & 0xC0, 0xC0);

    /* 3X5.5C[7]: Horizontal Retrace Start Bits[10] */
    ViaCrtcMask(hwp, 0x5C, (temp >> 3) & 0x80, 0x80);

    /* For UniChrome Pro and Chrome9. */
    if ((pVia->Chipset != VIA_CLE266)
        && (pVia->Chipset != VIA_KM400)) {

        /* 3X5.5D[7]: Horizontal Retrace Start Bits[11] */
        ViaCrtcMask(hwp, 0x5D, (temp >> 4) & 0x80, 0x80);
    }


    /* Set IGA2 horizontal synchronization end. */
    /* Horizontal Retrace End: 511 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcHSyncEnd: %d\n", mode->CrtcHSyncEnd));
    temp = mode->CrtcHSyncEnd - mode->CrtcHSyncStart;

    /* 3X5.57[7:0]: Horizontal Retrace End Bits[7:0] */
    hwp->writeCrtc(hwp, 0x57, temp & 0xFF);

    /* 3X5.5C[6]: Horizontal Retrace End Bits[8] */
    ViaCrtcMask(hwp, 0x5C, (temp >> 2) & 0x40, 0x40);


    /* Set IGA2 vertical total pixels. */
    /* Vertical Total Period: 2048 - 1 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcVTotal: %d\n", mode->CrtcVTotal));
    temp = mode->CrtcVTotal - 1;

    /* 3X5.58[7:0]: Vertical Total Period Bits[7:0] */
    hwp->writeCrtc(hwp, 0x58, temp & 0xFF);

    /* 3X5.5D[2:0]: Vertical Total Period Bits[10:8] */
    ViaCrtcMask(hwp, 0x5D, (temp >> 8) & 0x07, 0x07);


    /* Set IGA2 vertical display end position. */
    /* Vertical Active Data Period: 2048 - 1 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcVDisplay: %d\n", mode->CrtcVDisplay));
    temp = mode->CrtcVDisplay - 1;

    /* 3X5.59[7:0]: Vertical Active Data Period Bits[7:0] */
    hwp->writeCrtc(hwp, 0x59, temp & 0xFF);

    /* 3X5.5D[5:3]: Vertical Active Data Period Bits[10:8] */
    ViaCrtcMask(hwp, 0x5D, (temp >> 5) & 0x38, 0x38);


    /* Set IGA2 vertical blank start. */
    /* Vertical Blanking Start: 2048 - 1 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcVBlankStart: %d\n", mode->CrtcVBlankStart));
    temp = mode->CrtcVBlankStart - 1;

    /* 3X5.5A[7:0]: Vertical Blanking Start Bits[7:0] */
    hwp->writeCrtc(hwp, 0x5A, temp & 0xFF);

    /* 3X5.5C[2:0]: Vertical Blanking Start Bits[10:8] */
    ViaCrtcMask(hwp, 0x5C, (temp >> 8) & 0x07, 0x07);


    /* Set IGA2 vertical blank end. */
    /* Vertical Blanking End: 4096 - 1 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcVBlankEnd: %d\n", mode->CrtcVBlankEnd));
    temp = mode->CrtcVBlankEnd - 1;

    /* 3X5.5B[7:0]: Vertical Blanking End Bits[7:0] */
    hwp->writeCrtc(hwp, 0x5B, temp & 0xFF);

    /* 3X5.5C[5:3]: Vertical Blanking End Bits[10:8] */
    ViaCrtcMask(hwp, 0x5C, (temp >> 5) & 0x38, 0x38);


    /* Set IGA2 vertical synchronization start. */
    /* Horizontal Retrace Start: 2047 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcVSyncStart: %d\n", mode->CrtcVSyncStart));
    temp = mode->CrtcVSyncStart;

    /* 3X5.5E[7:0]: Vertical Retrace Start Bits[7:0] */
    hwp->writeCrtc(hwp, 0x5E, temp & 0xFF);

    /* 3X5.5F[7:5]: Vertical Retrace Start Bits[10:8] */
    ViaCrtcMask(hwp, 0x5F, (temp >> 3) & 0xE0, 0xE0);


    /* Set IGA2 vertical synchronization end. */
    /* Vertical Retrace End: 32 (max) */
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "IGA2 CrtcVSyncEnd: %d\n", mode->CrtcVSyncEnd));
    temp = mode->CrtcVSyncEnd - mode->CrtcVSyncStart - 1;

    /*3X5.5F[4:0]: Vertical Retrace End[4:0] */
    ViaCrtcMask(hwp, 0x5F, temp & 0x1F, 0x1F);


    /* Set IGA2 horizontal offset adjustment. */
    temp = (pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) >> 3;

    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
    }

    /* 3X5.66[7:0]: Second Display Horizontal Offset Bits[7:0] */
    hwp->writeCrtc(hwp, 0x66, temp & 0xFF);

    /* 3X5.67[1:0]: Second Display Horizontal Offset Bits[9:8] */
    ViaCrtcMask(hwp, 0x67, (temp >> 8) & 0x03, 0x03);


    /* Set IGA2 alignment. */
    temp = (mode->CrtcHDisplay * (pScrn->bitsPerPixel >> 3)) >> 3;

    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
    }

    /* 3X5.65[7:0]: Second Display Horizontal
     * 2-Quadword Count Data Bits[7:0] */
    hwp->writeCrtc(hwp, 0x65, (temp >> 1) & 0xFF);

    /* 3X5.67[3:2]: Second Display Horizontal
     * 2-Quadword Count Data Bits[9:8] */
    ViaCrtcMask(hwp, 0x67, temp >> 7, 0x0C);


    switch (pVia->Chipset) {
        case VIA_CX700:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            break;
        default:
            /* some leftovers */
            hwp->writeCrtc(hwp, 0x08, 0x00);
            ViaCrtcMask(hwp, 0x32, 0, 0xFF);  /* ? */
            ViaCrtcMask(hwp, 0x33, 0, 0xC8);
            break;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA2SetDisplayRegister.\n"));
}

static ModeStatus
viaIGA2ModeValid(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA2ModeValid.\n"));

    if (mode->CrtcHTotal > 4096)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHDisplay > 2048)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHBlankStart > 2048)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHBlankEnd > 4096)
        return MODE_HBLANK_WIDE;

    if ((((pVia->Chipset == VIA_CLE266) || (pVia->Chipset == VIA_KM400))
            && (mode->CrtcHSyncStart > 2048))
        || (((pVia->Chipset != VIA_CLE266) && (pVia->Chipset != VIA_KM400))
            && (mode->CrtcHSyncStart > 4096)))
        return MODE_BAD_HVALUE;

    if ((mode->CrtcHSyncEnd - mode->CrtcHSyncStart) > 512)
        return MODE_HSYNC_WIDE;

    if (mode->CrtcVTotal > 2048)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVDisplay > 2048)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVBlankStart > 2048)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVBlankEnd > 2048)
        return MODE_VBLANK_WIDE;

    if (mode->CrtcVSyncStart > 2048)
        return MODE_BAD_VVALUE;

    if ((mode->CrtcVSyncEnd - mode->CrtcVSyncStart) > 32)
        return MODE_VSYNC_WIDE;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA2ModeValid.\n"));
    return MODE_OK;
}

void
viaIGA2Save(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIARegPtr Regs = &pVia->SavedReg;
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA2Save.\n"));

    /* Unlock extended registers. */
    hwp->writeSeq(hwp, 0x10, 0x01);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Saving IGA2 registers.\n"));

    for (i = 0; i < (0x88 - 0x50 + 1); i++) {
        Regs->EXCR[i + (0x50 - 0x50)] = hwp->readCrtc(hwp, i + 0x50);

    }

    for (i = 0; i < (0x92 - 0x8A + 1); i++) {
        Regs->EXCR[i + (0x8A - 0x50)] = hwp->readCrtc(hwp, i + 0x8A);

    }

    for (i = 0; i < (0xA3 - 0x94 + 1); i++) {
        Regs->EXCR[i + (0x94 - 0x50)] = hwp->readCrtc(hwp, i + 0x94);

    }

    Regs->EXCR[0xA4 - 0x50] = hwp->readCrtc(hwp, 0xA4);

    for (i = 0; i < (0xAC - 0xA5 + 1); i++) {
        Regs->EXCR[i + (0xA5 - 0x50)] = hwp->readCrtc(hwp, i + 0xA5);

    }

    /* Chrome 9 */
    switch (pVia->Chipset) {
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        Regs->EXCR[0xAF - 0x50] = hwp->readCrtc(hwp, 0xAF);
        break;
    default:
        break;
    }

    /* Chrome 9, Chrome 9 HC, and Chrome 9 HC3 */
    switch (pVia->Chipset) {
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
        for (i = 0; i < (0xCD - 0xB0 + 1); i++) {
            Regs->EXCR[i + (0xB0 - 0x50)] = hwp->readCrtc(hwp, i + 0xB0);

        }

        break;
    default:
        break;
    }

    switch (pVia->Chipset) {

    /* UniChrome Pro and UniChrome Pro II */
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
        for (i = 0; i < (0xD7 - 0xD0 + 1); i++) {
            Regs->EXCR[i + (0xD0 - 0x50)] = hwp->readCrtc(hwp, i + 0xD0);

        }

        break;

    /* Chrome 9 */
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        for (i = 0; i < (0xEC - 0xD0 + 1); i++) {
            Regs->EXCR[i + (0xD0 - 0x50)] = hwp->readCrtc(hwp, i + 0xD0);

        }

        break;
    default:
        break;
    }

    /* Chrome 9 */
    switch (pVia->Chipset) {
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        for (i = 0; i < (0xF5 - 0xF0 + 1); i++) {
            Regs->EXCR[i + (0xF0 - 0x50)] = hwp->readCrtc(hwp, i + 0xF0);

        }

        break;
    default:
        break;
    }

    /* Chrome 9 HCM and Chrome 9 HD */
    if ((pVia->Chipset == VIA_VX855) || (pVia->Chipset == VIA_VX900)) {
        for (i = 0; i < (0xFC - 0xF6 + 1); i++) {
            Regs->EXCR[i + (0xF6 - 0x50)] = hwp->readCrtc(hwp, i + 0xF6);

        }
    }

    /* Chrome 9 HD */
    if (pVia->Chipset == VIA_VX900) {
        Regs->EXCR[0xFD - 0x50] = hwp->readCrtc(hwp, 0xFD);

    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Finished saving IGA2 registers.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA2Save.\n"));
}

void
viaIGA2Restore(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIARegPtr Regs = &pVia->SavedReg;
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaIGA2Restore.\n"));

    /* Unlock extended registers. */
    hwp->writeSeq(hwp, 0x10, 0x01);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Restoring IGA2 registers.\n"));

    for (i = 0; i < (0x5F - 0x50 + 1); i++) {
        hwp->writeCrtc(hwp, i + 0x50, Regs->EXCR[i + (0x50 - 0x50)]);

    }

    for (i = 0; i < (0x69 - 0x62 + 1); i++) {
        hwp->writeCrtc(hwp, i + 0x62, Regs->EXCR[i + (0x62 - 0x50)]);

    }

    for (i = 0; i < (0x88 - 0x6D + 1); i++) {
        hwp->writeCrtc(hwp, i + 0x6D, Regs->EXCR[i + (0x6D - 0x50)]);

    }

    for (i = 0; i < (0x92 - 0x8A + 1); i++) {
        hwp->writeCrtc(hwp, i + 0x8A, Regs->EXCR[i + (0x8A - 0x50)]);

    }

    for (i = 0; i < (0xA3 - 0x94 + 1); i++) {
        hwp->writeCrtc(hwp, i + 0x94, Regs->EXCR[i + (0x94 - 0x50)]);

    }

    /* UniChrome Pro and UniChrome Pro II */
    switch (pVia->Chipset) {
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
        hwp->writeCrtc(hwp, 0xA4, Regs->EXCR[0xA4 - 0x50]);
        break;
    default:
        break;
    }

    for (i = 0; i < (0xAC - 0xA5 + 1); i++) {
        hwp->writeCrtc(hwp, i + 0xA5, Regs->EXCR[i + (0xA5 - 0x50)]);

    }

    /* Chrome 9 */
    switch (pVia->Chipset) {
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        hwp->writeCrtc(hwp, 0xAF, Regs->EXCR[0xAF - 0x50]);
        break;
    default:
        break;
    }

    /* Chrome 9, Chrome 9 HC, and Chrome 9 HC3 */
    switch (pVia->Chipset) {
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
        for (i = 0; i < (0xCD - 0xB0 + 1); i++) {
            hwp->writeCrtc(hwp, i + 0xB0, Regs->EXCR[i + (0xB0 - 0x50)]);

        }

        break;
    default:
        break;
    }

    switch (pVia->Chipset) {
    /* UniChrome Pro and UniChrome Pro II */
    case VIA_PM800:
    case VIA_K8M800:
    case VIA_P4M800PRO:
    case VIA_CX700:
    case VIA_P4M890:
        for (i = 0; i < (0xD7 - 0xD0 + 1); i++) {
            hwp->writeCrtc(hwp, i + 0xD0, Regs->EXCR[i + (0xD0 - 0x50)]);

        }

        break;

    /* Chrome 9 */
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        for (i = 0; i < (0xEC - 0xD0 + 1); i++) {
            hwp->writeCrtc(hwp, i + 0xD0, Regs->EXCR[i + (0xD0 - 0x50)]);

        }

        break;
    default:
        break;
    }

    /* Chrome 9 */
    switch (pVia->Chipset) {
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        for (i = 0; i < (0xF5 - 0xF0 + 1); i++) {
            hwp->writeCrtc(hwp, i + 0xF0, Regs->EXCR[i + (0xF0 - 0x50)]);

        }

        break;
    default:
        break;
    }

    /* Chrome 9 HCM and Chrome 9 HD */
    if ((pVia->Chipset == VIA_VX855) || (pVia->Chipset == VIA_VX900)) {
        for (i = 0; i < (0xFC - 0xF6 + 1); i++) {
            hwp->writeCrtc(hwp, i + 0xF6, Regs->EXCR[i + (0xF6 - 0x50)]);

        }
    }

    /* Chrome 9 HD */
    if (pVia->Chipset == VIA_VX900) {
        hwp->writeCrtc(hwp, 0xFD, Regs->EXCR[0xFD - 0x50]);

    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Finished restoring IGA2 registers.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaIGA2Restore.\n"));
}

/*
 * Not tested yet
 */
void
ViaShadowCRTCSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaShadowCRTCSetMode\n"));

    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD16 temp;

    temp = (mode->CrtcHTotal >> 3) - 5;
    hwp->writeCrtc(hwp, 0x6D, temp & 0xFF);
    ViaCrtcMask(hwp, 0x71, temp >> 5, 0x08);

    temp = (mode->CrtcHBlankEnd >> 3) - 1;
    hwp->writeCrtc(hwp, 0x6E, temp & 0xFF);

    temp = mode->CrtcVTotal - 2;
    hwp->writeCrtc(hwp, 0x6F, temp & 0xFF);
    ViaCrtcMask(hwp, 0x71, temp >> 8, 0x07);

    temp = mode->CrtcVDisplay - 1;
    hwp->writeCrtc(hwp, 0x70, temp & 0xFF);
    ViaCrtcMask(hwp, 0x71, temp >> 4, 0x70);

    temp = mode->CrtcVBlankStart - 1;
    hwp->writeCrtc(hwp, 0x72, temp & 0xFF);
    ViaCrtcMask(hwp, 0x74, temp >> 4, 0x70);

    temp = mode->CrtcVTotal - 1;
    hwp->writeCrtc(hwp, 0x73, temp & 0xFF);
    ViaCrtcMask(hwp, 0x74, temp >> 8, 0x07);

    ViaCrtcMask(hwp, 0x76, mode->CrtcVSyncEnd, 0x0F);

    temp = mode->CrtcVSyncStart;
    hwp->writeCrtc(hwp, 0x75, temp & 0xFF);
    ViaCrtcMask(hwp, 0x76, temp >> 4, 0x70);
}

static void
iga1_crtc_dpms(xf86CrtcPtr crtc, int mode)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga1_crtc_dpms.\n"));

    switch (mode) {
    case DPMSModeOn:
        viaIGA1DPMSControl(pScrn, 0x00);

        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayEnableSimultaneous(pScrn);
        break;

    case DPMSModeStandby:
        viaIGA1DPMSControl(pScrn, 0x01);

        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayDisableSimultaneous(pScrn);
        break;

    case DPMSModeSuspend:
        viaIGA1DPMSControl(pScrn, 0x02);

        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayDisableSimultaneous(pScrn);
        break;

    case DPMSModeOff:
        viaIGA1DPMSControl(pScrn, 0x03);

        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayDisableSimultaneous(pScrn);
        break;

	default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Invalid DPMS Mode: %d\n",
                    mode);
        break;
    }
    //vgaHWSaveScreen(pScrn->pScreen, mode);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga1_crtc_dpms.\n"));
}

static void
iga1_crtc_save(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga1_crtc_save.\n"));

    vgaHWProtect(pScrn, TRUE);

    /* Save the standard VGA registers. */
    if (xf86IsPrimaryPci(pVia->PciInfo)) {
        vgaHWSave(pScrn, &hwp->SavedReg, VGA_SR_ALL);
    } else {
        vgaHWSave(pScrn, &hwp->SavedReg, VGA_SR_MODE);
    }

    viaIGA1Save(pScrn);

    vgaHWProtect(pScrn, FALSE);
    vgaHWUnlock(hwp);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga1_crtc_save.\n"));
}

static void
iga1_crtc_restore(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 tmp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga1_crtc_restore.\n"));

    vgaHWProtect(pScrn, TRUE);

    /* Restore the standard VGA registers. */
    if (xf86IsPrimaryPci(pVia->PciInfo)) {
        vgaHWRestore(pScrn, &hwp->SavedReg, VGA_SR_ALL);
    } else {
        vgaHWRestore(pScrn, &hwp->SavedReg, VGA_SR_MODE);
    }

    /* Gamma must be disabled before restoring palette. */
    ViaGammaDisable(pScrn);

    viaIGA1Restore(pScrn);

    ViaDisablePrimaryFIFO(pScrn);

    vgaHWProtect(pScrn, FALSE);
    vgaHWLock(hwp);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga1_crtc_restore.\n"));
}

static Bool
iga1_crtc_lock(xf86CrtcPtr crtc)
{
    return FALSE;
}

static void
iga1_crtc_unlock(xf86CrtcPtr crtc)
{
}

static Bool
iga1_crtc_mode_fixup(xf86CrtcPtr crtc, DisplayModePtr mode,
                        DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;
    ModeStatus modestatus;

    if ((mode->Clock < pScrn->clockRanges->minClock) ||
        (mode->Clock > pScrn->clockRanges->maxClock)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                   "Clock for mode \"%s\" outside of allowed range (%u (%u - %u))\n",
                   mode->name, mode->Clock, pScrn->clockRanges->minClock,
                   pScrn->clockRanges->maxClock);
        return FALSE;
    }

    modestatus = viaIGA1ModeValid(pScrn, mode);
    if (modestatus != MODE_OK) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Not using mode \"%s\" : %s.\n",
                   mode->name, xf86ModeStatusToString(modestatus));
        return FALSE;
    }

    temp = mode->CrtcHDisplay * mode->CrtcVDisplay * mode->VRefresh *
            (pScrn->bitsPerPixel >> 3);
    if (pVia->pBIOSInfo->Bandwidth < temp) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Required bandwidth is not available. (%u > %u)\n",
                    (unsigned)temp, (unsigned)pVia->pBIOSInfo->Bandwidth);
        return FALSE;
    }
    return TRUE;
}

static void
iga1_crtc_prepare (xf86CrtcPtr crtc)
{
}

static void
iga1_crtc_set_origin(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga1_crtc_set_origin.\n"));

    viaIGA1SetFBStartingAddress(crtc, x, y);
    VIAVidAdjustFrame(pScrn, x, y);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga1_crtc_set_origin.\n"));
}

static void
iga1_crtc_mode_set(xf86CrtcPtr crtc, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode,
                    int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga1_crtc_mode_set.\n"));

    if (!vgaHWInit(pScrn, adjusted_mode)) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "vgaHWInit failed.\n"));
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting iga1_crtc_mode_set.\n"));
        return;
    }

    /* Turn off IGA1 during mode setting. */
    viaIGA1DPMSControl(pScrn, 0x03);

    viaIGAInitCommon(pScrn);
    ViaCRTCInit(pScrn);

    /* Turn off Screen */
    ViaCrtcMask(hwp, 0x17, 0x00, 0x80);

    /* Disable IGA1 */
    ViaSeqMask(hwp, 0x59, 0x00, 0x80);

    viaIGA1SetDisplayRegister(pScrn, adjusted_mode);
    ViaSetPrimaryFIFO(pScrn, adjusted_mode);

    pBIOSInfo->Clock = ViaModeDotClockTranslate(pScrn, adjusted_mode);
    pBIOSInfo->ClockExternal = FALSE;
    ViaSetPrimaryDotclock(pScrn, pBIOSInfo->Clock);
    ViaSetUseExternalClock(hwp);
    ViaCrtcMask(hwp, 0x6B, 0x00, 0x01);

    hwp->disablePalette(hwp);

    /* Enable IGA1 */
    ViaSeqMask(hwp, 0x59, 0x80, 0x80);

    /* Turn on Screen */
    ViaCrtcMask(hwp, 0x17, 0x80, 0x80);

    if (pVia->pBIOSInfo->SimultaneousEnabled)
        ViaDisplayEnableSimultaneous(pScrn);
    else
        ViaDisplayDisableSimultaneous(pScrn);

    iga1_crtc_set_origin(crtc, crtc->x, crtc->y);

    /* Turn on IGA1 now that mode setting is done. */
    viaIGA1DPMSControl(pScrn, 0x00);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga1_crtc_mode_set.\n"));
}

static void
iga1_crtc_gamma_set(xf86CrtcPtr crtc, CARD16 *red, CARD16 *green, CARD16 *blue,
					int size)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    int SR1A, SR1B, CR67, CR6A;
    LOCO colors[size];
    int i;

    for (i = 0; i < size; i++) {
        colors[i].red = red[i] >> 8;
        colors[i].green = green[i] >> 8;
        colors[i].blue = blue[i] >> 8;
    }

    if (pScrn->bitsPerPixel != 8) {
        switch (pVia->Chipset) {
        case VIA_CLE266:
        case VIA_KM400:
            ViaSeqMask(hwp, 0x16, 0x80, 0x80);
            break;
        default:
            ViaCrtcMask(hwp, 0x33, 0x80, 0x80);
            break;
        }

        ViaSeqMask(hwp, 0x1A, 0x00, 0x01);
        VIALoadRgbLut(pScrn, 0, size, colors);

    } else {

        SR1A = hwp->readSeq(hwp, 0x1A);
        SR1B = hwp->readSeq(hwp, 0x1B);
        CR67 = hwp->readCrtc(hwp, 0x67);
        CR6A = hwp->readCrtc(hwp, 0x6A);

        for (i = 0; i < size; i++) {
            hwp->writeDacWriteAddr(hwp, i);
            hwp->writeDacData(hwp, colors[i].red);
            hwp->writeDacData(hwp, colors[i].green);
            hwp->writeDacData(hwp, colors[i].blue);
        }
    }
}

static void *
iga1_crtc_shadow_allocate (xf86CrtcPtr crtc, int width, int height)
{
    return NULL;
}

static PixmapPtr
iga1_crtc_shadow_create(xf86CrtcPtr crtc, void *data, int width, int height)
{
    return NULL;
}

static void
iga1_crtc_shadow_destroy(xf86CrtcPtr crtc, PixmapPtr rotate_pixmap, void *data)
{
}

/*
    Set the cursor foreground and background colors.  In 8bpp, fg and
    bg are indices into the current colormap unless the
    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP flag is set.  In that case
    and in all other bpps the fg and bg are in 8-8-8 RGB format.
*/
static void
iga1_crtc_set_cursor_colors (xf86CrtcPtr crtc, int bg, int fg)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;

    if (xf86_config->cursor_fg)
        return;

    /* Don't recolour the image if we don't have to. */
    if (fg == xf86_config->cursor_fg && bg == xf86_config->cursor_bg)
        return;

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        temp = VIAGETREG(PRIM_HI_CTRL);
        VIASETREG(PRIM_HI_CTRL, temp & 0xFFFFFFFE);
        break;

    default:
        temp = VIAGETREG(HI_CONTROL);
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFE);
        break;
    }

    xf86_config->cursor_fg = fg;
    xf86_config->cursor_bg = bg;
}

static void
iga1_crtc_set_cursor_position (xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    unsigned xoff, yoff;

    if (x < 0) {
        xoff = ((-x) & 0xFE);
        x = 0;
    } else {
        xoff = 0;
    }

    if (y < 0) {
        yoff = ((-y) & 0xFE);
        y = 0;
    } else {
        yoff = 0;
    }

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        VIASETREG(PRIM_HI_POSSTART,    ((x    << 16) | (y    & 0x07ff)));
        VIASETREG(PRIM_HI_CENTEROFFSET, ((xoff << 16) | (yoff & 0x07ff)));
        break;

    default:
        VIASETREG(HI_POSSTART,    ((x    << 16) | (y    & 0x07ff)));
        VIASETREG(HI_CENTEROFFSET, ((xoff << 16) | (yoff & 0x07ff)));
        break;
    }
}

static void
iga1_crtc_show_cursor (xf86CrtcPtr crtc)
{
    drmmode_crtc_private_ptr iga = crtc->driver_private;
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        VIASETREG(PRIM_HI_FBOFFSET, iga->cursor_bo->offset);
        VIASETREG(PRIM_HI_CTRL, 0x36000005);
        break;

    default:
        /* Mono Cursor Display Path [bit31]: Primary */
        VIASETREG(HI_FBOFFSET, iga->cursor_bo->offset);
        VIASETREG(HI_CONTROL, 0x76000005);
        break;
    }
}

static void
iga1_crtc_hide_cursor (xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        temp = VIAGETREG(PRIM_HI_CTRL);
        VIASETREG(PRIM_HI_CTRL, temp & 0xFFFFFFFA);
        break;

    default:
        temp = VIAGETREG(HI_CONTROL);
        /* Hardware cursor disable [bit0] */
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFA);
        break;
    }
}

static void
iga_crtc_load_cursor_argb(xf86CrtcPtr crtc, CARD32 *image)
{
    drmmode_crtc_private_ptr iga = crtc->driver_private;
    ScrnInfoPtr pScrn = crtc->scrn;
    void *dst;

    dst = drm_bo_map(pScrn, iga->cursor_bo);
    memset(dst, 0x00, iga->cursor_bo->size);
    memcpy(dst, image, iga->cursor_bo->size);
    drm_bo_unmap(pScrn, iga->cursor_bo);
}

static void
iga_crtc_commit(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    if (crtc->scrn->pScreen != NULL && pVia->drmmode.hwcursor)
        xf86_reload_cursors(crtc->scrn->pScreen);
}

static void
iga_crtc_destroy(xf86CrtcPtr crtc)
{
    if (crtc->driver_private)
        free(crtc->driver_private);
}

const xf86CrtcFuncsRec iga1_crtc_funcs = {
    .dpms                   = iga1_crtc_dpms,
    .save                   = iga1_crtc_save,
    .restore                = iga1_crtc_restore,
    .lock                   = iga1_crtc_lock,
    .unlock                 = iga1_crtc_unlock,
    .mode_fixup             = iga1_crtc_mode_fixup,
    .prepare                = iga1_crtc_prepare,
    .mode_set               = iga1_crtc_mode_set,
    .commit                 = iga_crtc_commit,
    .gamma_set              = iga1_crtc_gamma_set,
    .shadow_create          = iga1_crtc_shadow_create,
    .shadow_allocate        = iga1_crtc_shadow_allocate,
    .shadow_destroy         = iga1_crtc_shadow_destroy,
    .set_cursor_colors      = iga1_crtc_set_cursor_colors,
    .set_cursor_position    = iga1_crtc_set_cursor_position,
    .show_cursor            = iga1_crtc_show_cursor,
    .hide_cursor            = iga1_crtc_hide_cursor,
    .load_cursor_argb       = iga_crtc_load_cursor_argb,
#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) > 2
    .set_origin             = iga1_crtc_set_origin,
#endif
    .destroy                = iga_crtc_destroy,
};

static void
iga2_crtc_dpms(xf86CrtcPtr crtc, int mode)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga2_crtc_dpms.\n"));

    switch (mode) {
    case DPMSModeOn:
        viaIGA2DisplayOutput(pScrn, TRUE);

        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayEnableSimultaneous(pScrn);
        break;

    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        viaIGA2DisplayOutput(pScrn, FALSE);

        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayDisableSimultaneous(pScrn);
        break;

    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Invalid DPMS mode: %d\n",
                    mode);
        break;
    }
    //vgaHWSaveScreen(pScrn->pScreen, mode);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga2_crtc_dpms.\n"));
}

static void
iga2_crtc_save(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga2_crtc_save.\n"));

    viaIGA2Save(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga2_crtc_save.\n"));
}

static void
iga2_crtc_restore(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 tmp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga2_crtc_restore.\n"));

    viaIGA2Restore(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga2_crtc_restore.\n"));
}

static Bool
iga2_crtc_lock(xf86CrtcPtr crtc)
{
    return FALSE;
}

static void
iga2_crtc_unlock(xf86CrtcPtr crtc)
{
}

static Bool
iga2_crtc_mode_fixup(xf86CrtcPtr crtc, DisplayModePtr mode,
                        DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;
    ModeStatus modestatus;

    if ((mode->Clock < pScrn->clockRanges->minClock) ||
        (mode->Clock > pScrn->clockRanges->maxClock)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                   "Clock for mode \"%s\" outside of allowed range (%u (%u - %u))\n",
                   mode->name, mode->Clock, pScrn->clockRanges->minClock,
                   pScrn->clockRanges->maxClock);
        return FALSE;
    }

    modestatus = viaIGA2ModeValid(pScrn, mode);
    if (modestatus != MODE_OK) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Not using mode \"%s\" : %s.\n",
                   mode->name, xf86ModeStatusToString(modestatus));
        return FALSE;
    }

    temp = mode->CrtcHDisplay * mode->CrtcVDisplay * mode->VRefresh *
            (pScrn->bitsPerPixel >> 3);
    if (pVia->pBIOSInfo->Bandwidth < temp) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Required bandwidth is not available. (%u > %u)\n",
			        (unsigned)temp, (unsigned)pVia->pBIOSInfo->Bandwidth);
        return FALSE;
    }
    return TRUE;
}

static void
iga2_crtc_prepare (xf86CrtcPtr crtc)
{
}

static void
iga2_crtc_set_origin(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga2_crtc_set_origin.\n"));

    viaIGA2SetFBStartingAddress(crtc, x, y);
    VIAVidAdjustFrame(pScrn, x, y);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga2_crtc_set_origin.\n"));
}

static void
iga2_crtc_mode_set(xf86CrtcPtr crtc, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga2_crtc_mode_set.\n"));

    if (!vgaHWInit(pScrn, adjusted_mode)) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "vgaHWInit failed.\n"));
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting iga2_crtc_mode_set.\n"));
        return;
    }

    /* Turn off IGA2 during mode setting. */
    viaIGA2DisplayOutput(pScrn, FALSE);

    viaIGAInitCommon(pScrn);
    ViaCRTCInit(pScrn);

    viaIGA2SetDisplayRegister(pScrn, adjusted_mode);
    ViaSetSecondaryFIFO(pScrn, adjusted_mode);
    pBIOSInfo->Clock = ViaModeDotClockTranslate(pScrn, adjusted_mode);
    pBIOSInfo->ClockExternal = FALSE;
    ViaSetSecondaryDotclock(pScrn, pBIOSInfo->Clock);
    ViaSetUseExternalClock(hwp);

    hwp->disablePalette(hwp);

    viaIGA2DisplayChannel(pScrn, TRUE);

    if (pVia->pBIOSInfo->SimultaneousEnabled)
        ViaDisplayEnableSimultaneous(pScrn);
    else
        ViaDisplayDisableSimultaneous(pScrn);

    iga2_crtc_set_origin(crtc, crtc->x, crtc->y);

    /* Turn on IGA2 now that mode setting is done. */
    viaIGA2DisplayOutput(pScrn, TRUE);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting iga2_crtc_mode_set.\n"));
}

static void
iga2_crtc_gamma_set(xf86CrtcPtr crtc, CARD16 *red, CARD16 *green, CARD16 *blue,
					int size)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    int SR1A, SR1B, CR67, CR6A;
    LOCO colors[size];
    int i;

    for (i = 0; i < size; i++) {
        colors[i].red = red[i] >> 8;
        colors[i].green = green[i] >> 8;
        colors[i].blue = blue[i] >> 8;
    }

    if (pScrn->bitsPerPixel != 8) {
        if (!(pVia->Chipset == VIA_CLE266 &&
            CLE266_REV_IS_AX(pVia->ChipRev))) {
            ViaSeqMask(hwp, 0x1A, 0x01, 0x01);
            ViaCrtcMask(hwp, 0x6A, 0x02, 0x02);

            switch (pVia->Chipset) {
            case VIA_CLE266:
            case VIA_KM400:
            case VIA_K8M800:
            case VIA_PM800:
                break;

            default:
                ViaCrtcMask(hwp, 0x6A, 0x20, 0x20);
                break;
            }
            VIALoadRgbLut(pScrn, 0, size, colors);
        }
    } else {
        SR1A = hwp->readSeq(hwp, 0x1A);
        SR1B = hwp->readSeq(hwp, 0x1B);
        CR67 = hwp->readCrtc(hwp, 0x67);
        CR6A = hwp->readCrtc(hwp, 0x6A);

        ViaSeqMask(hwp, 0x1A, 0x01, 0x01);
        ViaSeqMask(hwp, 0x1B, 0x80, 0x80);
        ViaCrtcMask(hwp, 0x67, 0x00, 0xC0);
        ViaCrtcMask(hwp, 0x6A, 0xC0, 0xC0);

        for (i = 0; i < size; i++) {
            hwp->writeDacWriteAddr(hwp, i);
            hwp->writeDacData(hwp, colors[i].red);
            hwp->writeDacData(hwp, colors[i].green);
            hwp->writeDacData(hwp, colors[i].blue);
        }

        hwp->writeSeq(hwp, 0x1A, SR1A);
        hwp->writeSeq(hwp, 0x1B, SR1B);
        hwp->writeCrtc(hwp, 0x67, CR67);
        hwp->writeCrtc(hwp, 0x6A, CR6A);

        /* Screen 0 palette was changed by mode setting of Screen 1,
         * so load it again. */
        for (i = 0; i < size; i++) {
            hwp->writeDacWriteAddr(hwp, i);
            hwp->writeDacData(hwp, colors[i].red);
            hwp->writeDacData(hwp, colors[i].green);
            hwp->writeDacData(hwp, colors[i].blue);
        }
    }
}

static void *
iga2_crtc_shadow_allocate (xf86CrtcPtr crtc, int width, int height)
{
    return NULL;
}

static PixmapPtr
iga2_crtc_shadow_create(xf86CrtcPtr crtc, void *data, int width, int height)
{
    return NULL;
}

static void
iga2_crtc_shadow_destroy(xf86CrtcPtr crtc, PixmapPtr rotate_pixmap, void *data)
{
}

/*
    Set the cursor foreground and background colors.  In 8bpp, fg and
    bg are indices into the current colormap unless the
    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP flag is set.  In that case
    and in all other bpps the fg and bg are in 8-8-8 RGB format.
*/
static void
iga2_crtc_set_cursor_colors(xf86CrtcPtr crtc, int bg, int fg)
{
    drmmode_crtc_private_ptr iga = crtc->driver_private;
    ScrnInfoPtr pScrn = crtc->scrn;
    xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    int height = 64, width = 64, i;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 pixel, temp, *dst;

    if (xf86_config->cursor_fg)
        return;

    fg |= 0xff000000;
    bg |= 0xff000000;

    /* Don't recolour the image if we don't have to. */
    if (fg == xf86_config->cursor_fg && bg == xf86_config->cursor_bg)
        return;

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        temp = VIAGETREG(HI_CONTROL);
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFE);
        break;

    default:
        temp = VIAGETREG(HI_CONTROL);
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFE);
        height = width = 32;
        break;
    }

    dst = drm_bo_map(pScrn, iga->cursor_bo);
    for (i = 0; i < width * height; i++, dst++)
        if ((pixel = *dst))
            *dst = (pixel == xf86_config->cursor_fg) ? fg : bg;
    drm_bo_unmap(pScrn, iga->cursor_bo);

    xf86_config->cursor_fg = fg;
    xf86_config->cursor_bg = bg;
}

static void
iga2_crtc_set_cursor_position(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    unsigned xoff, yoff;

    if (x < 0) {
        xoff = ((-x) & 0xFE);
        x = 0;
    } else {
        xoff = 0;
    }

    if (y < 0) {
        yoff = ((-y) & 0xFE);
        y = 0;
    } else {
        yoff = 0;
    }

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        VIASETREG(HI_POSSTART,    ((x    << 16) | (y    & 0x07ff)));
        VIASETREG(HI_CENTEROFFSET, ((xoff << 16) | (yoff & 0x07ff)));
        break;

    default:
        VIASETREG(HI_POSSTART,    ((x    << 16) | (y    & 0x07ff)));
        VIASETREG(HI_CENTEROFFSET, ((xoff << 16) | (yoff & 0x07ff)));
        break;
    }
}

static void
iga2_crtc_show_cursor(xf86CrtcPtr crtc)
{
    drmmode_crtc_private_ptr iga = crtc->driver_private;
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        VIASETREG(HI_FBOFFSET, iga->cursor_bo->offset);
        VIASETREG(HI_CONTROL, 0xB6000005);
        break;

    default:
        /* Mono Cursor Display Path [bit31]: Secondary */
        /* FIXME For CLE266 and KM400 try to enable 32x32 cursor size [bit1] */
        VIASETREG(HI_FBOFFSET, iga->cursor_bo->offset);
        VIASETREG(HI_CONTROL, 0xF6000005);
        break;
    }
}

static void
iga2_crtc_hide_cursor(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
	    temp = VIAGETREG(HI_CONTROL);
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFA);
        break;

    default:
        temp = VIAGETREG(HI_CONTROL);
        /* Hardware cursor disable [bit0] */
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFA);
        break;
	}
}

const xf86CrtcFuncsRec iga2_crtc_funcs = {
    .dpms                   = iga2_crtc_dpms,
    .save                   = iga2_crtc_save,
    .restore                = iga2_crtc_restore,
    .lock                   = iga2_crtc_lock,
    .unlock                 = iga2_crtc_unlock,
    .mode_fixup             = iga2_crtc_mode_fixup,
    .prepare                = iga2_crtc_prepare,
    .mode_set               = iga2_crtc_mode_set,
    .commit                 = iga_crtc_commit,
    .gamma_set              = iga2_crtc_gamma_set,
    .shadow_create          = iga2_crtc_shadow_create,
    .shadow_allocate        = iga2_crtc_shadow_allocate,
    .shadow_destroy         = iga2_crtc_shadow_destroy,
    .set_cursor_colors      = iga2_crtc_set_cursor_colors,
    .set_cursor_position    = iga2_crtc_set_cursor_position,
    .show_cursor            = iga2_crtc_show_cursor,
    .hide_cursor            = iga2_crtc_hide_cursor,
    .load_cursor_argb       = iga_crtc_load_cursor_argb,
#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) > 2
    .set_origin             = iga2_crtc_set_origin,
#endif
    .destroy                = iga_crtc_destroy,
};
