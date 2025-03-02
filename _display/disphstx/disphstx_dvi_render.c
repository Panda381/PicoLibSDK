
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                     HSTX DVI driver - render C functions
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

// use C-alternative
#if !(DISPHSTX_ARM_ASM && !RISCV) && !(DISPHSTX_RISCV_ASM && RISCV) && DISPHSTX_USE_DVI

// do-nothing render (DISPHSTX_FORMAT_NONE)
void FASTCODE NOFLASH(DispHstxDviRender_No)(sDispHstxVSlot* slot, int line, u32* cmd)
{

}

// default render - read from frame buffer (DISPHSTX_FORMAT_1, DISPHSTX_FORMAT_2*, DISPHSTX_FORMAT_3,
//   DISPHSTX_FORMAT_4, DISPHSTX_FORMAT_6, DISPHSTX_FORMAT_8, DISPHSTX_FORMAT_15, DISPHSTX_FORMAT_16)
void FASTCODE NOFLASH(DispHstxDviRender_Default)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	*cmd = (u32)&((u8*)slot->buf)[line * slot->pitch];
}

#if DISPHSTX_USE_FORMAT_12
// 12 bits per pixel
void FASTCODE NOFLASH(DispHstxDviRender_12)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u32* src = (const u32*)&slot->buf[line * slot->pitch];

	int i;
	u32 k1, k2, k3;

//	[2222 2222 1111 1111 1111 0000 0000 0000]
//	[5555 4444 4444 4444 3333 3333 3333 2222]
//	[7777 7777 7777 6666 6666 6666 5555 5555]

	for (i = slot->w/8; i > 0; i--)
	{
		k1 = src[0];
		dst[0] = (u16)k1;
		dst[1] = (u16)(k1 >> 12);

		k2 = src[1];
		dst[2] = (u16)((k1 >> 24) | (k2 << 8));
		dst[3] = (u16)(k2 >> 4);
		dst[4] = (u16)(k2 >> 16);

		k3 = src[2];
		dst[5] = (u16)((k2 >> 28) | (k3 << 4));
		dst[6] = (u16)(k3 >> 8);
		dst[7] = (u16)(k3 >> 20);

		src += 3;
		dst += 8;
	}

	i = slot->w & 7;
	if (i > 0)
	{
		k1 = src[0];
		dst[0] = (u16)k1;

		if (--i != 0)
		{
			dst[1] = (u16)(k1 >> 12);

			if (--i != 0)
			{
				k2 = src[1];
				dst[2] = (u16)((k1 >> 24) | (k2 << 8));

				if (--i != 0)
				{
					dst[3] = (u16)(k2 >> 4);

					if (--i != 0)
					{
						dst[4] = (u16)(k2 >> 16);

						if (--i != 0)
						{
							k3 = src[2];
							dst[5] = (u16)((k2 >> 28) | (k3 << 4));

							if (--i != 0)
							{
								dst[6] = (u16)(k3 >> 8);

								if (--i != 0)
								{
									dst[7] = (u16)(k3 >> 20);
								}
							}
						}
					}
				}
			}
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_1_PAL
// 1 bit per pixel, paletted colors (DISPHSTX_FORMAT_1_PAL)
void FASTCODE NOFLASH(DispHstxDviRender_1_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u32* dst = (u32*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;

	u32 map2[4];
	u16 col0 = map[0];
	u16 col1 = map[1];
	map2[0] = col0 + ((u32)col0 << 16);
	map2[1] = col0 + ((u32)col1 << 16);
	map2[2] = col1 + ((u32)col0 << 16);
	map2[3] = col1 + ((u32)col1 << 16);

	int i;
	u32 k;
	for (i = slot->w/32; i > 0; i--)
	{
		k = *src++;
		dst[0] = map2[(k >> 6) & 3];
		dst[1] = map2[(k >> 4) & 3];
		dst[2] = map2[(k >> 2) & 3];
		dst[3] = map2[k & 3];

		dst[4] = map2[(k >> 14) & 3];
		dst[5] = map2[(k >> 12) & 3];
		dst[6] = map2[(k >> 10) & 3];
		dst[7] = map2[(k >> 8) & 3];

		dst[8] = map2[(k >> 22) & 3];
		dst[9] = map2[(k >> 20) & 3];
		dst[10] = map2[(k >> 18) & 3];
		dst[11] = map2[(k >> 16) & 3];

		dst[12] = map2[k >> 30];
		dst[13] = map2[(k >> 28) & 3];
		dst[14] = map2[(k >> 26) & 3];
		dst[15] = map2[(k >> 24) & 3];

		dst += 16;
	}

	i = slot->w & 0x1f;
	if (i > 0)
	{
		k = Endian(*src);
		u16* dst2 = (u16*)dst;
		for (; i > 0; i--)
		{
			*dst2++ = map[k >> 31];
			k <<= 1;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_2_PAL
// 2 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_2_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	int i;
	u32 k;
	for (i = slot->w/16; i > 0; i--)
	{
		k = *src++;
		dst[0] = map[(k >> 6) & 3];
		dst[1] = map[(k >> 4) & 3];
		dst[2] = map[(k >> 2) & 3];
		dst[3] = map[k & 3];

		dst[4] = map[(k >> 14) & 3];
		dst[5] = map[(k >> 12) & 3];
		dst[6] = map[(k >> 10) & 3];
		dst[7] = map[(k >> 8) & 3];

		dst[8] = map[(k >> 22) & 3];
		dst[9] = map[(k >> 20) & 3];
		dst[10] = map[(k >> 18) & 3];
		dst[11] = map[(k >> 16) & 3];

		dst[12] = map[k >> 30];
		dst[13] = map[(k >> 28) & 3];
		dst[14] = map[(k >> 26) & 3];
		dst[15] = map[(k >> 24) & 3];

		dst += 16;
	}

	i = slot->w & 0x0f;
	if (i > 0)
	{
		k = Endian(*src);
		for (; i > 0; i--)
		{
			*dst++ = map[k >> 30];
			k <<= 2;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_3_PAL
// 3 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_3_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	int i;
	u32 k;

	int w = slot->w;
	i = w/20;
	w -= i*20;

	for (; i > 0; i--)
	{
		k = src[0];
		dst[0] = map[k & 0x7];
		dst[1] = map[(k >> 1*3) & 0x07];
		dst[2] = map[(k >> 2*3) & 0x07];
		dst[3] = map[(k >> 3*3) & 0x07];
		dst[4] = map[(k >> 4*3) & 0x07];
		dst[5] = map[(k >> 5*3) & 0x07];
		dst[6] = map[(k >> 6*3) & 0x07];
		dst[7] = map[(k >> 7*3) & 0x07];
		dst[8] = map[(k >> 8*3) & 0x07];
		dst[9] = map[(k >> 9*3) & 0x07];

		k = src[1];
		dst[10] = map[k & 0x7];
		dst[11] = map[(k >> 1*3) & 0x07];
		dst[12] = map[(k >> 2*3) & 0x07];
		dst[13] = map[(k >> 3*3) & 0x07];
		dst[14] = map[(k >> 4*3) & 0x07];
		dst[15] = map[(k >> 5*3) & 0x07];
		dst[16] = map[(k >> 6*3) & 0x07];
		dst[17] = map[(k >> 7*3) & 0x07];
		dst[18] = map[(k >> 8*3) & 0x07];
		dst[19] = map[(k >> 9*3) & 0x07];

		src += 2;
		dst += 20;
	}

	if (w > 0)
	{
		k = src[0];
		if (w >= 10)
		{
			dst[0] = map[k & 0x7];
			dst[1] = map[(k >> 1*3) & 0x07];
			dst[2] = map[(k >> 2*3) & 0x07];
			dst[3] = map[(k >> 3*3) & 0x07];
			dst[4] = map[(k >> 4*3) & 0x07];
			dst[5] = map[(k >> 5*3) & 0x07];
			dst[6] = map[(k >> 6*3) & 0x07];
			dst[7] = map[(k >> 7*3) & 0x07];
			dst[8] = map[(k >> 8*3) & 0x07];
			dst[9] = map[(k >> 9*3) & 0x07];
			k = src[1];
			w -= 10;
			dst += 10;
		}

		for (; w > 0; w--)
		{
			*dst++ = map[k & 0x07];
			k >>= 3;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_4_PAL
// 4 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_4_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	int i;
	u32 k;
	for (i = slot->w/8; i > 0; i--)
	{
		k = *src++;
		dst[0] = map[(k >> 4) & 0x0f];
		dst[1] = map[k & 0xf];

		dst[2] = map[(k >> 12) & 0x0f];
		dst[3] = map[(k >> 8) & 0x0f];

		dst[4] = map[(k >> 20) & 0x0f];
		dst[5] = map[(k >> 16) & 0x0f];

		dst[6] = map[k >> 28];
		dst[7] = map[(k >> 24) & 0x0f];

		dst += 8;
	}

	i = slot->w & 7;
	if (i > 0)
	{
		k = Endian(*src);
		for (; i > 0; i--)
		{
			*dst++ = map[k >> 28];
			k <<= 4;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_6_PAL
// 6 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_6_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	int i;
	u32 k;

	int w = slot->w;
	i = w/20;
	w -= i*20;

	for (; i > 0; i--)
	{
		k = src[0];
		dst[0] = map[k & 0x3f];
		dst[1] = map[(k >> 1*6) & 0x3f];
		dst[2] = map[(k >> 2*6) & 0x3f];
		dst[3] = map[(k >> 3*6) & 0x3f];
		dst[4] = map[(k >> 4*6) & 0x3f];

		k = src[1];
		dst[5] = map[k & 0x3f];
		dst[6] = map[(k >> 1*6) & 0x3f];
		dst[7] = map[(k >> 2*6) & 0x3f];
		dst[8] = map[(k >> 3*6) & 0x3f];
		dst[9] = map[(k >> 4*6) & 0x3f];

		k = src[2];
		dst[10] = map[k & 0x3f];
		dst[11] = map[(k >> 1*6) & 0x3f];
		dst[12] = map[(k >> 2*6) & 0x3f];
		dst[13] = map[(k >> 3*6) & 0x3f];
		dst[14] = map[(k >> 4*6) & 0x3f];

		k = src[3];
		dst[15] = map[k & 0x3f];
		dst[16] = map[(k >> 1*6) & 0x3f];
		dst[17] = map[(k >> 2*6) & 0x3f];
		dst[18] = map[(k >> 3*6) & 0x3f];
		dst[19] = map[(k >> 4*6) & 0x3f];

		src += 4;
		dst += 20;
	}

	if (w > 0)
	{
		k = *src;
		while (w >= 5)
		{
			dst[0] = map[k & 0x3f];
			dst[1] = map[(k >> 1*6) & 0x3f];
			dst[2] = map[(k >> 2*6) & 0x3f];
			dst[3] = map[(k >> 3*6) & 0x3f];
			dst[4] = map[(k >> 4*6) & 0x3f];

			src++;
			k = *src;
			dst += 5;
			w -= 5;
		}
			
		for (; w > 0; w--)
		{
			*dst++ = map[k & 0x3f];
			k >>= 6;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_8_PAL
// 8 bits per pixel, paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	int i;
	u32 k;
	for (i = slot->w/16; i > 0; i--)
	{
		k = src[0];
		dst[0] = map[k & 0xff];
		dst[1] = map[(k >> 8) & 0xff];
		dst[2] = map[(k >> 16) & 0xff];
		dst[3] = map[k >> 24];

		k = src[1];
		dst[4] = map[k & 0xff];
		dst[5] = map[(k >> 8) & 0xff];
		dst[6] = map[(k >> 16) & 0xff];
		dst[7] = map[k >> 24];

		k = src[2];
		dst[8] = map[k & 0xff];
		dst[9] = map[(k >> 8) & 0xff];
		dst[10] = map[(k >> 16) & 0xff];
		dst[11] = map[k >> 24];

		k = src[3];
		dst[12] = map[k & 0xff];
		dst[13] = map[(k >> 8) & 0xff];
		dst[14] = map[(k >> 16) & 0xff];
		dst[15] = map[k >> 24];

		src += 4;
		dst += 16;
	}

	i = slot->w & 0x0f;
	const u8* src2 = (const u8*)src;
	for (; i > 0; i--)
	{
		*dst++ = map[*src2++];
	}
}
#endif

#if DISPHSTX_USE_FORMAT_COL
// multiply color lines (given as palette color RGB565 per every line, color index modulo fonth)
void FASTCODE NOFLASH(DispHstxDviRender_COL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	u16* dst = (u16*)*cmd; // pointer to render buffer
	u16 col = slot->pal[line % slot->fonth];
	dst[0] = col;
}
#endif

#if DISPHSTX_USE_FORMAT_MTEXT
// monocolor text, font width 8 pixels, colors of every row given as 2 palette colors RGB565 (BG,FG)
void FASTCODE NOFLASH(DispHstxDviRender_MTEXT)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	int fonth = slot->fonth;
	int row = line/fonth;
	const u8* src = &((u8*)slot->buf)[row * slot->pitch];
	const u16* map = &((u16*)slot->pal)[row * 2];
	u32* dst = (u32*)*cmd; // pointer to render buffer
	int subline = line - row*fonth;
	const u8* font = &slot->font[subline * 256];
	int i;
	u8 k;

	int curpos = 0;
	if ((row == slot->currow) && (subline >= slot->curbeg) && (subline <= slot->curend)
		&& (((Time() >> slot->curspeed) & 1) != 0))
	{
		curpos = slot->curpos + 1;
	}

	u32 map2[4];
	u16 col0 = map[0];
	u16 col1 = map[1];
	map2[0] = col0 + ((u32)col0 << 16);
	map2[1] = col0 + ((u32)col1 << 16);
	map2[2] = col1 + ((u32)col0 << 16);
	map2[3] = col1 + ((u32)col1 << 16);

	for (i = slot->w/8; i > 0; i--)
	{
		k = font[*src++];
		curpos--;
		if (curpos == 0) k = ~k;
		dst[0] = map2[k >> 6];
		dst[1] = map2[(k >> 4) & 3];
		dst[2] = map2[(k >> 2) & 3];
		dst[3] = map2[k & 3];

		dst += 4;
	}

	i = slot->w & 7;
	if (i > 0)
	{
		u16* dst2 = (u16*)dst;
		u32 k2 = font[*src];
		curpos--;
		if (curpos == 0) k2 = ~k2;
		for (; i > 0; i--)
		{
			*dst2++ = map[(k2 >> 7) & 1];
			k2 <<= 1;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_ATEXT
// attribute text, font width 8 pixels, 16 palette entries, character + 2x4 bit attribute low=FG high=BG index of color
void FASTCODE NOFLASH(DispHstxDviRender_ATEXT)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	int fonth = slot->fonth;
	int row = line/fonth;
	const u32* src = (u32*)&((u8*)slot->buf)[row * slot->pitch];
	u32* dst = (u32*)*cmd; // pointer to render buffer
	const u16* map = (u16*)slot->pal;
	int subline = line - row*slot->fonth;
	const u8* font = &slot->font[subline * 256];
	u32 map2[4];
	u16 map3[2];
	int i;
	u8 k;
	u32 kk;
	u16 col0, col1;

	int curpos = 0;
	if ((row == slot->currow) && (subline >= slot->curbeg) && (subline <= slot->curend)
		&& (((Time() >> slot->curspeed) & 1) != 0))
	{
		curpos = slot->curpos + 1;
	}

	// big loop by 2 characters, 16 pixels
	for (i = slot->w/16; i > 0; i--)
	{
		kk = *src++;

		// 1st character
		col0 = map[(kk >> 12) & 0x0f];	// background
		col1 = map[(kk >> 8) & 0x0f];	// foreground
		map2[0] = col0 + ((u32)col0 << 16);
		map2[1] = col0 + ((u32)col1 << 16);
		map2[2] = col1 + ((u32)col0 << 16);
		map2[3] = col1 + ((u32)col1 << 16);

		k = font[kk & 0xff];
		curpos--;
		if (curpos == 0) k = ~k;

		dst[0] = map2[k >> 6];
		dst[1] = map2[(k >> 4) & 3];
		dst[2] = map2[(k >> 2) & 3];
		dst[3] = map2[k & 3];

		// 2nd character
		col0 = map[kk >> 28];	// background
		col1 = map[(kk >> 24) & 0x0f];	// foreground
		map2[0] = col0 + ((u32)col0 << 16);
		map2[1] = col0 + ((u32)col1 << 16);
		map2[2] = col1 + ((u32)col0 << 16);
		map2[3] = col1 + ((u32)col1 << 16);

		k = font[(kk >> 16) & 0xff];
		curpos--;
		if (curpos == 0) k = ~k;

		dst[4] = map2[k >> 6];
		dst[5] = map2[(k >> 4) & 3];
		dst[6] = map2[(k >> 2) & 3];
		dst[7] = map2[k & 3];

		dst += 8;
	}

	i = slot->w & 0x0f;
	if (i > 0)
	{
		kk = *src;
		u16* dst2 = (u16*)dst;

		map3[0] = map[(kk >> 12) & 0x0f];	// background
		map3[1] = map[(kk >> 8) & 0x0f];	// foreground
		k = font[kk & 0xff];
		curpos--;
		if (curpos == 0) k = ~k;

		if (i >= 8)
		{
			dst2[0] = map3[k >> 7];
			dst2[1] = map3[(k >> 6) & 1];
			dst2[2] = map3[(k >> 5) & 1];
			dst2[3] = map3[(k >> 4) & 1];
			dst2[4] = map3[(k >> 3) & 1];
			dst2[5] = map3[(k >> 2) & 1];
			dst2[6] = map3[(k >> 1) & 1];
			dst2[7] = map3[k & 1];

			i -= 8;
			if (i > 0)
			{
				map3[0] = map[kk >> 28];		// background
				map3[1] = map[(kk >> 24) & 0x0f];	// foreground
				k = font[(kk >> 16) & 0xff];
				curpos--;
				if (curpos == 0) k = ~k;
				dst2 += 8;
			}
		}

		for (; i > 0; i--)
		{
			*dst2++ = map3[k >> 7];
			k <<= 1;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8
// tiles 4x4 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxDviRender_TILE4_8)(sDispHstxVSlot* slot, int line, u32* cmd)
{
#define RENDER_TILEW_BIT	2
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

	int row = (line >> RENDER_TILEW_BIT); // row index
	const u32* src = (u32*)&((u8*)slot->buf)[row * slot->pitch]; // pointer to source
	u32* dst = (u32*)*cmd; // pointer to render buffer
	int subline = line & (RENDER_TILEW-1); // subline index in the tile
	int fonth = slot->fonth; // tile pitch
	int d = RENDER_TILEWB; // tile delta in case of horizontal row
	if (fonth <= d) // vertical mode
	{
		fonth = d; // tile pitch
		d = RENDER_TILEWB*RENDER_TILEW; // tile delta
	}
	const u8* tiles = &slot->font[subline * fonth]; // pointer to line in tiles
	u32 k;
	const u32* s;
	int i;

	for (i = slot->w >> (RENDER_TILEW_BIT+2); i > 0; i--)
	{
		k = *src++; // get 4 tile indices

		s = (u32*)&tiles[(k & 0xff) * d];
		dst[0] = s[0];

		s = (u32*)&tiles[((k >> 8) & 0xff) * d];
		dst[1] = s[0];

		s = (u32*)&tiles[((k >> 16) & 0xff) * d];
		dst[2] = s[0];

		s = (u32*)&tiles[(k >> 24) * d];
		dst[3] = s[0];

		dst += 4;
	}

	i = slot->w & (4*RENDER_TILEW-1);
	if (i > 0)
	{
		const u8* src2 = (const u8*)src;
		int j = i >> RENDER_TILEW_BIT;
		for (; j > 0; j--)
		{
			k = *src2++;
			s = (u32*)&tiles[k * d];
			dst[0] = s[0];
			dst += 1;
		}

		i &= RENDER_TILEW-1;
		if (i > 0)
		{
			k = *src2;
			src2 = &tiles[k * d];
			u8* d2 = (u8*)dst;
			for (; i > 0; i--) *d2++ = *src2++;
		}
	}
#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
}
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8
// tiles 8x8 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxDviRender_TILE8_8)(sDispHstxVSlot* slot, int line, u32* cmd)
{
#define RENDER_TILEW_BIT	3
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

	int row = (line >> RENDER_TILEW_BIT); // row index
	const u32* src = (u32*)&((u8*)slot->buf)[row * slot->pitch]; // pointer to source
	u32* dst = (u32*)*cmd; // pointer to render buffer
	int subline = line & (RENDER_TILEW-1); // subline index in the tile
	int fonth = slot->fonth; // tile pitch
	int d = RENDER_TILEWB; // tile delta in case of horizontal row
	if (fonth <= d) // vertical mode
	{
		fonth = d; // tile pitch
		d = RENDER_TILEWB*RENDER_TILEW; // tile delta
	}
	const u8* tiles = &slot->font[subline * fonth]; // pointer to line in tiles
	u32 k;
	const u32* s;
	int i;

	for (i = slot->w >> (RENDER_TILEW_BIT+2); i > 0; i--)
	{
		k = *src++; // get 4 tile indices

		s = (u32*)&tiles[(k & 0xff) * d];
		dst[0] = s[0];
		dst[1] = s[1];

		s = (u32*)&tiles[((k >> 8) & 0xff) * d];
		dst[2] = s[0];
		dst[3] = s[1];

		s = (u32*)&tiles[((k >> 16) & 0xff) * d];
		dst[4] = s[0];
		dst[5] = s[1];

		s = (u32*)&tiles[(k >> 24) * d];
		dst[6] = s[0];
		dst[7] = s[1];

		dst += 8;
	}

	i = slot->w & (4*RENDER_TILEW-1);
	if (i > 0)
	{
		const u8* src2 = (const u8*)src;
		int j = i >> RENDER_TILEW_BIT;
		for (; j > 0; j--)
		{
			k = *src2++;
			s = (u32*)&tiles[k * d];
			dst[0] = s[0];
			dst[1] = s[1];
			dst += 2;
		}

		i &= RENDER_TILEW-1;
		if (i > 0)
		{
			k = *src2;
			src2 = &tiles[k * d];
			u8* d2 = (u8*)dst;
			for (; i > 0; i--) *d2++ = *src2++;
		}
	}
#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
}
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8
// tiles 16x16 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxDviRender_TILE16_8)(sDispHstxVSlot* slot, int line, u32* cmd)
{
#define RENDER_TILEW_BIT	4
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

	int row = (line >> RENDER_TILEW_BIT); // row index
	const u32* src = (u32*)&((u8*)slot->buf)[row * slot->pitch]; // pointer to source
	u32* dst = (u32*)*cmd; // pointer to render buffer
	int subline = line & (RENDER_TILEW-1); // subline index in the tile
	int fonth = slot->fonth; // tile pitch
	int d = RENDER_TILEWB; // tile delta in case of horizontal row
	if (fonth <= d) // vertical mode
	{
		fonth = d; // tile pitch
		d = RENDER_TILEWB*RENDER_TILEW; // tile delta
	}
	const u8* tiles = &slot->font[subline * fonth]; // pointer to line in tiles
	u32 k;
	const u32* s;
	int i;

	for (i = slot->w >> (RENDER_TILEW_BIT+2); i > 0; i--)
	{
		k = *src++; // get 4 tile indices

		s = (u32*)&tiles[(k & 0xff) * d];
		dst[0] = s[0];
		dst[1] = s[1];
		dst[2] = s[2];
		dst[3] = s[3];

		s = (u32*)&tiles[((k >> 8) & 0xff) * d];
		dst[4] = s[0];
		dst[5] = s[1];
		dst[6] = s[2];
		dst[7] = s[3];

		s = (u32*)&tiles[((k >> 16) & 0xff) * d];
		dst[8] = s[0];
		dst[9] = s[1];
		dst[10] = s[2];
		dst[11] = s[3];

		s = (u32*)&tiles[(k >> 24) * d];
		dst[12] = s[0];
		dst[13] = s[1];
		dst[14] = s[2];
		dst[15] = s[3];

		dst += 16;
	}

	i = slot->w & (4*RENDER_TILEW-1);
	if (i > 0)
	{
		const u8* src2 = (const u8*)src;
		int j = i >> RENDER_TILEW_BIT;
		for (; j > 0; j--)
		{
			k = *src2++;
			s = (u32*)&tiles[k * d];
			dst[0] = s[0];
			dst[1] = s[1];
			dst[2] = s[2];
			dst[3] = s[3];
			dst += 4;
		}

		i &= RENDER_TILEW-1;
		if (i > 0)
		{
			k = *src2;
			src2 = &tiles[k * d];
			u8* d2 = (u8*)dst;
			for (; i > 0; i--) *d2++ = *src2++;
		}
	}
#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
}
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8
// tiles 32x32 pixels, 8-bit format RGB332
void FASTCODE NOFLASH(DispHstxDviRender_TILE32_8)(sDispHstxVSlot* slot, int line, u32* cmd)
{
#define RENDER_TILEW_BIT	5
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

	int row = (line >> RENDER_TILEW_BIT); // row index
	const u32* src = (u32*)&((u8*)slot->buf)[row * slot->pitch]; // pointer to source
	u32* dst = (u32*)*cmd; // pointer to render buffer
	int subline = line & (RENDER_TILEW-1); // subline index in the tile
	int fonth = slot->fonth; // tile pitch
	int d = RENDER_TILEWB; // tile delta in case of horizontal row
	if (fonth <= d) // vertical mode
	{
		fonth = d; // tile pitch
		d = RENDER_TILEWB*RENDER_TILEW; // tile delta
	}
	const u8* tiles = &slot->font[subline * fonth]; // pointer to line in tiles
	u32 k;
	const u32* s;
	int i;

	for (i = slot->w >> (RENDER_TILEW_BIT+2); i > 0; i--)
	{
		k = *src++; // get 4 tile indices

		s = (u32*)&tiles[(k & 0xff) * d];
		dst[0] = s[0];
		dst[1] = s[1];
		dst[2] = s[2];
		dst[3] = s[3];
		dst[4] = s[4];
		dst[5] = s[5];
		dst[6] = s[6];
		dst[7] = s[7];

		s = (u32*)&tiles[((k >> 8) & 0xff) * d];
		dst[8] = s[0];
		dst[9] = s[1];
		dst[10] = s[2];
		dst[11] = s[3];
		dst[12] = s[4];
		dst[13] = s[5];
		dst[14] = s[6];
		dst[15] = s[7];

		s = (u32*)&tiles[((k >> 16) & 0xff) * d];
		dst[16] = s[0];
		dst[17] = s[1];
		dst[18] = s[2];
		dst[19] = s[3];
		dst[20] = s[4];
		dst[21] = s[5];
		dst[22] = s[6];
		dst[23] = s[7];

		s = (u32*)&tiles[(k >> 24) * d];
		dst[24] = s[0];
		dst[25] = s[1];
		dst[26] = s[2];
		dst[27] = s[3];
		dst[28] = s[4];
		dst[29] = s[5];
		dst[30] = s[6];
		dst[31] = s[7];

		dst += 32;
	}

	i = slot->w & (4*RENDER_TILEW-1);
	if (i > 0)
	{
		const u8* src2 = (const u8*)src;
		int j = i >> RENDER_TILEW_BIT;
		for (; j > 0; j--)
		{
			k = *src2++;
			s = (u32*)&tiles[k * d];
			dst[0] = s[0];
			dst[1] = s[1];
			dst[2] = s[2];
			dst[3] = s[3];
			dst[4] = s[4];
			dst[5] = s[5];
			dst[6] = s[6];
			dst[7] = s[7];
			dst += 8;
		}

		i &= RENDER_TILEW-1;
		if (i > 0)
		{
			k = *src2;
			src2 = &tiles[k * d];
			u8* d2 = (u8*)dst;
			for (; i > 0; i--) *d2++ = *src2++;
		}
	}
#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
}
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8_PAL
// tiles 4x4 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_TILE4_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
#define RENDER_TILEW_BIT	2
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

	int row = (line >> RENDER_TILEW_BIT); // row index
	const u8* src = &((u8*)slot->buf)[row * slot->pitch]; // pointer to source
	u16* dst = (u16*)*cmd; // pointer to render buffer
	int subline = line & (RENDER_TILEW-1); // subline index in the tile
	int fonth = slot->fonth; // tile pitch
	int d = RENDER_TILEWB; // tile delta in case of horizontal row
	if (fonth <= d) // vertical mode
	{
		fonth = d; // tile pitch
		d = RENDER_TILEWB*RENDER_TILEW; // tile delta
	}
	const u8* tiles = &slot->font[subline * fonth]; // pointer to line in tiles
	const u16* map = slot->pal; // palette mapping
	u8 k;
	u32 kk;
	const u32* s;
	int i;

	for (i = slot->w >> (RENDER_TILEW_BIT+2); i > 0; i--)
	{
		k = src[0]; // get tile index
		s = (u32*)&tiles[k * d];
		kk = s[0];
		dst[0] = map[kk & 0xff];
		dst[1] = map[(kk >> 8) & 0xff];
		dst[2] = map[(kk >> 16) & 0xff];
		dst[3] = map[kk >> 24];

		k = src[1]; // get tile index
		s = (u32*)&tiles[k * d];
		kk = s[0];
		dst[4] = map[kk & 0xff];
		dst[5] = map[(kk >> 8) & 0xff];
		dst[6] = map[(kk >> 16) & 0xff];
		dst[7] = map[kk >> 24];

		k = src[2]; // get tile index
		s = (u32*)&tiles[k * d];
		kk = s[0];
		dst[8] = map[kk & 0xff];
		dst[9] = map[(kk >> 8) & 0xff];
		dst[10] = map[(kk >> 16) & 0xff];
		dst[11] = map[kk >> 24];

		k = src[3]; // get tile index
		s = (u32*)&tiles[k * d];
		kk = s[0];
		dst[12] = map[kk & 0xff];
		dst[13] = map[(kk >> 8) & 0xff];
		dst[14] = map[(kk >> 16) & 0xff];
		dst[15] = map[kk >> 24];

		src += 4;
		dst += 16;
	}

	i = slot->w & (4*RENDER_TILEW-1);
	if (i > 0)
	{
		while (i >= RENDER_TILEW)
		{
			k = *src++; // get tile index
			s = (u32*)&tiles[k * d];
			kk = s[0];
			dst[0] = map[kk & 0xff];
			dst[1] = map[(kk >> 8) & 0xff];
			dst[2] = map[(kk >> 16) & 0xff];
			dst[3] = map[kk >> 24];

			dst += 4;
			i -= RENDER_TILEW;
		}

		if (i > 0)
		{
			k = *src;
			src = &tiles[k * d];
			for (; i > 0; i--) *dst++ = map[*src++];
		}
	}
#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
}
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8_PAL
// tiles 8x8 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_TILE8_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
#define RENDER_TILEW_BIT	3
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

	int row = (line >> RENDER_TILEW_BIT); // row index
	const u8* src = &((u8*)slot->buf)[row * slot->pitch]; // pointer to source
	u16* dst = (u16*)*cmd; // pointer to render buffer
	int subline = line & (RENDER_TILEW-1); // subline index in the tile
	int fonth = slot->fonth; // tile pitch
	int d = RENDER_TILEWB; // tile delta in case of horizontal row
	if (fonth <= d) // vertical mode
	{
		fonth = d; // tile pitch
		d = RENDER_TILEWB*RENDER_TILEW; // tile delta
	}
	const u8* tiles = &slot->font[subline * fonth]; // pointer to line in tiles
	const u16* map = slot->pal; // palette mapping
	u8 k;
	u32 kk;
	const u32* s;
	int i;

	for (i = slot->w >> (RENDER_TILEW_BIT+1); i > 0; i--)
	{
		k = src[0]; // get tile index
		s = (u32*)&tiles[k * d];
		kk = s[0];
		dst[0] = map[kk & 0xff];
		dst[1] = map[(kk >> 8) & 0xff];
		dst[2] = map[(kk >> 16) & 0xff];
		dst[3] = map[kk >> 24];
		kk = s[1];
		dst[4] = map[kk & 0xff];
		dst[5] = map[(kk >> 8) & 0xff];
		dst[6] = map[(kk >> 16) & 0xff];
		dst[7] = map[kk >> 24];

		k = src[1]; // get tile index
		s = (u32*)&tiles[k * d];
		kk = s[0];
		dst[8] = map[kk & 0xff];
		dst[9] = map[(kk >> 8) & 0xff];
		dst[10] = map[(kk >> 16) & 0xff];
		dst[11] = map[kk >> 24];
		kk = s[1];
		dst[12] = map[kk & 0xff];
		dst[13] = map[(kk >> 8) & 0xff];
		dst[14] = map[(kk >> 16) & 0xff];
		dst[15] = map[kk >> 24];

		src += 2;
		dst += 16;
	}

	i = slot->w & (2*RENDER_TILEW-1);
	if (i > 0)
	{
		if (i >= RENDER_TILEW)
		{
			k = *src++; // get tile index
			s = (u32*)&tiles[k * d];
			kk = s[0];
			dst[0] = map[kk & 0xff];
			dst[1] = map[(kk >> 8) & 0xff];
			dst[2] = map[(kk >> 16) & 0xff];
			dst[3] = map[kk >> 24];
			kk = s[1];
			dst[4] = map[kk & 0xff];
			dst[5] = map[(kk >> 8) & 0xff];
			dst[6] = map[(kk >> 16) & 0xff];
			dst[7] = map[kk >> 24];

			dst += 8;

			i -= RENDER_TILEW;
		}

		if (i > 0)
		{
			k = *src;
			src = &tiles[k * d];
			for (; i > 0; i--) *dst++ = map[*src++];
		}
	}
#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
}
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8_PAL
// tiles 16x16 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_TILE16_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
#define RENDER_TILEW_BIT	4
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

	int row = (line >> RENDER_TILEW_BIT); // row index
	const u8* src = &((u8*)slot->buf)[row * slot->pitch]; // pointer to source
	u16* dst = (u16*)*cmd; // pointer to render buffer
	int subline = line & (RENDER_TILEW-1); // subline index in the tile
	int fonth = slot->fonth; // tile pitch
	int d = RENDER_TILEWB; // tile delta in case of horizontal row
	if (fonth <= d) // vertical mode
	{
		fonth = d; // tile pitch
		d = RENDER_TILEWB*RENDER_TILEW; // tile delta
	}
	const u8* tiles = &slot->font[subline * fonth]; // pointer to line in tiles
	const u16* map = slot->pal; // palette mapping
	u8 k;
	u32 kk;
	const u32* s;
	int i;

	for (i = slot->w >> RENDER_TILEW_BIT; i > 0; i--)
	{
		k = *src++; // get tile index
		s = (u32*)&tiles[k * d];
		kk = s[0];
		dst[0] = map[kk & 0xff];
		dst[1] = map[(kk >> 8) & 0xff];
		dst[2] = map[(kk >> 16) & 0xff];
		dst[3] = map[kk >> 24];
		kk = s[1];
		dst[4] = map[kk & 0xff];
		dst[5] = map[(kk >> 8) & 0xff];
		dst[6] = map[(kk >> 16) & 0xff];
		dst[7] = map[kk >> 24];
		kk = s[2];
		dst[8] = map[kk & 0xff];
		dst[9] = map[(kk >> 8) & 0xff];
		dst[10] = map[(kk >> 16) & 0xff];
		dst[11] = map[kk >> 24];
		kk = s[3];
		dst[12] = map[kk & 0xff];
		dst[13] = map[(kk >> 8) & 0xff];
		dst[14] = map[(kk >> 16) & 0xff];
		dst[15] = map[kk >> 24];

		dst += 16;
	}

	i = slot->w & (RENDER_TILEW-1);
	if (i > 0)
	{
		k = *src;
		src = &tiles[k * d];
		for (; i > 0; i--) *dst++ = map[*src++];
	}
#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
}
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8_PAL
// tiles 32x32 pixels, 8-bit paletted colors
void FASTCODE NOFLASH(DispHstxDviRender_TILE32_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
#define RENDER_TILEW_BIT	5
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

	int row = (line >> RENDER_TILEW_BIT); // row index
	const u8* src = &((u8*)slot->buf)[row * slot->pitch]; // pointer to source
	u16* dst = (u16*)*cmd; // pointer to render buffer
	int subline = line & (RENDER_TILEW-1); // subline index in the tile
	int fonth = slot->fonth; // tile pitch
	int d = RENDER_TILEWB; // tile delta in case of horizontal row
	if (fonth <= d) // vertical mode
	{
		fonth = d; // tile pitch
		d = RENDER_TILEWB*RENDER_TILEW; // tile delta
	}
	const u8* tiles = &slot->font[subline * fonth]; // pointer to line in tiles
	const u16* map = slot->pal; // palette mapping
	u8 k;
	u32 kk;
	const u32* s;
	int i;

	for (i = slot->w >> RENDER_TILEW_BIT; i > 0; i--)
	{
		k = *src++; // get tile index
		s = (u32*)&tiles[k * d];
		kk = s[0];
		dst[0] = map[kk & 0xff];
		dst[1] = map[(kk >> 8) & 0xff];
		dst[2] = map[(kk >> 16) & 0xff];
		dst[3] = map[kk >> 24];
		kk = s[1];
		dst[4] = map[kk & 0xff];
		dst[5] = map[(kk >> 8) & 0xff];
		dst[6] = map[(kk >> 16) & 0xff];
		dst[7] = map[kk >> 24];
		kk = s[2];
		dst[8] = map[kk & 0xff];
		dst[9] = map[(kk >> 8) & 0xff];
		dst[10] = map[(kk >> 16) & 0xff];
		dst[11] = map[kk >> 24];
		kk = s[3];
		dst[12] = map[kk & 0xff];
		dst[13] = map[(kk >> 8) & 0xff];
		dst[14] = map[(kk >> 16) & 0xff];
		dst[15] = map[kk >> 24];
		kk = s[4];
		dst[16] = map[kk & 0xff];
		dst[17] = map[(kk >> 8) & 0xff];
		dst[18] = map[(kk >> 16) & 0xff];
		dst[19] = map[kk >> 24];
		kk = s[5];
		dst[20] = map[kk & 0xff];
		dst[21] = map[(kk >> 8) & 0xff];
		dst[22] = map[(kk >> 16) & 0xff];
		dst[23] = map[kk >> 24];
		kk = s[6];
		dst[24] = map[kk & 0xff];
		dst[25] = map[(kk >> 8) & 0xff];
		dst[26] = map[(kk >> 16) & 0xff];
		dst[27] = map[kk >> 24];
		kk = s[7];
		dst[28] = map[kk & 0xff];
		dst[29] = map[(kk >> 8) & 0xff];
		dst[30] = map[(kk >> 16) & 0xff];
		dst[31] = map[kk >> 24];

		dst += 32;
	}

	i = slot->w & (RENDER_TILEW-1);
	if (i > 0)
	{
		k = *src;
		src = &tiles[k * d];
		for (; i > 0; i--) *dst++ = map[*src++];
	}
#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
}
#endif

#if DISPHSTX_USE_FORMAT_HSTX_15 || DISPHSTX_USE_FORMAT_HSTX_16
// HSTX RLE compression (DISPHSTX_FORMAT_HSTX_15, DISPHSTX_FORMAT_HSTX_16)
void FASTCODE NOFLASH(DispHstxDviRender_HSTX)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	// array of line offsets
	u32* font = (u32*)slot->font;

	// get line offset and length
	u32 off = font[line];
	u32 len = font[line+1] - off;

	// data buffer
	u16* buf = (u16*)slot->buf;

	// set data length and address
	cmd[-1] = len;
	cmd[0] = (u32)(buf + off);
}
#endif

#if DISPHSTX_USE_FORMAT_PAT_8
// repeated pattern, 8-bit format RGB332, width derived from pitch, height from fonth
void FASTCODE NOFLASH(DispHstxDviRender_Pat_8)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	line = line % slot->fonth;
	int pitch = slot->pitch;
	const u32* src0 = (u32*)&((u8*)slot->buf)[line * pitch];
	u32* dst = (u32*)*cmd; // pointer to render buffer
	int i, j;
	u32 k;
	int w = slot->w;
	const u32* src;
	u8* dst2;
	const u8* src2;
	while (w > 0)
	{
		src = src0;
		i = pitch;
		if (i > w) i = w;
		w -= i;

		for (j = i/16; j > 0; j--)
		{
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = src[3];

			src += 4;
			dst += 4;
		}

		src2 = (const u8*)src;
		dst2 = (u8*)dst;
		for (j = i & 0x0f; j > 0; j--)
		{
			*dst2++ = *src2++;
		}
		dst = (u32*)dst2;
	}
}
#endif

#if DISPHSTX_USE_FORMAT_PAT_8_PAL
// repeated pattern, 8-bit format, paletted colors, width derived from pitch, height from fonth
void FASTCODE NOFLASH(DispHstxDviRender_Pat_8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	line = line % slot->fonth;
	int pitch = slot->pitch;
	const u32* src0 = (u32*)&((u8*)slot->buf)[line * pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	int i, j;
	u32 k;
	int w = slot->w;
	const u32* src;
	const u8* src2;
	while (w > 0)
	{
		src = src0;
		i = pitch;
		if (i > w) i = w;
		w -= i;

		for (j = i/16; j > 0; j--)
		{
			k = src[0];
			dst[0] = map[k & 0xff];
			dst[1] = map[(k >> 8) & 0xff];
			dst[2] = map[(k >> 16) & 0xff];
			dst[3] = map[k >> 24];

			k = src[1];
			dst[4] = map[k & 0xff];
			dst[5] = map[(k >> 8) & 0xff];
			dst[6] = map[(k >> 16) & 0xff];
			dst[7] = map[k >> 24];

			k = src[2];
			dst[8] = map[k & 0xff];
			dst[9] = map[(k >> 8) & 0xff];
			dst[10] = map[(k >> 16) & 0xff];
			dst[11] = map[k >> 24];

			k = src[3];
			dst[12] = map[k & 0xff];
			dst[13] = map[(k >> 8) & 0xff];
			dst[14] = map[(k >> 16) & 0xff];
			dst[15] = map[k >> 24];

			src += 4;
			dst += 16;
		}

		src2 = (const u8*)src;
		for (j = i & 0x0f; j > 0; j--)
		{
			*dst++ = map[*src2++];
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_RLE8
// RLE compression, 8-bit format (font=line offsets; slot width must match image width)
void FASTCODE NOFLASH(DispHstxDviRender_Rle8)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	// array of line offsets
	u32* font = (u32*)slot->font;

	// source address
	const u8* src = (u8*)slot->buf + font[line];

	// pointer to render buffer
	u8* dst = (u8*)*cmd;

	int i, j, n;
	u8 b;

	for (i = slot->w; i > 0;)
	{
		// get token
		b = *src++;

		// repeat
		if (b >= 128)
		{
			// get length
			n = b - 128 + 3;
			if (n > i) n = i;
			i -= n; // decrease width

			// get color
			b = *src++;

			// store color
			for (j = n/4; j > 0; j--)
			{
				dst[0] = b;
				dst[1] = b;
				dst[2] = b;
				dst[3] = b;
				dst += 4;
			}
			for (n &= 0x03; n > 0; n--) *dst++ = b;
		}

		// raw data
		else
		{
			// get length
			n = b + 1;
			if (n > i) n = i;
			i -= n; // decrease width

			// transfer pixels
			for (j = n/4; j > 0; j--)
			{
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = src[3];
				dst += 4;
				src += 4;
			}
			for (n &= 0x03; n > 0; n--) *dst++ = *src++;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_RLE8_PAL
// RLE compression, 8-bit format, paletted (font=line offsets; slot width must match image width)
void FASTCODE NOFLASH(DispHstxDviRender_Rle8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	// array of line offsets
	u32* font = (u32*)slot->font;

	// source address
	const u8* src = (u8*)slot->buf + font[line];

	// pointer to render buffer
	u16* dst = (u16*)*cmd;

	// palette
	const u16* map = slot->pal;

	int i, j, n;
	u8 b;
	u16 col;

	for (i = slot->w; i > 0;)
	{
		// get token
		b = *src++;

		// repeat
		if (b >= 128)
		{
			// get length
			n = b - 128 + 3;
			if (n > i) n = i;
			i -= n; // decrease width

			// get color
			col = map[*src++];

			// store color
			for (j = n/4; j > 0; j--)
			{
				dst[0] = col;
				dst[1] = col;
				dst[2] = col;
				dst[3] = col;
				dst += 4;
			}
			for (n &= 0x03; n > 0; n--) *dst++ = col;
		}

		// raw data
		else
		{
			// get length
			n = b + 1;
			if (n > i) n = i;
			i -= n; // decrease width

			// transfer pixels
			for (j = n/4; j > 0; j--)
			{
				dst[0] = map[src[0]];
				dst[1] = map[src[1]];
				dst[2] = map[src[2]];
				dst[3] = map[src[3]];
				dst += 4;
				src += 4;
			}
			for (n &= 0x03; n > 0; n--) *dst++ = map[*src++];
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_ATTR1_PAL
// DISPHSTX_FORMAT_ATTR1_PAL, attribute compression 1 (to 28%), 4-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr1_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	const u32* pix = (const u32*)&slot->font[line/8 * slot->fonth];
	u8 map2[2];

	int i, j;
	u32 k, c;

	for (i = slot->w/32; i > 0; i--)
	{
		k = *src++;
		c = *pix++;
		map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
		dst[0] = map[map2[(k >> 7) & 1]];
		dst[1] = map[map2[(k >> 6) & 1]];
		dst[2] = map[map2[(k >> 5) & 1]];
		dst[3] = map[map2[(k >> 4) & 1]];
		dst[4] = map[map2[(k >> 3) & 1]];
		dst[5] = map[map2[(k >> 2) & 1]];
		dst[6] = map[map2[(k >> 1) & 1]];
		dst[7] = map[map2[k & 1]];

		map2[0] = (c >> 12) & 0x0f; map2[1] = (c >> 8) & 0x0f;
		dst[8] = map[map2[(k >> 15) & 1]];
		dst[9] = map[map2[(k >> 14) & 1]];
		dst[10] = map[map2[(k >> 13) & 1]];
		dst[11] = map[map2[(k >> 12) & 1]];
		dst[12] = map[map2[(k >> 11) & 1]];
		dst[13] = map[map2[(k >> 10) & 1]];
		dst[14] = map[map2[(k >> 9) & 1]];
		dst[15] = map[map2[(k >> 8) & 1]];

		map2[0] = (c >> 20) & 0x0f; map2[1] = (c >> 16) & 0x0f;
		dst[16] = map[map2[(k >> 23) & 1]];
		dst[17] = map[map2[(k >> 22) & 1]];
		dst[18] = map[map2[(k >> 21) & 1]];
		dst[19] = map[map2[(k >> 20) & 1]];
		dst[20] = map[map2[(k >> 19) & 1]];
		dst[21] = map[map2[(k >> 18) & 1]];
		dst[22] = map[map2[(k >> 17) & 1]];
		dst[23] = map[map2[(k >> 16) & 1]];

		map2[0] = (c >> 28) & 0x0f; map2[1] = (c >> 24) & 0x0f;
		dst[24] = map[map2[k >> 31]];
		dst[25] = map[map2[(k >> 30) & 1]];
		dst[26] = map[map2[(k >> 29) & 1]];
		dst[27] = map[map2[(k >> 28) & 1]];
		dst[28] = map[map2[(k >> 27) & 1]];
		dst[29] = map[map2[(k >> 26) & 1]];
		dst[30] = map[map2[(k >> 25) & 1]];
		dst[31] = map[map2[(k >> 24) & 1]];

		dst += 32;
	}

	i = slot->w & 0x1f;
	if (i > 0)
	{
		k = *src;
		c = *pix;
		for (j = i/8; j > 0; j--)
		{
			map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
			dst[0] = map[map2[(k >> 7) & 1]];
			dst[1] = map[map2[(k >> 6) & 1]];
			dst[2] = map[map2[(k >> 5) & 1]];
			dst[3] = map[map2[(k >> 4) & 1]];
			dst[4] = map[map2[(k >> 3) & 1]];
			dst[5] = map[map2[(k >> 2) & 1]];
			dst[6] = map[map2[(k >> 1) & 1]];
			dst[7] = map[map2[k & 1]];
			dst += 8;
			c >>= 8;
			k >>= 8;
		}

		map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
		for (i &= 7; i > 0; i--)
		{
			*dst++ = map[map2[(k >> 7) & 1]];
			k <<= 1;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_ATTR2_PAL
// DISPHSTX_FORMAT_ATTR2_PAL, attribute compression 2 (to 37%), 4-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr2_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line/2 * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	const u32* pix = (const u32*)&slot->font[line/4 * slot->fonth];
	u8 map2[2];

	int i, j;
	u32 k, c;
	if ((line & 1) == 0)
	{
		for (i = slot->w/16; i > 0; i--)
		{
			k = *src++;
			c = *pix++;
			map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
			dst[0] = map[map2[(k >> 7) & 1]];
			dst[1] = map[map2[(k >> 6) & 1]];
			dst[2] = map[map2[(k >> 5) & 1]];
			dst[3] = map[map2[(k >> 4) & 1]];

			map2[0] = (c >> 12) & 0x0f; map2[1] = (c >> 8) & 0x0f;
			dst[4] = map[map2[(k >> 15) & 1]];
			dst[5] = map[map2[(k >> 14) & 1]];
			dst[6] = map[map2[(k >> 13) & 1]];
			dst[7] = map[map2[(k >> 12) & 1]];

			map2[0] = (c >> 20) & 0x0f; map2[1] = (c >> 16) & 0x0f;
			dst[8] = map[map2[(k >> 23) & 1]];
			dst[9] = map[map2[(k >> 22) & 1]];
			dst[10] = map[map2[(k >> 21) & 1]];
			dst[11] = map[map2[(k >> 20) & 1]];

			map2[0] = (c >> 28) & 0x0f; map2[1] = (c >> 24) & 0x0f;
			dst[12] = map[map2[k >> 31]];
			dst[13] = map[map2[(k >> 30) & 1]];
			dst[14] = map[map2[(k >> 29) & 1]];
			dst[15] = map[map2[(k >> 28) & 1]];

			dst += 16;
		}

		i = slot->w & 0x0f;
		if (i > 0)
		{
			k = *src;
			c = *pix;
			for (j = i/4; j > 0; j--)
			{
				map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
				dst[0] = map[map2[(k >> 7) & 1]];
				dst[1] = map[map2[(k >> 6) & 1]];
				dst[2] = map[map2[(k >> 5) & 1]];
				dst[3] = map[map2[(k >> 4) & 1]];
				dst += 4;
				k >>= 8;
				c >>= 8;
			}

			map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
			for (i &= 3; i > 0; i--)
			{
				*dst++ = map[map2[(k >> 7) & 1]];
				k <<= 1;
			}
		}
	}
	else
	{
		for (i = slot->w/16; i > 0; i--)
		{
			k = *src++;
			c = *pix++;
			map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
			dst[0] = map[map2[(k >> 3) & 1]];
			dst[1] = map[map2[(k >> 2) & 1]];
			dst[2] = map[map2[(k >> 1) & 1]];
			dst[3] = map[map2[k & 1]];

			map2[0] = (c >> 12) & 0x0f; map2[1] = (c >> 8) & 0x0f;
			dst[4] = map[map2[(k >> 11) & 1]];
			dst[5] = map[map2[(k >> 10) & 1]];
			dst[6] = map[map2[(k >> 9) & 1]];
			dst[7] = map[map2[(k >> 8) & 1]];

			map2[0] = (c >> 20) & 0x0f; map2[1] = (c >> 16) & 0x0f;
			dst[8] = map[map2[(k >> 19) & 1]];
			dst[9] = map[map2[(k >> 18) & 1]];
			dst[10] = map[map2[(k >> 17) & 1]];
			dst[11] = map[map2[(k >> 16) & 1]];

			map2[0] = (c >> 28) & 0x0f; map2[1] = (c >> 24) & 0x0f;
			dst[12] = map[map2[(k >> 27) & 1]];
			dst[13] = map[map2[(k >> 26) & 1]];
			dst[14] = map[map2[(k >> 25) & 1]];
			dst[15] = map[map2[(k >> 24) & 1]];

			dst += 16;
		}

		i = slot->w & 0x0f;
		if (i > 0)
		{
			k = *src;
			c = *pix;
			for (j = i/4; j > 0; j--)
			{
				map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
				dst[0] = map[map2[(k >> 3) & 1]];
				dst[1] = map[map2[(k >> 2) & 1]];
				dst[2] = map[map2[(k >> 1) & 1]];
				dst[3] = map[map2[k & 1]];
				dst += 4;
				k >>= 8;
				c >>= 8;
			}

			map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
			for (i &= 3; i > 0; i--)
			{
				*dst++ = map[map2[(k >> 3) & 1]];
				k <<= 1;
			}
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_ATTR3_PAL
// DISPHSTX_FORMAT_ATTR3_PAL, attribute compression 3 (to 56%), 4-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr3_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	const u32* pix = (const u32*)&slot->font[line/8 * slot->fonth];

	u8 map2[4];
	int i, j;
	u32 k, c;

	for (i = slot->w/16; i > 0; i--)
	{
		k = *src++;
		c = *pix++;
		map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
		map2[2] = (c >> 12) & 0x0f; map2[3] = (c >> 8) & 0x0f;

		dst[0] = map[map2[(k >> 6) & 3]];
		dst[1] = map[map2[(k >> 4) & 3]];
		dst[2] = map[map2[(k >> 2) & 3]];
		dst[3] = map[map2[k & 3]];

		dst[4] = map[map2[(k >> 14) & 3]];
		dst[5] = map[map2[(k >> 12) & 3]];
		dst[6] = map[map2[(k >> 10) & 3]];
		dst[7] = map[map2[(k >> 8) & 3]];

		map2[0] = (c >> 20) & 0x0f; map2[1] = (c >> 16) & 0x0f;
		map2[2] = (c >> 28) & 0x0f; map2[3] = (c >> 24) & 0x0f;

		dst[8] = map[map2[(k >> 22) & 3]];
		dst[9] = map[map2[(k >> 20) & 3]];
		dst[10] = map[map2[(k >> 18) & 3]];
		dst[11] = map[map2[(k >> 16) & 3]];

		dst[12] = map[map2[k >> 30]];
		dst[13] = map[map2[(k >> 28) & 3]];
		dst[14] = map[map2[(k >> 26) & 3]];
		dst[15] = map[map2[(k >> 24) & 3]];

		dst += 16;
	}

	i = slot->w & 0x0f;
	if (i > 0)
	{
		k = Endian(*src);
		c = *pix;
		map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
		map2[2] = (c >> 12) & 0x0f; map2[3] = (c >> 8) & 0x0f;

		if (i >= 8)
		{
			i -= 8;
			dst[0] = map[map2[k >> 30]];
			dst[1] = map[map2[(k >> 28) & 3]];
			dst[2] = map[map2[(k >> 26) & 3]];
			dst[3] = map[map2[(k >> 24) & 3]];

			dst[4] = map[map2[(k >> 22) & 3]];
			dst[5] = map[map2[(k >> 20) & 3]];
			dst[6] = map[map2[(k >> 18) & 3]];
			dst[7] = map[map2[(k >> 16) & 3]];

			map2[0] = (c >> 20) & 0x0f; map2[1] = (c >> 16) & 0x0f;
			map2[2] = (c >> 28) & 0x0f; map2[3] = (c >> 24) & 0x0f;

			dst += 8;
			k <<= 16;
		}

		for (; i > 0; i--)
		{
			*dst++ = map[map2[k >> 30]];
			k <<= 2;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_ATTR4_PAL
// DISPHSTX_FORMAT_ATTR4_PAL, attribute compression 4 (to 75%), 4-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr4_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	const u32* pix = (const u32*)&slot->font[line/4 * slot->fonth];
	u8 map2[4];

	int i, j;
	u32 k, c;
	for (i = slot->w/16; i > 0; i--)
	{
		k = *src++;
		c = pix[0];
		map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
		map2[2] = (c >> 12) & 0x0f; map2[3] = (c >> 8) & 0x0f;

		dst[0] = map[map2[(k >> 6) & 3]];
		dst[1] = map[map2[(k >> 4) & 3]];
		dst[2] = map[map2[(k >> 2) & 3]];
		dst[3] = map[map2[k & 3]];

		map2[0] = (c >> 20) & 0x0f; map2[1] = (c >> 16) & 0x0f;
		map2[2] = (c >> 28) & 0x0f; map2[3] = (c >> 24) & 0x0f;

		dst[4] = map[map2[(k >> 14) & 3]];
		dst[5] = map[map2[(k >> 12) & 3]];
		dst[6] = map[map2[(k >> 10) & 3]];
		dst[7] = map[map2[(k >> 8) & 3]];

		c = pix[1];
		map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
		map2[2] = (c >> 12) & 0x0f; map2[3] = (c >> 8) & 0x0f;

		dst[8] = map[map2[(k >> 22) & 3]];
		dst[9] = map[map2[(k >> 20) & 3]];
		dst[10] = map[map2[(k >> 18) & 3]];
		dst[11] = map[map2[(k >> 16) & 3]];

		map2[0] = (c >> 20) & 0x0f; map2[1] = (c >> 16) & 0x0f;
		map2[2] = (c >> 28) & 0x0f; map2[3] = (c >> 24) & 0x0f;

		dst[12] = map[map2[k >> 30]];
		dst[13] = map[map2[(k >> 28) & 3]];
		dst[14] = map[map2[(k >> 26) & 3]];
		dst[15] = map[map2[(k >> 24) & 3]];

		pix += 2;
		dst += 16;
	}

	i = slot->w & 0x0f;
	if (i > 0)
	{
		k = Endian(*src++);

		const u16* pix2 = (const u16*)pix;
		c = *pix2++;
		map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
		map2[2] = (c >> 12) & 0x0f; map2[3] = (c >> 8) & 0x0f;

		for (j = i/4; j > 0; j--)
		{
			dst[0] = map[map2[k >> 30]];
			dst[1] = map[map2[(k >> 28) & 3]];
			dst[2] = map[map2[(k >> 26) & 3]];
			dst[3] = map[map2[(k >> 24) & 3]];

			c = *pix2++;
			map2[0] = (c >> 4) & 0x0f; map2[1] = c & 0x0f;
			map2[2] = (c >> 12) & 0x0f; map2[3] = (c >> 8) & 0x0f;

			dst += 4;
			k <<= 8;
		}

		for (i &= 3; i > 0; i--)
		{
			*dst++ = map[map2[k >> 30]];
			k <<= 2;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_ATTR5_PAL
// DISPHSTX_FORMAT_ATTR5_PAL, attribute compression 5 (to 15%), 8-bit paletted colors, cell 8x8, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr5_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	const u8* pix = &slot->font[line/8 * slot->fonth];

	int i, j;
	u32 k;
	for (i = slot->w/32; i > 0; i--)
	{
		k = *src++;
		dst[0] = map[pix[(k >> 7) & 1]];
		dst[1] = map[pix[(k >> 6) & 1]];
		dst[2] = map[pix[(k >> 5) & 1]];
		dst[3] = map[pix[(k >> 4) & 1]];
		dst[4] = map[pix[(k >> 3) & 1]];
		dst[5] = map[pix[(k >> 2) & 1]];
		dst[6] = map[pix[(k >> 1) & 1]];
		dst[7] = map[pix[k & 1]];

		dst[8] = map[pix[((k >> 15) & 1)+2]];
		dst[9] = map[pix[((k >> 14) & 1)+2]];
		dst[10] = map[pix[((k >> 13) & 1)+2]];
		dst[11] = map[pix[((k >> 12) & 1)+2]];
		dst[12] = map[pix[((k >> 11) & 1)+2]];
		dst[13] = map[pix[((k >> 10) & 1)+2]];
		dst[14] = map[pix[((k >> 9) & 1)+2]];
		dst[15] = map[pix[((k >> 8) & 1)+2]];

		dst[16] = map[pix[((k >> 23) & 1)+4]];
		dst[17] = map[pix[((k >> 22) & 1)+4]];
		dst[18] = map[pix[((k >> 21) & 1)+4]];
		dst[19] = map[pix[((k >> 20) & 1)+4]];
		dst[20] = map[pix[((k >> 19) & 1)+4]];
		dst[21] = map[pix[((k >> 18) & 1)+4]];
		dst[22] = map[pix[((k >> 17) & 1)+4]];
		dst[23] = map[pix[((k >> 16) & 1)+4]];

		dst[24] = map[pix[(k >> 31)+6]];
		dst[25] = map[pix[((k >> 30) & 1)+6]];
		dst[26] = map[pix[((k >> 29) & 1)+6]];
		dst[27] = map[pix[((k >> 28) & 1)+6]];
		dst[28] = map[pix[((k >> 27) & 1)+6]];
		dst[29] = map[pix[((k >> 26) & 1)+6]];
		dst[30] = map[pix[((k >> 25) & 1)+6]];
		dst[31] = map[pix[((k >> 24) & 1)+6]];

		dst += 32;
		pix += 8;
	}

	i = slot->w & 0x1f;
	if (i > 0)
	{
		k = *src;
		for (j = i/8; j > 0; j--)
		{
			dst[0] = map[pix[(k >> 7) & 1]];
			dst[1] = map[pix[(k >> 6) & 1]];
			dst[2] = map[pix[(k >> 5) & 1]];
			dst[3] = map[pix[(k >> 4) & 1]];
			dst[4] = map[pix[(k >> 3) & 1]];
			dst[5] = map[pix[(k >> 2) & 1]];
			dst[6] = map[pix[(k >> 1) & 1]];
			dst[7] = map[pix[k & 1]];
			dst += 8;
			pix += 2;
			k >>= 8;
		}

		for (i &= 7; i > 0; i--)
		{
			*dst++ = map[pix[(k >> 7) & 1]];
			k <<= 1;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_ATTR6_PAL
// DISPHSTX_FORMAT_ATTR6_PAL, attribute compression 6 (to 25%), 8-bit paletted colors, cell 4x4, 2 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr6_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line/2 * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	const u8* pix = &slot->font[line/4 * slot->fonth];

	int i, j;
	u32 k;
	if ((line & 1) == 0)
	{
		for (i = slot->w/16; i > 0; i--)
		{
			k = *src++;
			dst[0] = map[pix[(k >> 7) & 1]];
			dst[1] = map[pix[(k >> 6) & 1]];
			dst[2] = map[pix[(k >> 5) & 1]];
			dst[3] = map[pix[(k >> 4) & 1]];

			dst[4] = map[pix[((k >> 15) & 1)+2]];
			dst[5] = map[pix[((k >> 14) & 1)+2]];
			dst[6] = map[pix[((k >> 13) & 1)+2]];
			dst[7] = map[pix[((k >> 12) & 1)+2]];

			dst[8] = map[pix[((k >> 23) & 1)+4]];
			dst[9] = map[pix[((k >> 22) & 1)+4]];
			dst[10] = map[pix[((k >> 21) & 1)+4]];
			dst[11] = map[pix[((k >> 20) & 1)+4]];

			dst[12] = map[pix[(k >> 31)+6]];
			dst[13] = map[pix[((k >> 30) & 1)+6]];
			dst[14] = map[pix[((k >> 29) & 1)+6]];
			dst[15] = map[pix[((k >> 28) & 1)+6]];

			dst += 16;
			pix += 8;
		}

		i = slot->w & 0x0f;
		if (i > 0)
		{
			k = *src++;
			for (j = i/4; j > 0; j--)
			{
				dst[0] = map[pix[(k >> 7) & 1]];
				dst[1] = map[pix[(k >> 6) & 1]];
				dst[2] = map[pix[(k >> 5) & 1]];
				dst[3] = map[pix[(k >> 4) & 1]];
				dst += 4;
				pix += 2;
				k >>= 8;
			}

			for (i &= 3; i > 0; i--)
			{
				*dst++ = map[pix[(k >> 7) & 1]];
				k <<= 1;
			}
		}
	}
	else
	{
		for (i = slot->w/16; i > 0; i--)
		{
			k = *src++;
			dst[0] = map[pix[(k >> 3) & 1]];
			dst[1] = map[pix[(k >> 2) & 1]];
			dst[2] = map[pix[(k >> 1) & 1]];
			dst[3] = map[pix[k & 1]];

			dst[4] = map[pix[((k >> 11) & 1)+2]];
			dst[5] = map[pix[((k >> 10) & 1)+2]];
			dst[6] = map[pix[((k >> 9) & 1)+2]];
			dst[7] = map[pix[((k >> 8) & 1)+2]];

			dst[8] = map[pix[((k >> 19) & 1)+4]];
			dst[9] = map[pix[((k >> 18) & 1)+4]];
			dst[10] = map[pix[((k >> 17) & 1)+4]];
			dst[11] = map[pix[((k >> 16) & 1)+4]];

			dst[12] = map[pix[((k >> 27) & 1)+6]];
			dst[13] = map[pix[((k >> 26) & 1)+6]];
			dst[14] = map[pix[((k >> 25) & 1)+6]];
			dst[15] = map[pix[((k >> 24) & 1)+6]];

			dst += 16;
			pix += 8;
		}

		i = slot->w & 0x0f;
		if (i > 0)
		{
			k = *src++;
			for (j = i/4; j > 0; j--)
			{
				dst[0] = map[pix[(k >> 3) & 1]];
				dst[1] = map[pix[(k >> 2) & 1]];
				dst[2] = map[pix[(k >> 1) & 1]];
				dst[3] = map[pix[k & 1]];
				dst += 4;
				pix += 2;
				k >>= 8;
			}

			for (i &= 3; i > 0; i--)
			{
				*dst++ = map[pix[(k >> 3) & 1]];
				k <<= 1;
			}
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_ATTR7_PAL
// DISPHSTX_FORMAT_ATTR7_PAL, attribute compression 7 (to 31%), 8-bit paletted colors, cell 8x8, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr7_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	const u8* pix = &slot->font[line/8 * slot->fonth];

	int i;
	u32 k;
	for (i = slot->w/16; i > 0; i--)
	{
		k = *src++;
		dst[0] = map[pix[(k >> 6) & 3]];
		dst[1] = map[pix[(k >> 4) & 3]];
		dst[2] = map[pix[(k >> 2) & 3]];
		dst[3] = map[pix[k & 3]];

		dst[4] = map[pix[(k >> 14) & 3]];
		dst[5] = map[pix[(k >> 12) & 3]];
		dst[6] = map[pix[(k >> 10) & 3]];
		dst[7] = map[pix[(k >> 8) & 3]];

		dst[8] = map[pix[((k >> 22) & 3)+4]];
		dst[9] = map[pix[((k >> 20) & 3)+4]];
		dst[10] = map[pix[((k >> 18) & 3)+4]];
		dst[11] = map[pix[((k >> 16) & 3)+4]];

		dst[12] = map[pix[(k >> 30)+4]];
		dst[13] = map[pix[((k >> 28) & 3)+4]];
		dst[14] = map[pix[((k >> 26) & 3)+4]];
		dst[15] = map[pix[((k >> 24) & 3)+4]];

		dst += 16;
		pix += 8;
	}

	i = slot->w & 0x0f;
	if (i > 0)
	{
		k = Endian(*src++);
		if (i >= 8)
		{
			i -= 8;
			dst[0] = map[pix[k >> 30]];
			dst[1] = map[pix[(k >> 28) & 3]];
			dst[2] = map[pix[(k >> 26) & 3]];
			dst[3] = map[pix[(k >> 24) & 3]];

			dst[4] = map[pix[(k >> 22) & 3]];
			dst[5] = map[pix[(k >> 20) & 3]];
			dst[6] = map[pix[(k >> 18) & 3]];
			dst[7] = map[pix[(k >> 16) & 3]];
			dst += 8;
			pix += 4;
			k <<= 16;
		}

		for (; i > 0; i--)
		{
			*dst++ = map[pix[k >> 30]];
			k <<= 2;
		}
	}
}
#endif

#if DISPHSTX_USE_FORMAT_ATTR8_PAL
// DISPHSTX_FORMAT_ATTR8_PAL, attribute compression 8 (to 50%), 8-bit paletted colors, cell 4x4, 4 attributes (font=attributes, fonth=pitch of attributes)
void FASTCODE NOFLASH(DispHstxDviRender_Attr8_PAL)(sDispHstxVSlot* slot, int line, u32* cmd)
{
	const u32* src = (u32*)&((u8*)slot->buf)[line * slot->pitch];
	u16* dst = (u16*)*cmd; // pointer to render buffer
	const u16* map = slot->pal;
	const u8* pix = &slot->font[line/4 * slot->fonth];

	int i, j;
	u32 k;
	for (i = slot->w/16; i > 0; i--)
	{
		k = *src++;
		dst[0] = map[pix[(k >> 6) & 3]];
		dst[1] = map[pix[(k >> 4) & 3]];
		dst[2] = map[pix[(k >> 2) & 3]];
		dst[3] = map[pix[k & 3]];

		dst[4] = map[pix[((k >> 14) & 3)+4]];
		dst[5] = map[pix[((k >> 12) & 3)+4]];
		dst[6] = map[pix[((k >> 10) & 3)+4]];
		dst[7] = map[pix[((k >> 8) & 3)+4]];

		dst[8] = map[pix[((k >> 22) & 3)+8]];
		dst[9] = map[pix[((k >> 20) & 3)+8]];
		dst[10] = map[pix[((k >> 18) & 3)+8]];
		dst[11] = map[pix[((k >> 16) & 3)+8]];

		dst[12] = map[pix[(k >> 30)+12]];
		dst[13] = map[pix[((k >> 28) & 3)+12]];
		dst[14] = map[pix[((k >> 26) & 3)+12]];
		dst[15] = map[pix[((k >> 24) & 3)+12]];

		dst += 16;
		pix += 16;
	}

	i = slot->w & 0x0f;
	if (i > 0)
	{
		k = Endian(*src++);
		for (j = i/4; j > 0; j--)
		{
			dst[0] = map[pix[k >> 30]];
			dst[1] = map[pix[(k >> 28) & 3]];
			dst[2] = map[pix[(k >> 26) & 3]];
			dst[3] = map[pix[(k >> 24) & 3]];
			dst += 4;
			pix += 4;
			k <<= 8;
		}

		for (i &= 3; i > 0; i--)
		{
			*dst++ = map[pix[k >> 30]];
			k <<= 2;
		}
	}
}
#endif

#endif // !(DISPHSTX_ARM_ASM && !RISCV) && !(DISPHSTX_RISCV_ASM && RISCV) && DISPHSTX_USE_DVI

#endif // USE_DISPHSTX
