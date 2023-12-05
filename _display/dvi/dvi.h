
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

#define DVI_IRQTIME		1	// debug flag - measure delta time of DVI service
// Measured: IN=28 us, OUT=4 us, IN2=1 us, OUT2=31 us

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
void DviEncRG(u16* inbuf, u32* outbufR, u32* outbufG, int count);

// decode scanline data - Red and Green components
//  R0: inbuf ... input buffer (u16), must be u32 aligned
//  R1: outbuf ... output buffer, blue component (u32)
//  R2: count ... number of pixels (must be multiply of 16)
void DviEncB(u16* inbuf, u32* outbuf, int count);

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
