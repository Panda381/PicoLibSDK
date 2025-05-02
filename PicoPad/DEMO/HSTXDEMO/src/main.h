
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                                 Main code
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

#ifndef _MAIN_H
#define _MAIN_H

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 480 lines
// image pitch: 640 bytes
extern const u16 ImgLogo_Pal[256] __attribute__ ((aligned(4)));
extern const u8 ImgLogo[307200] __attribute__ ((aligned(4)));

// format: 16-bit pixel graphics
// image width: 532 pixels
// image height: 400 lines
// image pitch: 1064 bytes
extern const u16 Img532x400_16b[212800] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 54 pixels
// image height: 54 lines
// image pitch: 56 bytes
extern const u16 ImgPattern_Pal[256] __attribute__ ((aligned(4)));
extern const u8 ImgPattern[3024] __attribute__ ((aligned(4)));

// format: 1-bit pixel graphics
// image width: 214 pixels
// image height: 160 lines
// image pitch: 28 bytes
extern const u8 Img1b[4480] __attribute__ ((aligned(4)));

// format: 2-bit paletted pixel graphics
// image width: 214 pixels
// image height: 160 lines
// image pitch: 56 bytes
extern const u16 Img2b_Pal[4] __attribute__ ((aligned(4)));
extern const u8 Img2b[8960] __attribute__ ((aligned(4)));

// format: 3-bit paletted pixel graphics (10 pixels per u32)
// image width: 214 pixels
// image height: 160 lines
// image pitch: 88 bytes
extern const u16 Img3b_Pal[8] __attribute__ ((aligned(4)));
extern const u32 Img3b[3520] __attribute__ ((aligned(4)));

// format: 4-bit paletted pixel graphics
// image width: 214 pixels
// image height: 160 lines
// image pitch: 108 bytes
extern const u16 Img4b_Pal[16] __attribute__ ((aligned(4)));
extern const u8 Img4b[17280] __attribute__ ((aligned(4)));

// format: 6-bit paletted pixel graphics (5 pixels per u32)
// image width: 214 pixels
// image height: 160 lines
// image pitch: 172 bytes
extern const u16 Img6b_Pal[64] __attribute__ ((aligned(4)));
extern const u32 Img6b[6880] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 214 pixels
// image height: 160 lines
// image pitch: 216 bytes
extern const u16 Img8b_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Img8b[34560] __attribute__ ((aligned(4)));

// format: 12-bit pixel graphics with dithering (8 pixels per three 32-bit)
// image width: 214 pixels
// image height: 160 lines
// image pitch: 324 bytes
extern const u32 Img12b[12960] __attribute__ ((aligned(4)));

// format: 15-bit pixel graphics
// image width: 214 pixels
// image height: 160 lines
// image pitch: 428 bytes
extern const u16 Img15b[34240] __attribute__ ((aligned(4)));

// format: 16-bit pixel graphics
// image width: 214 pixels
// image height: 160 lines
// image pitch: 428 bytes
extern const u16 Img16b[34240] __attribute__ ((aligned(4)));

#endif // _MAIN_H
