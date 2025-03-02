
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                         Video modes - Color format list
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

// Note: The following 2 switches are not yet defined in the PicoLibSDK at this point, so the global.h file is included.
#if USE_DISPHSTX && DISPHSTX_PICOSDK	// 0=use PicoLibSDK library, 1=use PicoSDK original Raspberry SDK library
#include "disphstx_picolibsk.h"
#else
#include "../../global.h"
#endif

#if USE_DISPHSTX		// 1=use HSTX Display driver

#include "disphstx.h"

// list of pixel color formats
const sDispHstxVColor DispHstxVColorList[DISPHSTX_FORMAT_NUM] = {

	// DISPHSTX_FORMAT_NONE = 0, invalid pixel format (used as invalid slot of the strip, with width 0)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		0,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16,// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		NULL,		// u16*	defpal;		// default palettes (NULL=none)
		NULL,		// u32*	defpalvga;	// default VGA palettes (NULL=none)

		// render
		0,		// u32	expand_tmds;	// setup of expand_tmds register
		DispHstxDviRender_No, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_No, // disphstx_vga_render render_vga; // VGA render function
	},

#if DISPHSTX_USE_FORMAT_1
	// DISPHSTX_FORMAT_1, 1 bit per pixel, black & white format 0..1 (8 pixels per 1 byte; width must be aligned to 8 pixels; hdbl > 1 not supported)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		1,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_1,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal1b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga1b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(0 << 21) |	// lane 2: get 1 bit
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B31 to position B7)
			(0 << 13) |	// lane 1: get 1 bit
			(24 << 8) |	// lane 1: number of right-pre-rotate = 24 (shift green 24 bits right from position B31 to position B7)
			(0 << 5)  |	// lane 0: get 1 bit
			(24 << 0),	// lane 0: number of right-pre-rotate = 24 (shift blue 24 bits right from position B31 to position B7)
		DispHstxDviRender_Default, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_1, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_2
	// DISPHSTX_FORMAT_2, 2 bits per pixel, grayscale format 0..3 (4 pixels per 1 byte; width must be aligned to 4 pixels; hdbl > 1 not supported)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		2,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_2,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal2b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga2b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(1 << 21) |	// lane 2: get 2 bits
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B30-B31 to position B6-B7)
			(1 << 13) |	// lane 1: get 2 bits
			(24 << 8) |	// lane 1: number of right-pre-rotate = 24 (shift green 24 bits right from position B30-B31 to position B6-B7)
			(1 << 5)  |	// lane 0: get 2 bits
			(24 << 0),	// lane 0: number of right-pre-rotate = 24 (shift blue 24 bits right from position B30-B31 to position B6-B7)
		DispHstxDviRender_Default, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_2, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_3
	// DISPHSTX_FORMAT_3, 3 bits per pixel, format RGB111 (10 pixels per 32-bit word; width must be aligned to 10 pixels; hdbl > 1 not supported)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		3,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_3,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal3b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga3b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(0 << 21) |	// lane 2: get 1 bit
			(27 << 16) |	// lane 2: number of right-pre-rotate = 27 (shift red 27 bits right from position B2(+32) to position B7)
			(0 << 13) |	// lane 1: get 1 bit
			(26 << 8) |	// lane 1: number of right-pre-rotate = 26 (shift green 26 bits right from position B1(+32) to position B7)
			(0 << 5)  |	// lane 0: get 1 bit
			(25 << 0),	// lane 0: number of right-pre-rotate = 25 (shift blue 25 bits right from position B0(+32) to position B7)
		DispHstxDviRender_Default, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_3, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_4
	// DISPHSTX_FORMAT_4, 4 bits per pixel, format gray 16 levels (2 pixels per 1 byte; width must be aligned to 2 pixels; hdbl > 1 not supported)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		4,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_4,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal4b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga4b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(3 << 21) |	// lane 2: get 4 bits
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B28-B31 to position B4-B7)
			(3 << 13) |	// lane 1: get 4 bits
			(24 << 8) |	// lane 1: number of right-pre-rotate = 24 (shift green 24 bits right from position B28-B31 to position B4-B7)
			(3 << 5)  |	// lane 0: get 4 bits
			(24 << 0),	// lane 0: number of right-pre-rotate = 24 (shift blue 24 bits right from position B28-B31 to position B4-B7)
		DispHstxDviRender_Default, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_4, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_6
	// DISPHSTX_FORMAT_6, 6 bits per pixel, format RGB222 (5 pixels per 32-bit; width must be aligned to 5 pixels; hdbl > 1 not supported)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		6,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_6,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal6b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga6b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(1 << 21) |	// lane 2: get 2 bits
			(30 << 16) |	// lane 2: number of right-pre-rotate = 30 (shift red 30 bits right from position B4-5(+32) to position B6-B7)
			(1 << 13) |	// lane 1: get 2 bits
			(28 << 8) |	// lane 1: number of right-pre-rotate = 28 (shift green 28 bits right from position B2-3(+32) to position B6-B7)
			(1 << 5)  |	// lane 0: get 2 bits
			(26 << 0),	// lane 0: number of right-pre-rotate = 26 (shift blue 26 bits right from position B0-1(+32) to position B6-B7)
		DispHstxDviRender_Default, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_6, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_8
	// DISPHSTX_FORMAT_8, 8 bits per pixel, format RGB332 (1 pixel per 1 byte)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_8,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(2 << 21) |	// lane 2: get 3 bits
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B29-B31 to position B5-B7)
			(2 << 13) |	// lane 1: get 3 bits
			(21 << 8) |	// lane 1: number of right-pre-rotate = 21 (shift green 21 bits right from position B26-B28 to position B5-B7)
			(1 << 5)  |	// lane 0: get 2 bits
			(18 << 0),	// lane 0: number of right-pre-rotate = 18 (shift blue 18 bits right from position B24-B25 to position B6-B7)
		DispHstxDviRender_Default, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_12
	// DISPHSTX_FORMAT_12, 12 bits per pixel, format RGB444 (8 pixels per three 32-bit words)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		12,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_12, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		NULL,		// u16*	defpal;		// default palettes (NULL=none)
		NULL,		// u32*	defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(3 << 21) |	// lane 2: get 4 bits
			(4 << 16) |	// lane 2: number of right-pre-rotate = 4 (shift red 4 bits right from position B8-B11 to position B4-B7)
			(3 << 13) |	// lane 1: get 4 bits
			(0 << 8) |	// lane 1: number of right-pre-rotate = 0 (shift green 0 bits right from position B4-B7 to position B4-B7)
			(3 << 5)  |	// lane 0: get 4 bits
			(28 << 0),	// lane 0: number of right-pre-rotate = 28 (shift blue 28 bits right from position B0-B3(+32) to position B4-B7)
		DispHstxDviRender_12,	// disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_12,	// disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_15
	// DISPHSTX_FORMAT_15, 15 bits per pixel, format RGB555 (1 pixel per 2 bytes; bit 15 is ignored)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		16,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_15, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		NULL,		// u16*	defpal;		// default palettes (NULL=none)
		NULL,		// u32*	defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(7 << 16) |	// lane 2: number of right-pre-rotate = 7 (shift red 7 bits right from position B10-B14 to position B3-B7)
			(4 << 13) |	// lane 1: get 5 bits
			(2 << 8) |	// lane 1: number of right-pre-rotate = 2 (shift green 2 bits right from position B5-B9 to position B3-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Default, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_15,	// disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_16
	// DISPHSTX_FORMAT_16, 16 bits per pixel, format RGB565 (1 pixel per 2 bytes)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		16,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		NULL,		// u16*	defpal;		// default palettes (NULL=none)
		NULL,		// u32*	defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Default, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_16,	// disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_1_PAL
	// DISPHSTX_FORMAT_1_PAL, 1 bit per pixel, paletted colors (8 pixels per 1 byte)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		1,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		2,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal1b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga1b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_1_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_1, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_2_PAL
	// DISPHSTX_FORMAT_2_PAL, 2 bits per pixel, paletted colors (4 pixels per 1 byte)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		2,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		4,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal2bcol, // u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga2bcol, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_2_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_2, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_3_PAL
	// DISPHSTX_FORMAT_3_PAL, 3 bits per pixel, paletted colors (10 pixels per 32-bit word)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		3,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		8,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal3b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga3b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_3_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_3, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_4_PAL
	// DISPHSTX_FORMAT_4_PAL, 4 bits per pixel, paletted colors (2 pixels per 1 byte)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		4,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		16,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal4bcol, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga4bcol, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_4_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_4, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_6_PAL
	// DISPHSTX_FORMAT_6_PAL, 6 bits per pixel, paletted colors (5 pixels per 32-bit word)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		6,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_6,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		64,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal6b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga6b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_6_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_6, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_8_PAL
	// DISPHSTX_FORMAT_8_PAL, 8 bits per pixel, paletted colors (1 pixel per 1 byte)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		True,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_8_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_8,	// disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_COL
	// DISPHSTX_FORMAT_COL, simple color (given as palette color RGB565 per every line, color index modulo fonth)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		True,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		16,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_COL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_COL,	// disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_MTEXT
	// DISPHSTX_FORMAT_MTEXT, monocolor text, font width 8 pixels, colors of every row given as 2 palette colors RGB565 (BG,FG)
	{
		// Bool flags
		True,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		16,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_MTEXT, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_MTEXT, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_ATEXT
	// DISPHSTX_FORMAT_ATEXT, attribute text, font width 8 pixels, 16 palette entries, character + 2x4 bit attribute low=FG high=BG index of color
	{
		// Bool flags
		True,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		16,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		16,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal4bcol, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga4bcol, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_ATEXT, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_ATEXT, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8
	// DISPHSTX_FORMAT_TILE4_8, tiles 4x4 pixels, 1-byte index, 8-bit format RGB332
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		True,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_8,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		4,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(2 << 21) |	// lane 2: get 3 bits
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B29-B31 to position B5-B7)
			(2 << 13) |	// lane 1: get 3 bits
			(21 << 8) |	// lane 1: number of right-pre-rotate = 21 (shift green 21 bits right from position B26-B28 to position B5-B7)
			(1 << 5)  |	// lane 0: get 2 bits
			(18 << 0),	// lane 0: number of right-pre-rotate = 18 (shift blue 18 bits right from position B24-B25 to position B6-B7)
		DispHstxDviRender_TILE4_8, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_TILE4_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8
	// DISPHSTX_FORMAT_TILE8_8, tiles 8x8 pixels, 1-byte index, 8-bit format RGB332
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		True,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_8,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		8,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(2 << 21) |	// lane 2: get 3 bits
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B29-B31 to position B5-B7)
			(2 << 13) |	// lane 1: get 3 bits
			(21 << 8) |	// lane 1: number of right-pre-rotate = 21 (shift green 21 bits right from position B26-B28 to position B5-B7)
			(1 << 5)  |	// lane 0: get 2 bits
			(18 << 0),	// lane 0: number of right-pre-rotate = 18 (shift blue 18 bits right from position B24-B25 to position B6-B7)
		DispHstxDviRender_TILE8_8, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_TILE8_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8
	// DISPHSTX_FORMAT_TILE16_8, tiles 16x16 pixels, 1-byte index, 8-bit format RGB332
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		True,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_8,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		16,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(2 << 21) |	// lane 2: get 3 bits
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B29-B31 to position B5-B7)
			(2 << 13) |	// lane 1: get 3 bits
			(21 << 8) |	// lane 1: number of right-pre-rotate = 21 (shift green 21 bits right from position B26-B28 to position B5-B7)
			(1 << 5)  |	// lane 0: get 2 bits
			(18 << 0),	// lane 0: number of right-pre-rotate = 18 (shift blue 18 bits right from position B24-B25 to position B6-B7)
		DispHstxDviRender_TILE16_8, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_TILE16_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8
	// DISPHSTX_FORMAT_TILE32_8, tiles 32x32 pixels, 1-byte index, 8-bit format RGB332
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		True,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_8,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		32,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(2 << 21) |	// lane 2: get 3 bits
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B29-B31 to position B5-B7)
			(2 << 13) |	// lane 1: get 3 bits
			(21 << 8) |	// lane 1: number of right-pre-rotate = 21 (shift green 21 bits right from position B26-B28 to position B5-B7)
			(1 << 5)  |	// lane 0: get 2 bits
			(18 << 0),	// lane 0: number of right-pre-rotate = 18 (shift blue 18 bits right from position B24-B25 to position B6-B7)
		DispHstxDviRender_TILE32_8, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_TILE32_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8_PAL
	// DISPHSTX_FORMAT_TILE4_8_PAL, tiles 4x4 pixels, 1-byte index, 8-bit paletted colors
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		True,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		4,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_TILE4_8_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_TILE4_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8_PAL
	// DISPHSTX_FORMAT_TILE8_8_PAL, tiles 8x8 pixels, 1-byte index, 8-bit paletted colors
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		True,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		8,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_TILE8_8_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_TILE8_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8_PAL
	// DISPHSTX_FORMAT_TILE16_8_PAL, tiles 16x16 pixels, 1-byte index, 8-bit paletted colors
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		True,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		16,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_TILE16_8_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_TILE16_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8_PAL
	// DISPHSTX_FORMAT_TILE32_8_PAL, tiles 32x32 pixels, 1-byte index, 8-bit paletted colors
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		True,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		32,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_TILE32_8_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_TILE32_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_HSTX_15
	// DISPHSTX_FORMAT_HSTX_15, HSTX RLE compression, 15 bits per pixel, format RGB555
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		True,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		16,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_15, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		NULL,		// u16*	defpal;		// default palettes (NULL=none)
		NULL,		// u32*	defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(7 << 16) |	// lane 2: number of right-pre-rotate = 7 (shift red 7 bits right from position B10-B14 to position B3-B7)
			(4 << 13) |	// lane 1: get 5 bits
			(2 << 8) |	// lane 1: number of right-pre-rotate = 2 (shift green 2 bits right from position B5-B9 to position B3-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_HSTX, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_HSTX_15, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_HSTX_16
	// DISPHSTX_FORMAT_HSTX_16, HSTX RLE compression, 16 bits per pixel, format RGB565
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		True,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		16,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		NULL,		// u16*	defpal;		// default palettes (NULL=none)
		NULL,		// u32*	defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_HSTX, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_HSTX_16, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_PAT_8
	// DISPHSTX_FORMAT_PAT_8, repeated pattern, 8-bit format RGB332, width derived from pitch, height from fonth
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_8,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(2 << 21) |	// lane 2: get 3 bits
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B29-B31 to position B5-B7)
			(2 << 13) |	// lane 1: get 3 bits
			(21 << 8) |	// lane 1: number of right-pre-rotate = 21 (shift green 21 bits right from position B26-B28 to position B5-B7)
			(1 << 5)  |	// lane 0: get 2 bits
			(18 << 0),	// lane 0: number of right-pre-rotate = 18 (shift blue 18 bits right from position B24-B25 to position B6-B7)
		DispHstxDviRender_Pat_8, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Pat_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_PAT_8_PAL
	// DISPHSTX_FORMAT_PAT_8_PAL, 8 bits per pixel, paletted colors
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Pat_8_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Pat_8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_RLE8
	// DISPHSTX_FORMAT_RLE8, RLE compression, 8-bit format RGB332 (font=line offsets; slot width must match image width)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_8,	// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(2 << 21) |	// lane 2: get 3 bits
			(24 << 16) |	// lane 2: number of right-pre-rotate = 24 (shift red 24 bits right from position B29-B31 to position B5-B7)
			(2 << 13) |	// lane 1: get 3 bits
			(21 << 8) |	// lane 1: number of right-pre-rotate = 21 (shift green 21 bits right from position B26-B28 to position B5-B7)
			(1 << 5)  |	// lane 0: get 2 bits
			(18 << 0),	// lane 0: number of right-pre-rotate = 18 (shift blue 18 bits right from position B24-B25 to position B6-B7)
		DispHstxDviRender_Rle8, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Rle8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_RLE8_PAL
	// DISPHSTX_FORMAT_RLE8_PAL, RLE compression, 8-bit format, paletted colors (font=line offsets; slot width must match image width)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b,	// u16*	defpal;		// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Rle8_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Rle8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_ATTR1_PAL
	// DISPHSTX_FORMAT_ATTR1_PAL, attribute compression 1 (to 28%), 4-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		True,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		4,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		16,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal4bcol, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga4bcol, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Attr1_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Attr1, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_ATTR2_PAL
	// DISPHSTX_FORMAT_ATTR2_PAL, attribute compression 2 (to 37%), 4-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		True,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		4,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		16,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal4bcol, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga4bcol, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Attr2_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Attr2, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_ATTR3_PAL
	// DISPHSTX_FORMAT_ATTR3_PAL, attribute compression 3 (to 56%), 4-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		True,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		4,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		16,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal4bcol, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga4bcol, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Attr3_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Attr3, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_ATTR4_PAL
	// DISPHSTX_FORMAT_ATTR4_PAL, attribute compression 4 (to 75%), 4-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		True,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		4,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		16,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal4bcol, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga4bcol, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Attr4_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Attr4, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_ATTR5_PAL
	// DISPHSTX_FORMAT_ATTR5_PAL, attribute compression 5 (to 15%), 8-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		True,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Attr5_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Attr5, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_ATTR6_PAL
	// DISPHSTX_FORMAT_ATTR6_PAL, attribute compression 6 (to 25%), 8-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		True,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Attr6_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Attr6, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_ATTR7_PAL
	// DISPHSTX_FORMAT_ATTR7_PAL, attribute compression 7 (to 31%), 8-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		True,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Attr7_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Attr7, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

#if DISPHSTX_USE_FORMAT_ATTR8_PAL
	// DISPHSTX_FORMAT_ATTR8_PAL, attribute compression 8 (to 50%), 8-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes)
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		True,		// u8	userender;	// use render buffer (software rendering scanline)
		True,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		8,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16, // u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		256,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		DispHstxPal8b, // u16* defpal;	// default palettes (NULL=none)
		DispHstxPalVga8b, // u32* defpalvga;	// default VGA palettes (NULL=none)

		// render
		0 |		// u32	expand_tmds;	// setup of expand_tmds register
			(4 << 21) |	// lane 2: get 5 bits
			(8 << 16) |	// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |	// lane 1: get 6 bits
			(3 << 8) |	// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |	// lane 0: get 5 bits
			(29 << 0),	// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)
		DispHstxDviRender_Attr8_PAL, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_Attr8, // disphstx_vga_render render_vga; // VGA render function
	},
#endif

	// DISPHSTX_FORMAT_CUSTOM, custom user format - set DispHstxVColorCustom to your own format descriptor sDispHstxVColor
	// - this entry is not used, DispHstxVColorCustom pointer is used instead of it
	{
		// Bool flags
		False,		// u8	textmode;	// text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
		False,		// u8	colmode;	// simple color mode DISPHSTX_FORMAT_COL
		False,		// u8	tilemode;	// tile mode
		False,		// u8	hstxmode;	// HSTX RLE compression mode
		False,		// u8	userender;	// use render buffer (software rendering scanline)
		False,		// u8	attrmode;	// attribute mode
		False,		// u8	graphmode;	// graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

		// format
		0,		// u8	colbits;	// number of bits per pixel (including unused bits)
		DISPHSTX_GRP_16,// u8	grp;		// pixel format group DISPHSTX_GRP_*
		0,		// u8	tilew;		// width and height of tile in graphics pixels
		0,		// u16	palnum;		// number of palette entries (0 = palette not used)

		// palettes
		NULL,		// u16*	defpal;		// default palettes (NULL=none)
		NULL,		// u32*	defpalvga;	// default VGA palettes (NULL=none)

		// render
		0,		// u32	expand_tmds;	// setup of expand_tmds register
		DispHstxDviRender_No, // disphstx_dvi_render render_dvi;	// HDMI render function
		DispHstxVgaRender_No, // disphstx_vga_render render_vga; // VGA render function
	},
};

// custom user format descriptor (used with DISPHSTX_FORMAT_CUSTOM format)
// - set to point to your own format descriptor
const sDispHstxVColor* DispHstxVColorCustom = &DispHstxVColorList[DISPHSTX_FORMAT_CUSTOM];

#endif // USE_DISPHSTX
