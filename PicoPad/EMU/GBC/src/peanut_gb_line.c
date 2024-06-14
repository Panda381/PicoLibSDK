/**
 * MIT License
 *
 * Copyright (c) 2018-2023 Mahyar Koshkouei
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Please note that at least three parts of source code within this project was
 * taken from the SameBoy project at https://github.com/LIJI32/SameBoy/ which at
 * the time of this writing is released under the MIT License. Occurrences of
 * this code is marked as being taken from SameBoy with a comment.
 * SameBoy, and code marked as being taken from SameBoy,
 * is Copyright (c) 2015-2019 Lior Halphon.
 */

// Picopad modification: (2024) Miroslav Nemecek

#include "../include.h"

#if PEANUT_GB_HIGH_LCD_ACCURACY
// compare sprites for quick-sort
static int FASTCODE NOFLASH(compare_sprites)(const void *in1, const void *in2)
{
	const struct sprite_data *sd1, *sd2;
	int x_res;

	sd1 = (struct sprite_data *)in1;
	sd2 = (struct sprite_data *)in2;
	x_res = (int)sd1->x - (int)sd2->x;
	if(x_res != 0) return x_res;

	return (int)sd1->sprite_number - (int)sd2->sprite_number;
}
#endif

// draw line to output LCD buffer
void FASTCODE NOFLASH(__gb_draw_line)(struct gb_s *gb)
{
//	if(gb->direct.frame_skip && !gb->display.frame_skip_count)
//		return;

	u8 lcdc = gb->hram_io[IO_LCDC];
	u8 ly = gb->hram_io[IO_LY];

/*
	//If interlaced mode is activated, check if we need to draw the current line.
	if(gb->direct.interlace)
	{
		if((gb->display.interlace_count == 0 && (ly & 1) == 0)
			|| (gb->display.interlace_count == 1 && (ly & 1) == 1))
		{
			// Compensate for missing window draw if required.
			if(lcdc & LCDC_WINDOW_ENABLE
					&& ly >= gb->display.WY
					&& gb->hram_io[IO_WX] <= 166)
				gb->display.window_clear++;

			return;
		}
	}
*/

	// skip this line if buffer is full or this is not correct scan line
	{
		u8 winx = gb->frame_write; // write index into frame buffer
		winx++; // next write index
		if (winx >= LCD_FRAMEHEIGHT) winx = 0; // wrap write index

		if ((winx == gb->frame_read) || (ly != gb->frame_wline)) // ring buffer is full or we have incorrect scan line
		{
			// Compensate for missing window draw if required.
			if(lcdc & LCDC_WINDOW_ENABLE
				&& ly >= gb->display.WY
				&& gb->hram_io[IO_WX] <= 166)
					gb->display.window_clear++;
			return;
		}
	}

	u8 pixels[LCD_WIDTH];
	u8 pixelsPrio[LCD_WIDTH];  // do these pixels have priority over OAM?
	memset(pixels, 0, LCD_WIDTH);
	memset(pixelsPrio, 0, LCD_WIDTH);

	Bool cgbmode = gb->cgb.cgbMode;

	// If background is enabled, draw it.
	if(cgbmode || lcdc & LCDC_BG_ENABLE)
	{
		u8 bg_y, disp_x, bg_x, idx, py, px, t1, t2;
		u16 bg_map, tile;

		// Calculate current background line to draw. Constant because
		// this function draws only this one line each time it is
		// called.
		bg_y = ly + gb->hram_io[IO_SCY];

		// Get selected background map address for first tile
		// corresponding to current line.
		// 0x20 (32) is the width of a background tile, and the bit
		// shift is to calculate the address.
		bg_map =
			((lcdc & LCDC_BG_MAP) ?
			 VRAM_BMAP_2 : VRAM_BMAP_1)
			+ (bg_y >> 3) * 0x20;

		// The displays (what the player sees) X coordinate, drawn right
		// to left.
		disp_x = LCD_WIDTH - 1;

		// The X coordinate to begin drawing the background at.
		bg_x = disp_x + gb->hram_io[IO_SCX];

		// Get tile index for current background tile.
		idx = gb->vram[bg_map + (bg_x >> 3)];
		u8 idxAtt = gb->vram[bg_map + (bg_x >> 3) + 0x2000];

		// Y coordinate of tile pixel to draw.
		py = (bg_y & 0x07);
		// X coordinate of tile pixel to draw.
		px = 7 - (bg_x & 0x07);

		// Select addressing mode.
		if(lcdc & LCDC_TILE_SELECT)
			tile = VRAM_TILES_1 + idx * 0x10;
		else
			tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

		if(cgbmode)
		{
			if(idxAtt & 0x08) tile += 0x2000; // VRAM bank 2
			if(idxAtt & 0x40) tile += 2 * (7 - py);
		}
		if(!(idxAtt & 0x40))
		{
			tile += 2 * py;
		}

		// fetch first tile
		if(cgbmode && (idxAtt & 0x20))
		{  // Horizantal Flip
			t1 = gb->vram[tile] << px;
			t2 = gb->vram[tile + 1] << px;
		}
		else
		{
			t1 = gb->vram[tile] >> px;
			t2 = gb->vram[tile + 1] >> px;
		}

		for(; disp_x != 0xFF; disp_x--)
		{
			u8 c;

			if(px == 8)
			{
				// fetch next tile
				px = 0;
				bg_x = disp_x + gb->hram_io[IO_SCX];
				idx = gb->vram[bg_map + (bg_x >> 3)];
				idxAtt = gb->vram[bg_map + (bg_x >> 3) + 0x2000];

				if(lcdc & LCDC_TILE_SELECT)
					tile = VRAM_TILES_1 + idx * 0x10;
				else
					tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

				if(cgbmode)
				{
					if(idxAtt & 0x08) tile += 0x2000; // VRAM bank 2
					if(idxAtt & 0x40) tile += 2 * (7 - py);
				}
				if(!(idxAtt & 0x40))
				{
					tile += 2 * py;
				}

				t1 = gb->vram[tile];
				t2 = gb->vram[tile + 1];
			}

			// copy background
			if(cgbmode && (idxAtt & 0x20))
			{  // Horizantal Flip
				c = (((t1 & 0x80) >> 1) | (t2 & 0x80)) >> 6;
				pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
				pixelsPrio[disp_x] = (idxAtt >> 7);
				t1 = t1 << 1;
				t2 = t2 << 1;
			}
			else
			{
				c = (t1 & 0x1) | ((t2 & 0x1) << 1);
				if(cgbmode)
				{
					pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
					pixelsPrio[disp_x] = (idxAtt >> 7);
				}
				else
				{
					pixels[disp_x] = gb->display.bg_palette[c];
					pixels[disp_x] |= LCD_PALETTE_BG;
				}
				t1 = t1 >> 1;
				t2 = t2 >> 1;
			}

			px++;
		}
	}

	// draw window
	if(lcdc & LCDC_WINDOW_ENABLE
			&& ly >= gb->display.WY
			&& gb->hram_io[IO_WX] <= 166)
	{
		u16 win_line, tile;
		u8 disp_x, win_x, py, px, idx, t1, t2, end;

		// Calculate Window Map Address.
		win_line = (lcdc & LCDC_WINDOW_MAP) ?
				    VRAM_BMAP_2 : VRAM_BMAP_1;
		win_line += (gb->display.window_clear >> 3) * 0x20;

		disp_x = LCD_WIDTH - 1;
		win_x = disp_x - gb->hram_io[IO_WX] + 7;

		// look up tile
		py = gb->display.window_clear & 0x07;
		px = 7 - (win_x & 0x07);
		idx = gb->vram[win_line + (win_x >> 3)];
		u8 idxAtt = gb->vram[win_line + (win_x >> 3) + 0x2000];

		if(lcdc & LCDC_TILE_SELECT)
			tile = VRAM_TILES_1 + idx * 0x10;
		else
			tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

		if(cgbmode)
		{
			if(idxAtt & 0x08) tile += 0x2000; // VRAM bank 2
			if(idxAtt & 0x40) tile += 2 * (7 - py);
		}
		if(!(idxAtt & 0x40))
		{
			tile += 2 * py;
		}

		// fetch first tile
		if(cgbmode && (idxAtt & 0x20))
		{  // Horizantal Flip
			t1 = gb->vram[tile] << px;
			t2 = gb->vram[tile + 1] << px;
		}
		else
		{
			t1 = gb->vram[tile] >> px;
			t2 = gb->vram[tile + 1] >> px;
		}

		// loop & copy window
		end = (gb->hram_io[IO_WX] < 7 ? 0 : gb->hram_io[IO_WX] - 7) - 1;

		for(; disp_x != end; disp_x--)
		{
			u8 c;

			if(px == 8)
			{
				// fetch next tile
				px = 0;
				win_x = disp_x - gb->hram_io[IO_WX] + 7;
				idx = gb->vram[win_line + (win_x >> 3)];
				idxAtt = gb->vram[win_line + (win_x >> 3) + 0x2000];

				if(lcdc & LCDC_TILE_SELECT)
					tile = VRAM_TILES_1 + idx * 0x10;
				else
					tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

				if(cgbmode)
				{
					if(idxAtt & 0x08) tile += 0x2000; // VRAM bank 2
					if(idxAtt & 0x40) tile += 2 * (7 - py);
				}
				if(!(idxAtt & 0x40))
				{
					tile += 2 * py;
				}

				t1 = gb->vram[tile];
				t2 = gb->vram[tile + 1];
			}

			// copy window
			if(idxAtt & 0x20)
			{  // Horizantal Flip
				c = (((t1 & 0x80) >> 1) | (t2 & 0x80)) >> 6;
				pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
				pixelsPrio[disp_x] = (idxAtt >> 7);
				t1 = t1 << 1;
				t2 = t2 << 1;
			}
			else
			{
				c = (t1 & 0x1) | ((t2 & 0x1) << 1);
				if(cgbmode)
				{
					pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
					pixelsPrio[disp_x] = (idxAtt >> 7);
				}
				else
				{
					pixels[disp_x] = gb->display.bg_palette[c];
					pixels[disp_x] |= LCD_PALETTE_BG;
				}
				t1 = t1 >> 1;
				t2 = t2 >> 1;
			}

			px++;
		}

		gb->display.window_clear++; // advance window line
	}

	// draw sprites
	if(lcdc & LCDC_OBJ_ENABLE)
	{
		u8 sprite_number;
#if PEANUT_GB_HIGH_LCD_ACCURACY
		u8 number_of_sprites = 0;

		struct sprite_data sprites_to_render[NUM_SPRITES];

		// Record number of sprites on the line being rendered, limited
		// to the maximum number sprites that the Game Boy is able to
		// render on each line (10 sprites).
		for(sprite_number = 0;
				sprite_number < PEANUT_GB_ARRAYSIZE(sprites_to_render);
				sprite_number++)
		{
			// Sprite Y position.
			u8 OY = gb->oam[4 * sprite_number + 0];
			// Sprite X position.
			u8 OX = gb->oam[4 * sprite_number + 1];

			// If sprite isn't on this line, continue.
			if(ly +
				(lcdc & LCDC_OBJ_SIZE ? 0 : 8) >= OY
					|| ly + 16 < OY)
				continue;

			sprites_to_render[number_of_sprites].sprite_number = sprite_number;
			sprites_to_render[number_of_sprites].x = OX;
			number_of_sprites++;
		}

		if(!cgbmode)
		{
			// If maximum number of sprites reached, prioritise X
			// coordinate and object location in OAM.
			qsort(&sprites_to_render[0], number_of_sprites,
					sizeof(sprites_to_render[0]), compare_sprites);
		}

		if(number_of_sprites > MAX_SPRITES_LINE)
			number_of_sprites = MAX_SPRITES_LINE;
#endif

		// Render each sprite, from low priority to high priority.
#if PEANUT_GB_HIGH_LCD_ACCURACY
		// Render the top ten prioritised sprites on this scanline.
		for(sprite_number = number_of_sprites - 1;
				sprite_number != 0xFF;
				sprite_number--)
		{
			u8 s = sprites_to_render[sprite_number].sprite_number;
#else
		for (sprite_number = NUM_SPRITES - 1;
			sprite_number != 0xFF;
			sprite_number--)
		{
			u8 s = sprite_number;
#endif
			u8 py, t1, t2, dir, start, end, shift, disp_x;
			// Sprite Y position.
			u8 OY = gb->oam[4 * s + 0];
			// Sprite X position.
			u8 OX = gb->oam[4 * s + 1];
			// Sprite Tile/Pattern Number.
			u8 OT = gb->oam[4 * s + 2]
				     & (lcdc & LCDC_OBJ_SIZE ? 0xFE : 0xFF);
			// Additional attributes.
			u8 OF = gb->oam[4 * s + 3];

#if !PEANUT_GB_HIGH_LCD_ACCURACY
			// If sprite isn't on this line, continue.
			if(ly + (lcdc & LCDC_OBJ_SIZE ? 0 : 8) >= OY || ly + 16 < OY)
				continue;
#endif

			// Continue if sprite not visible.
			if(OX == 0 || OX >= 168)
				continue;

			// y flip
			py = ly - OY + 16;

			if(OF & OBJ_FLIP_Y)
				py = (lcdc & LCDC_OBJ_SIZE ? 15 : 7) - py;

			// fetch the tile
			if(cgbmode)
			{
				t1 = gb->vram[((OF & OBJ_BANK) << 10) + VRAM_TILES_1 + OT * 0x10 + 2 * py];
				t2 = gb->vram[((OF & OBJ_BANK) << 10) + VRAM_TILES_1 + OT * 0x10 + 2 * py + 1];
			}
			else
			{
				t1 = gb->vram[VRAM_TILES_1 + OT * 0x10 + 2 * py];
				t2 = gb->vram[VRAM_TILES_1 + OT * 0x10 + 2 * py + 1];
			}

			// handle x flip
			if(OF & OBJ_FLIP_X)
			{
				dir = 1;
				start = (OX < 8 ? 0 : OX - 8);
				end = MIN(OX, LCD_WIDTH);
				shift = 8 - OX + start;
			}
			else
			{
				dir = (u8)-1;
				start = MIN(OX, LCD_WIDTH) - 1;
				end = (OX < 8 ? 0 : OX - 8) - 1;
				shift = OX - (start + 1);
			}

			// copy tile
			t1 >>= shift;
			t2 >>= shift;

			// @TODO: Put for loop within the to if statements
			// because the BG priority bit will be the same for
			// all the pixels in the tile.
			for(disp_x = start; disp_x != end; disp_x += dir)
			{
				u8 c = (t1 & 0x1) | ((t2 & 0x1) << 1);
				// check transparency / sprite overlap / background overlap

				if(cgbmode)
				{
					u8 isBackgroundDisabled = c && !(lcdc & LCDC_BG_ENABLE);
					u8 isPixelPriorityNonConflicting = c &&
							!(pixelsPrio[disp_x] && (pixels[disp_x] & 0x3)) &&
							!((OF & OBJ_PRIORITY) && (pixels[disp_x] & 0x3));

					if(isBackgroundDisabled || isPixelPriorityNonConflicting)
					{
						// Set pixel colour.
						// add 0x20 to differentiate from BG
						pixels[disp_x] = ((OF & OBJ_CGB_PALETTE) << 2) + c + 0x20;
					}
				}
				else if(c && !(OF & OBJ_PRIORITY && !((pixels[disp_x] & 0x3) == gb->display.bg_palette[0])))
				{
					// Set pixel colour.
					pixels[disp_x] = (OF & OBJ_PALETTE)
						? gb->display.sp_palette[c + 4]
						: gb->display.sp_palette[c];
					// Set pixel palette (OBJ0 or OBJ1).
					pixels[disp_x] |= (OF & OBJ_PALETTE);

					// Deselect BG palette.
					pixels[disp_x] &= ~LCD_PALETTE_BG;
				}

				t1 = t1 >> 1;
				t2 = t2 >> 1;
			}
		}
	}

	// get write index into frame buffer
	u8 winx = gb->frame_write; // write index into frame buffer

	// prepare palette
	u16* pal = cgbmode ? gb->cgb.fixPalette : gbPalette;

	// convert pixels
	u8* s = pixels;
	u16* d = &gb->framebuf[winx*LCD_WIDTH];
	int x;
	for (x = LCD_WIDTH; x > 0; x--) *d++ = pal[*s++];

	// increase absolute index
	ly++;
	if (ly >= LCD_HEIGHT) ly = 0; // wrap current line
	gb->frame_wline = ly;

	// synchronize data
	dmb();

	// update write index
	winx++; // next write index
	if (winx >= LCD_FRAMEHEIGHT) winx = 0; // wrap write index
	gb->frame_write = winx;
}
