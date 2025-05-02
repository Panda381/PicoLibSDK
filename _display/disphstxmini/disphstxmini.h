
// ****************************************************************************
//
//    HDMI and VGA Mini display driver for Pico2 RP2350 over HSTX interface
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

// Uses videomode 640x480/60Hz/31.5kHz (VGA 4:3, VESA DMT ID 04h), resolution set by USE_DISPHSTXMINI_VMODE

/*
Picopad pinout (8x R=270 ohm):

GPIO12 ... D2+
GPIO13 ... D2-
GPIO14 ... D1+
GPIO15 ... D1-
GPIO16 ... D0+
GPIO17 ... D0-
GPIO18 ... CLK+
GPIO19 ... CLK-

DVI breakout board pinout (8x R=270 ohm):

GPIO12 ... D0+
GPIO13 ... D0-
GPIO14 ... CLK+
GPIO15 ... CLK-
GPIO16 ... D2+
GPIO17 ... D2-
GPIO18 ... D1+
GPIO19 ... D1-

VGA output:

GPIO12 ... B0, R=800 (800) ohm
GPIO13 ... B1, R=400 (400) ohm
GPIO14 ... G0, R=800 (800) ohm
GPIO15 ... G1, R=400 (400) ohm
GPIO16 ... R0, R=800 (800) ohm
GPIO17 ... R1, R=400 (400) ohm
GPIO18 ... HSYNC, R=47 ohm
GPIO19 ... VSYNC, R=47 ohm
*/

#include "_config.h"

#ifndef _DISPHSTXMINI_H
#define _DISPHSTXMINI_H

#ifdef __cplusplus
extern "C" {
#endif

#if USE_DISPHSTXMINI		// 1=use HSTX Display Mini driver

// selected display mode
#define DISPHSTX_DISPMODE_NONE	0	// display mode - none (use auto-detection)
#define DISPHSTX_DISPMODE_DVI	1	// display mode - DVI (HDMI)
#define DISPHSTX_DISPMODE_VGA	2	// display mode - VGA

// frame buffer
#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
extern FRAMETYPE ALIGNED DispBuf[FRAMESIZE];	// display buffer
#endif

#if USE_FRAMEBUF
extern FRAMETYPE ALIGNED FrameBuf[FRAMESIZE];	// frame buffer
#else // USE_FRAMEBUF
extern FRAMETYPE ALIGNED FrameBuf[];
#endif // USE_FRAMEBUF

// flag - HSTX VGA mode is active
extern Bool DispHstxVgaRunning;

// flag - HSTX DVI mode is active
extern Bool DispHstxDviRunning;

// current selected display mode DISPHSTX_DISPMODE_*
extern int DispHstxDispMode;
extern Bool DispHstxDispSelInitOK;

// get current display selection switch (display selection switch is auto-initialized)
int DispHstxCheckDispSel();

// auto-detect display selection on program start (returns DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA)
int DispHstxAutoDispSel();

// Initialize HSTX Mini display driver
//  dispmode ... display mode DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA; set to DISPHSTX_DISPMODE_NONE or 0 to auto-detect display mode selection
void DispHstxInit(int dispmode);

// Terminate HSTX Mini display driver
void DispHstxAllTerm();

// flag - HSTX core1 is running
extern volatile Bool DispHstxRunning;

// execute core 1 remote function
void DispHstxCore1Exec(void (*fnc)());

// check if core 1 is busy (executing remote function)
Bool DispHstxCore1Busy();

// wait if core 1 is busy (executing remote function)
void DispHstxCore1Wait();

// start display driver on core 1 from core 0 (must be paired with DispHstxStop())
void DispHstxStart(int dispmode);

// terminate display driver on core 1 from core 0 (must be paired with DispHstxStart())
void DispHstxStop();

// get current scanline (0..524: 0-479 active, 480-489 FP, 490-491 VSYNC, 492-524 BP)
int DispHstxScanline();

// check VSYNC
Bool DispHstxIsVSync();

// wait for VSync scanline
void DispHstxWaitVSync();
INLINE void WaitVSync() { DispHstxWaitVSync(); }
INLINE void VgaWaitVSync() { WaitVSync(); }

#if USE_DRAW		// use drawing to frame buffer (lib_draw.c, lib_draw.h)

// set strip of back buffer (-1 = use full FrameBuffer)
INLINE void DispSetStrip(int inx) {}
INLINE void DispSetStripNext() {}

// switch off the back buffer, use only frame buffer to output
INLINE void DispSetStripOff() {}

// set dirty all frame buffer
INLINE void DispDirtyAll() {}

// set dirty none (clear after update)
INLINE void DispDirtyNone() {}

// update dirty area by rectangle (check valid range)
INLINE void DispDirtyRect(int x, int y, int w, int h) {}

// update dirty area by pixel (check valid range)
INLINE void DispDirtyPoint(int x, int y) {}

#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
// load back buffer from frame buffer
void DispLoad();

// update - send dirty window to display
void DispUpdate();

// auto update after delta time in [ms] of running program
void DispAutoUpdate(u32 ms);
#else  // USE_DISPHSTX_DISPBUF
// load back buffer from frame buffer
INLINE void DispLoad() {}

// update - send dirty window to display
INLINE void DispUpdate() {}

// auto update after delta time in [ms] of running program
INLINE void DispAutoUpdate(u32 ms) {}
#endif // USE_DISPHSTX_DISPBUF

// refresh update all display
INLINE void DispUpdateAll() { DispUpdate(); }

#endif // USE_DRAW

// LCD simulation: 320x240 pixels, 16-bit colors

#if USE_EMUSCREENSHOT && USE_PICOPAD	// use emulator screen shots
extern volatile Bool DoEmuScreenShot;	// request to do emulator screenshot
#endif

// LOW level control: start sending image data to display window (DispSendImg() must follow)
//   Only window on full display is supported (parameters are ignored)
void DispStartImg(u16 x1, u16 x2, u16 y1, u16 y2);

// LOW level control: send one word of image data to display (follows after DispStartImg())
void DispSendImg2(u16 data);

// LOW level control: stop sending image data (follows after DispStartImg() and DispSendImg())
void DispStopImg();

#endif // USE_DISPHSTXMINI

#ifdef __cplusplus
}
#endif

#endif // _DISPHSTXMINI_H

