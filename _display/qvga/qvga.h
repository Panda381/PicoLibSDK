
// ****************************************************************************
//
//                                  QVGA
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#if USE_QVGA		// use QVGA display 320x240/8, 1=use 1 frame buffer 76.8 KB, 2=use 2 frame buffers 153.6 KB (qvga.c, qvga.h)

#ifndef _QVGA_H
#define _QVGA_H

#ifdef __cplusplus
extern "C" {
#endif

// QVGA horizonal timings
#define QVGA_CLKDIV	2	// SM divide clock ticks
#define QVGA_CPP	5	// state machine clock ticks per pixel
//#define QVGA_SYSCPP	(QVGA_CLKDIV*QVGA_CPP) // system clock ticks per pixel (=10)

// 126 MHz timings (126000000 Hz)
/*
#define QVGA_VCO	(1008*1000*1000) // PLL VCO frequency in Hz
#define QVGA_FBDIV	84	// PLL FBDIV
#define QVGA_PD1	4	// PLL PD1
#define QVGA_PD2	2	// PLL PD2
*/
#define QVGA_TOTAL	2002	// total clock ticks (= QVGA_HSYNC + QVGA_BP + WIDTH*QVGA_CPP[1600] + QVGA_FP)
#define QVGA_HSYNC	240	// horizontal sync clock ticks
#define QVGA_BP		121	// back porch clock ticks
#define QVGA_FP		41	// front porch clock ticks

// QVGA vertical timings
#define QVGA_VTOT	525	// total scanlines (= QVGA_VSYNC + QVGA_VBACK + QVGA_VACT + QVGA_VFRONT)
#define QVGA_VSYNC	2	// length of V sync (number of scanlines)
#define QVGA_VBACK	33	// V back porch
#define QVGA_VACT	480	// V active scanlines (= 2*HEIGHT)
#define QVGA_VFRONT	10	// V front porch

// default timings (125 MHz)
//#define QVGA_DEF_VCO	(1500*1000*1000) // PLL VCO frequency in Hz
//#define QVGA_DEF_FBDIV	125	// PLL FBDIV
//#define QVGA_DEF_PD1	4	// PLL PD1
//#define QVGA_DEF_PD2	3	// PLL PD2

// frame buffer in RGB 3-3-2 pixel format
extern ALIGNED u8 FrameBuf[FRAMESIZE]; // display buffer
#if USE_QVGA > 1
extern ALIGNED u8 BackBuf[FRAMESIZE]; // back buffer
#endif

// initialize QVGA (required system clock CLK_SYS to be set to 126 MHz)
void QVgaInit();

// terminate QVGA
void QVgaTerm();

// execute core 1 remote function
void QCore1Exec(void (*fnc)());

// check if core 1 is busy (executing remote function)
Bool QCore1Busy();

// wait if core 1 is busy (executing remote function)
void QCore1Wait();

// check VSYNC
Bool QIsVSync();

// wait for VSync scanline
void QWaitVSync();

// start QVGA (must be paired with QVgaStop())
void QVgaStart();

// terminate QVGA (must be paired with QVgaStart())
void QVgaStop();

#if USE_QVGA > 1

// set dirty all frame buffer
void DispDirtyAll();

// set dirty none (clear after update)
void DispDirtyNone();

// update dirty area by rectangle (check valid range)
void DispDirtyRect(int x, int y, int w, int h);

// update dirty area by pixel (check valid range)
void DispDirtyPoint(int x, int y);

// update - send dirty window to display
void DispUpdate();

// auto update after delta time in [ms] of running program
void DispAutoUpdate(u32 ms);

// refresh update all display
void DispUpdateAll();

#else // USE_QVGA > 1

// set dirty all frame buffer
INLINE void DispDirtyAll() {}

// set dirty none (clear after update)
INLINE void DispDirtyNone() {}

// update dirty area by rectangle (check valid range)
INLINE void DispDirtyRect(int x, int y, int w, int h) {}

// update dirty area by pixel (check valid range)
INLINE void DispDirtyPoint(int x, int y) {}

// update - send dirty window to display
INLINE void DispUpdate() {}

// auto update after delta time in [ms] of running program
INLINE void DispAutoUpdate(u32 ms) {}

// refresh update all display
INLINE void DispUpdateAll() {}

#endif // USE_QVGA > 1

#ifdef __cplusplus
}
#endif

#endif // _QVGA_H

#endif // USE_QVGA		// use QVGA display 320x240/8, 1=use 1 frame buffer 76.8 KB, 2=use 2 frame buffers 153.6 KB (qvga.c, qvga.h)
