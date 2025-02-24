
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                            Video modes - Time list
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico/Pico2 and RP2040/RP2350
// Copyright (c) 2023-2025 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#if USE_DISPHSTX		// 1=use HSTX Display driver

#ifndef _DISPHSTX_VMODE_TIME_H
#define _DISPHSTX_VMODE_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

// Based on standard VGA videomodes:
//  640x350@70Hz/31.5kHz PN +- .... VGA: pixel clock 25.176 MHz, sync PN, hor 640+16+96+48=800/31.470kHz, ver 350+37+2+60=449/70.089Hz
//  640x480@60Hz/31.5kHz NN -- ... VESA DMT ID 04h: pixel clock 25.175 MHz, sync NN, hor 640+16+96+48=800/31.469kHz, ver 480+10+2+33=525/59.940Hz
//  720x400@70Hz/31.5kHz NP -+ ... VGA: pixel clock 28.320 MHz, sync NP, hor 720+18+108+54=900/31.467kHz, ver 400+12+2+35=449/70.082Hz
//  800x600@56Hz/35.2kHz PP ++ ... VESA DMT ID 08h: pixel clock 36.000 MHz, sync PP, hor 800+24+72+128=1024/35.156kHz, ver 600+1+2+22=625/56.250Hz
//  800x600@60Hz/37.9kHz PP ++ ... VESA DMT ID 09h: pixel clock 40.000 MHz, sync PP, hor 800+40+128+88=1056/37.879kHz, ver 600+1+4+23=628/60.317Hz
//  848x480@60Hz/31.0kHz PP ++ ... VESA DMT ID 0Eh: pixel clock 33.750 MHz, sync PP, hor 848+16+112+112=1088/31.020kHz, ver 480+6+8+23=517/60.000Hz
//  1024x768@60Hz/47.7kHz NN -- ... VESA DMT ID 10h: pixel clock 65.000 MHz, sync NN, hor 1024+24+136+160=1344/48.363kHz, ver 768+3+6+29=806/60.004Hz
//  1280x720@60Hz/45kHz PP ++ ... VESA DMT ID 55h: pixel clock 74.250 MHz, sync PP, hor 1280+110+40+220=1650/45.000kHz, ver 720+5+5+20=750/60.000Hz

// Video modes with system frequency > 400 MHz require USE_VREG_LOCKED switch to be ON.

// default videomode timings
enum {
	vmodetime_532x400,		// 532x400/70.2Hz/31.5kHz (EGA 4:3), pixel clock 20.9 MHz, system clock 104.571 MHz ... detected as 720x400@70Hz on VGA
	vmodetime_532x400_fast,		// 532x400/70.2Hz/31.5kHz (EGA 4:3), pixel clock 20.9 MHz, system clock 209.143 MHz ... detected as 720x400@70Hz on VGA

#if DISPHSTX_WIDTHMAX >= 640	// max. supported width
	vmodetime_640x350,		// 640x350/70.2Hz/31.5kHz (EGA), pixel clock 25.2 MHz, system clock 126 MHz ... detected as 640x350@70Hz on VGA
	vmodetime_640x350_fast,		// 640x350/70.2Hz/31.5kHz (EGA), pixel clock 25.2 MHz, system clock 252 MHz ... detected as 640x350@70Hz on VGA
	vmodetime_640x400,		// 640x400/70.2Hz/31.5kHz (EGA), pixel clock 25.2 MHz, system clock 126 MHz ... detected as 720x400@70Hz on VGA
	vmodetime_640x400_fast,		// 640x400/70.2Hz/31.5kHz (EGA), pixel clock 25.2 MHz, system clock 252 MHz ... detected as 720x400@70Hz on VGA
	vmodetime_640x480,		// 640x480/60Hz/31.5kHz (VGA 4:3, VESA DMT ID 04h), pixel clock 25.2 MHz, system clock 126 MHz ... detected as 640x480@60Hz on VGA
	vmodetime_640x480_fast,		// 640x480/60Hz/31.5kHz (VGA 4:3, VESA DMT ID 04h), pixel clock 25.2 MHz, system clock 252 MHz ... detected as 640x480@60Hz on VGA
	vmodetime_640x720,		// 640x720/60Hz/45kHz (half-HD), pixel clock 37.2 MHz, system clock 186 MHz ... detected as 1280x720@60Hz on VGA
#endif

#if DISPHSTX_WIDTHMAX >= 720	// max. supported width
	vmodetime_720x400,		// 720x400/70.1Hz/31.5kHz (VGA near 16:9), pixel clock 28.32 MHz, system clock 141.6 MHz ... detected as 720x400@70Hz on VGA
	vmodetime_720x400_fast,		// 720x400/70.1Hz/31.5kHz (VGA near 16:9), pixel clock 28.32 MHz, system clock 283.2 MHz ... detected as 720x400@70Hz on VGA
	vmodetime_720x480,		// 720x480/60Hz/31.5kHz (VGA 3:2, SD video NTSC), pixel clock 28.32 MHz, system clock 141.6 MHz ... detected as 640x480@60Hz on VGA
	vmodetime_720x480_fast,		// 720x480/60Hz/31.5kHz (VGA 3:2, SD video NTSC), pixel clock 28.32 MHz, system clock 283.2 MHz ... detected as 640x480@60Hz on VGA
#endif

#if DISPHSTX_WIDTHMAX >= 800	// max. supported width
	vmodetime_800x480,		// 800x480/60Hz/31.5kHz (MAME 5:3), pixel clock 31.5 MHz, system clock 157.333 MHz ... detected as 640x480@60Hz on VGA
	vmodetime_800x600,		// 800x600/60.3Hz/37.9kHz (SVGA 4:3, VESA DMT ID 09h), pixel clock 40 MHz, system clock 200 MHz ... detected as 800x600@60Hz on VGA
#endif

#if DISPHSTX_WIDTHMAX >= 848	// max. supported width
	vmodetime_848x480,		// 848x480/60Hz/31.4kHz (DMT SVGA 16:9), pixel clock 33.33 MHz, system clock 166.666 MHz ... detected as 640x480@60Hz on VGA
#endif

#if DISPHSTX_WIDTHMAX >= 960	// max. supported width
	vmodetime_960x720,		// 960x720/60Hz/45kHz (VESA 4:3), pixel clock 55.7 MHz, system clock 278.4 MHz ... detected as 1280x720@60Hz on VGA
#endif

#if DISPHSTX_WIDTHMAX >= 1024	// max. supported width
	vmodetime_1024x768,		// 1024x768/59.9Hz/48.3kHz (XGA 4:3, VESA DMT ID 10h), pixel clock 64.8 MHz, system clock 324 MHz ... detected as 1024x768@60Hz on VGA
#endif

#if DISPHSTX_WIDTHMAX >= 1056	// max. supported width
	vmodetime_1056x600,		// 1056x600/60Hz/37.9kHz (near 16:9), pixel clock 52.8 MHz, system clock 264 MHz ... detected as 800x600@60Hz on VGA
#endif

#if DISPHSTX_WIDTHMAX >= 1152	// max. supported width
	vmodetime_1152x600,		// 1152x600/60Hz/37.9kHz, pixel clock 57.6 MHz, system clock 288 MHz ... detected as 800x600@60Hz on VGA
#endif

#if DISPHSTX_WIDTHMAX >= 1280	// max. supported width
	vmodetime_1280x400,		// 1280x400/70Hz (16:5), pixel clock 50.4 MHz, system clock 252 MHz... detected as 720x400@70Hz on VGA
	vmodetime_1280x480,		// 1280x480/60Hz (8:3), pixel clock 50.4 MHz, system clock 252 MHz ... detected as 640x480@60Hz on VGA
	vmodetime_1280x600,		// 1280x600/56Hz/35.2kHz, pixel clock 57.6 MHz, system clock 288 MHz ... detected as 800x600@56Hz on VGA
	vmodetime_1280x720,		// 1280x720/60.1Hz/45kHz (16:9, VESA DMT ID 55h), pixel clock 74.4 MHz, system clock 372 MHz ... detected as 1280x720@60Hz on VGA
	vmodetime_1280x768,		// 1280x768/59.6Hz/47.6kHz (VESA DMT ID 17h), pixel clock 79.2 MHz, system clock 396 MHz ... detected as 1280x768@60Hz on VGA
#if USE_VREG_LOCKED		// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
	// This video mode requires DISPHSTX_DISP_SEL switch to be enabled.
	vmodetime_1280x800,		// 1280x800/59.6Hz/49.5kHz (VESA DMT ID 1Ch), pixel clock 83.2 MHz, system clock 416 MHz ... detected as 1280x800@60Hz on VGA
#endif // USE_VREG_LOCKED
#endif

#if DISPHSTX_WIDTHMAX >= 1360	// max. supported width
#if USE_VREG_LOCKED		// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
	// This video mode requires DISPHSTX_DISP_SEL switch to be enabled.
	vmodetime_1360x768,		// 1360x768/60.1Hz/47.8kHz (VESA DMT ID 27h), pixel clock 85.6 MHz, system clock 428 MHz ... detected as 1024x768@60Hz on VGA
#endif // USE_VREG_LOCKED
#endif

#if DISPHSTX_WIDTHMAX >= 1440	// max. supported width
	vmodetime_1440x600,		// 1440x600/56Hz/35kHz, pixel clock 64 MHz, system clock 320 MHz ... detected as 800x600@56Hz on VGA
#endif
	vmodetime_num
};

// videomode timings descriptor
// - Structure definition must match definition in files disphstx_*.S
typedef struct {
	u8	hpol;		// 0x00: HSYNC polarity 0=negative, 1=positive
	u8	vpol;		// 0x01: VSYNC polarity 0=negative, 1=positive
	u8	clkdiv;		// 0x02: system clock divider (1 or 2)
	u8	res;		// 0x03: ... reserved (align)
// aligned

	// horizontal timings
	u16	hactive;	// 0x04: horizontal active video pixels (must be even number)
	u16	hfront;		// 0x06: horizontal front porch in video pixels (must be even number)
	u16	hsync;		// 0x08: HSYNC width in video pixels (must be even number)
	u16	hback;		// 0x0A: horizontal back porch in video pixels (must be even number)
// aligned
	u16	htotal;		// 0x0C: horizontal total width in video pixels

	// vertical timings
	u16	vactive;	// 0x0E: vertical active scanlines
	u16	vfront;		// 0x10: vertical front porch in scanlines
	u16	vsync;		// 0x12: VSYNC height in scanlines
	u16	vback;		// 0x14: vertical back porch in scanlines
	u16	vtotal;		// 0x16: vertical total height in scanlines
// aligned

	// clock (shifter: 5 clock cycles per video pixel, expander: 1 loop per shifter, divider: 1 or 2)
	u32	sysclk;		// 0x18: system clock in kHz (= 5x or 10x video pixel clock)

} sDispHstxVModeTime;

#define DISPHSTXVMODETIME_SIZE 0x1C	// size of the sDispHstxVModeTime structure
STATIC_ASSERT(sizeof(sDispHstxVModeTime) == DISPHSTXVMODETIME_SIZE, "Incorrect sDispHstxVModeTime!");

// list of predefined videomode timings
extern const sDispHstxVModeTime DispHstxVModeTimeList[vmodetime_num];

#ifdef __cplusplus
}
#endif

#endif // _DISPHSTX_VMODE_TIME_H

#endif // USE_DISPHSTX
