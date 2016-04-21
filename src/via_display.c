/*
 * Copyright 2005-2015 The Openchrome Project
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
 * Enables the second display channel.
 */
void
ViaSecondDisplayChannelEnable(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaSecondDisplayChannelEnable\n"));
    ViaCrtcMask(hwp, 0x6A, 0x00, 1 << 6);
    ViaCrtcMask(hwp, 0x6A, 1 << 7, 1 << 7);
    ViaCrtcMask(hwp, 0x6A, 1 << 6, 1 << 6);
}

/*
 * Disables the second display channel.
 */
void
ViaSecondDisplayChannelDisable(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaSecondDisplayChannelDisable\n"));

    ViaCrtcMask(hwp, 0x6A, 0x00, 1 << 6);
    ViaCrtcMask(hwp, 0x6A, 0x00, 1 << 7);
    ViaCrtcMask(hwp, 0x6A, 1 << 6, 1 << 6);
}

/*
 * Initial settings for displays.
 */
void
ViaDisplayInit(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplayInit\n"));

    ViaSecondDisplayChannelDisable(pScrn);
    ViaCrtcMask(hwp, 0x6A, 0x00, 0x3D);

    hwp->writeCrtc(hwp, 0x6B, 0x00);
    hwp->writeCrtc(hwp, 0x6C, 0x00);
    hwp->writeCrtc(hwp, 0x79, 0x00);

    /* (IGA1 Timing Plus 2, added in VT3259 A3 or later) */
    if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400)
        ViaCrtcMask(hwp, 0x47, 0x00, 0xC8);
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

void
viaIGA1SetDisplayRegister(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD16 temp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "viaIGA1SetDisplayRegister\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Setting up %s\n", mode->name));

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

    /* bpp */
    switch (pScrn->bitsPerPixel) {
        case 8:
            /* Only CLE266.AX use 6bits LUT. */
            if (pVia->Chipset == VIA_CLE266 && pVia->ChipRev < 15)
                ViaSeqMask(hwp, 0x15, 0x22, 0xFE);
            else
                ViaSeqMask(hwp, 0x15, 0xA2, 0xFE);
            break;
        case 16:
            ViaSeqMask(hwp, 0x15, 0xB6, 0xFE);
            break;
        case 24:
        case 32:
            ViaSeqMask(hwp, 0x15, 0xAE, 0xFE);
            break;
        default:
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Unhandled bitdepth: %d\n",
                       pScrn->bitsPerPixel);
            break;
    }

    switch (pVia->ChipId) {
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

    /* Crtc registers */
    /* horizontal total : 4100 */
    temp = (mode->CrtcHTotal >> 3) - 5;
    hwp->writeCrtc(hwp, 0x00, temp & 0xFF);
    ViaCrtcMask(hwp, 0x36, temp >> 5, 0x08);

    /* horizontal address : 2048 */
    temp = (mode->CrtcHDisplay >> 3) - 1;
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

    /* FIXME: check if this is really necessary here */
    switch (pVia->ChipId) {
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

    switch (pVia->ChipId) {
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

void
viaIGA1SetMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "viaIGA1SetMode\n");
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    /* Turn off Screen */
    ViaCrtcMask(hwp, 0x17, 0x00, 0x80);

    /* Disable IGA1 */
    ViaSeqMask(hwp, 0x59, 0x00, 0x80);

    viaIGA1SetDisplayRegister(pScrn, mode);
    ViaSetPrimaryFIFO(pScrn, mode);

    pBIOSInfo->Clock = ViaModeDotClockTranslate(pScrn, mode);
    pBIOSInfo->ClockExternal = FALSE;
    ViaSetPrimaryDotclock(pScrn, pBIOSInfo->Clock);
    ViaSetUseExternalClock(hwp);
    ViaCrtcMask(hwp, 0x6B, 0x00, 0x01);

    hwp->disablePalette(hwp);

    /* Enable IGA1 */
    ViaSeqMask(hwp, 0x59, 0x80, 0x80);

    /* Turn on Screen */
    ViaCrtcMask(hwp, 0x17, 0x80, 0x80);
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

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "mode: %p\n", mode);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "mode->name: %p\n", mode->name);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "mode->name: %s\n", mode->name);


    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "viaIGA2SetDisplayRegister\n"));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Setting up %s\n", mode->name));
    /* bpp */
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
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Unhandled bitdepth: %d\n",
                       pScrn->bitsPerPixel);
            break;
    }

    switch (pVia->ChipId) {
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

    /* Crtc registers */
    /* horizontal total : 4096 */
    temp = mode->CrtcHTotal - 1;
    hwp->writeCrtc(hwp, 0x50, temp & 0xFF);
    ViaCrtcMask(hwp, 0x55, temp >> 8, 0x0F);

    /* horizontal address : 2048 */
    temp = mode->CrtcHDisplay - 1;
    hwp->writeCrtc(hwp, 0x51, temp & 0xFF);
    ViaCrtcMask(hwp, 0x55, temp >> 4, 0x70);

    /* horizontal blanking start : 2048 */
    /* temp = mode->CrtcHDisplay - 1; */
    temp = mode->CrtcHBlankStart - 1;
    hwp->writeCrtc(hwp, 0x52, temp & 0xFF);
    ViaCrtcMask(hwp, 0x54, temp >> 8, 0x07);

    /* horizontal blanking end : 4096 */
    /* temp = mode->CrtcHTotal - 1; */
    temp = mode->CrtcHBlankEnd - 1;
    hwp->writeCrtc(hwp, 0x53, temp & 0xFF);
    ViaCrtcMask(hwp, 0x54, temp >> 5, 0x38);
    ViaCrtcMask(hwp, 0x5D, temp >> 5, 0x40);

    /* horizontal sync start : 2047 */
    temp = mode->CrtcHSyncStart;
    hwp->writeCrtc(hwp, 0x56, temp & 0xFF);
    ViaCrtcMask(hwp, 0x54, temp >> 2, 0xC0);
    ViaCrtcMask(hwp, 0x5C, temp >> 3, 0x80);

    if (pVia->ChipId != VIA_CLE266 && pVia->ChipId != VIA_KM400)
        ViaCrtcMask(hwp, 0x5D, temp >> 4, 0x80);

    /* horizontal sync end : sync start + 512 */
    temp = mode->CrtcHSyncEnd;
    hwp->writeCrtc(hwp, 0x57, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5C, temp >> 2, 0x40);

    /* vertical total : 2048 */
    temp = mode->CrtcVTotal - 1;
    hwp->writeCrtc(hwp, 0x58, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5D, temp >> 8, 0x07);


    /* vertical address : 2048 */
    temp = mode->CrtcVDisplay - 1;
    hwp->writeCrtc(hwp, 0x59, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5D, temp >> 5, 0x38);

    /* vertical blanking start : 2048 */
    /* temp = mode->CrtcVDisplay - 1; */
    temp = mode->CrtcVBlankStart - 1;
    hwp->writeCrtc(hwp, 0x5A, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5C, temp >> 8, 0x07);

    /* vertical blanking end : 2048 */
    /* temp = mode->CrtcVTotal - 1; */
    temp = mode->CrtcVBlankEnd - 1;
    hwp->writeCrtc(hwp, 0x5B, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5C, temp >> 5, 0x38);

    /* vertical sync start : 2047 */
    temp = mode->CrtcVSyncStart;
    hwp->writeCrtc(hwp, 0x5E, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5F, temp >> 3, 0xE0);

    /* vertical sync end : start + 32 */
    temp = mode->CrtcVSyncEnd;
    ViaCrtcMask(hwp, 0x5F, temp, 0x1F);

    /* offset */
    temp = (pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) >> 3;
    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
    }

    hwp->writeCrtc(hwp, 0x66, temp & 0xFF);
    ViaCrtcMask(hwp, 0x67, temp >> 8, 0x03);

    /* Fix LCD scaling. */
    /* fetch count */
    temp = (mode->CrtcHDisplay * (pScrn->bitsPerPixel >> 3)) >> 3;
    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
    }

    hwp->writeCrtc(hwp, 0x65, (temp >> 1) & 0xFF);
    ViaCrtcMask(hwp, 0x67, temp >> 7, 0x0C);

    switch (pVia->ChipId) {
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
}

void
viaIGA2SetMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    DisplayModePtr realMode = mode;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "viaIGA2SetMode\n"));

    viaIGA2SetDisplayRegister(pScrn, realMode);
    ViaSetSecondaryFIFO(pScrn, realMode);
    pBIOSInfo->Clock = ViaModeDotClockTranslate(pScrn, realMode);
    pBIOSInfo->ClockExternal = FALSE;
    ViaSetSecondaryDotclock(pScrn, pBIOSInfo->Clock);
    ViaSetUseExternalClock(hwp);

    hwp->disablePalette(hwp);
}

/*
 * Checks for limitations imposed by the available VGA timing registers.
 */
static ModeStatus
ViaFirstCRTCModeValid(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaFirstCRTCModeValid\n"));

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

    return MODE_OK;
}

static ModeStatus
ViaSecondCRTCModeValid(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaSecondCRTCModeValid\n"));

    if (mode->CrtcHTotal > 4096)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHDisplay > 2048)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHBlankStart > 2048)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHBlankEnd > 4096)
        return MODE_HBLANK_WIDE;

    if (mode->CrtcHSyncStart > 2047)
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

    if (mode->CrtcVSyncStart > 2047)
        return MODE_BAD_VVALUE;

    if ((mode->CrtcVSyncEnd - mode->CrtcVSyncStart) > 32)
        return MODE_VSYNC_WIDE;

    return MODE_OK;
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

    switch (mode) {
    case DPMSModeOn:
        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayEnableSimultaneous(pScrn);
        break;

    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayDisableSimultaneous(pScrn);
        break;

	default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Invalid DPMS mode %d\n",
                    mode);
        break;
    }
    //vgaHWSaveScreen(pScrn->pScreen, mode);

}

static void
iga1_crtc_save(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    VIASave(pScrn);
    vgaHWUnlock(hwp);
}

static void
iga1_crtc_restore(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    VIARestore(pScrn);

    /* A soft reset helps to avoid a 3D hang on VT switch. */
    switch (pVia->Chipset) {
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        break;

    default:
        hwp->writeSeq(hwp, 0x1A, pVia->SavedReg.SR1A | 0x40);
        break;
    }
    vgaHWLock(hwp);
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

    modestatus = ViaFirstCRTCModeValid(pScrn, mode);
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

    viaIGA1SetFBStartingAddress(crtc, x, y);
    VIAVidAdjustFrame(pScrn, x, y);
}

static void
iga1_crtc_mode_set(xf86CrtcPtr crtc, DisplayModePtr mode,
					DisplayModePtr adjusted_mode,
					int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga1_crtc_mode_set.\n"));

    if (!vgaHWInit(pScrn, adjusted_mode)) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "vgaHWInit failed.\n"));
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting iga1_crtc_mode_set.\n"));
        return;
    }

    ViaCRTCInit(pScrn);
    viaIGA1SetMode(pScrn, adjusted_mode);

    if (pVia->pBIOSInfo->SimultaneousEnabled)
        ViaDisplayEnableSimultaneous(pScrn);
    else
        ViaDisplayDisableSimultaneous(pScrn);

    iga1_crtc_set_origin(crtc, crtc->x, crtc->y);

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

    switch (mode) {
    case DPMSModeOn:
        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayEnableSimultaneous(pScrn);
        break;

    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayDisableSimultaneous(pScrn);
        break;

    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Invalid DPMS mode %d\n",
                    mode);
        break;
    }
    //vgaHWSaveScreen(pScrn->pScreen, mode);
}

static void
iga2_crtc_save(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    VIASave(pScrn);
    vgaHWUnlock(hwp);
}

static void
iga2_crtc_restore(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    VIARestore(pScrn);
    vgaHWLock(hwp);
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

    modestatus = ViaFirstCRTCModeValid(pScrn, mode);
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

    viaIGA2SetFBStartingAddress(crtc, x, y);
    VIAVidAdjustFrame(pScrn, x, y);
}

static void
iga2_crtc_mode_set(xf86CrtcPtr crtc, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered iga2_crtc_mode_set.\n"));

    if (!vgaHWInit(pScrn, adjusted_mode)) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "vgaHWInit failed.\n"));
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting iga2_crtc_mode_set.\n"));
        return;
    }

    ViaCRTCInit(pScrn);
    viaIGA2SetMode(pScrn, adjusted_mode);
    ViaSecondDisplayChannelEnable(pScrn);

    if (pVia->pBIOSInfo->SimultaneousEnabled)
        ViaDisplayEnableSimultaneous(pScrn);
    else
        ViaDisplayDisableSimultaneous(pScrn);

    iga2_crtc_set_origin(crtc, crtc->x, crtc->y);

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
