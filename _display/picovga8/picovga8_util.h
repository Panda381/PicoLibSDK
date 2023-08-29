
// ****************************************************************************
//
//                                 PicoVGA8 utilities
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

#ifndef _PICOVGA8_UTIL_H
#define _PICOVGA8_UTIL_H

#include "picovga8_config.h"		// common definitions of C and ASM

#ifdef __cplusplus
extern "C" {
#endif

#if USE_PICOVGA8		// use PicoVGA 8-bit display

// convert image from 16-color to 8x8 attribute format
void Attr8Conv(u8* dst, u8* attr, const u8* src, int w, int h, const u8* pal);

// convert image from 4-color to 2-plane format (width must be multiply of 8)
void Plane2Conv(u8* plane0, u8* plane1, const u8* src, int w, int h);

// invert image
void ImgInvert(u8* dst, int num);

// prepare image with white key transparency (copy and increment pixels)
void CopyWhiteImg(u8* dst, const u8* src, int num);

#endif // USE_PICOVGA8		// use PicoVGA 8-bit display

#ifdef __cplusplus
}
#endif

#endif // _PICOVGA8_UTIL_H
