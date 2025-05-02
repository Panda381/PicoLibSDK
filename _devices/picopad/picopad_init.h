
// ****************************************************************************
//
//                              PicoPad Init
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

#if USE_PICOPAD

#ifndef _PICOPAD_INIT_H
#define _PICOPAD_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#if USE_DISPHSTX && USE_DISPHSTX_VMODE	// 1=use HSTX Display driver

#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
extern FRAMETYPE ALIGNED DispBuf[FRAMESIZE];	// display buffer
#endif

#if USE_FRAMEBUF && USE_DISPHSTX_VMODE	// use videomode 0=custom, 1=320x240/16 BackBuf, ...
extern FRAMETYPE ALIGNED FrameBuf[FRAMESIZE];	// frame buffer
#else // USE_FRAMEBUF
extern FRAMETYPE ALIGNED FrameBuf[];
#endif // USE_FRAMEBUF

#if USE_DRAW		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#define pDrawBuf FrameBuf	// current draw buffer
#define DispStripInx  0		// current index of back buffer strip
#define DispMinY  0		// minimal Y; base of back buffer strip
#define DispMaxY HEIGHT		// maximal Y + 1; end of back buffer strip

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
#else // USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
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

// LOW level control: start sending image data to display window (DispSendImg() must follow)
//   Only window on full display is supported (parameters are ignored)
void DispStartImg(u16 x1, u16 x2, u16 y1, u16 y2);

// LOW level control: send one word of image data to display (follows after DispStartImg())
void DispSendImg2(u16 data);

// LOW level control: stop sending image data (follows after DispStartImg() and DispSendImg())
void DispStopImg();

#endif // USE_DISPHSTX && USE_DISPHSTX_VMODE	// 1=use HSTX Display driver

// Device init
void DeviceInit();

// Device terminate
void DeviceTerm();

#ifdef __cplusplus
}
#endif

#endif // _PICOPAD_INIT_H

#endif // USE_PICOPAD
