
// ****************************************************************************
//
//                       IBM PC Emulator - output to LCD
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

// >>> Keep tables and functions in RAM (without "const") for faster access.

// current zoom level (0=full screen)
u8 PC_LCDZoom = 0;

// vertical sync wait
int PC_VSyncWait = 2;

// VSYNC flag
volatile Bool PC_VSync = False;

#if USE_EMU_PC_LCD		// use output PC to LCD display

#define EMU_LCD_WIDTH	320	// LCD display width
#define EMU_LCD_HEIGHT	240	// LCD display height

// max. zoom level by videomode (0=full screen)
//  Keep this table in RAM (without "const") for faster access.
u8 PC_LCDZoomMax[PC_VMODE_NUM] =
{
	0,	// 0 (CGA, EGA, VGA) text 40x25, 16 colors, resolution 320x200
	0,	// 1 (CGA, EGA, VGA) text 40x25, 16 colors, resolution 320x200
	2,	// 2 (CGA, EGA, VGA) text 80x25, 16 colors, resolution 640x200
	2,	// 3 (CGA, EGA, VGA) text 80x25, 16 colors, resolution 640x200
	0,	// 4 (CGA, EGA, VGA) graphics 320x200, 4 colors
	0,	// 5 (CGA, EGA, VGA) graphics 320x200, 4 colors
	2,	// 6 (CGA, EGA, VGA) graphics 640x200, 2 colors
	2,	// 7 (MDA, EGA, VGA) text 80x25, 2 colors, resolution 640x200
	0,	// 8 (PCjr, Tandy) graphics 160x200, 16 colors
	0,	// 9 (PCjr, Tandy) graphics 320x200, 16 colors
	2,	// 10 (PCjr, Tandy) graphics 640x200, 4 colors
	2,	// 11 (Tandy) graphics 640x200, 16 colors
	0,	// 12 (Emulator) text 20x15, 16 colors, resolution 160x120
	0,	// 13 (EGA, VGA) graphics 320x200, 16 colors
	2,	// 14 (EGA, VGA) graphics 640x200, 16 colors
	4,	// 15 (EGA, VGA) graphics 640x350, 2 colors
	4,	// 16 (EGA, VGA) graphics 640x350, 16 colors ... or 4 colors on EGA64 (planes 0 and 2)
	4,	// 17 (VGA) graphics 640x480, 2 colors
	4,	// 18 (VGA) graphics 640x480, 16 colors
	0,	// 19 (VGA) graphics 320x200, 256 colors
};

// shift zoom level
void PC_LCDShiftZoom()
{
	u8 n = PC_LCDZoom + 1;
	if (n > PC_LCDZoomMax[PC_Vmode.vmode]) n = 0;
	PC_LCDZoom = n;
}

// blend 2 colors together (used 6x)
static NOINLINE u16 FASTCODE NOFLASH(PC_LCDBlend2)(u16 col1, u16 col2)
{
	u16 col = (((col1 & 0xf81f) + (col2 & 0xf81f)) >> 1) & 0xf81f; // blend red and blue components
	col |= (((col1 & 0x07e0) + (col2 & 0x07e0)) >> 1) & 0x07e0; // blend green components
	return col;
}

// blend 4 colors together (used 4x)
static NOINLINE u16 FASTCODE NOFLASH(PC_LCDBlend4)(u16 col1, u16 col2, u16 col3, u16 col4)
{
	u16 col = (((col1 & 0xf81f) + (col2 & 0xf81f) + (col3 & 0xf81f) + (col4 & 0xf81f)) >> 2) & 0xf81f; // blend red and blue components
	col |= (((col1 & 0x07e0) + (col2 & 0x07e0) + (col3 & 0x07e0) + (col4 & 0x07e0)) >> 2) & 0x07e0; // blend green components
	return col;
}

// redraw mono line 320 pixels, no blending (used 2x)
static NOINLINE void FASTCODE NOFLASH(PC_LCDRedrawMono320)(const u8* base, int off, const u16* pal)
{
	int mask = PC_Vmode.mask;
	int col, m;
	for (col = 40; col > 0; col--) // columns
	{
		// read pixel mask
		m = base[off];
		off = (off + 1) & mask;

		// send 8 pixels
		DispSendImg2(pal[m >> 7]);
		DispSendImg2(pal[(m >> 6) & 1]);
		DispSendImg2(pal[(m >> 5) & 1]);
		DispSendImg2(pal[(m >> 4) & 1]);
		DispSendImg2(pal[(m >> 3) & 1]);
		DispSendImg2(pal[(m >> 2) & 1]);
		DispSendImg2(pal[(m >> 1) & 1]);
		DispSendImg2(pal[(m >> 0) & 1]);
	}
}

// redraw mono line 640 pixels, blending 2 pixels (used 2x)
static NOINLINE void FASTCODE NOFLASH(PC_LCDRedrawMono640)(const u8* base, int off, const u16* cc)
{
	int mask = PC_Vmode.mask;
	int col, m;
	for (col = 80; col > 0; col--) // columns
	{
		// read pixel mask
		m = base[off];
		off = (off + 1) & mask;

		// send 4 pixels
		DispSendImg2(cc[m >> 6]);
		DispSendImg2(cc[(m >> 4) & 3]);
		DispSendImg2(cc[(m >> 2) & 3]);
		DispSendImg2(cc[m & 3]);
	}
}

// redraw 16-color line 320 pixels, 4 planes, no blending (used 2x)
static NOINLINE void FASTCODE NOFLASH(PC_LCDRedraw16col320)(int off, const u16* pal)
{
	int col, bits, m;
	const u8* base = PC_Vmode.base;
	m = PC_Vmode.planesize;
	const u8* base2 = base + m;
	const u8* base3 = base2 + m;
	const u8* base4 = base3 + m;
	int mask = PC_Vmode.mask;
	u8 c1, c2, c3, c4;

	for (col = 0; col < 40; col++) // columns
	{
		c1 = base[off];
		c2 = base2[off];
		c3 = base3[off];
		c4 = base4[off];
		off = (off + 1) & mask;

		for (bits = 8; bits > 0; bits--) // bits
		{
			m = ((c4 >> 4) & B3) | ((c3 >> 5) & B2) | ((c2 >> 6) & B1) | ((c1 >> 7) & B0);
			DispSendImg2(pal[m]);

			c1 <<= 1;
			c2 <<= 1;
			c3 <<= 1;
			c4 <<= 1;
		}
	}
}

// redraw 16-color line 640 pixels, 4 planes, blending 2 pixels (used 2x)
static NOINLINE void FASTCODE NOFLASH(PC_LCDRedraw16col640)(int off, const u16* pal)
{
	int col, bits, m, m2;
	const u8* base = PC_Vmode.base;
	m = PC_Vmode.planesize;
	const u8* base2 = base + m;
	const u8* base3 = base2 + m;
	const u8* base4 = base3 + m;
	int mask = PC_Vmode.mask;
	u8 c1, c2, c3, c4;

	for (col = 0; col < 80; col++) // columns
	{
		c1 = base[off];
		c2 = base2[off];
		c3 = base3[off];
		c4 = base4[off];
		off = (off + 1) & mask;

		for (bits = 4; bits > 0; bits--) // bits
		{
			m = ((c4 >> 4) & B3) | ((c3 >> 5) & B2) | ((c2 >> 6) & B1) | ((c1 >> 7) & B0);
			m2 = ((c4 >> 3) & B3) | ((c3 >> 4) & B2) | ((c2 >> 5) & B1) | ((c1 >> 6) & B0);
			DispSendImg2(PC_LCDBlend2(pal[m], pal[m2]));

			c1 <<= 2;
			c2 <<= 2;
			c3 <<= 2;
			c4 <<= 2;
		}
	}
}

// redraw display to LCD
//  Size of the function, including static sub-functions: 2816+612=3428 bytes
void FASTCODE NOFLASH(PC_LCDRedraw)()
{
	int vmode = PC_Vmode.vmode;
	if (vmode >= PC_VMODE_NUM) return; // invalid video mode
	int zoom = PC_LCDZoom;

	// zoom correction
	const u8* base = PC_Vmode.base;
	int mask = PC_Vmode.mask;
	int off = PC_Vmode.off & mask; // offset of start of current plane
	int wb = PC_Vmode.wb;
	int cols = wb >> 1;
	int cgamask = (vmode == PC_VMODE_9) ? 3 : 1;
	if (zoom != 0)
	{
		if ((vmode == PC_VMODE_2) || (vmode == PC_VMODE_3) || (vmode == PC_VMODE_7))
		{
			vmode = PC_VMODE_0;
			cols = 40;
			off = (off + (zoom-1)*80) & mask;
		}

		if (vmode == PC_VMODE_10)
		{
			vmode = PC_VMODE_4;
			cgamask = 3;
			off = (off + (zoom-1)*80) & mask;
			wb = 160;
		}

		if (vmode == PC_VMODE_11)
		{
			vmode = PC_VMODE_9;
			cgamask = 7;
			off = (off + (zoom-1)*160) & mask;
			wb = 320;
		}

		if (vmode == PC_VMODE_14)
		{
			vmode = PC_VMODE_13;
			off = (off + (zoom-1)*40) & mask;
			wb = 80;
		}
	}
	const u16* pal = PC_Vmode.pal;

	// start sending image data
	DispStartImg(0, EMU_LCD_WIDTH, 0, EMU_LCD_HEIGHT);

	// VSYNC flag
	dmb();
	PC_VSync = False;
	dmb();

	switch (vmode)
	{
	// (CGA, EGA, VGA) text 40x25, 16 colors (4 + 4 bits), memory 2000 bytes at 0xB800,
	//  1 byte character + 1 byte attribute, font 8x8, resolution 320x200
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: 50 ms (20 fps)
	case PC_VMODE_0:
	case PC_VMODE_1:
	// (CGA, EGA, VGA) text 80x25, 16 colors (4 + 4 bits), memory 4000 bytes at 0xB800,
	// (MDA) text 80x25, 2 colors (4 + 4 bits), memory 4000 bytes at 0xB000
	//  1 byte character + 1 byte attribute, font 8x8, resolution 640x200
	// Width 640 pixels: every 2 pixels are merged to 1 pixel
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: full screen or zoom 50 ms (20 fps)
	case PC_VMODE_2:
	case PC_VMODE_3:
	case PC_VMODE_7:
		{
			// blink state (should be 1 - 2 Hz)
			Bool blink_on = ((TimeMs() & B9) == 0);

			// cursor (should be 2 - 4 Hz; cursor blinking rate should be double to text blinking rate)
			Bool cur_on = ((TimeMs() & B8) == 0);
			sPC_VmodeCur* cur = &PC_Vmode.cur[PC_Vmode.page]; // cursor on current page
			int cur_off = -1;
			if (cur_on && (cur->cury < 25)) // cursor can be disabled by placing it after last row
				cur_off = (PC_Vmode.off + 2*cur->curx + cur->cury*wb + 2) & mask; // cursor offset
			int curstart = PC_Vmode.curstart;
			int curend = PC_Vmode.curend;			

			// send data
			const u8* f = PC_Vmode.font;
			const u8* f2;
			int row, col, line, w;
			u8 ch, a, m;
			u16 cf, cb, cc[4], c0;
			int dupl = 1; // duplication counter
			Bool mda = (PC_Vmode.card == PC_CARD_MDA) || (PC_Vmode.vmode == PC_VMODE_7);
			for (row = 0; row < 25; row++) // text rows
			{
				for (line = 0; line < 8; line++) // character line
				{
					f2 = f + line; // current line of the font
					for (col = 0; col < cols; col++) // text columns
					{
						// read character and color attribute
						ch = base[off]; // read character
						off = (off + 1) & mask;
						a = base[off]; // read color attribute
						off = (off + 1) & mask;

						// get font mask
						m = f2[ch*8];

						// MDA underline
						if (mda && (line == 7) && ((a & 7) == 1)) m = 0xff;

						// prepare colors
						cf = pal[a & 0x0f]; // foreground color
						if (!PC_Vmode.blink)
						{
							// not blinking
							cb = pal[a >> 4]; // background color
						}
						else
						{
							// blinking
							cb = pal[(a >> 4) & 7]; // background color
							if ((blink_on) && ((a & B7) != 0)) m = 0;
						}

						// cursor (inverting)
						if (off == cur_off)
						{
							if (curstart <= curend)
							{
								if ((line >= curstart) && (line <= curend)) m = ~m;
							}
							else
							{
								if ((line >= curstart) || (line <= curend)) m = ~m;
							}
						}

						// blending pixels (blend 640 pixels to 320 pixels)
						if (cols == 80)
						{
							// blend colors
							c0 = PC_LCDBlend2(cf, cb);

							// initialize color table
							cc[0] = cb; // background color
							cc[1] = c0;
							cc[2] = c0;
							cc[3] = cf; // foreground color

							// send one line of the character
							for (w = 4; w > 0; w--)
							{
								// send color of the pixel
								DispSendImg2(cc[m >> 6]);

								// shift mask
								m <<= 2;
							}
						}

						// no blend
						else
						{
							// send one line of the character
							for (w = 8; w > 0; w--)
							{
								// send color of the pixel
								DispSendImg2(((m & B7) == 0) ? cb : cf);

								// shift mask
								m <<= 1;
							}
						}
					}

					// return to start of the row
					off = (off - (cols << 1)) & mask;

					// duplicate every 5th line
					dupl++;
					if (dupl == 6)
					{
						dupl = 0;
						line--; // this line will be repeated
					}
				}

				// shift to next row
				off = (off + wb) & mask;
			}
		}
		break;

	// (CGA, EGA, VGA) graphics 320x200, 4 colors (2 bits), memory 16000 bytes at 0xB800
	//		... interlaced 8 KB even lines + 8 KB odd lines (each line is 80 bytes)
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: 50 ms (20 fps)
	case PC_VMODE_4:
	case PC_VMODE_5:
		{
			// send data
			int col, line, m;
			int dupl = 1; // duplication counter
			int cgaline = 0;
			for (line = 0; line < 240; line++) // lines
			{
				for (col = 0; col < 80; col++) // columns
				{
					// read pixel mask
					m = base[off];
					off = (off + 1) & mask;

					// send 4 pixels
					DispSendImg2(pal[m >> 6]);
					DispSendImg2(pal[(m >> 4) & 3]);
					DispSendImg2(pal[(m >> 2) & 3]);
					DispSendImg2(pal[m & 3]);
				}
				off = (off - 80) & mask;

				// duplicate every 5th line
				dupl++;
				if (dupl == 6)
					dupl = 0; // repeat this line
				else
				{
					if ((cgaline & cgamask) != cgamask)
						base += 0x2000;
					else
					{
						off = (off + wb) & mask;
						base -= cgamask*0x2000;
					}
					cgaline++;
				}
			}
		}
		break;

	// (CGA, EGA, VGA) graphics 640x200, 2 colors (1 bit), memory 16000 bytes at 0xB800
	//		... interlaced 8 KB even lines + 8 KB odd lines (each line is 80 bytes)
	// Width 640 pixels: every 2 pixels are merged to 1 pixel
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: full screen or zoom 50 ms (20 fps)
	case PC_VMODE_6:
		{
			int line;
			int dupl = 1; // duplication counter
			int cgaline = 0;
			if (zoom == 2) off = (off + 40) & mask;

			// prepare colors
			u16 cc[4];
			if (zoom == 0)
			{
				cc[0] = pal[0];
				cc[3] = pal[1];
				cc[1] = PC_LCDBlend2(pal[0], pal[1]);
				cc[2] = cc[1];
			}

			// send data
			for (line = 0; line < 240; line++) // lines
			{
				if (zoom != 0)
					// redraw mono line 320 pixels, no blending
					PC_LCDRedrawMono320(base, off, pal);
				else
					// redraw mono line 640 pixels, blending 2 pixels
					PC_LCDRedrawMono640(base, off, cc);

				// duplicate every 5th line
				dupl++;
				if (dupl == 6)
					dupl = 0; // repeat this line
				else
				{
					if ((cgaline & 1) == 0)
						base += 0x2000;
					else
					{
						off = (off + 80) & mask;
						base -= 0x2000;
					}
					cgaline++;
				}
			}
		}
		break;

	// (PCjr, Tandy) graphics 160x200, 16 colors (4 bits), memory 16000 bytes at 0xB8000, packed format, 1 byte contains 2 pixels
	//		... interlaced 8 KB even lines + 8 KB odd lines (each line is 80 bytes)
	// Width 160 pixels: every pixel is duplicated
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: 50 ms (20 fps)
	case PC_VMODE_8:
		{
			// send data
			int col, line, m;
			u16 cc;
			int dupl = 1; // duplication counter
			int cgaline = 0;
			for (line = 0; line < 240; line++) // lines
			{
				for (col = 0; col < 80; col++) // columns
				{
					// read pixel mask
					m = base[off];
					off = (off + 1) & mask;

					// send 4 pixels
					cc = pal[m >> 4];
					DispSendImg2(cc);
					DispSendImg2(cc);
					cc = pal[m & 0x0f];
					DispSendImg2(cc);
					DispSendImg2(cc);
				}
				off = (off - 80) & mask;

				// duplicate every 5th line
				dupl++;
				if (dupl == 6)
					dupl = 0; // repeat this line
				else
				{
					if ((cgaline & 1) == 0)
						base += 0x2000;
					else
					{
						off = (off + 80) & mask;
						base -= 0x2000;
					}
					cgaline++;
				}
			}
		}
		break;

	// (PCjr, Tandy) graphics 320x200, 16 colors (4 bits), memory 32000 bytes at 0xB8000, packed format, 1 byte contains 2 pixels
	//		... interlaced 8 KB line 0 4 8... + 8 KB line 1 5 9... + 8 KB line 2 6 10... + 8 KB line 3 7 11.. (each line is 160 bytes)
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: 50 ms (20 fps)
	case PC_VMODE_9:
		{
			// send data
			int col, line, m;
			u16 cc;
			int dupl = 1; // duplication counter
			int cgaline = 0;
			for (line = 0; line < 240; line++) // lines
			{
				for (col = 0; col < 160; col++) // columns
				{
					// read pixel mask
					m = base[off];
					off = (off + 1) & mask;

					// send 2 pixels
					cc = pal[m >> 4];
					DispSendImg2(cc);
					cc = pal[m & 0x0f];
					DispSendImg2(cc);
				}
				off = (off - 160) & mask;

				// duplicate every 5th line
				dupl++;
				if (dupl == 6)
					dupl = 0; // repeat this line
				else
				{
					if ((cgaline & cgamask) != cgamask)
						base += 0x2000;
					else
					{
						off = (off + wb) & mask;
						base -= cgamask*0x2000;
					}
					cgaline++;
				}
			}
		}
		break;

	// (PCjr, Tandy) graphics 640x200, 4 colors (2 bits), memory 32000 bytes at 0xB8000, packed format, 1 byte contains 4 pixels
	//		... interlaced 8 KB line 0 4 8... + 8 KB line 1 5 9... + 8 KB line 2 6 10... + 8 KB line 3 7 11.. (each line is 160 bytes)
	// Width 640 pixels: every 2 pixels are merged to 1 pixel
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: full screen or zoom 50 ms (20 fps)
	case PC_VMODE_10:
		{
			// send data
			int col, line, m;
			int dupl = 1; // duplication counter
			int cgaline = 0;

			// prepare blending colors
			u16 cc[16], cc1;
			int i,j,k;
			i = 0;
			for (j = 0; j < 4; j++)
			{
				cc1 = pal[j];

				for (k = 0; k < 4; k++)
				{
					cc[i] = PC_LCDBlend2(cc1, pal[k]);
					i++;
				}
			}

			for (line = 0; line < 240; line++) // lines
			{
				for (col = 0; col < 160; col++) // columns
				{
					// read pixel mask
					m = base[off];
					off = (off + 1) & mask;

					// send 2 pixels
					DispSendImg2(cc[m >> 4]);
					DispSendImg2(cc[m & 0x0f]);
				}
				off = (off - 160) & mask;

				// duplicate every 5th line
				dupl++;
				if (dupl == 6)
					dupl = 0; // repeat this line
				else
				{
					if ((cgaline & 3) != 3)
						base += 0x2000;
					else
					{
						off = (off + 160) & mask;
						base -= 3*0x2000;
					}
					cgaline++;
				}
			}
		}
		break;

	// (Tandy) graphics 640x200, 16 colors (4 bits), memory 64000 bytes at 0xB0000, packed format, 1 byte contains 2 pixels ... interlaced on 8 segments 8 KB
	// Width 640 pixels: every 2 pixels are merged to 1 pixel
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: full screen or zoom 50 ms (20 fps)
	case PC_VMODE_11:
		{
			// send data
			int col, line, m;
			int dupl = 1; // duplication counter
			int cgaline = 0;

			for (line = 0; line < 240; line++) // lines
			{
				for (col = 0; col < 320; col++) // columns
				{
					// read pixel mask
					m = base[off];
					off = (off + 1) & mask;

					// send 2 pixels
					DispSendImg2(PC_LCDBlend2(pal[m >> 4], pal[m & 0x0f]));
				}
				off = (off - 320) & mask;

				// duplicate every 5th line
				dupl++;
				if (dupl == 6)
					dupl = 0; // repeat this line
				else
				{
					if ((cgaline & 7) != 7)
						base += 0x2000;
					else
					{
						off = (off + 320) & mask;
						base -= 7*0x2000;
					}
					cgaline++;
				}
			}
		}
		break;

	// (Emulator) text 20x15, 16 colors (4 + 4 bits), memory 600 bytes at 0xB80000, 1 byte character + 1 byte attribute, font 8x8, resolution 160x120
	// Redraw time: 50 ms (20 fps)
	case PC_VMODE_12:
		{
			// blink state (should be 1 - 2 Hz)
			Bool blink_on = ((TimeMs() & B9) == 0);

			// cursor (should be 2 - 4 Hz; cursor blinking rate should be double to text blinking rate)
			Bool cur_on = ((TimeMs() & B8) == 0);
			sPC_VmodeCur* cur = &PC_Vmode.cur[PC_Vmode.page]; // cursor on current page
			int cur_off = -1;
			if (cur_on && (cur->cury < 15)) // cursor can be disabled by placing it after last row
				cur_off = (PC_Vmode.off + 2*cur->curx + cur->cury*wb + 2) & mask; // cursor offset
			int curstart = PC_Vmode.curstart;
			int curend = PC_Vmode.curend;			

			// send data
			const u8* f = PC_Vmode.font;
			const u8* f2;
			int row, col, line, w;
			u8 ch, a, m;
			u16 cf, cb, c0;
			int dupl = 1; // duplication counter
			Bool mda = PC_Vmode.card == PC_CARD_MDA;
			for (row = 0; row < 15; row++) // text rows
			{
				for (line = 0; line < 8; line++) // character line
				{
					f2 = f + line; // current line of the font
					for(col = 0; col < 20; col++) // text columns
					{
						// read character and color attribute
						ch = base[off]; // read character
						off = (off + 1) & mask;
						a = base[off]; // read color attribute
						off = (off + 1) & mask;

						// get font mask
						m = f2[ch*8];

						// MDA underline
						if (mda && (line == 7) && ((a & 7) == 1)) m = 0xff;

						// prepare colors
						cf = pal[a & 0x0f]; // foreground color
						if (!PC_Vmode.blink)
						{
							// not blinking
							cb = pal[a >> 4]; // background color
						}
						else
						{
							// blinking
							cb = pal[(a >> 4) & 7]; // background color
							if ((blink_on) && ((a & B7) != 0)) m = 0;
						}

						// cursor (inverting)
						if (off == cur_off)
						{
							if (curstart <= curend)
							{
								if ((line >= curstart) && (line <= curend)) m = ~m;
							}
							else
							{
								if ((line >= curstart) || (line <= curend)) m = ~m;
							}
						}

						// send one line of the character
						for (w = 8; w > 0; w--)
						{
							c0 = ((m & B7) == 0) ? cb : cf;

							// send color of the pixel
							DispSendImg2(c0);
							DispSendImg2(c0);

							// shift mask
							m <<= 1;
						}
					}

					// return to start of the row
					off = (off - 40) & mask;

					// duplicate every 2nd line
					dupl ^= 1;
					if (dupl == 0) line--; // this line will be repeated
				}

				// shift to next row
				off = (off + 40) & mask;
			}
		}
		break;

	// (EGA, VGA) graphics 320x200, 16 colors (4 bits), memory 32000 bytes at 0xA000
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: 50 ms (20 fps)
	case PC_VMODE_13:
		{
			// prepare palettes
			int dupl = 1; // duplication counter
			int line;

			// send data
			for (line = 0; line < 240; line++) // lines
			{
				// redraw 16-color line 320 pixels, 4 planes, no blending
				PC_LCDRedraw16col320(off, pal);
				off = (off + wb) & mask;

				// duplicate every 5th line
				dupl++;
				if (dupl == 6)
				{
					dupl = 0;
					off = (off - wb) & mask;
				}
			}
		}
		break;

	// (EGA, VGA) graphics 640x200, 16 colors (4 bits), memory 64000 bytes at 0xA000
	// Width 640 pixels: every 2 pixels are merged to 1 pixel
	// Height 200 lines: every 5th line is duplicated (as 6th line) to expand to 240 lines
	// Redraw time: full screen 57 ms (17 fps), zoom 50 ms (20 fps)
	case PC_VMODE_14:
		{
			// prepare palettes
			int dupl = 1; // duplication counter
			int line;

			// send data
			for (line = 0; line < 240; line++) // lines
			{
				// redraw 16-color line 640 pixels, 4 planes, blending 2 pixels
				PC_LCDRedraw16col640(off, pal);
				off = (off + 80) & mask;

				// duplicate every 5th line
				dupl++;
				if (dupl == 6)
				{
					dupl = 0;
					off = (off - 80) & mask;
				}
			}
		}
		break;

	// (EGA, VGA) graphics 640x350, 2 colors (1 bits), memory 28000 bytes at 0xA000
	// Width 640 pixels: every 2 pixels are merged to 1 pixel
	// Height 350 lines: every 2nd and 3rd lines are merged, draw 233 lines
	// Redraw time: full screen or zoom 50 ms (20 fps)
	case PC_VMODE_15:
	// (VGA) graphics 640x480, 2 colors (1 bits), memory 38400 bytes at 0xA000
	// Width 640 pixels: every 2 pixels are merged to 1 pixel
	// Height 480 lines: every 2 lines are merged, draw 240 lines
	// Redraw time: full screen or zoom 50 ms (20 fps)
	case PC_VMODE_17:
		{
			int col, line, m, m2;
			int dupl = 0; // duplication counter

			if (zoom != 0)
			{
				zoom--;
				int k = (zoom & 1) * 40;
				if (zoom >= 2) k += (vmode == PC_VMODE_17) ? ((480 - 240)*80) : ((350 - 240)*80);
				off = (off + k) & mask;

				// send data
				for (line = 0; line < 240; line++) // lines
				{
					// redraw mono line 320 pixels, no blending
					PC_LCDRedrawMono320(base, off, pal);
					off = (off + 80) & mask;
				}
			}
			else
			{
				// prepare colors
				u16 cc[4];
				cc[0] = pal[0];
				cc[3] = pal[1];
				cc[1] = PC_LCDBlend2(pal[0], pal[1]);
				cc[2] = cc[1];

				u16 cc2[16];
				cc2[0] = pal[0]; // 0000: 4:0
				cc2[1] = PC_LCDBlend4(pal[0], pal[0], pal[0], pal[1]); // 0001: 3:1
				cc2[2] = cc2[1]; // 0010: 3:1
				cc2[3] = PC_LCDBlend2(pal[0], pal[1]); // 0011: 2:2
				cc2[4] = cc2[1]; // 0100: 3:1
				cc2[5] = cc2[3]; // 0101: 2:2
				cc2[6] = cc2[3]; // 0110: 2:2
				cc2[7] = PC_LCDBlend4(pal[0], pal[1], pal[1], pal[1]); // 0111: 1:3
				cc2[8] = cc2[1]; // 1000: 3:1
				cc2[9] = cc2[3]; // 1001: 2:2
				cc2[10] = cc2[3]; // 1010: 2:2
				cc2[11] = cc2[7]; // 1011: 1:3
				cc2[12] = cc2[3]; // 1100: 2:2
				cc2[13] = cc2[7]; // 1101: 1:3
				cc2[14] = cc2[7]; // 1110: 1:3
				cc2[15] = pal[1]; // 1111: 0:4

				// draw 3 black lines
				if (vmode == PC_VMODE_15) for (line = 3*320; line > 0; line--) DispSendImg2(0);

				// send data
				int n = (vmode == PC_VMODE_15) ? 233 : 240;
				int dupl_n = (vmode == PC_VMODE_15) ? 2 : 1;
				for (line = 0; line < n; line++) // lines
				{
					// merge lines
					dupl++;
					if (dupl == dupl_n)
					{
						dupl = 0;

						for (col = 0; col < 80; col++) // columns
						{
							// read pixel mask
							m = base[off];
							off = (off + 1) & mask;
							m2 = base[(off + 80-1) & mask];

							// send 4 pixels
							DispSendImg2(cc2[(m >> 6) | ((m2 >> 4) & 0x0c)]);
							DispSendImg2(cc2[((m >> 4) & 3) | ((m2 >> 2) & 0x0c)]);
							DispSendImg2(cc2[((m >> 2) & 3) | (m2 & 0x0c)]);
							DispSendImg2(cc2[(m & 3) | ((m2 << 2) & 0x0c)]);
						}

						off = (off + 80) & mask;
					}

					// draw lines not merged
					else
					{
						// redraw mono line 640 pixels, blending 2 pixels
						PC_LCDRedrawMono640(base, off, cc);
						off = (off + 80) & mask;
					}
				}

				// draw 4 black lines
				if (vmode == PC_VMODE_15) for (line = 4*320; line > 0; line--) DispSendImg2(0);
			}
		}
		break;

	// (EGA, VGA) graphics 640x350, 16 colors (4 bits), memory 112000 bytes at 0xA000 ... or 4 colors (2 bits) on EGA64
	// Width 640 pixels: every 2 pixels are merged to 1 pixel
	// Height 350 lines: every 2nd and 3rd lines are merged, draw 233 lines
	// Redraw time EGA: full screen 72 ms (13 fps), zoom 50 ms (20 fps)
	// Redraw time EGA64: full screen 58 ms (17 fps), zoom 50 ms (20 fps)
	case PC_VMODE_16:
		if (PC_Vmode.usedmem < BIT(PC_VMEM_128K)) // EGA card 64 KB
		{
			// prepare palettes
			int dupl = 1; // duplication counter
			int line, col, bits, m, m2, m3, m4;
			const u8* base2 = base + PC_Vmode.planesize;
			u8 c1, c3, c5, c7;

			// zoom
			if (zoom != 0)
			{
				zoom--;
				int k = (zoom & 1) * 40;
				if (zoom >= 2) k += ((350 - 240)*80);
				off = (off + k) & mask;

				// send data
				for (line = 0; line < 240; line++) // lines
				{
					for (col = 0; col < 40; col++) // columns
					{
						c1 = base[off];
						c3 = base2[off];
						off = (off + 1) & mask;

						for (bits = 8; bits > 0; bits--) // bits
						{
							m = ((c3 >> 5) & B2) | ((c1 >> 7) & B0);
							DispSendImg2(pal[m]);

							c1 <<= 1;
							c3 <<= 1;
						}
					}

					off = (off + 40) & mask;
				}
			}
			else
			{
				// draw 3 black lines
				for (line = 3*320; line > 0; line--) DispSendImg2(0);

				// send data
				for (line = 0; line < 233; line++) // lines
				{
					// merge lines
					dupl++;
					if (dupl == 2)
					{
						dupl = 0;

						for (col = 0; col < 80; col++) // columns
						{
							c1 = base[off];
							c3 = base2[off];
							off = (off + 1) & mask;

							int k = (off + 80-1) & mask;
							c5 = base[k];
							c7 = base2[k];

							for (bits = 4; bits > 0; bits--) // bits
							{
								m = ((c3 >> 5) & B2) | ((c1 >> 7) & B0);
								m2 = ((c3 >> 4) & B2) | ((c1 >> 6) & B0);
								c1 <<= 2;
								c3 <<= 2;

								m3 = ((c7 >> 5) & B2) | ((c5 >> 7) & B0);
								m4 = ((c7 >> 4) & B2) | ((c5 >> 6) & B0);
								c5 <<= 2;
								c7 <<= 2;
			
								DispSendImg2(PC_LCDBlend4(pal[m], pal[m2], pal[m3], pal[m4]));
							}
						}

						off = (off + 80) & mask;
					}

					// draw lines not merged
					else
					{
						for (col = 0; col < 80; col++) // columns
						{
							c1 = base[off];
							c3 = base2[off];
							off = (off + 1) & mask;

							for (bits = 4; bits > 0; bits--) // bits
							{
								m = ((c3 >> 5) & B2) | ((c1 >> 7) & B0);
								m2 = ((c3 >> 4) & B2) | ((c1 >> 6) & B0);
								c1 <<= 2;
								c3 <<= 2;

								DispSendImg2(PC_LCDBlend2(pal[m], pal[m2]));
							}
						}
					}
				}

				// draw 4 black lines
				for (line = 4*320; line > 0; line--) DispSendImg2(0);
			}
			break;
		}
		// continue to mode 18 (no break!)

	// (VGA) graphics 640x480, 16 colors (4 bits), memory 153600 bytes at 0xA000
	// Width 640 pixels: every 2 pixels are merged to 1 pixel
	// Height 480 lines: every 2 lines are merged, draw 240 lines
	// Redraw time: full screen 90 ms (11 fps), zoom 50 ms (20 fps)
	case PC_VMODE_18:
		{
			// prepare palettes
			int dupl = 0; // duplication counter
			int line, col, bits, m, m2, m3, m4;
			m = PC_Vmode.planesize;
			const u8* base2 = base + m;
			const u8* base3 = base2 + m;
			const u8* base4 = base3 + m;
			u8 c1, c2, c3, c4, c5, c6, c7, c8;

			// zoom
			if (zoom != 0)
			{
				zoom--;
				int k = (zoom & 1) * 40;
				if (zoom >= 2) k += (vmode == PC_VMODE_18) ? ((480 - 240)*80) : ((350 - 240)*80);
				off = (off + k) & mask;

				// send data
				for (line = 0; line < 240; line++) // lines
				{
					// redraw 16-color line 320 pixels, 4 planes, no blending
					PC_LCDRedraw16col320(off, pal);
					off = (off + 80) & mask;
				}
			}
			else
			{
				// draw 3 black lines
				if (vmode == PC_VMODE_16) for (line = 3*320; line > 0; line--) DispSendImg2(0);

				// send data
				int n = (vmode == PC_VMODE_16) ? 233 : 240;
				int dupl_n = (vmode == PC_VMODE_16) ? 2 : 1;
				for (line = 0; line < n; line++) // lines
				{
					// merge lines
					dupl++;
					if (dupl == dupl_n)
					{
						dupl = 0;

						for (col = 0; col < 80; col++) // columns
						{
							c1 = base[off];
							c2 = base2[off];
							c3 = base3[off];
							c4 = base4[off];
							off = (off + 1) & mask;

							int k = (off + 80-1) & mask;
							c5 = base[k];
							c6 = base2[k];
							c7 = base3[k];
							c8 = base4[k];

							for (bits = 4; bits > 0; bits--) // bits
							{
								m = ((c4 >> 4) & B3) | ((c3 >> 5) & B2) | ((c2 >> 6) & B1) | ((c1 >> 7) & B0);
								m2 = ((c4 >> 3) & B3) | ((c3 >> 4) & B2) | ((c2 >> 5) & B1) | ((c1 >> 6) & B0);
								c1 <<= 2;
								c2 <<= 2;
								c3 <<= 2;
								c4 <<= 2;

								m3 = ((c8 >> 4) & B3) | ((c7 >> 5) & B2) | ((c6 >> 6) & B1) | ((c5 >> 7) & B0);
								m4 = ((c8 >> 3) & B3) | ((c7 >> 4) & B2) | ((c6 >> 5) & B1) | ((c5 >> 6) & B0);
								c5 <<= 2;
								c6 <<= 2;
								c7 <<= 2;
								c8 <<= 2;
			
								DispSendImg2(PC_LCDBlend4(pal[m], pal[m2], pal[m3], pal[m4]));
							}
						}
					}

					// draw lines not merged
					else
					{
						// redraw 16-color line 640 pixels, 4 planes, blending 2 pixels
						PC_LCDRedraw16col640(off, pal);
					}

					off = (off + 80) & mask;
				}

				// draw 4 black lines
				if (vmode == PC_VMODE_16) for (line = 4*320; line > 0; line--) DispSendImg2(0);
			}
		}
		break;

	// (VGA) graphics 320x200, 256 colors (8 bits), memory 64000 bytes at 0xA000
	// Redraw time: full screen or zoom 50 ms (20 fps)
	case PC_VMODE_19:
		{
			// prepare palettes
			int col, line;
			int dupl = 1; // duplication counter
			for (line = 0; line < 240; line++) // lines
			{
				for (col = 0; col < 320; col++) // pixels
				{
					DispSendImg2(pal[base[off]]);
					off = (off + 1) & mask;
				}

				// duplicate every 5th line
				dupl++;
				if (dupl == 6)
				{
					dupl = 0;
					off = (off - 320) & mask; // this line will be repeated
				}
			}
		}
		break;

	// unsupported videomode
	default:
		break;
	}

	// VSYNC flag
	dmb();
	PC_VSync = True;
	dmb();
	WaitMs(PC_VSyncWait);

	// stop sending data
	DispStopImg();
}

#endif // USE_EMU_PC_LCD
