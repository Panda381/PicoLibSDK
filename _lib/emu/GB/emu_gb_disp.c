
// ****************************************************************************
//
//                        Game Boy Emulator - display
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

// display mode
volatile u8 GB_DispMode;		// display mode GB_DISPMODE_*

// Game Boy Mono palette
#if GB_MONO_PAL == 3

// soft green
#define GB_GRAY3	COLOR(255*15/16, 255, 255*12/16)
#define GB_GRAY2	COLOR(170*15/16, 170, 170*12/16)
#define GB_GRAY1	COLOR( 85*15/16,  85,  85*12/16)
#define GB_GRAY0	COLOR(        0,   0,         0)

#elif GB_MONO_PAL == 2

// blue (Game Boy Light, with backlight)
#define GB_GRAY3	COLOR(255-16, 255*7/8-8, 255)
#define GB_GRAY2	COLOR( 170-8, 170*7/8-8, 170)
#define GB_GRAY1	COLOR(    85,    85*7/8,  85)
#define GB_GRAY0	COLOR(     0,         0,   0)

#elif GB_MONO_PAL == 1

// green (original Game Boy, without backlight)
#define GB_GRAY3	COLOR(255*3/4, 255, 255*1/4)
#define GB_GRAY2	COLOR(170*3/4, 170, 170*1/4)
#define GB_GRAY1	COLOR( 85*3/4,  85,  85*1/4)
#define GB_GRAY0	COLOR(      0,   0,       0)

#else

// black & white
#define GB_GRAY3	COLOR(255, 255, 255)
#define GB_GRAY2	COLOR(170, 170, 170)
#define GB_GRAY1	COLOR( 85,  85,  85)
#define GB_GRAY0	COLOR(  0,   0,   0)

#endif

// palettes of GB mono
const u16 GB_GreyPal[64] = {
	// OBJ0
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	// OBJ1
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	// BG
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	// invalid
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
};

/*
const u16 GB_ColPal[64] = {
	// OBJ0
	COL_WHITE, 0xED13, 0xA207, COL_BLACK,
	COL_WHITE, 0xED13, 0xA207, COL_BLACK,
	COL_WHITE, 0xED13, 0xA207, COL_BLACK,
	COL_WHITE, 0xED13, 0xA207, COL_BLACK,
	// OBJ1
	COL_WHITE, 0xED13, 0xA207, COL_BLACK,
	COL_WHITE, 0xED13, 0xA207, COL_BLACK,
	COL_WHITE, 0xED13, 0xA207, COL_BLACK,
	COL_WHITE, 0xED13, 0xA207, COL_BLACK,
	// BG
	COL_WHITE, 0x9E89, 0x3C9B, COL_BLACK,
	COL_WHITE, 0x9E89, 0x3C9B, COL_BLACK,
	COL_WHITE, 0x9E89, 0x3C9B, COL_BLACK,
	COL_WHITE, 0x9E89, 0x3C9B, COL_BLACK,
	// invalid
	COL_WHITE, 0x9E89, 0x3C9B, COL_BLACK,
	COL_WHITE, 0x9E89, 0x3C9B, COL_BLACK,
	COL_WHITE, 0x9E89, 0x3C9B, COL_BLACK,
	COL_WHITE, 0x9E89, 0x3C9B, COL_BLACK,
};
*/

// redraw display to LCD
void FASTCODE GB_LCDRedraw()
{
	// message text mode
	if (GB_DispMode == GB_DISPMODE_MSG)
	{
		GB_TextUpdate();
	}
	else
	{
		// start sending image data
		DispStartImg(0, WIDTH, 0, HEIGHT);

		int x, y;
		const u8 *s;
		u16 c;

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
		const u16* pal = (GBC->gbmode != GB_MODE_GB) ? GBC->fixpal : GB_GreyPal; //GB_ColPal;
#else
		const u16* pal = GB_GreyPal;
#endif

/*
		// linear interpolation
		u16 c2;
		s = GBC->framebuf;
		for (y = HEIGHT/5; y > 1; y--)
		{
			// line 0 and 1: 100% + 0%
			for (x = WIDTH/2; x > 0; x--)
			{
				c = pal[*s & 0x3f];
				s++;
				DispSendImg2(c);
				DispSendImg2(c);
			}
			s -= GB_WIDTH;

			// line 0 and 1: 50% + 50% (should be 40% + 60%)
			for (x = WIDTH/2; x > 0; x--)
			{
				c = pal[*s & 0x3f];
				c2 = pal[s[GB_WIDTH] & 0x3f];
				s++;
				c = RGB16Blend50(c, c2);
				DispSendImg2(c);
				DispSendImg2(c);
			}

			// line 1 and 2: 75% + 25% (should be 80% + 20%)
			for (x = WIDTH/2; x > 0; x--)
			{
				c = pal[*s & 0x3f];
				c2 = pal[s[GB_WIDTH] & 0x3f];
				s++;
				c = RGB16Blend25(c2, c);
				DispSendImg2(c);
				DispSendImg2(c);
			}
			s -= GB_WIDTH;

			// line 1 and 2: 25% + 75% (should be 20% + 80%)
			for (x = WIDTH/2; x > 0; x--)
			{
				c = pal[*s & 0x3f];
				c2 = pal[s[GB_WIDTH] & 0x3f];
				s++;
				c = RGB16Blend25(c, c2);
				DispSendImg2(c);
				DispSendImg2(c);
			}

			// line 2 and 3: 50% + 50% (should be 60% + 40%)
			for (x = WIDTH/2; x > 0; x--)
			{
				c = pal[*s & 0x3f];
				c2 = pal[s[GB_WIDTH] & 0x3f];
				s++;
				c = RGB16Blend25(c, c2);
				DispSendImg2(c);
				DispSendImg2(c);
			}
		}

		// line 141 and 142: 100% + 0%
		for (x = WIDTH/2; x > 0; x--)
		{
			c = pal[*s & 0x3f];
			s++;
			DispSendImg2(c);
			DispSendImg2(c);
		}
		s -= GB_WIDTH;

		// line 141 and 142: 50% + 50% (should be 40% + 60%)
		for (x = WIDTH/2; x > 0; x--)
		{
			c = pal[*s & 0x3f];
			c2 = pal[s[GB_WIDTH] & 0x3f];
			s++;
			c = RGB16Blend50(c, c2);
			DispSendImg2(c);
			DispSendImg2(c);
		}

		// line 142 and 143: 75% + 25% (should be 80% + 20%)
		for (x = WIDTH/2; x > 0; x--)
		{
			c = pal[*s & 0x3f];
			c2 = pal[s[GB_WIDTH] & 0x3f];
			s++;
			c = RGB16Blend25(c2, c);
			DispSendImg2(c);
			DispSendImg2(c);
		}
		s -= GB_WIDTH;

		// line 142 and 143: 25% + 75% (should be 20% + 80%)
		for (x = WIDTH/2; x > 0; x--)
		{
			c = pal[*s & 0x3f];
			c2 = pal[s[GB_WIDTH] & 0x3f];
			s++;
			c = RGB16Blend25(c, c2);
			DispSendImg2(c);
			DispSendImg2(c);
		}

		// line 143: 100%
		for (x = WIDTH/2; x > 0; x--)
		{
			c = pal[*s & 0x3f];
			s++;
			DispSendImg2(c);
			DispSendImg2(c);
		}
*/

		for (y = 0; y < HEIGHT; y++)
		{
			int ys = y*GB_HEIGHT/HEIGHT;
			s = &GBC->framebuf[ys*GB_WIDTH];

			for (x = WIDTH/2; x > 0; x--)
			{
				c = pal[(*s++) & 0x3f];
				DispSendImg2(c);
				DispSendImg2(c);
			}
		}

		// stop sending data
		DispStopImg();
	}
}

// render current line
void FASTCODE NOFLASH(GB_RenderLine)()
{
	// prepare line buffer
	int line = GBC->hram[GB_IO_LY];
	if (line >= GB_HEIGHT) return; // invalid line
	if ((GBC->frame & 3) != 0) return; // skip some frames to speed up rendering

	u8 pixels[GB_WIDTH];		// pixel buffer
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
	u8 pixelsPrio[GB_WIDTH];	// priority pixel buffer
	memset(pixelsPrio, 0, GB_WIDTH);
#endif
	memset(pixels, 0, GB_WIDTH);

// ==== draw background

	// 0x40: LCD control register
	// (default 0x83 = BG/OBJ ON, LCDC operation)
	//    B0: 1=BG display on (only GB; GBC is always on)
	//    B1: 1=OBJ is on
	//    B2: 1=8x16 dots, 0=8x8 dots
	//    B3: background code area 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
	//    B4: character data 1=0x8000-0x8FFF, 0=0x8800-0x97FF
	//    B5: 1=windowing on
	//    B6: window code are selection 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
	//    B7: LCDC controller operation stop 1=LCDC on, 0=LCDC off
	if (((GBC->hram[GB_IO_LCDC] & B0) != 0) // check if background is enabled
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
		|| (GBC->gbmode != GB_MODE_GB) // or if GBC is enabled
#endif
	){
		// prepare current background line to draw
		u8 bg_y = line + GBC->hram[GB_IO_SCY]; // line + scroll background Y

		// prepare offset of first tile (width 32 pixels); select background map 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
		u16 bg_map = (((GBC->hram[GB_IO_LCDC] & B3) != 0) ? 0x1c00 : 0x1800) + (bg_y >> 3)*0x20;

		// X coordinate to begin drawing (draw right to left)
		u8 disp_x = GB_WIDTH - 1;
		u8 bg_x = disp_x + GBC->hram[GB_IO_SCX]; // add scroll background X

		// get tile index for current background tile
		u8 idx = GBC->vram[bg_map + (bg_x >> 3)];
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
		u8 idxAtt = GBC->vram[bg_map + (bg_x >> 3) + 0x2000]; // attributes
#endif
		// tile pixel coordinates
		u8 py = bg_y & 7;
		u8 px = 7 - (bg_x & 7);

		// select addressing mode
		u16 tile;
		if ((GBC->hram[GB_IO_LCDC] & B4) != 0)
			tile = 0 + idx*0x10;
		else
			tile = 0x0800 + ((idx + 0x80) & 0xff)*0x10;

		u8 t1, t2;
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
		if (GBC->gbmode != GB_MODE_GB)
		{
			if ((idxAtt & 0x08) != 0) tile += 0x2000; // VRAM character bank 2
			if ((idxAtt & 0x40) != 0) tile += 2*(7 - py); // flip up/down
		}
		if ((idxAtt & 0x40) == 0) tile += 2*py;

		// fetch first tile
		if ((GBC->gbmode != GB_MODE_GB) && ((idxAtt & 0x20) != 0)) // flip left/right
		{
			// horizontal flip
			t1 = GBC->vram[tile] << px;
			t2 = GBC->vram[tile+1] << px;
		}
		else
		{
			t1 = GBC->vram[tile] >> px;
			t2 = GBC->vram[tile+1] >> px;
		}
#else
		tile += 2*py;

		// fetch first tile
		t1 = GBC->vram[tile] >> px;
		t2 = GBC->vram[tile+1] >> px;
#endif
		// draw tiles
		for (; disp_x != 0xff; disp_x--)
		{
			u8 c;
			if (px == 8)
			{
				// fetch next tile
				px = 0;
				bg_x = disp_x + GBC->hram[GB_IO_SCX];
				idx = GBC->vram[bg_map + (bg_x >> 3)];
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
				idxAtt = GBC->vram[bg_map + (bg_x >> 3) + 0x2000]; // attributes
#endif
				if ((GBC->hram[GB_IO_LCDC] & B4) != 0)
					tile = 0 + idx*0x10;
				else
					tile = 0x0800 + ((idx + 0x80) & 0xff)*0x10;

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
				if (GBC->gbmode != GB_MODE_GB)
				{
					if ((idxAtt & 0x08) != 0) tile += 0x2000; // VRAM character bank 2
					if ((idxAtt & 0x40) != 0) tile += 2*(7 - py); // flip up/down
				}
				if ((idxAtt & 0x40) == 0) tile += 2*py;
#else
				tile += 2*py;
#endif
				t1 = GBC->vram[tile];
				t2 = GBC->vram[tile+1];

			}

			// copy background
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
			if ((GBC->gbmode != GB_MODE_GB) && ((idxAtt & 0x20) != 0)) // flip left/right
			{
				// horizontal flip
				c = (((t1 & 0x80) >> 1) | (t2 & 0x80)) >> 6;
				pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
				pixelsPrio[disp_x] = (idxAtt >> 7);
				t1 <<= 1;
				t2 <<= 1;
			}
			else
			{
				c = (t1 & 0x01) | ((t2 & 0x01) << 1);
				if (GBC->gbmode != GB_MODE_GB)
				{
					pixels[disp_x] = (u8)(((idxAtt & 0x07) << 2) + c);
					pixelsPrio[disp_x] = (u8)(idxAtt >> 7);
				}
				else
				{
					pixels[disp_x] = GBC->bg_pal[c] | 0x20;
				}
				t1 >>= 1;
				t2 >>= 1;
			}
#else
			c = (t1 & 0x01) | ((t2 & 0x01) << 1);
			pixels[disp_x] = GBC->bg_pal[c] | 0x20;
			t1 >>= 1;
			t2 >>= 1;
#endif
			px++;
		}
	}

// ==== draw window

	// 0x40: LCD control register
	// (default 0x83 = BG/OBJ ON, LCDC operation)
	//    B0: 1=BG display on (only GB; GBC is always on)
	//    B1: 1=OBJ is on
	//    B2: 1=8x16 dots, 0=8x8 dots
	//    B3: background code area 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
	//    B4: character data 1=0x8000-0x8FFF, 0=0x8800-0x97FF
	//    B5: 1=windowing on
	//    B6: window code are selection 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
	//    B7: LCDC controller operation stop 1=LCDC on, 0=LCDC off
	int line2 = line - GBC->hram[GB_IO_WY];
	if (((GBC->hram[GB_IO_LCDC] & B5) != 0) && // if windowing is enabled
		(line2 >= 0) &&
		(GBC->hram[GB_IO_WX] <= 166))
	{
		// window map
		u16 win_line = ((GBC->hram[GB_IO_LCDC] & B6) != 0) ? 0x1c00 : 0x1800;
		win_line += (line2 >> 3) * 0x20;

		// X coordinate
		u8 disp_x = GB_WIDTH - 1;
		u8 win_x = disp_x - GBC->hram[GB_IO_WX] + 7;

		// look up tile
		u8 py = line2 & 7;
		u8 px = 7 - (win_x & 7);
		u8 idx = GBC->vram[win_line + (win_x >> 3)];
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
		u8 idxAtt = GBC->vram[win_line + (win_x >> 3) + 0x2000];
#endif
		u16 tile;
		if ((GBC->hram[GB_IO_LCDC] & B4) != 0)
			tile = 0 + idx*0x10;
		else
			tile = 0x0800 + ((idx + 0x80) & 0xff)*0x10;

		u8 t1, t2;
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
		if (GBC->gbmode != GB_MODE_GB)
		{
			if ((idxAtt & 0x08) != 0) tile += 0x2000; // VRAM character bank 2
			if ((idxAtt & 0x40) != 0) tile += 2*(7 - py); // flip up/down
		}
		if ((idxAtt & 0x40) == 0) tile += 2*py;

		// fetch first tile
		if ((GBC->gbmode != GB_MODE_GB) && ((idxAtt & 0x20) != 0)) // flip left/right
		{
			// horizontal flip
			t1 = GBC->vram[tile] << px;
			t2 = GBC->vram[tile+1] << px;
		}
		else
		{
			t1 = GBC->vram[tile] >> px;
			t2 = GBC->vram[tile+1] >> px;
		}
#else
		tile += 2*py;

		// fetch first tile
		t1 = GBC->vram[tile] >> px;
		t2 = GBC->vram[tile+1] >> px;
#endif

		// loop and copy window
		u8 end = ((GBC->hram[GB_IO_WX] < 7) ? 0 : (GBC->hram[GB_IO_WX] - 7)) - 1;

		for (; disp_x != end; disp_x--)
		{
			u8 c;
			if (px == 8)
			{
				// fetch next tile
				px = 0;
				win_x = disp_x - GBC->hram[GB_IO_WX] + 7;
				idx = GBC->vram[win_line + (win_x >> 3)];
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
				idxAtt = GBC->vram[win_line + (win_x >> 3) + 0x2000]; // attributes
#endif
				if ((GBC->hram[GB_IO_LCDC] & B4) != 0)
					tile = 0 + idx*0x10;
				else
					tile = 0x0800 + ((idx + 0x80) & 0xff)*0x10;

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
				if (GBC->gbmode != GB_MODE_GB)
				{
					if ((idxAtt & 0x08) != 0) tile += 0x2000; // VRAM character bank 2
					if ((idxAtt & 0x40) != 0) tile += 2*(7 - py); // flip up/down
				}
				if ((idxAtt & 0x40) == 0) tile += 2*py;
#else
				tile += 2*py;
#endif
				t1 = GBC->vram[tile];
				t2 = GBC->vram[tile+1];
			}

			// copy window
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
			if ((idxAtt & 0x20) != 0) // flip left/right
			{
				// horizontal flip
				c = (((t1 & 0x80) >> 1) | (t2 & 0x80)) >> 6;
				pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
				pixelsPrio[disp_x] = (idxAtt >> 7);
				t1 <<= 1;
				t2 <<= 1;
			}
			else
			{
				c = (t1 & 0x01) | ((t2 & 0x01) << 1);
				if (GBC->gbmode != GB_MODE_GB)
				{
					pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
					pixelsPrio[disp_x] = (u8)(idxAtt >> 7);
				}
				else
				{
					pixels[disp_x] = GBC->bg_pal[c] | 0x20;
				}
				t1 >>= 1;
				t2 >>= 1;
			}
#else
			c = (t1 & 0x01) | ((t2 & 0x01) << 1);
			pixels[disp_x] = GBC->bg_pal[c] | 0x20;
			t1 >>= 1;
			t2 >>= 1;
#endif
			px++;
		}
	}

// ==== draw sprites

	// 0x40: LCD control register
	// (default 0x83 = BG/OBJ ON, LCDC operation)
	//    B0: 1=BG display on (only GB; GBC is always on)
	//    B1: 1=OBJ is on
	//    B2: 1=8x16 dots, 0=8x8 dots
	//    B3: backghround code area 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
	//    B4: character data 1=0x8000-0x8FFF, 0=0x8800-0x97FF
	//    B5: 1=windowing on
	//    B6: window code are selection 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
	//    B7: LCDC controller operation stop 1=LCDC on, 0=LCDC off
	if ((GBC->hram[GB_IO_LCDC] & B1) != 0) // if objects are enabled
	{
		u8 sprite_number;
		for (sprite_number = GB_NUM_SPRITES-1; sprite_number != 0xff; sprite_number--)
		{
			u8 s = sprite_number;

			// load sprite parameters
			u8 oy = GBC->oam[4*s+0]; // sprite Y coordinate
			u8 ox = GBC->oam[4*s+1]; // sprite X coordinate
			u8 ot = GBC->oam[4*s+2] & (((GBC->hram[GB_IO_LCDC] & B2) != 0) ? 0xfe : 0xff); // tile number
			u8 of = GBC->oam[4*s+3]; // additional attributes

			// sprite is not on this line (object size = 8 or 16 lines)
			if ((line + (((GBC->hram[GB_IO_LCDC] & B2) != 0) ? 0 : 8) >= oy)
				|| (line + 16 < oy)) continue;

			// sprite is not visible
			if ((ox == 0) || (ox >= 168)) continue;

			// y flip
			u8 py = line - oy + 16;
			if ((of & B6) != 0) py = (((GBC->hram[GB_IO_LCDC] & B2) != 0) ? 15 : 7) - py;

			// fetch the tile
			u8 t1, t2;
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
			if (GBC->gbmode != GB_MODE_GB)
			{
				t1 = GBC->vram[((of & B3) << 10) + 0 + ot*0x10 + 2*py];
				t2 = GBC->vram[((of & B3) << 10) + 0 + ot*0x10 + 2*py + 1];
			}
			else
#endif
			{
				t1 = GBC->vram[0 + ot*0x10 + 2*py];
				t2 = GBC->vram[0 + ot*0x10 + 2*py + 1];
			}

			// handle x flip
			u8 dir, start, end, shift;
			if ((of & B5) != 0)
			{
				dir = 1;
				start = (ox < 8) ? 0 : (ox - 8);
				end = ox;
				if (end > GB_WIDTH) end = GB_WIDTH;
				shift = 8 - ox + start;
			}
			else
			{
				dir = (u8)-1;
				start = ox;
				if (start > GB_WIDTH) start = GB_WIDTH;
				start -= 1;
				end = ((ox < 8) ? 0 : (ox - 8)) - 1;
				shift = ox - (start + 1);
			}

			// copy tile
			t1 >>= shift;
			t2 >>= shift;

			u8 disp_x;
			for (disp_x = start; disp_x != end; disp_x += dir)
			{
				u8 c = (t1 & 1) | ((t2 & 1) << 1);

				// check transparency
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
				if (GBC->gbmode != GB_MODE_GB)
				{
					u8 isBackgroundDisabled = c && ((GBC->hram[GB_IO_LCDC] & B0) == 0);
					u8 isPixelPriorityNonConflicting = c &&
						!(pixelsPrio[disp_x] && (pixels[disp_x] & 3)) &&
						!((of & B7) && (pixels[disp_x] & 3));

					if (isBackgroundDisabled || isPixelPriorityNonConflicting)
					{
						// set pixel color
						pixels[disp_x] = ((of & 7) << 2) + c + 0x20;
					}
				}
				else
#endif

				if (c && !(of & B7 && !((pixels[disp_x] & 3) == GBC->bg_pal[0])))
				{
					// set pixel color
					pixels[disp_x] = (of & B4) ? GBC->sp_pal[c+4] : GBC->sp_pal[c];
					pixels[disp_x] |= (of & B4); // select OBJ0 or OBJ1

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
					// deselect BG palette
					pixels[disp_x] &= ~0x20;
#endif
				}

				t1 >>= 1;
				t2 >>= 1;
			}
		}
	}

	// copy line to frame buffer
	memcpy(&GBC->framebuf[line*GB_WIDTH], pixels, GB_WIDTH);
}

