
// ****************************************************************************
//                                 
//                             DVI configuration
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


// Color modes: 16 bits RGB565


// --- 320*240 (*2, real 640*480)
//  VGA 4:3 640x480, vert. 60 Hz, hor. 31.4685 kHz, pixel clock 12.5874 MHz (real l , sys_clk 252 MHz
#if WIDTH == 320

#ifndef HEIGHT
#define HEIGHT 240			// display height
#endif
#if HEIGHT != 240
#error Incorrect HEIGHT!
#endif

#ifndef PLL_KHZ
#define PLL_KHZ		252000		// PLL system frequency in kHz (default 125000 kHz)
#endif

// horizontal ticks must be even number
//  HSYNC=negative, VSYNC=negative
#define DVI_HACT	640		// horizontal active pixels (visible area)
#define DVI_HFRONT	16		// horizontal front porch in pixels
#define DVI_HSYNC	96		// horizontal sync clock in pixels
#define DVI_HBACK	48		// horizontal back porch in pixels
#define DVI_HTOTAL	800		// horizontal total length in pixels (=hact + hfront + hsync + hback)

#define DVI_VACT	480		// vertical active scanlines (= 2*HEIGHT)
#define DVI_VFRONT	10		// vertical front porch scanlines
#define DVI_VSYNC	2		// vertical sync scanlines
#define DVI_VBACK	33		// vertical back porch scanlines
#define DVI_VTOTAL	525		// vertical total scanlines (=vact + vfront + vsync + vback)

#define DVI_HMUL	2		// horizontal multiplier (= hact / WIDTH)
#define DVI_VMUL	2		// vertical multiplier (= vact / HEIGHT)

// --- unsupported
#else
#error Unsupported WIDTH!
#endif
