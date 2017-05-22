/*
 * Copyright 2016 Kevin Brace
 * Copyright 2005-2016 The OpenChrome Project
 *                     [https://www.freedesktop.org/wiki/Openchrome]
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

#ifndef _VIA_UMS_H_
#define _VIA_UMS_H_ 1

#include "via_vgahw.h"

#define     VIA_PANEL6X4                    0
#define     VIA_PANEL8X6                    1
#define     VIA_PANEL10X7                   2
#define     VIA_PANEL12X7                   3
#define     VIA_PANEL12X10                  4
#define     VIA_PANEL14X10                  5
#define     VIA_PANEL16X12                  6
#define     VIA_PANEL12X8                   7
#define     VIA_PANEL8X4                    8
#define     VIA_PANEL1366X7                 9
#define     VIA_PANEL1360X7                 10
#define     VIA_PANEL1920x1080              11
#define     VIA_PANEL1920x1200              12
#define     VIA_PANEL10X6                   13
#define     VIA_PANEL14X9                   14
#define     VIA_PANEL1280X720               15
#define     VIA_PANEL12X9                   16
#define     VIA_PANEL_INVALID               255

#define     TVTYPE_NONE                     0x00
#define     TVTYPE_NTSC                     0x01
#define     TVTYPE_PAL                      0x02
#define     TVTYPE_480P                     0X03
#define     TVTYPE_576P                     0X04
#define     TVTYPE_720P                     0X05
#define     TVTYPE_1080I                    0X06

#define     TVOUTPUT_NONE                   0x00
#define     TVOUTPUT_COMPOSITE              0x01
#define     TVOUTPUT_SVIDEO                 0x02
#define     TVOUTPUT_RGB                    0x04
#define     TVOUTPUT_YCBCR                  0x08
#define     TVOUTPUT_SC                     0x16

#define  VIA_NONETV   0
#define  VIA_VT1621   1 /* TV2PLUS */
#define  VIA_VT1622   2 /* TV3 */
#define  VIA_VT1623   3 /* also VT1622A */
#define  VIA_VT1625   4
#define  VIA_CH7011   5
#define  VIA_CH7019A  6
#define  VIA_CH7019B  7
#define  VIA_CH7017   8
#define  VIA_CH7304   9
#define  VIA_CH7305   10


#define     VIA_TVNORMAL                    0
#define     VIA_TVOVER                      1

#define     VIA_DEVICE_NONE                 0x00
#define	    VIA_DEVICE_CRT		    0x01
#define	    VIA_DEVICE_LCD		    0x02
#define	    VIA_DEVICE_TV		    0x04
#define	    VIA_DEVICE_DFP		    0x08

#define     VIA_I2C_NONE                    0x00
#define     VIA_I2C_BUS1                    0x01
#define     VIA_I2C_BUS2                    0x02
#define     VIA_I2C_BUS3                    0x04

/* System Memory CLK */
#define VIA_MEM_SDR66   0x00
#define VIA_MEM_SDR100  0x01
#define VIA_MEM_SDR133  0x02
#define VIA_MEM_DDR200  0x03
#define VIA_MEM_DDR266  0x04
#define VIA_MEM_DDR333  0x05
#define VIA_MEM_DDR400  0x06
#define VIA_MEM_DDR533  0x07
#define VIA_MEM_DDR667  0x08
#define VIA_MEM_DDR800  0x09
#define VIA_MEM_DDR1066 0x0A
#define VIA_MEM_END     0x0B
#define VIA_MEM_NONE    0xFF

#define VIA_BW_MIN       74000000 /* > 640x480@60Hz@32bpp */
#define VIA_BW_DDR200   394000000
#define VIA_BW_DDR400   553000000 /* > 1920x1200@60Hz@32bpp */
#define VIA_BW_DDR667   922000000
#define VIA_BW_DDR1066  922000000

/* Digital Output Bus Width */
#define	    VIA_DI_12BIT		    0x00
#define	    VIA_DI_24BIT		    0x01

/* Digital Port */
#define     VIA_DI_PORT_NONE        0x0
#define     VIA_DI_PORT_DIP0        0x1
#define     VIA_DI_PORT_DVP0        0x1
#define     VIA_DI_PORT_DIP1        0x2
#define     VIA_DI_PORT_DVP1        0x2
#define     VIA_DI_PORT_FPDPLOW     0x4
#define     VIA_DI_PORT_DVP2        0x4
#define     VIA_DI_PORT_LVDS1       0x4
#define     VIA_DI_PORT_TMDS        0x4
#define     VIA_DI_PORT_FPDPHIGH    0x8
#define     VIA_DI_PORT_LVDS2       0x8

/* External TMDS (DVI) Transmitter Type */
#define     VIA_TMDS_NONE           0x0
#define     VIA_TMDS_VT1632         0x1

#define BIT(x) (1 << x)


typedef struct ViaPanelMode {
    int Width;
    int Height;
    Bool useDualEdge;
    Bool useDithering;
} ViaPanelModeRec, *ViaPanelModePtr ;

typedef struct _VIADISPLAY {
    Bool        analogPresence;
    CARD8       analogI2CBus;

    /* Keeping track of the number of analog VGA connectors. */
    unsigned int        numberVGA;

    xf86OutputPtr tv;

    CARD32      Clock; /* register value for the dotclock */
    Bool        ClockExternal;
    CARD32      Bandwidth; /* available memory bandwidth */

    /* TV entries */
    int         TVEncoder;
    int         TVOutput;
    I2CDevPtr   TVI2CDev;
    int         TVType;
    Bool        TVDotCrawl;
    int         TVDeflicker;
    CARD8       TVRegs[0xFF];
    int         TVNumRegs;

    /* TV Callbacks */
    void (*TVSave) (ScrnInfoPtr pScrn);
    void (*TVRestore) (ScrnInfoPtr pScrn);
    Bool (*TVDACSense) (ScrnInfoPtr pScrn);
    ModeStatus (*TVModeValid) (ScrnInfoPtr pScrn, DisplayModePtr mode);
    void (*TVModeI2C) (ScrnInfoPtr pScrn, DisplayModePtr mode);
    void (*TVModeCrtc) (xf86CrtcPtr crtc, DisplayModePtr mode);
    void (*TVPower) (ScrnInfoPtr pScrn, Bool On);
    void (*LCDPower) (ScrnInfoPtr pScrn, Bool On);
    DisplayModePtr TVModes;
    int            TVNumModes;
    void (*TVPrintRegs) (ScrnInfoPtr pScrn);

} VIADisplayRec, *VIADisplayPtr;

typedef struct _VIAANALOG {
    CARD8       analogI2CBus;
} VIAAnalogRec, *VIAAnalogPtr;

/*
 * Record for storing FP (Flat Panel) specific information.
 */
typedef struct _VIAFP {
    Bool IsActive ;
    /* Native physical resolution */
    int NativeHeight;
    int NativeWidth;
    Bool useDualEdge;
    Bool useDithering;

    /* Native resolution index, see via_panel.c */
    CARD8 NativeModeIndex;
    /* Determine if we must use the hardware scaler
     * It might be false even if the "Center" option
     * was specified
     */
    Bool            Scale;

    /* Panel/LCD entries */
    CARD16      ResolutionIndex;
    int         PanelIndex;
    Bool        Center;
    Bool        SetDVI;
    /* LCD Simultaneous Expand Mode HWCursor Y Scale */
    Bool        scaleY;
    int         resY;

    I2CBusPtr pVIAFPI2CBus;
} VIAFPRec, *VIAFPPtr;

typedef struct _VIATMDS {
    I2CBusPtr pVIATMDSI2CBus;
    CARD8       diPortType;
} VIATMDSRec, *VIATMDSPtr;

typedef struct
{
    CARD16 X;
    CARD16 Y;
    CARD16 Bpp;
    CARD8 bRamClock;
    CARD8 bTuningValue;
} ViaExpireNumberTable;

union pllparams {
    struct {
        CARD32 dtz : 2;
        CARD32 dr  : 3;
        CARD32 dn  : 7;
        CARD32 dm  :10;
    } params;
    CARD32 packed;
};


/*
 * Sets analog (VGA) DAC output state.
 */
static inline void
viaAnalogSetDACOutput(ScrnInfoPtr pScrn, Bool outputState)
{
    /* Set analog (VGA) DAC output state. */
    /* 3X5.47[2] - DACOFF Backdoor Register
     *             0: DAC on
     *             1: DAC off */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x47, outputState ? 0x00 : BIT(2),
                BIT(2));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Analog (VGA) DAC Output: %s\n",
                        outputState ? "On" : "Off"));
}

static inline void
viaAnalogSetSyncPolarity(ScrnInfoPtr pScrn, CARD8 syncPolarity)
{
    /* Set analog (VGA) sync polarity. */
    /* 3C2[7] - Analog Vertical Sync Polarity
     *          0: Positive
     *          1: Negative
     * 3C2[6] - Analog Horizontal Sync Polarity
     *          0: Positive
     *          1: Negative */
    VGAHWPTR(pScrn)->writeMiscOut(VGAHWPTR(pScrn),
            ((VGAHWPTR(pScrn)->readMiscOut(VGAHWPTR(pScrn)))
                & ~((BIT(1) | BIT(0)) << 6))
            | ((syncPolarity & (BIT(1) | BIT(0))) << 6));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Analog (VGA) Horizontal Sync Polarity: %s\n",
                        (syncPolarity & BIT(0)) ? "-" : "+"));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Analog (VGA) Vertical Sync Polarity: %s\n",
                        (syncPolarity & BIT(1)) ? "-" : "+"));
}

/*
 * Sets analog (VGA) display source.
 */
static inline void
viaAnalogSetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    /* Set analog (VGA) display source. */
    /* 3C5.16[6] - CRT Display Source
     *             0: Primary Display Stream (IGA1)
     *             1: Secondary Display Stream (IGA2) */
    ViaSeqMask(VGAHWPTR(pScrn), 0x16, displaySource << 6, BIT(6));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Analog (VGA) Display Source: IGA%d\n",
                        (displaySource & 0x01) + 1));
}

/*
 * Sets KM400 or later chipset's FP primary power sequence control
 * type.
 */
static inline void
viaFPSetPrimaryPowerSeqType(ScrnInfoPtr pScrn, Bool ctrlType)
{
    /* Set FP primary power sequence control type. */
    /* 3X5.91[0] - FP Primary Power Sequence Control Type
     *             0: Hardware Control
     *             1: Software Control */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x91, ctrlType ? 0x00 : BIT(0),
                BIT(0));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "FP Primary Power Sequence Control Type: "
                        "%s Control\n",
                        ctrlType ? "Hardware" : "Software"));
}

/*
 * Sets KM400 or later chipset's FP primary direct back light control
 * bit.
 */
static inline void
viaFPSetPrimaryDirectBackLightCtrl(ScrnInfoPtr pScrn, Bool directOn)
{
    /* Set FP primary direct back light control bit. */
    /* 3X5.91[6] - FP Primary Direct Back Light Control
     *             0: On
     *             1: Off */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x91, directOn ? 0x00 : BIT(6),
                BIT(6));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "FP Primary Direct Back Light Control: %s\n",
                        directOn ? "On" : "Off"));
}

/*
 * Sets KM400 or later chipset's FP primary direct display period
 * control bit.
 */
static inline void
viaFPSetPrimaryDirectDisplayPeriodCtrl(ScrnInfoPtr pScrn, Bool directOn)
{
    /* Set FP primary direct display period bit. */
    /* 3X5.91[7] - FP Primary Direct Display Period Control
     *             0: On
     *             1: Off */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x91, directOn ? 0x00 : BIT(7),
                BIT(7));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "FP Primary Direct Display Period Control: %s\n",
                        directOn ? "On" : "Off"));
}

/*
 * Sets KM400 or later chipset's FP primary hardware controlled
 * power sequence bit.
 */
static inline void
viaFPSetPrimaryHardPower(ScrnInfoPtr pScrn, Bool powerState)
{
    /* Set FP primary hardware controlled power sequence bit. */
    /* 3X5.6A[3] - FP Primary Hardware Controlled Power Sequence
     *             0: Hardware Controlled Power Off
     *             1: Hardware Controlled Power On */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x6A, powerState ? BIT(3) : 0x00,
                BIT(3));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "FP Primary Hardware Controlled Power "
                        "Sequence: %s\n",
                        powerState ? "On" : "Off"));
}

/*
 * Sets CX700 or later chipset's LVDS1 power state.
 */
static inline void
viaLVDS1SetPower(ScrnInfoPtr pScrn, Bool powerState)
{
    /* Set LVDS1 power state. */
    /* 3X5.D2[7] - Power Down (Active High) for Channel 1 LVDS
     *             0: LVDS1 power on
     *             1: LVDS1 power down */
    ViaCrtcMask(VGAHWPTR(pScrn), 0xD2, powerState ? 0x00 : BIT(7),
                BIT(7));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "LVDS1 Power State: %s\n",
                        powerState ? "On" : "Off"));
}

/*
 * Sets CX700 or later single chipset's LVDS1 power sequence type.
 */
static inline void
viaLVDS1SetPowerSeq(ScrnInfoPtr pScrn, Bool ctrlType)
{
    /* Set LVDS1 power sequence type. */
    /* 3X5.91[0] - LVDS1 Hardware or Software Control Power Sequence
     *             0: Hardware Control
     *             1: Software Control */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x91, ctrlType ? BIT(0) : 0x00,
                BIT(0));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "LVDS1 Power Sequence: %s Control\n",
                        ctrlType ? "Software" : "Hardware"));
}

/*
 * Sets CX700 or later single chipset's LVDS1 software controlled
 * data path state.
 */
static inline void
viaLVDS1SetSoftData(ScrnInfoPtr pScrn, Bool softOn)
{
    /* Set LVDS1 software controlled data path state. */
    /* 3X5.91[3] - Software Data On
     *             0: Off
     *             1: On */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x91, softOn ? BIT(3) : 0,
                BIT(3));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "LVDS1 Software Controlled Data Path: %s\n",
                        softOn ? "On" : "Off"));
}

/*
 * Sets CX700 or later single chipset's LVDS1 software controlled Vdd.
 */
static inline void
viaLVDS1SetSoftVdd(ScrnInfoPtr pScrn, Bool softOn)
{
    /* Set LVDS1 software controlled Vdd. */
    /* 3X5.91[4] - Software VDD On
     *             0: Off
     *             1: On */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x91, softOn ? BIT(4) : 0,
                BIT(4));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "LVDS1 Software Controlled Vdd: %s\n",
                        softOn ? "On" : "Off"));
}

/*
 * Sets CX700 or later single chipset's LVDS1 software controlled
 * display period.
 */
static inline void
viaLVDS1SetSoftDisplayPeriod(ScrnInfoPtr pScrn, Bool softOn)
{
    /* Set LVDS1 software controlled display period state. */
    /* 3X5.91[7] - Software Direct On / Off Display Period
     *             in the Panel Path
     *             0: On
     *             1: Off */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x91, softOn ? 0 : BIT(7),
                BIT(7));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "LVDS1 Software Controlled Display Period: %s\n",
                        softOn ? "On" : "Off"));
}

/*
 * Sets CX700 / VX700 and VX800 chipsets' LVDS2 power state.
 */
static inline void
viaLVDS2SetPower(ScrnInfoPtr pScrn, Bool powerState)
{
    /* Set LVDS2 power state. */
    /* 3X5.D2[6] - Power Down (Active High) for Channel 2 LVDS
     *             0: LVDS2 power on
     *             1: LVDS2 power down */
    ViaCrtcMask(VGAHWPTR(pScrn), 0xD2, powerState ? 0x00 : BIT(6),
                BIT(6));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "LVDS2 Power State: %s\n",
                        powerState ? "On" : "Off"));
}

/*
 * Sets CX700 / VX700 and VX800 chipsets' TMDS (DVI) power state.
 */
static inline void
viaTMDSSetPower(ScrnInfoPtr pScrn, Bool powerState)
{
    /* Set TMDS (DVI) power state. */
    /* 3X5.D2[3] - Power Down (Active High) for DVI
     *             0: TMDS power on
     *             1: TMDS power down */
    ViaCrtcMask(VGAHWPTR(pScrn), 0xD2, powerState ? 0 : BIT(3),
                BIT(3));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "TMDS (DVI) Power State: %s\n",
                        powerState ? "On" : "Off"));
}

/*
 * Sets CX700 / VX700 and VX800 chipsets' TMDS (DVI) sync polarity.
 */
static inline void
viaTMDSSetSyncPolarity(ScrnInfoPtr pScrn, CARD8 syncPolarity)
{
    /* Set TMDS (DVI) sync polarity. */
    /* 3X5.97[6] - DVI (TMDS) VSYNC Polarity
     *             0: Positive
     *             1: Negative
     * 3X5.97[5] - DVI (TMDS) HSYNC Polarity
     *             0: Positive
     *             1: Negative */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x97, syncPolarity << 5, BIT(6) | BIT(5));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "TMDS (DVI) Horizontal Sync Polarity: %s\n",
                        (syncPolarity & BIT(0)) ? "-" : "+"));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "TMDS (DVI) Vertical Sync Polarity: %s\n",
                        (syncPolarity & BIT(1)) ? "-" : "+"));
}

/*
 * Sets TMDS (DVI) display source.
 */
static inline void
viaTMDSSetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    /* Set integrated TMDS transmitter display output source.
     * The integrated TMDS transmitter appears to utilize LVDS1's data
     * source selection bit (3X5.99[4]). */
    /* 3X5.99[4] - LVDS Channel1 Data Source Selection
     *             0: Primary Display
     *             1: Secondary Display */
    ViaCrtcMask(VGAHWPTR(pScrn), 0x99, displaySource << 4, BIT(4));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "TMDS (DVI) Display Source: IGA%d\n",
                        (displaySource & 0x01) + 1));
}


/* via_ums.c */
void viaUnmapMMIO(ScrnInfoPtr pScrn);
void viaDisableVQ(ScrnInfoPtr pScrn);
Bool umsAccelInit(ScreenPtr pScreen);
Bool umsCreate(ScrnInfoPtr pScrn);
Bool umsPreInit(ScrnInfoPtr pScrn);
Bool umsCrtcInit(ScrnInfoPtr pScrn);

/* via_output.c */
void viaDIP0SetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource);
void viaDIP0EnableIOPads(ScrnInfoPtr pScrn, CARD8 ioPadState);
void viaDIP0SetClockDriveStrength(ScrnInfoPtr pScrn,
                                    CARD8 clockDriveStrength);
void viaDIP0SetDataDriveStrength(ScrnInfoPtr pScrn,
                                    CARD8 dataDriveStrength);
void viaDVP0SetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource);
void viaDVP0EnableIOPads(ScrnInfoPtr pScrn, CARD8 ioPadState);
void viaDVP0SetClockDriveStrength(ScrnInfoPtr pScrn,
                                    CARD8 clockDriveStrength);
void viaDVP0SetDataDriveStrength(ScrnInfoPtr pScrn,
                                    CARD8 dataDriveStrength);
void viaDVP1SetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource);
void viaDVP1EnableIOPads(ScrnInfoPtr pScrn, CARD8 ioPadState);
void viaDVP1SetClockDriveStrength(ScrnInfoPtr pScrn,
                                    CARD8 clockDriveStrength);
void viaDVP1SetDataDriveStrength(ScrnInfoPtr pScrn,
                                    CARD8 dataDriveStrength);
void viaDFPLowSetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource);
void viaDFPLowEnableIOPads(ScrnInfoPtr pScrn, CARD8 ioPadState);
void viaOutputDetect(ScrnInfoPtr pScrn);
CARD32 ViaGetMemoryBandwidth(ScrnInfoPtr pScrn);
CARD32 ViaModeDotClockTranslate(ScrnInfoPtr pScrn, DisplayModePtr mode);
void viaProbePinStrapping(ScrnInfoPtr pScrn);
void ViaSetPrimaryDotclock(ScrnInfoPtr pScrn, CARD32 clock);
void ViaSetSecondaryDotclock(ScrnInfoPtr pScrn, CARD32 clock);
void ViaSetUseExternalClock(vgaHWPtr hwp);

/* via_display.c */
void viaIGA2DisplayChannel(ScrnInfoPtr pScrn, Bool channelState);
void viaDisplayInit(ScrnInfoPtr pScrn);
void ViaGammaDisable(ScrnInfoPtr pScrn);
void viaIGAInitCommon(ScrnInfoPtr pScrn);
void viaIGA1Init(ScrnInfoPtr pScrn);
void viaIGA1SetFBStartingAddress(xf86CrtcPtr crtc, int x, int y);
void viaIGA1SetDisplayRegister(ScrnInfoPtr pScrn, DisplayModePtr mode);
void viaIGA1Save(ScrnInfoPtr pScrn);
void viaIGA1Restore(ScrnInfoPtr pScrn);
void viaIGA2Init(ScrnInfoPtr pScrn);
void viaIGA2SetFBStartingAddress(xf86CrtcPtr crtc, int x, int y);
void viaIGA2SetDisplayRegister(ScrnInfoPtr pScrn, DisplayModePtr mode);
void viaIGA2Save(ScrnInfoPtr pScrn);
void viaIGA2Restore(ScrnInfoPtr pScrn);
void ViaShadowCRTCSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode);

/* via_analog.c */
void via_analog_init(ScrnInfoPtr pScrn);

/* via_lvds.c */
void viaLVDS1SetIOPadSetting(ScrnInfoPtr pScrn, CARD8 ioPadState);
void via_lvds_init(ScrnInfoPtr pScrn);

/* via_tmds.c */
void viaExtTMDSSetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource);
void viaExtTMDSEnableIOPads(ScrnInfoPtr pScrn, CARD8 ioPadState);
void viaExtTMDSSetClockDriveStrength(ScrnInfoPtr pScrn,
                                        CARD8 clockDriveStrength);
void viaExtTMDSSetDataDriveStrength(ScrnInfoPtr pScrn,
                                        CARD8 dataDriveStrength);
void via_dvi_init(ScrnInfoPtr pScrn);

/*via_tv.c */
#ifdef HAVE_DEBUG
void ViaTVPrintRegs(ScrnInfoPtr pScrn);
#endif
Bool via_tv_init(ScrnInfoPtr pScrn);

/* via_vt162x.c */
I2CDevPtr ViaVT162xDetect(ScrnInfoPtr pScrn, I2CBusPtr pBus, CARD8 Address);
void ViaVT162xInit(ScrnInfoPtr pScrn);

/* via_ch7xxx.c */
I2CDevPtr ViaCH7xxxDetect(ScrnInfoPtr pScrn, I2CBusPtr pBus, CARD8 Address);
void ViaCH7xxxInit(ScrnInfoPtr pScrn);

#endif /* _VIA_UMS_H_ */
