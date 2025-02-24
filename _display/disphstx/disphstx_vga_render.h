
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                     HSTX VGA driver - render functions
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

#ifndef _DISPHSTX_VGA_RENDER_H
#define _DISPHSTX_VGA_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)

// do-nothing render
void FASTCODE NOFLASH(DispHstxVgaRender_No)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// 1 bit per pixel
void FASTCODE NOFLASH(DispHstxVgaRender_1)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// 2 bits per pixel
void FASTCODE NOFLASH(DispHstxVgaRender_2)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// 3 bits per pixel
void FASTCODE NOFLASH(DispHstxVgaRender_3)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// 4 bits per pixel
void FASTCODE NOFLASH(DispHstxVgaRender_4)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// 6 bits per pixel
void FASTCODE NOFLASH(DispHstxVgaRender_6)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// 8 bits per pixel
void FASTCODE NOFLASH(DispHstxVgaRender_8)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// 12 bits per pixel
void FASTCODE NOFLASH(DispHstxVgaRender_12)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// 15 bits per pixel
void FASTCODE NOFLASH(DispHstxVgaRender_15)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// 16 bits per pixel
void FASTCODE NOFLASH(DispHstxVgaRender_16)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// multiply color lines (given as palette color RGB565 per every line, color index modulo fonth)
void FASTCODE NOFLASH(DispHstxVgaRender_COL)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// monocolor text, font width 8 pixels, colors of every row given as 2 palette colors RGB565 (BG,FG)
void FASTCODE NOFLASH(DispHstxVgaRender_MTEXT)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// attribute text, font width 8 pixels, 16 palette entries, character + 2x4 bit attribute low=FG high=BG index of color
void FASTCODE NOFLASH(DispHstxVgaRender_ATEXT)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// tiles 4x4 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxVgaRender_TILE4_8)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// tiles 8x8 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxVgaRender_TILE8_8)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// tiles 16x16 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxVgaRender_TILE16_8)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// tiles 32x32 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxVgaRender_TILE32_8)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// tiles 4x4 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxVgaRender_TILE4_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// tiles 8x8 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxVgaRender_TILE8_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// tiles 16x16 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxVgaRender_TILE16_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// tiles 32x32 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxVgaRender_TILE32_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// HSTX RLE compression 15-bit (DISPHSTX_FORMAT_HSTX_15)
void FASTCODE NOFLASH(DispHstxVgaRender_HSTX_15)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// HSTX RLE compression 16-bit (DISPHSTX_FORMAT_HSTX_16)
void FASTCODE NOFLASH(DispHstxVgaRender_HSTX_16)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// repeated pattern, 8-bit format, width derived from pitch, height from fonth
void FASTCODE NOFLASH(DispHstxVgaRender_Pat_8)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// RLE compression, 8-bit format (font=line offsets; slot width must match image width)
void FASTCODE NOFLASH(DispHstxVgaRender_Rle8)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// DISPHSTX_FORMAT_ATTR1_PAL, attribute compression 1 (to 28%), 4-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxVgaRender_Attr1)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// DISPHSTX_FORMAT_ATTR2_PAL, attribute compression 2 (to 37%), 4-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxVgaRender_Attr2)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// DISPHSTX_FORMAT_ATTR3_PAL, attribute compression 3 (to 56%), 4-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxVgaRender_Attr3)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// DISPHSTX_FORMAT_ATTR4_PAL, attribute compression 4 (to 75%), 4-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxVgaRender_Attr4)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// DISPHSTX_FORMAT_ATTR5_PAL, attribute compression 5 (to 15%), 8-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxVgaRender_Attr5)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// DISPHSTX_FORMAT_ATTR6_PAL, attribute compression 6 (to 25%), 8-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxVgaRender_Attr6)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// DISPHSTX_FORMAT_ATTR7_PAL, attribute compression 7 (to 31%), 8-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxVgaRender_Attr7)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// DISPHSTX_FORMAT_ATTR8_PAL, attribute compression 8 (to 50%), 8-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxVgaRender_Attr8)(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

#else // DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)

#define DispHstxVgaRender_No NULL
#define DispHstxVgaRender_1 NULL
#define DispHstxVgaRender_2 NULL
#define DispHstxVgaRender_3 NULL
#define DispHstxVgaRender_4 NULL
#define DispHstxVgaRender_6 NULL
#define DispHstxVgaRender_8 NULL
#define DispHstxVgaRender_12 NULL
#define DispHstxVgaRender_15 NULL
#define DispHstxVgaRender_16 NULL
#define DispHstxVgaRender_COL NULL
#define DispHstxVgaRender_MTEXT NULL
#define DispHstxVgaRender_ATEXT NULL
#define DispHstxVgaRender_TILE4_8 NULL
#define DispHstxVgaRender_TILE8_8 NULL
#define DispHstxVgaRender_TILE16_8 NULL
#define DispHstxVgaRender_TILE32_8 NULL
#define DispHstxVgaRender_TILE4_8_PAL NULL
#define DispHstxVgaRender_TILE8_8_PAL NULL
#define DispHstxVgaRender_TILE16_8_PAL NULL
#define DispHstxVgaRender_TILE32_8_PAL NULL
#define DispHstxVgaRender_HSTX_15 NULL
#define DispHstxVgaRender_HSTX_16 NULL
#define DispHstxVgaRender_Pat_8 NULL
#define DispHstxVgaRender_Rle8 NULL
#define DispHstxVgaRender_Attr1 NULL
#define DispHstxVgaRender_Attr2 NULL
#define DispHstxVgaRender_Attr3 NULL
#define DispHstxVgaRender_Attr4 NULL
#define DispHstxVgaRender_Attr5 NULL
#define DispHstxVgaRender_Attr6 NULL
#define DispHstxVgaRender_Attr7 NULL
#define DispHstxVgaRender_Attr8 NULL

#endif // DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)

#ifdef __cplusplus
}
#endif

#endif // _DISPHSTX_VGA_RENDER_H

#endif // USE_DISPHSTX
