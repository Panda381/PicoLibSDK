
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

#if USE_DISPHSTX		// 1=use HSTX Display driver

#ifndef _DISPHSTX_VMODE_FORMAT_H
#define _DISPHSTX_VMODE_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

// pixel color format
//  Some modes are too time consuming and may require either using fast
//  sys_clock (video modes "_fast") or generating a lower resolution
//  image (hdbl > 1) of add some borders around image.
//  They are usually formats with a claimed CPU load of 85% or more:
//    DISPHSTX_FORMAT_15 in VGA mode, DISPHSTX_FORMAT_16 in VGA mode,
//    DISPHSTX_FORMAT_TILE4_8_PAL in RISC-V or VGA mode,
//    DISPHSTX_FORMAT_ATEXT in RISC-V VGA mode
enum {
	DISPHSTX_FORMAT_NONE = 0,	// invalid pixel format (used as invalid slot of the strip, with width 0)

#if DISPHSTX_USE_FORMAT_1
	DISPHSTX_FORMAT_1,		// 1 bit per pixel, black & white format 0..1 (8 pixels per 1 byte; width must be aligned to 8 pixels; hdbl > 1 not supported) (CPU load 2% DVI, 55% ARM VGA, 67% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_2
	DISPHSTX_FORMAT_2,		// 2 bits per pixel, grayscale format 0..3 (4 pixels per 1 byte; width must be aligned to 4 pixels; hdbl > 1 not supported) (CPU load 2% DVI, 57% ARM VGA, 70% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_3
	DISPHSTX_FORMAT_3,		// 3 bits per pixel, format RGB111 (10 pixels per 32-bit word; width must be aligned to 10 pixels; hdbl > 1 not supported)
#endif

#if DISPHSTX_USE_FORMAT_4
	DISPHSTX_FORMAT_4,		// 4 bits per pixel, format gray 16 levels (2 pixels per 1 byte; width must be aligned to 2 pixels; hdbl > 1 not supported) (CPU load 2% DVI, 63% ARM VGA, 74% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_6
	DISPHSTX_FORMAT_6,		// 6 bits per pixel, format RGB222 (5 pixels per 32-bit; width must be aligned to 5 pixels; hdbl > 1 not supported)
#endif

#if DISPHSTX_USE_FORMAT_8
	DISPHSTX_FORMAT_8,		// 8 bits per pixel, format RGB332 (1 pixel per 1 byte) (CPU load 2% DVI, 61% ARM VGA, 73% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_12
	DISPHSTX_FORMAT_12,		// 12 bits per pixel, format RGB444 (8 pixels per three 32-bit words) (CPU load 112% ARM VGA)
#endif

#if DISPHSTX_USE_FORMAT_15
	DISPHSTX_FORMAT_15,		// 15 bits per pixel, format RGB555 (1 pixel per 2 bytes; bit 15 is ignored) (CPU load 2% DVI, 114% ARM VGA, 156% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_16
	DISPHSTX_FORMAT_16,		// 16 bits per pixel, format RGB565 (1 pixel per 2 bytes) (CPU load 2% DVI, 114% ARM VGA, 148% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_1_PAL
	DISPHSTX_FORMAT_1_PAL,		// 1 bit per pixel, paletted colors (8 pixels per 1 byte) (CPU load 30% ARM DVI, 35% RISC-V DVI, 55% ARM VGA, 67% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_2_PAL
	DISPHSTX_FORMAT_2_PAL,		// 2 bits per pixel, paletted colors (4 pixels per 1 byte) (CPU load 56% ARM DVI, 71% RISC-V DVI, 57% ARM VGA, 70% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_3_PAL
	DISPHSTX_FORMAT_3_PAL,		// 3 bits per pixel, paletted colors (10 pixels per 32-bit word)
#endif

#if DISPHSTX_USE_FORMAT_4_PAL
	DISPHSTX_FORMAT_4_PAL,		// 4 bits per pixel, paletted colors (2 pixels per 1 byte) (CPU load 65% ARM DVI, 78% RISC-V DVI, 63% ARM VGA, 74% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_6_PAL
	DISPHSTX_FORMAT_6_PAL,		// 6 bits per pixel, paletted colors (5 pixels per 32-bit word)
#endif

#if DISPHSTX_USE_FORMAT_8_PAL
	DISPHSTX_FORMAT_8_PAL,		// 8 bits per pixel, paletted colors (1 pixel per 1 byte) (CPU load 60% ARM DVI, 78% RISC-V DVI, 61% ARM VGA, 73% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_COL
	DISPHSTX_FORMAT_COL,		// simple color (given as palette color RGB565 per every line, color index modulo fonth) (CPU load 3%)
#endif

#if DISPHSTX_USE_FORMAT_MTEXT
	DISPHSTX_FORMAT_MTEXT,		// monocolor text, font width 8 pixels, colors of every row given as 2 palette colors RGB565 (BG,FG) (CPU load 41% ARM DVI, 47% RISC-V DVI, 66% ARM VGA, 79% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_ATEXT
	DISPHSTX_FORMAT_ATEXT,		// attribute text, font width 8 pixels, 16 palette entries, character + 2x4 bit attribute low=FG high=BG index of color (CPU load 61% ARM DVI, 77% RISC-V DVI, 71% ARM VGA, 93% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8
	DISPHSTX_FORMAT_TILE4_8,	// tiles 4x4 pixels, 1-byte index, 8-bit format RGB332 (CPU load 28% ARM DVI, 26% RISC-V DVI, 82% ARM VGA, 90% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8
	DISPHSTX_FORMAT_TILE8_8,	// tiles 8x8 pixels, 1-byte index, 8-bit format RGB332 (CPU load 20% ARM DVI, 20% RISC-V DVI, 73% ARM VGA, 84% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8
	DISPHSTX_FORMAT_TILE16_8,	// tiles 16x16 pixels, 1-byte index, 8-bit format RGB332 (CPU load 15% ARM DVI, 16% RISC-V DVI, 68% ARM VGA, 80% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8
	DISPHSTX_FORMAT_TILE32_8,	// tiles 32x32 pixels, 1-byte index, 8-bit format RGB332 (CPU load 13% ARM DVI, 13% RISC-V DVI, 64% ARM VGA, 79% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8_PAL
	DISPHSTX_FORMAT_TILE4_8_PAL,	// tiles 4x4 pixels, 1-byte index, 8-bit paletted colors (CPU load 79% ARM DVI, 90% RISC-V DVI, 82% ARM VGA, 90% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8_PAL
	DISPHSTX_FORMAT_TILE8_8_PAL,	// tiles 8x8 pixels, 1-byte index, 8-bit paletted colors (CPU load 70% ARM DVI, 82% RISC-V DVI, 73% ARM VGA, 84% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8_PAL
	DISPHSTX_FORMAT_TILE16_8_PAL,	// tiles 16x16 pixels, 1-byte index, 8-bit paletted colors (CPU load 67% ARM DVI, 80% RISC-V DVI, 68% ARM DVI, 80% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8_PAL
	DISPHSTX_FORMAT_TILE32_8_PAL,	// tiles 32x32 pixels, 1-byte index, 8-bit paletted colors (CPU load 63% ARM DVI, 78% RISC-V DVI, 64% ARM VGA, 79% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_HSTX_15
	DISPHSTX_FORMAT_HSTX_15,	// HSTX RLE compression, 15 bits per pixel, format RGB555 (font=line offsets; slot width must match image width) (CPU load 3% DVI, 128% ARM VGA, 152% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_HSTX_16
	DISPHSTX_FORMAT_HSTX_16,	// HSTX RLE compression, 16 bits per pixel, format RGB565 (font=line offsets; slot width must match image width) (CPU load 3% DVI, 128% ARM VGA, 144% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_PAT_8
	DISPHSTX_FORMAT_PAT_8,		// repeated pattern, 8-bit format RGB332, width derived from pitch, height from fonth (CPU load with pattern 64x48: 19% ARM DVI, 17% RISC-V DVI, 64% ARM VGA, 79% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_PAT_8_PAL
	DISPHSTX_FORMAT_PAT_8_PAL,	// repeated pattern, 8-bit format, paletted colors, width derived from pitch, height from fonth (CPU load with pattern 64x48: 63% ARM DVI, 81% RISC-V DVI, 64% ARM VGA, 79% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_RLE8
	DISPHSTX_FORMAT_RLE8,		// RLE compression, 8-bit format RGB332 (font=line offsets) (CPU load %55 ARM DVI, 71% RISC-V DVI, 83% ARM VGA, 90% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_RLE8_PAL
	DISPHSTX_FORMAT_RLE8_PAL,	// RLE compression, 8-bit format, paletted colors (font=line offsets) (CPU load 86% ARM DVI, 92% RISC-V DVI, 83% ARM VGA, 90% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_ATTR1_PAL
	DISPHSTX_FORMAT_ATTR1_PAL,	// attribute compression 1 (to 28%), 4-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes) (CPU load 76% ARM DVI, 108% RISC-V DVI, 78% ARM VGA, 105% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_ATTR2_PAL
	DISPHSTX_FORMAT_ATTR2_PAL,	// attribute compression 2 (to 37%), 4-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes) (CPU load 87% ARM DVI, 120% RISC-V DVI, 89% ARM VGA, 116% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_ATTR3_PAL
	DISPHSTX_FORMAT_ATTR3_PAL,	// attribute compression 3 (to 56%), 4-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes) (CPU load 87% ARM DVI, 120% RISC-V DVI, 88% ARM VGA, 115% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_ATTR4_PAL
	DISPHSTX_FORMAT_ATTR4_PAL,	// attribute compression 4 (to 75%), 4-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes) (CPU load 91% ARM DVI, 135% RISC-V DVI, 91% ARM VGA, 130% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_ATTR5_PAL
	DISPHSTX_FORMAT_ATTR5_PAL,	// attribute compression 5 (to 15%), 8-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes) (CPU load 74% ARM DVI, 105% RISC-V DVI, 74% ARM VGA, 102% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_ATTR6_PAL
	DISPHSTX_FORMAT_ATTR6_PAL,	// attribute compression 6 (to 25%), 8-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes) (CPU load 77% ARM DVI, 110% RISC-V DVI, 77% ARM VGA, 106% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_ATTR7_PAL
	DISPHSTX_FORMAT_ATTR7_PAL,	// attribute compression 7 (to 31%), 8-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes) (CPU load 75% ARM DVI, 108% RISC-V DVI, 79% ARM VGA, 104% RISC-V VGA)
#endif

#if DISPHSTX_USE_FORMAT_ATTR8_PAL
	DISPHSTX_FORMAT_ATTR8_PAL,	// attribute compression 8 (to 50%), 8-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes) (CPU load 76% ARM DVI, 110% RISC-V DVI, 77% ARM VGA, 105% RISC-V VGA)
#endif

	DISPHSTX_FORMAT_CUSTOM,		// custom user format - set DispHstxVColorCustom to your own format descriptor sDispHstxVColor

	DISPHSTX_FORMAT_NUM,
};

// DVI pixel format group
enum {
	DISPHSTX_GRP_1,			// 1 bit per pixel, black & white 0..1 (8 pixels per 1 byte)
	DISPHSTX_GRP_2,			// 2 bits per pixel, grayscale 0..3 (4 pixels per 1 byte)
	DISPHSTX_GRP_3,			// 3 bits per pixel, format RGB111 (10 pixels per 32-bit word)
	DISPHSTX_GRP_4,			// 4 bits per pixel, grayscale 0..15 (2 pixels per 1 byte)
	DISPHSTX_GRP_6,			// 6 bits per pixel, format RGB222 (5 pixels per 32-bit word)
	DISPHSTX_GRP_8,			// 8 bits per pixel, format RGB332 (1 pixel per 1 byte)
	DISPHSTX_GRP_12,		// 12 bits per pixel, format RGB444 (8 pixels per three 32-bit words)
	DISPHSTX_GRP_15,		// 15 bits per pixel, format RGB555 in bits 0..14, bit 15 is ignored (1 pixel per 2 bytes)
	DISPHSTX_GRP_16,		// 16 bits per pixel, format RGB565 (1 pixel per 2 bytes)

	DISPHSTX_GRP_NUM,
};

// color in 16-bit RGB565 format
#ifndef COLBITS
#define COLBITS		16		// number of output color bits
#endif

#ifndef COLTYPE
#define COLTYPE		u16		// type of color
#endif

#ifndef COLOR16
#define COLOR16(r,g,b)	((u16)( (((r)&0xf8)<<8) | (((g)&0xfc)<<3) | (((b)&0xf8)>>3) ))
#endif

typedef struct sDispHstxVSlot_ sDispHstxVSlot;

// line render function
typedef void (*disphstx_dvi_render)(sDispHstxVSlot* slot, int line, u32* cmd);
typedef void (*disphstx_vga_render)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// pixel color format descriptor
// - Structure definition must match definition in files disphstx_*.S
typedef struct {
	// Bool flags
	u8	textmode;	// 0x00: text mode DISPHSTX_FORMAT_MTEXT or DISPHSTX_FORMAT_ATEXT
	u8	colmode;	// 0x01: simple color mode DISPHSTX_FORMAT_COL
	u8	tilemode;	// 0x02: tile mode
	u8	hstxmode;	// 0x03: HSTX RLE compression mode
// aligned
	u8	userender;	// 0x04: use render buffer (software rendering scanline)
	u8	attrmode;	// 0x05: attribute mode
	u8	graphmode;	// 0x06: graphics mode DISPHSTX_FORMAT_1..DISPHSTX_FORMAT_8_PAL, supported by the DrawCan library

	// format
	u8	colbits;	// 0x07: number of bits per pixel (including unused bits)
// aligned
	u8	grp;		// 0x08: pixel format group DISPHSTX_GRP_*
	u8	tilew;		// 0x09: width and height of tile in graphics pixels
	u16	palnum;		// 0x0A: number of required palette entries (0 = palette not used)
// aligned

	// palettes
	u16*	defpal;		// 0x0C: default palettes (NULL=none)
	u32*	defpalvga;	// 0x10: default VGA palettes, double-size format with time dithering (NULL=none)

	// HDMI
	u32	expand_tmds;	// 0x14: setup of expand_tmds register
				// - setup TMDS encoder for RGB565 ... need to align color bits to B7:B6:...
				//   lane 2: red channel
				//   lane 1: green channel
				//   lane 0: blue channel
	disphstx_dvi_render render_dvi; // 0x18: DVI render function
	disphstx_vga_render render_vga; // 0x1C: VGA render function
} sDispHstxVColor;

#define DISPHSTXVCOLOR_SIZE 0x20	// size of the sDispHstxVColor structure
STATIC_ASSERT(sizeof(sDispHstxVColor) == DISPHSTXVCOLOR_SIZE, "Incorrect sDispHstxVColor!");

// list of pixel color formats
extern const sDispHstxVColor DispHstxVColorList[DISPHSTX_FORMAT_NUM];

// custom user format descriptor (used with DISPHSTX_FORMAT_CUSTOM format)
// - set to point to your own format descriptor
extern const sDispHstxVColor* DispHstxVColorCustom;

#ifdef __cplusplus
}
#endif

#endif // _DISPHSTX_VMODE_FORMAT_H

#endif // USE_DISPHSTX
