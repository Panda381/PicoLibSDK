
// ****************************************************************************
//
//                              DVI (HDMI) + VGA
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

#if USE_DVIVGA					// use DVI (HDMI) + VGA display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer

#ifndef _DVIVGA_H
#define _DVIVGA_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DVI_IRQTIME
#define DVI_IRQTIME	0	// debug flag - measure delta time of DVI service
#endif
// Measured: IN=28 us, OUT=4 us, IN2=1 us, OUT2=31 us

// frame buffer
//  Planar mode: Lines are interlaced in order blue, green and red.
extern ALIGNED FRAMETYPE FrameBuf[];

// back buffer
#if USE_FRAMEBUF && (BACKBUFSIZE > 0)
//  Planar mode: Lines are interlaced in order blue, green and red.
extern ALIGNED FRAMETYPE BackBuf[BACKBUFSIZE]; // back buffer strip
#endif

// display setup
extern FRAMETYPE* pDrawBuf;	// current draw buffer
extern int DispStripInx;	// current index of back buffer strip (-1 = use full FrameBuf)
extern int DispMinY;		// minimal Y; base of back buffer strip
extern int DispMaxY;		// maximal Y + 1; end of back buffer strip

// dirty window to update (used only with full back buffer, USE_MINIVGA = 2)
extern int DispDirtyX1, DispDirtyX2, DispDirtyY1, DispDirtyY2;

// data
extern volatile int DviScanLine;	// current scan line 1...
extern volatile u32 DviFrame;		// frame counter

#if DVI_IRQTIME				// debug flag - measure delta time of DVI service
extern volatile u32 DviTimeIn;		// time in interrupt service, in [us]
extern volatile u32 DviTimeOut;		// time out interrupt service, in [us]
extern volatile u32 DviTimeIn2;		// time in interrupt service, in [us]
extern volatile u32 DviTimeOut2;	// time out interrupt service, in [us]
#endif // DVI_IRQTIME

// decode scanline data - Red and Green components
//  inbuf ... input buffer (u16), must be u32 aligned
//  outbufR ... output buffer, red component (u32), interpolator 0
//  outbufG ... output buffer, green component (u32), interpolator 1
//  count ... number of pixels (must be multiply of 16)
void DviVgaEncRG(u16* inbuf, u32* outbufR, u32* outbufG, int count);

// decode scanline data - Red and Green components
//  R0: inbuf ... input buffer (u16), must be u32 aligned
//  R1: outbuf ... output buffer, blue component (u32)
//  R2: count ... number of pixels (must be multiply of 16)
void DviVgaEncB(u16* inbuf, u32* outbuf, int count);

// execute core 1 remote function
void DviCore1Exec(void (*fnc)());

// check if core 1 is busy (executing remote function)
Bool DviCore1Busy();

// wait if core 1 is busy (executing remote function)
void DviCore1Wait();

// check VSYNC
Bool DviIsVSync();

// wait for VSync scanline
void DviWaitVSync();

// start DVI on core 1 from core 0 (must be paired with DviStop())
// - system clock must be set to 252 MHz
void DviStart();

// terminate DVI on core 1 from core 0 (must be paired with DviStart())
void DviStop();

// use back buffer
#if BACKBUFSIZE > 0

// set strip of back buffer (-1 = use full FrameBuffer)
void DispSetStrip(int inx);
INLINE void DispSetStripNext() { DispSetStrip(DispStripInx + 1); }

// switch off the back buffer, use only frame buffer to output
INLINE void DispSetStripOff() { DispSetStrip(-1); }

// load back buffer from frame buffer
void DispLoad();

// use full back buffer
#if USE_DVIVGA == 2

// set dirty all frame buffer
void DispDirtyAll();

// set dirty none (clear after update)
void DispDirtyNone();

// update dirty area by rectangle (check valid range)
void DispDirtyRect(int x, int y, int w, int h);

// update dirty area by pixel (check valid range)
void DispDirtyPoint(int x, int y);

#else // USE_DVIVGA == 2

// set dirty all frame buffer
INLINE void DispDirtyAll() {}

// set dirty none (clear after update)
INLINE void DispDirtyNone() {}

// update dirty area by rectangle (check valid range)
INLINE void DispDirtyRect(int x, int y, int w, int h) {}

// update dirty area by pixel (check valid range)
INLINE void DispDirtyPoint(int x, int y) {}

#endif // USE_DVIVGA == 2

// update - send dirty window to display (or write back buffer to frame buffer)
void DispUpdate();

// auto update after delta time in [ms] of running program
void DispAutoUpdate(u32 ms);

// refresh update all display
void DispUpdateAll();

// no back bufer
#else // BACKBUFSIZE > 0

// set strip of back buffer (-1 = use full FrameBuffer)
INLINE void DispSetStrip(int inx) {}
INLINE void DispSetStripNext() {}

// switch off the back buffer, use only frame buffer to output
INLINE void DispSetStripOff() {}

// load back buffer from frame buffer
INLINE void DispLoad() {}

// set dirty all frame buffer
INLINE void DispDirtyAll() {}

// set dirty none (clear after update)
INLINE void DispDirtyNone() {}

// update dirty area by rectangle (check valid range)
INLINE void DispDirtyRect(int x, int y, int w, int h) {}

// update dirty area by pixel (check valid range)
INLINE void DispDirtyPoint(int x, int y) {}

// update - send dirty window to display (or write back buffer to frame buffer)
INLINE void DispUpdate() {}

// auto update after delta time in [ms] of running program
INLINE void DispAutoUpdate(u32 ms) {}

// refresh update all display
INLINE void DispUpdateAll() {}

#endif // BACKBUFSIZE > 0

#ifdef __cplusplus
}
#endif

#endif // _DVIVGA_H

#endif // USE_DVIVGA
