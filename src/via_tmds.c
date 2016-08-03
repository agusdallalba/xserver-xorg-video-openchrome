/*
 * Copyright 2016 Kevin Brace
 * Copyright 2015-2016 The OpenChrome Project
 *                     [http://www.freedesktop.org/wiki/Openchrome]
 * Copyright 2014 SHS SERVICES GmbH
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

/*
 * via_tmds.c
 *
 * Handles initialization of TMDS (DVI) related resources and 
 * controls the integrated TMDS transmitter found in CX700 and 
 * later VIA Technologies chipsets.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include "via_driver.h"
#include "via_vt1632.h"


/*
 * Initializes most registers related to VIA Technologies IGP
 * integrated TMDS transmitter. Synchronization polarity and
 * display output source need to be set separately. */
static void
viaTMDSInitRegisters(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSInitRegisters.\n"));

    /* Activate DVI + LVDS2 mode. */
    /* 3X5.D2[5:4] - Display Channel Select
     *               00: LVDS1 + LVDS2
     *               01: DVI + LVDS2
     *               10: One Dual LVDS Channel (High Resolution Pannel)
     *               11: Single Channel DVI */
    ViaCrtcMask(hwp, 0xD2, 0x10, 0x30);

    /* Various DVI PLL settings should be set to default settings. */
    /* 3X5.D1[7]   - PLL2 Reference Clock Edge Select Bit
     *               0: PLLCK lock to rising edge of reference clock
     *               1: PLLCK lock to falling edge of reference clock
     * 3X5.D1[6:5] - PLL2 Charge Pump Current Set Bits
     *               00: ICH = 12.5 uA
     *               01: ICH = 25.0 uA
     *               10: ICH = 37.5 uA
     *               11: ICH = 50.0 uA
     * 3X5.D1[4:1] - Reserved
     * 3X5.D1[0]   - PLL2 Control Voltage Measurement Enable Bit */
    ViaCrtcMask(hwp, 0xD1, 0x00, 0xE1);

    /* Disable DVI test mode. */
    /* 3X5.D5[7] - PD1 Enable Selection
     *             1: Select by power flag
     *             0: By register
     * 3X5.D5[5] - DVI Testing Mode Enable
     * 3X5.D5[4] - DVI Testing Format Selection
     *             0: Half cycle
     *             1: LFSR mode */
    ViaCrtcMask(hwp, 0xD5, 0x00, 0xB0);

    /* Disable DVI sense interrupt. */
    /* 3C5.2B[7] - DVI Sense Interrupt Enable
     *             0: Disable
     *             1: Enable */
    ViaSeqMask(hwp, 0x2B, 0x00, 0x80);

    /* Clear DVI sense interrupt status. */
    /* 3C5.2B[6] - DVI Sense Interrupt Status
     *             (This bit has a RW1C attribute.) */
    ViaSeqMask(hwp, 0x2B, 0x40, 0x40);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSInitRegisters.\n"));
}

/*
 * Sets the polarity of horizontal synchronization and vertical
 * synchronization.
 */
static void
viaTMDSSetSyncPolarity(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 cr97;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSSetSyncPolarity.\n"));

    /* 3X5.97[6] - DVI (TMDS) VSYNC Polarity
     *             0: Positive
     *             1: Negative
     * 3X5.97[5] - DVI (TMDS) HSYNC Polarity
     *             0: Positive
     *             1: Negative */
    cr97 = hwp->readCrtc(hwp, 0x97);
    if (mode->Flags & V_NHSYNC) {
        cr97 |= 0x20;
    } else {
        cr97 &= (~0x20);
    }

    if (mode->Flags & V_NVSYNC) {
        cr97 |= 0x40;
    } else {
        cr97 &= (~0x40);
    }

    ViaCrtcMask(hwp, 0x97, cr97, 0x60);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSSetSyncPolarity.\n"));
}

/*
 * Sets IGA1 or IGA2 as the display output source for VIA Technologies IGP
 * integrated TMDS transmitter.
 */
static void
viaTMDSSetSource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 temp = displaySource;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSSetSource.\n"));

    /* Set integrated TMDS transmitter display output source.
     * The integrated TMDS transmitter appears to utilize LVDS1's data
     * source selection bit (3X5.99[4]). */
    /* 3X5.99[4] - LVDS Channel1 Data Source Selection
     *             0: Primary Display
     *             1: Secondary Display */
    ViaCrtcMask(hwp, 0x99, temp << 4, 0x10);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Integrated TMDS Transmitter Display Output Source: IGA%d\n",
                (temp & 0x01) + 1);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSSetSource.\n"));
}

/*
 * Returns TMDS receiver detection state for VIA Technologies IGP
 * integrated TMDS transmitter.
 */
static Bool
viaTMDSSense(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 sr1a;
    Bool tmdsReceiverDetected = FALSE;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSSense.\n"));

    /* Detect the presence of DVI. */
    /* 3C5.1A[4] - DVI Sense
     *             0: No connect
     *             1: Connected */
    sr1a = hwp->readSeq(hwp, 0x1A);
    if (sr1a & 0x20) {
        tmdsReceiverDetected = TRUE;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Integrated TMDS transmitter %s a TMDS receiver.\n",
                tmdsReceiverDetected ? "detected" : "did not detect");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSSense.\n"));
    return tmdsReceiverDetected;
}

void
viaTMDSPower(ScrnInfoPtr pScrn, Bool On)
{

    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSPower.\n"));

    if (On) {
        /* Power on TMDS */
        ViaCrtcMask(hwp, 0xD2, 0x00, 0x08);
    } else {
        /* Power off TMDS */
        ViaCrtcMask(hwp, 0xD2, 0x08, 0x08);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Integrated TMDS (DVI) Power: %s\n",
                On ? "On" : "Off");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSPower.\n"));
}

static void
via_dvi_create_resources(xf86OutputPtr output)
{
}

#ifdef RANDR_12_INTERFACE
static Bool
via_dvi_set_property(xf86OutputPtr output, Atom property,
                     RRPropertyValuePtr value)
{
    return TRUE;
}

static Bool
via_dvi_get_property(xf86OutputPtr output, Atom property)
{
    return FALSE;
}
#endif

static void
via_dvi_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    switch (mode) {
    case DPMSModeOn:
        via_vt1632_power(output, TRUE);
        break;
    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        via_vt1632_power(output, FALSE);
        break;
    default:
        break;
    }
}

static void
via_dvi_save(xf86OutputPtr output)
{
    via_vt1632_save(output);
}

static void
via_dvi_restore(xf86OutputPtr output)
{
    via_vt1632_restore(output);
}

static int
via_dvi_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    return via_vt1632_mode_valid(output, pMode);
}

static Bool
via_dvi_mode_fixup(xf86OutputPtr output, DisplayModePtr mode,
                   DisplayModePtr adjusted_mode)
{
    return TRUE;
}

static void
via_dvi_prepare(xf86OutputPtr output)
{
}

static void
via_dvi_commit(xf86OutputPtr output)
{
}

static void
via_dvi_mode_set(xf86OutputPtr output, DisplayModePtr mode,
                 DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    via_vt1632_mode_set(output, mode, adjusted_mode);
}

static xf86OutputStatus
via_dvi_detect(xf86OutputPtr output)
{
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    ViaVT1632Ptr Private = output->driver_private;
    xf86MonPtr mon;

    /* Check for the DVI presence via VT1632A first before accessing
     * I2C bus. */
    status = via_vt1632_detect(output);
    if (status == XF86OutputStatusConnected) {

        /* Since DVI presence was established, access the I2C bus
         * assigned to DVI. */
        mon = xf86OutputGetEDID(output, Private->VT1632I2CDev->pI2CBus);

        /* Is the interface type digital? */
        if (mon && DIGITAL(mon->features.input_type)) {
            xf86OutputSetEDID(output, mon);
        } else {
            status = XF86OutputStatusDisconnected;
        }
    }

    return status;
}

static void
via_dvi_destroy(xf86OutputPtr output)
{
}

const xf86OutputFuncsRec via_dvi_funcs = {
    .create_resources   = via_dvi_create_resources,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_dvi_set_property,
#endif
#ifdef RANDR_13_INTERFACE
    .get_property       = via_dvi_get_property,
#endif
    .dpms               = via_dvi_dpms,
    .save               = via_dvi_save,
    .restore            = via_dvi_restore,
    .mode_valid         = via_dvi_mode_valid,
    .mode_fixup         = via_dvi_mode_fixup,
    .prepare            = via_dvi_prepare,
    .commit             = via_dvi_commit,
    .mode_set           = via_dvi_mode_set,
    .detect             = via_dvi_detect,
    .get_modes          = xf86OutputGetEDIDModes,
    .destroy            = via_dvi_destroy,
};


static void
via_tmds_create_resources(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_create_resources.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_create_resources.\n"));
}

static void
via_tmds_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_dpms.\n"));

    switch (mode) {
    case DPMSModeOn:
        viaTMDSPower(pScrn, TRUE);
        break;
    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        viaTMDSPower(pScrn, FALSE);
        break;
    default:
        break;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_dpms.\n"));
}

static void
via_tmds_save(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_save.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_save.\n"));
}

static void
via_tmds_restore(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_restore.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_restore.\n"));
}

static int
via_tmds_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    ScrnInfoPtr pScrn = output->scrn;
    int status;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_mode_valid.\n"));

    if (!ViaModeDotClockTranslate(pScrn, pMode)) {
        status = MODE_NOCLOCK;
    } else {
        status = MODE_OK;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_mode_valid.\n"));
    return status;
}

static Bool
via_tmds_mode_fixup(xf86OutputPtr output, DisplayModePtr mode,
                      DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_mode_fixup.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_mode_fixup.\n"));
    return TRUE;
}

static void
via_tmds_prepare(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_prepare.\n"));

    viaTMDSPower(pScrn, FALSE);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_prepare.\n"));
}

static void
via_tmds_commit(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_commit.\n"));

    viaTMDSPower(pScrn, TRUE);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_commit.\n"));
}

static void
via_tmds_mode_set(xf86OutputPtr output, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    drmmode_crtc_private_ptr iga = output->crtc->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_mode_set.\n"));

    /* Initialize VIA IGP integrated TMDS transmitter registers. */
    viaTMDSInitRegisters(pScrn);

    /* Set integrated TMDS transmitter synchronization polarity for
     * both horizontal synchronization and vertical synchronization. */
    viaTMDSSetSyncPolarity(pScrn, adjusted_mode);

    if (output->crtc) {
        viaTMDSSetSource(pScrn, iga->index ? 0x01 : 0x00);
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_mode_set.\n"));
}

static xf86OutputStatus
via_tmds_detect(xf86OutputPtr output)
{
    xf86MonPtr mon;
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    VIATMDSRecPtr pVIATMDSRec = output->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_detect.\n"));

    /* Check for DVI presence by sensing the TMDS receiver connected
     * to the integrated TMDS transmitter. */
    if (viaTMDSSense(pScrn)) {

        if (!pVia->pI2CBus2) {
            goto exit;
        }

        /* Assume that only I2C bus 2 is used for the DVI connected to the
         * integrated TMDS transmitter. */
        if (!xf86I2CProbeAddress(pVia->pI2CBus2, 0xA0)) {
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "I2C device on I2C Bus 2 does not support EDID.\n");
            goto exit;
        }

        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Obtaining EDID for DVI.\n");

        /* Since DVI presence was established, access the I2C bus,
         * in order to obtain EDID from the monitor. */
        mon = xf86OutputGetEDID(output, pVia->pI2CBus2);

        /* Is the interface type digital? */
        if (mon && DIGITAL(mon->features.input_type)) {
            status = XF86OutputStatusConnected;
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "Detected a monitor connected to DVI.\n");
            xf86OutputSetEDID(output, mon);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "Could not obtain EDID from a monitor "
                        "connected to DVI.\n");
        }
    }

exit:
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_detect.\n"));
    return status;
}

#ifdef RANDR_12_INTERFACE
static Bool
via_tmds_set_property(xf86OutputPtr output, Atom property,
                     RRPropertyValuePtr value)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_set_property.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_set_property.\n"));
    return TRUE;
}
#endif

#ifdef RANDR_13_INTERFACE
static Bool
via_tmds_get_property(xf86OutputPtr output, Atom property)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_get_property.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_get_property.\n"));
    return FALSE;
}
#endif

static void
via_tmds_destroy(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_destroy.\n"));

    if (output->driver_private) {
        free(output->driver_private);
    }

    output->driver_private = NULL;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_destroy.\n"));
}




static const xf86OutputFuncsRec via_tmds_funcs = {
    .create_resources   = via_tmds_create_resources,
    .dpms               = via_tmds_dpms,
    .save               = via_tmds_save,
    .restore            = via_tmds_restore,
    .mode_valid         = via_tmds_mode_valid,
    .mode_fixup         = via_tmds_mode_fixup,
    .prepare            = via_tmds_prepare,
    .commit             = via_tmds_commit,
    .mode_set           = via_tmds_mode_set,
    .detect             = via_tmds_detect,
    .get_modes          = xf86OutputGetEDIDModes,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_tmds_set_property,
#endif
#ifdef RANDR_13_INTERFACE
    .get_property       = via_tmds_get_property,
#endif
    .destroy            = via_tmds_destroy,
};


Bool
viaTMDSInit(ScrnInfoPtr pScrn)
{
    xf86OutputPtr output;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIATMDSRecPtr pVIATMDSRec = NULL;
    CARD8 sr13, sr5a, cr3e;
    Bool status = FALSE;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSInit.\n"));

    sr5a = hwp->readSeq(hwp, 0x5A);
    ViaSeqMask(hwp, 0x5A, sr5a | 0x01, 0x01);

    /* 3C5.13[7:6] - Integrated LVDS / DVI Mode Select
     *               (DVP1D15-14 pin strapping)
     *               00: LVDS1 + LVDS2
     *               01: DVI + LVDS2
     *               10: Dual LVDS Channel (High Resolution Panel)
     *               11: One DVI only (decrease the clock jitter) */
    sr13 = hwp->readSeq(hwp, 0x13);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR13: 0x%02X\n", sr13));
    hwp->writeSeq(hwp, 0x5A, sr5a);
    sr13 &= 0xC0;
    if ((sr13 == 0x40) || (sr13 == 0xC0)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Integrated TMDS transmitter found via pin strapping.\n");
    } else {
        /* 3X5.3E[5] supposedly signals the presence of
         * a DVI connector coming from an integrated TMDS transmitter.
         * This check is done if the pin strapping does not indicate
         * the presence of a DVI connector. Note that 3X5.3E is set by
         * VIA Technologies VGA BIOS. */
        cr3e = hwp->readCrtc(hwp, 0x3E);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "CR3E: 0x%02X\n", cr3e));
        if (cr3e & 0x20) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Integrated TMDS transmitter found via "
                        "VIA Technologies VGA BIOS scratch pad register.\n");
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Integrated TMDS transmitter not found.\n");
            goto exit;
        }
    }

    pVIATMDSRec = xnfcalloc(1, sizeof(VIATMDSRec));
    if (!pVIATMDSRec) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to allocate working storage for integrated "
                    "TMDS transmitter.\n");
        goto exit;
    }

    output = xf86OutputCreate(pScrn, &via_tmds_funcs, "DVI-1");
    if (!output) {
        free(pVIATMDSRec);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to allocate X Server display output record for "
                    "integrated TMDS transmitter.\n");
        goto exit;
    }

    output->driver_private = pVIATMDSRec;

    /* Since there are two (2) display controllers registered with the
     * X.Org Server and both IGA1 and IGA2 can handle DVI without any
     * limitations, possible_crtcs should be set to 0x3 (0b11) so that
     * either display controller can get assigned to handle DVI. */
    output->possible_crtcs = (1 << 1) | (1 << 0);

    output->possible_clones = 0;
    output->interlaceAllowed = FALSE;
    output->doubleScanAllowed = FALSE;

    status = TRUE;
exit:
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSInit.\n"));
    return status;
}

void
via_dvi_init(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    xf86OutputPtr output = NULL;
    ViaVT1632Ptr private_data = NULL;
    I2CBusPtr pBus = NULL;
    I2CDevPtr pDev = NULL;
    I2CSlaveAddr addr = 0x10;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_dvi_init.\n"));

    if (!pVia->pI2CBus2 || !pVia->pI2CBus3) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "I2C Bus 2 or I2C Bus 3 does not exist.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Exiting via_dvi_init.\n"));
        return;
    }

    /* Check to see if we are dealing with the latest VIA chipsets. */
    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        if (!viaTMDSInit(pScrn)) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Integrated TMDS transmitter for DVI not found.\n");
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Integrated TMDS transmitter for DVI was "
                        "initialized successfully.\n");
        }
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Probing I2C Bus 2 for VT1632.\n");
    if (!viaVT1632Init(pScrn, pVia->pI2CBus2)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "I2C Bus 2 was not initialized for DVI use.\n");
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "VT1632 attached to I2C Bus 2 was initialized "
                    "successfully for DVI use.\n");
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Probing I2C Bus 3 for VT1632.\n");
    if (!viaVT1632Init(pScrn, pVia->pI2CBus3)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "I2C Bus 3 was not initialized for DVI use.\n");
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "VT1632 attached to I2C Bus 3 was initialized "
                    "successfully for DVI use.\n");
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_dvi_init.\n"));
}