
// ****************************************************************************
//
//                                DVI (HDMI)
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

#if USE_DVI					// use DVI (HDMI) display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer

#ifndef _DVI_H
#define _DVI_H

#ifdef __cplusplus
extern "C" {
#endif

// data
extern volatile int DviScanLine;	// current scan line 1...
extern volatile u32 DviFrame;		// frame counter
//extern volatile Bool DviVSync;		// current scan line is vsync or dark

// decode scanline data - red and green channel
//  inbuf ... input buffer (u16), must be u32 aligned
//  outbuf ... output buffer (u32)
//  count ... number of pixels (must be multiply of 8)
u32* DviEnc(u16* inbuf, u32* outbuf, int count);

// decode scanline data - blue channel
//  inbuf ... input buffer (u16), must be u32 aligned
//  outbuf ... output buffer (u32)
//  count ... number of pixels (must be multiply of 8)
u32* DviEncShift(u16* inbuf, u32* outbuf, int count);

// start DVI on core 1 from core 0 (must be paired with DviStop())
// - system clock must be set to 252 MHz
void DviStart();

// terminate DVI on core 1 from core 0 (must be paired with DviStart())
void DviStop();

#ifdef __cplusplus
}
#endif

#endif // _DVI_H

#endif // USE_DVI
