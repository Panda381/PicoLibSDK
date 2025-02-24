
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                     HSTX DVI driver - render functions
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

#ifndef _DISPHSTX_DVI_RENDER_H
#define _DISPHSTX_DVI_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)

// do-nothing render
void FASTCODE NOFLASH(DispHstxDviRender_No)(sDispHstxVSlot* slot, int line, u32* cmd);

// default render - read from frame buffer (CPU load 2%)
void FASTCODE NOFLASH(DispHstxDviRender_Default)(sDispHstxVSlot* slot, int line, u32* cmd);

// 12 bits per pixel
void FASTCODE NOFLASH(DispHstxDviRender_12)(sDispHstxVSlot* slot, int line, u32* cmd);

// 1 bit per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_1_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// 2 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_2_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// 3 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_3_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// 4 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_4_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// 6 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_6_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// 8 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// multiply color lines (given as palette color RGB565 per every line, color index modulo fonth)
void FASTCODE NOFLASH(DispHstxDviRender_COL)(sDispHstxVSlot* slot, int line, u32* cmd);

// monocolor text, font width 8 pixels, colors of every row given as 2 palette colors RGB565 (BG,FG)
void FASTCODE NOFLASH(DispHstxDviRender_MTEXT)(sDispHstxVSlot* slot, int line, u32* cmd);

// attribute text, font width 8 pixels, 16 palette entries, character + 2x4 bit attribute low=FG high=BG index of color
void FASTCODE NOFLASH(DispHstxDviRender_ATEXT)(sDispHstxVSlot* slot, int line, u32* cmd);

// tiles 4x4 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxDviRender_TILE4_8)(sDispHstxVSlot* slot, int line, u32* cmd);

// tiles 8x8 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxDviRender_TILE8_8)(sDispHstxVSlot* slot, int line, u32* cmd);

// tiles 16x16 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxDviRender_TILE16_8)(sDispHstxVSlot* slot, int line, u32* cmd);

// tiles 32x32 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxDviRender_TILE32_8)(sDispHstxVSlot* slot, int line, u32* cmd);

// tiles 4x4 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_TILE4_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// tiles 8x8 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_TILE8_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// tiles 16x16 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_TILE16_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// tiles 32x32 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_TILE32_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// HSTX RLE compression (DISPHSTX_FORMAT_HSTX_15, DISPHSTX_FORMAT_HSTX_16)
void FASTCODE NOFLASH(DispHstxDviRender_HSTX)(sDispHstxVSlot* slot, int line, u32* cmd);

// repeated pattern, 8-bit format RGB332, width derived from pitch, height from fonth
void FASTCODE NOFLASH(DispHstxDviRender_Pat_8)(sDispHstxVSlot* slot, int line, u32* cmd);

// repeated pattern, 8-bit format, paletted colors, width derived from pitch, height from fonth
void FASTCODE NOFLASH(DispHstxDviRender_Pat_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// RLE compression, 8-bit format (font=line offsets; slot width must match image width)
void FASTCODE NOFLASH(DispHstxDviRender_Rle8)(sDispHstxVSlot* slot, int line, u32* cmd);

// RLE compression, 8-bit format, paletted (font=line offsets; slot width must match image width)
void FASTCODE NOFLASH(DispHstxDviRender_Rle8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// DISPHSTX_FORMAT_ATTR1_PAL, attribute compression 1 (to 28%), 4-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr1_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// DISPHSTX_FORMAT_ATTR2_PAL, attribute compression 2 (to 37%), 4-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr2_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// DISPHSTX_FORMAT_ATTR3_PAL, attribute compression 3 (to 56%), 4-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr3_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// DISPHSTX_FORMAT_ATTR4_PAL, attribute compression 4 (to 75%), 4-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr4_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// DISPHSTX_FORMAT_ATTR5_PAL, attribute compression 5 (to 15%), 8-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr5_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// DISPHSTX_FORMAT_ATTR6_PAL, attribute compression 6 (to 25%), 8-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr6_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// DISPHSTX_FORMAT_ATTR7_PAL, attribute compression 7 (to 31%), 8-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr7_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

// DISPHSTX_FORMAT_ATTR8_PAL, attribute compression 8 (to 50%), 8-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd);

#else // DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)

#define DispHstxDviRender_No NULL
#define DispHstxDviRender_Default NULL
#define DispHstxDviRender_12 NULL
#define DispHstxDviRender_1_PAL NULL
#define DispHstxDviRender_2_PAL NULL
#define DispHstxDviRender_3_PAL NULL
#define DispHstxDviRender_4_PAL NULL
#define DispHstxDviRender_6_PAL NULL
#define DispHstxDviRender_8_PAL NULL
#define DispHstxDviRender_COL NULL
#define DispHstxDviRender_MTEXT NULL
#define DispHstxDviRender_ATEXT NULL
#define DispHstxDviRender_TILE4_8 NULL
#define DispHstxDviRender_TILE8_8 NULL
#define DispHstxDviRender_TILE16_8 NULL
#define DispHstxDviRender_TILE32_8 NULL
#define DispHstxDviRender_TILE4_8_PAL NULL
#define DispHstxDviRender_TILE8_8_PAL NULL
#define DispHstxDviRender_TILE16_8_PAL NULL
#define DispHstxDviRender_TILE32_8_PAL NULL
#define DispHstxDviRender_HSTX NULL
#define DispHstxDviRender_Pat_8 NULL
#define DispHstxDviRender_Pat_8_PAL NULL
#define DispHstxDviRender_Rle8 NULL
#define DispHstxDviRender_Rle8_PAL NULL
#define DispHstxDviRender_Attr1_PAL NULL
#define DispHstxDviRender_Attr2_PAL NULL
#define DispHstxDviRender_Attr3_PAL NULL
#define DispHstxDviRender_Attr4_PAL NULL
#define DispHstxDviRender_Attr5_PAL NULL
#define DispHstxDviRender_Attr6_PAL NULL
#define DispHstxDviRender_Attr7_PAL NULL
#define DispHstxDviRender_Attr8_PAL NULL

#endif // DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)

#ifdef __cplusplus
}
#endif

#endif // _DISPHSTX_DVI_RENDER_H

#endif // USE_DISPHSTX
