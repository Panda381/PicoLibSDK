
// ****************************************************************************
//
//                      Drawing to canvas at 1-bit format
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

// Note: The following 2 switches are not yet defined in the PicoLibSDK at this point, so the global.h file is included.
#if USE_DISPHSTX && DISPHSTX_PICOSDK	// 0=use PicoLibSDK library, 1=use PicoSDK original Raspberry SDK library
#include "disphstx_picolibsk.h"
#else
#include "../../global.h"
#endif

#if USE_DRAWCAN	&& USE_DRAWCAN1		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#include "../../_font/_include.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_float.h"
#include "../inc/lib_text.h"
#include "../inc/lib_rand.h"
#include "../inc/lib_drawcan.h"
#include "../inc/lib_drawcan1.h"

#ifndef WIDTH
#define WIDTH		320		// display width
#endif

#ifndef HEIGHT
#define HEIGHT		240		// display height
#endif

// default drawing canvas for 1-bit format
sDrawCan DrawCan1 = {
	// format
	.format = DRAWCAN_FORMAT_1,	// u8	format;		// canvas format CANVAS_*
	.colbit = 1,			// u8	colbit;		// number of bits per pixel
	.strip = 0,			// u8	strip;		// current strip index
	.stripnum = 1,			// u8	stripnum;	// number of strips

	// font
	.fontw = FONTW,			// u8	fontw;		// font width (number of pixels, max. 8)
	.fonth = FONTH,			// u8	fonth;		// font height (number of lines)
	.printinv = 0,			// u8	printinv;	// offset added to printed character (print characters 128 = inverted, 0 = normal)
	.printsize = 0,			// u8	printsize;	// font size 0=normal, 1=double-height, 2=double-width, 3=double-size

	// dimension
	.w = WIDTH,			// s16	w;		// width
	.h = HEIGHT,			// s16	h;		// height
	.wb = ((WIDTH+7)/8+3)&~3,	// s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	.striph = HEIGHT,		// s16	striph;		// strip height (number of lines)

	// print
	.printposnum = WIDTH/FONTW,	// u16	printposnum;	// number of text positions per row (= w / fontw)
	.printrownum = HEIGHT/FONTH,	// u16	printrownum;	// number of text rows (= h / fonth)
	.printpos = 0,			// u16	printpos;	// console print character position
	.printrow = 0,			// u16	printrow;	// console print character row
	.printcol = COL1_WHITE,		// u16	printcol;	// console print color

	// clipping
	.basey = 0,			// s16	basey;		// base Y of buffer strip
	.clipx1 = 0,			// s16	clipx1;		// clip X min
	.clipx2 = WIDTH,		// s16	clipx2;		// clip X+1 max
	.clipy1 = 0,			// s16	clipy1;		// clip Y min (current, limited by buffer strip)
	.clipy2 = HEIGHT,		// s16	clipy2;		// clip Y+1 max (current, limited by buffer strip)
	.clipy1_orig = 0,		// s16	clipy1_orig;	// clip Y min (original, without strip clipping)
	.clipy2_orig = HEIGHT,		// s16	clipy2_orig;	// clip Y+1 max (original, without strip clipping)

	// dirty window
	.dirtyx1 = 0,			// s16	dirtyx1;	// dirty window start X
	.dirtyx2 = WIDTH,		// s16	dirtyx2;	// dirty window end X
	.dirtyy1 = 0,			// s16	dirtyy1;	// dirty window start Y
	.dirtyy2 = HEIGHT,		// s16	dirtyy2;	// dirty window end Y

	// data buffer
	.buf = NULL,			// u8*	buf;		// image data buffer
	.frontbuf = NULL,		// u8*	fronbuf;	// front buffer, or NULL = use only one buffer

	// last system time of auto update
	.autoupdatelast = 0,		// u32	autoupdatelast;	// last system time of auto update

	// font
	.font = FONT,			// const u8* font;	// pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)

	// drawing functions interfaces
	// - Don't set the pointer to the function table, it would increase the size of the Loader
	.drawfnc = NULL, //&DrawCan1Fnc,	// const struct sDrawCanFnc_* drawfnc; // drawing functions
};

// current drawing canvas for 1-bit format
sDrawCan* pDrawCan1 = &DrawCan1;

// set default drawing canvas for 1-bit format
void SetDrawCan1(sDrawCan* can) { pDrawCan1 = can; }

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw1Pitch(int w) { return ((w + 7)/8 + 3) & ~3; }

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw1MaxWidth(int pitch) { return pitch*8; }

// convert RGB888 color to 1-bit pixel color Y1
u16 Draw1ColRgb(u8 r, u8 g, u8 b) { return COLOR1(r,g,b); }

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 1-bit pixel color Y1
u16 Draw1ColRand() { return COL1_RANDOM; }
#endif

// ----------------------------------------------------------------------------
//                            Clear canvas
// ----------------------------------------------------------------------------

// Clear canvas with color
void DrawCan1ClearCol(sDrawCan* can, u16 col8)
{
	u8 col = (u8)col8;
	int x = can->clipx1;
	int y = can->clipy1;
	int w = can->clipx2 - x;
	int h = can->clipy2 - y;
	DrawCan1Rect(can, x, y, w, h, col);
}
void Draw1ClearCol(u16 col) { DrawCan1ClearCol(pDrawCan1, col); }

// Clear canvas with black color
void DrawCan1Clear(sDrawCan* can) { DrawCan1ClearCol(can, COL1_BLACK); }
void Draw1Clear() { DrawCan1Clear(pDrawCan1); }

// ----------------------------------------------------------------------------
//                               Draw point
// ----------------------------------------------------------------------------

// Draw point (does not check clipping limits)
void DrawCan1Point_Fast(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// draw pixel
	u8* d = &can->buf[(x>>3) + (y - can->basey)*can->wb];
	x = 7 - (x & 7);
	*d = (*d & ~(0x01<<x)) | (col<<x);
}
void Draw1Point_Fast(int x, int y, u16 col) { DrawCan1Point_Fast(pDrawCan1, x, y, col); }

// Draw point (checks clipping limits)
void DrawCan1Point(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// draw pixel
	u8* d = &can->buf[(x>>3) + (y - can->basey)*can->wb];
	x = 7 - (x & 7);
	*d = (*d & ~(0x01<<x)) | (col<<x);
}
void Draw1Point(int x, int y, u16 col) { DrawCan1Point(pDrawCan1, x, y, col); }

// Draw point inverted (does not check clipping limits)
void DrawCan1PointInv_Fast(sDrawCan* can, int x, int y)
{
	// invert pixel
	u8* d = &can->buf[(x>>3) + (y - can->basey)*can->wb];
	x = 7 - (x & 7);
	*d ^= 0x01<<x;
}
void Draw1PointInv_Fast(int x, int y) { DrawCan1PointInv_Fast(pDrawCan1, x, y); }

// Draw point inverted (checks clipping limits)
void DrawCan1PointInv(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// invert pixel
	u8* d = &can->buf[(x>>3) + (y - can->basey)*can->wb];
	x = 7 - (x & 7);
	*d ^= 0x01<<x;
}
void Draw1PointInv(int x, int y) { DrawCan1PointInv(pDrawCan1, x, y); }

// ----------------------------------------------------------------------------
//                               Get point
// ----------------------------------------------------------------------------

// Get point (does not check clipping limits)
u16 DrawCan1GetPoint_Fast(sDrawCan* can, int x, int y)
{
	u8* d = &can->buf[(x>>3) + (y - can->basey)*can->wb];
	x = 7 - (x & 7);
	return (*d >> x) & 1;
}
u16 Draw1GetPoint_Fast(int x, int y) { return DrawCan1GetPoint_Fast(pDrawCan1, x, y); }

// Get point (checks clipping limits)
u16 DrawCan1GetPoint(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return COL1_BLACK;

	// get pixel
	u8* d = &can->buf[(x>>3) + (y - can->basey)*can->wb];
	x = 7 - (x & 7);
	return (*d >> x) & 1;
}
u16 Draw1GetPoint(int x, int y) { return DrawCan1GetPoint(pDrawCan1, x, y); }

// ----------------------------------------------------------------------------
//                            Draw rectangle
// ----------------------------------------------------------------------------

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan1Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col8)
{
	u8 col = (u8)col8;

	// prepare destination address
	int wb = can->wb;
	u8* d0 = &can->buf[x/8 + (y - can->basey)*wb];
	u8* d;

	// prepare multiply color
	u8 colcol = col|(col<<1)|(col<<2)|(col<<3);
	colcol = colcol | (colcol << 4);

	// mask of first pixels
	x &= 7;
	u8 m1 = 0xff << (8 - x);
	u8 col1 = colcol >> x;
	
	// width of inner pixels
	if (x > 0) w -= 8 - x;
	int w2 = w/8;

	// mask of last pixels
	u8 m2 = 0xff;
	u8 col2 = 0;
	if (w >= 0) // width is large enough
	{
		w -= w2*8;
		m2 = 0xff >> w;
		col2 = colcol << (8 - w);
	}
	else // last pixels are not valid
	{
		w += 8; // end of first pixels
		u8 m3 = 0xff >> w;
		m1 |= m3;
		col1 &= ~m3;
		w = 0; // no pixels left as last ones
	}

	int i;
	for (; h > 0; h--)
	{
		d = d0;

		// store first pixels
		if (x > 0)
		{
			*d = (*d & m1) | col1;
			d++;
		}

		// store inner pixels
		i = w2;
		for (; i > 0; i--) *d++ = colcol;

		// store last pixels
		if (w > 0) *d = (*d & m2) | col2;

		// shift buffer address
		d0 += wb;
	}
}
void Draw1Rect_Fast(int x, int y, int w, int h, u16 col) { DrawCan1Rect_Fast(pDrawCan1, x, y, w, h, col); }

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan1Rect(sDrawCan* can, int x, int y, int w, int h, u16 col8)
{
	u8 col = (u8)col8;

	// flip rectangle
	if (w < 0)
	{
		x += w;
		w = -w;
	}

	if (h < 0)
	{
		y += h;
		h = -h;
	}

	// limit x
	int k = can->clipx1 - x;
	if (k > 0)
	{
		w -= k;
		x += k;
	}

	// limit w
	k = can->clipx2;
	if (x + w > k) w = k - x;
	if (w <= 0) return;

	// limit y
	k = can->clipy1 - y;
	if (k > 0)
	{
		h -= k;
		y += k;
	}

	// limit h
	k = can->clipy2;
	if (y + h > k) h = k - y;
	if (h <= 0) return;

	// update dirty area
	DrawCanDirtyRect_Fast(can, x, y, w, h);

	// draw
	DrawCan1Rect_Fast(can, x, y, w, h, col);
}
void Draw1Rect(int x, int y, int w, int h, u16 col) { DrawCan1Rect(pDrawCan1, x, y, w, h, col); }

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan1RectInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// prepare destination address
	int wb = can->wb;
	u8* d0 = &can->buf[x/8 + (y - can->basey)*wb];
	u8* d;

	// mask of first pixels
	x &= 7;
	u8 m1 = 0xff >> x;
	
	// width of inner pixels
	if (x > 0) w -= 8 - x;
	int w2 = w/8;

	// mask of last pixels
	u8 m2 = 0xff;
	if (w >= 0) // width is large enough
	{
		w -= w2*8;
		m2 = 0xff << (8 - w);
	}
	else // last pixels are not valid
	{
		w += 8; // end of first pixels
		u8 m3 = 0xff >> w;
		m1 &= ~m3;
		w = 0; // no pixels left as last ones
	}

	int i;
	for (; h > 0; h--)
	{
		d = d0;

		// store first pixels
		if (x > 0)
		{
			*d ^= m1;
			d++;
		}

		// store inner pixels
		i = w2;
		for (; i > 0; i--)
		{
			*d = ~*d;
			d++;
		}

		// store last pixels
		if (w > 0) *d ^= m2;

		// shift buffer address
		d0 += wb;
	}
}
void Draw1RectInv_Fast(int x, int y, int w, int h) { DrawCan1RectInv_Fast(pDrawCan1, x, y, w, h); }

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan1RectInv(sDrawCan* can, int x, int y, int w, int h)
{
	// flip rectangle
	if (w < 0)
	{
		x += w;
		w = -w;
	}

	if (h < 0)
	{
		y += h;
		h = -h;
	}

	// limit x
	int k = can->clipx1 - x;
	if (k > 0)
	{
		w -= k;
		x += k;
	}

	// limit w
	k = can->clipx2;
	if (x + w > k) w = k - x;
	if (w <= 0) return;

	// limit y
	k = can->clipy1 - y;
	if (k > 0)
	{
		h -= k;
		y += k;
	}

	// limit h
	k = can->clipy2;
	if (y + h > k) h = k - y;
	if (h <= 0) return;

	// update dirty area
	DrawCanDirtyRect_Fast(can, x, y, w, h);

	// invert
	DrawCan1RectInv_Fast(can, x, y, w, h);
}
void Draw1RectInv(int x, int y, int w, int h) { DrawCan1RectInv(pDrawCan1, x, y, w, h); }

// ----------------------------------------------------------------------------
//                          Draw horizontal line
// ----------------------------------------------------------------------------

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan1HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col8)
{
	u8 col = (u8)col8;

	// prepare destination address
	int wb = can->wb;
	u8* d = &can->buf[x/8 + (y - can->basey)*wb];

	// prepare multiply color
	u8 colcol = col|(col<<1)|(col<<2)|(col<<3);
	colcol = colcol | (colcol << 4);

	// mask of first pixels
	x &= 7;
	u8 m1 = 0xff << (8 - x);
	u8 col1 = colcol >> x;
	
	// width of inner pixels
	if (x > 0) w -= 8 - x;
	int w2 = w/8;

	// mask of last pixels
	u8 m2 = 0xff;
	u8 col2 = 0;
	if (w >= 0) // width is large enough
	{
		w -= w2*8;
		m2 = 0xff >> w;
		col2 = colcol << (8 - w);
	}
	else // last pixels are not valid
	{
		w += 8; // end of first pixels
		u8 m3 = 0xff >> w;
		m1 |= m3;
		col1 &= ~m3;
		w = 0; // no pixels left as last ones
	}

	// store first pixels
	if (x > 0)
	{
		*d = (*d & m1) | col1;
		d++;
	}

	// store inner pixels
	int i = w2;
	for (; i > 0; i--) *d++ = colcol;

	// store last pixels
	if (w > 0) *d = (*d & m2) | col2;
}
void Draw1HLine_Fast(int x, int y, int w, u16 col) { DrawCan1HLine_Fast(pDrawCan1, x, y, w, col); }

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan1HLine(sDrawCan* can, int x, int y, int w, u16 col8)
{
	u8 col = (u8)col8;

	// flip rectangle
	if (w < 0)
	{
		x += w;
		w = -w;
	}

	// limit x
	int k = can->clipx1 - x;
	if (k > 0)
	{
		w -= k;
		x += k;
	}

	// limit w
	k = can->clipx2;
	if (x + w > k) w = k - x;
	if (w <= 0) return;

	// check y
	if ((y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyRect_Fast(can, x, y, w, 1);

	// draw
	DrawCan1HLine_Fast(can, x, y, w, col);
}
void Draw1HLine(int x, int y, int w, u16 col) { DrawCan1HLine(pDrawCan1, x, y, w, col); }

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan1HLineInv_Fast(sDrawCan* can, int x, int y, int w)
{
	// prepare destination address
	int wb = can->wb;
	u8* d = &can->buf[x/8 + (y - can->basey)*wb];

	// mask of first pixels
	x &= 7;
	u8 m1 = 0xff >> x;
	
	// width of inner pixels
	if (x > 0) w -= 8 - x;
	int w2 = w/8;

	// mask of last pixels
	u8 m2 = 0xff;
	if (w >= 0) // width is large enough
	{
		w -= w2*8;
		m2 = 0xff << (8 - w);
	}
	else // last pixels are not valid
	{
		w += 8; // end of first pixels
		u8 m3 = 0xff >> w;
		m1 &= ~m3;
		w = 0; // no pixels left as last ones
	}

	// store first pixels
	if (x > 0)
	{
		*d ^= m1;
		d++;
	}

	// store inner pixels
	int i = w2;
	for (; i > 0; i--)
	{
		*d = ~*d;
		d++;
	}

	// store last pixels
	if (w > 0) *d ^= m2;
}
void Draw1HLineInv_Fast(int x, int y, int w) { DrawCan1HLineInv_Fast(pDrawCan1, x, y, w); }

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan1HLineInv(sDrawCan* can, int x, int y, int w)
{
	// flip rectangle
	if (w < 0)
	{
		x += w;
		w = -w;
	}

	// limit x
	int k = can->clipx1 - x;
	if (k > 0)
	{
		w -= k;
		x += k;
	}

	// limit w
	k = can->clipx2;
	if (x + w > k) w = k - x;
	if (w <= 0) return;

	// check y
	if ((y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyRect_Fast(can, x, y, w, 1);

	// invert
	DrawCan1HLineInv_Fast(can, x, y, w);
}
void Draw1HLineInv(int x, int y, int w) { DrawCan1HLineInv(pDrawCan1, x, y, w); }

// ----------------------------------------------------------------------------
//                           Draw vertical line
// ----------------------------------------------------------------------------

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan1VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col8)
{
	u8 col = (u8)col8;

	// prepare destination address
	int wb = can->wb;
	u8* d = &can->buf[x/8 + (y - can->basey)*wb];

	// mask
	x &= 7;
	u8 m = ~(0x80 >> x);
	col = (col << 7) >> x;

	// write pixels	
	for (; h > 0; h--)
	{
		*d = (*d & m) | col;
		d += wb;
	}
}
void Draw1VLine_Fast(int x, int y, int h, u16 col) { DrawCan1VLine_Fast(pDrawCan1, x, y, h, col); }

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan1VLine(sDrawCan* can, int x, int y, int h, u16 col8)
{
	u8 col = (u8)col8;

	// flip rectangle
	if (h < 0)
	{
		y += h;
		h = -h;
	}

	// check x
	if ((x < can->clipx1) || (x >= can->clipx2)) return;

	// limit y
	int k = can->clipy1 - y;
	if (k > 0)
	{
		h -= k;
		y += k;
	}

	// limit h
	k = can->clipy2;
	if (y + h > k) h = k - y;
	if (h <= 0) return;

	// update dirty area
	DrawCanDirtyRect_Fast(can, x, y, 1, h);

	// draw
	DrawCan1VLine_Fast(can, x, y, h, col);
}
void Draw1VLine(int x, int y, int h, u16 col) { DrawCan1VLine(pDrawCan1, x, y, h, col); }

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan1VLineInv_Fast(sDrawCan* can, int x, int y, int h)
{
	// prepare destination address
	int wb = can->wb;
	u8* d = &can->buf[x/8 + (y - can->basey)*wb];

	// mask
	x &= 7;
	u8 m = 0x80 >> x;

	// write pixels	
	for (; h > 0; h--)
	{
		*d ^= m;
		d += wb;
	}
}
void Draw1VLineInv_Fast(int x, int y, int h) { DrawCan1VLineInv_Fast(pDrawCan1, x, y, h); }

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan1VLineInv(sDrawCan* can, int x, int y, int h)
{
	// flip rectangle
	if (h < 0)
	{
		y += h;
		h = -h;
	}

	// check x
	if ((x < can->clipx1) || (x >= can->clipx2)) return;

	// limit y
	int k = can->clipy1 - y;
	if (k > 0)
	{
		h -= k;
		y += k;
	}

	// limit h
	k = can->clipy2;
	if (y + h > k) h = k - y;
	if (h <= 0) return;

	// update dirty area
	DrawCanDirtyRect_Fast(can, x, y, 1, h);

	// invert
	DrawCan1VLineInv_Fast(can, x, y, h);
}
void Draw1VLineInv(int x, int y, int h) { DrawCan1VLineInv(pDrawCan1, x, y, h); }

// ----------------------------------------------------------------------------
//                           Draw frame
// ----------------------------------------------------------------------------

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan1Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
{
	// bottom line (dark)
	DrawCan1HLine_Fast(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan1VLine_Fast(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan1HLine_Fast(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan1VLine_Fast(can, x, y+1, h-2, col_light);
	}
}
void Draw1Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan1Frame_Fast(pDrawCan1, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan1Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
{
	// flip frame
	if (w < 0)
	{
		x += w;
		w = -w;
	}

	if (h < 0)
	{
		y += h;
		h = -h;
	}

	// bottom line (dark)
	DrawCan1HLine(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan1VLine(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan1HLine(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan1VLine(can, x, y+1, h-2, col_light);
	}
}
void Draw1Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan1Frame(pDrawCan1, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan1FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// bottom line
	DrawCan1HLineInv_Fast(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan1VLineInv_Fast(can, x+w-1, y, h-1);

		// top line
		DrawCan1HLineInv_Fast(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan1VLineInv_Fast(can, x, y+1, h-2);
	}
}
void Draw1FrameInv_Fast(int x, int y, int w, int h) { DrawCan1FrameInv_Fast(pDrawCan1, x, y, w, h); }

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan1FrameInv(sDrawCan* can, int x, int y, int w, int h)
{
	// flip rectangle
	if (w < 0)
	{
		x += w;
		w = -w;
	}

	if (h < 0)
	{
		y += h;
		h = -h;
	}

	// bottom line
	DrawCan1HLineInv(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan1VLineInv(can, x+w-1, y, h-1);

		// top line
		DrawCan1HLineInv(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan1VLineInv(can, x, y+1, h-2);
	}
}
void Draw1FrameInv(int x, int y, int w, int h) { DrawCan1FrameInv(pDrawCan1, x, y, w, h); }

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan1FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col8, int thick)
{
	u8 col = (u8)col8;

	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan1Rect_Fast(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan1Rect_Fast(can, x, y, w, thick, col);

		// bottom line
		DrawCan1Rect_Fast(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan1Rect_Fast(can, x, y, thick, h, col);

		// right line
		DrawCan1Rect_Fast(can, x+w-thick, y, thick, h, col);
	}
}
void Draw1FrameW_Fast(int x, int y, int w, int h, u16 col, int thick) { DrawCan1FrameW_Fast(pDrawCan1, x, y, w, h, col, thick); }

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan1FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col8, int thick)
{
	if (thick < 1) return;
	u8 col = (u8)col8;

	// flip frame
	if (w < 0)
	{
		x += w;
		w = -w;
	}

	if (h < 0)
	{
		y += h;
		h = -h;
	}

	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan1Rect(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan1Rect(can, x, y, w, thick, col);

		// bottom line
		DrawCan1Rect(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan1Rect(can, x, y, thick, h, col);

		// right line
		DrawCan1Rect(can, x+w-thick, y, thick, h, col);
	}
}
void Draw1FrameW(int x, int y, int w, int h, u16 col, int thick) { DrawCan1Frame(pDrawCan1, x, y, w, h, col, thick); }

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan1FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan1RectInv_Fast(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan1RectInv_Fast(can, x, y, w, thick);

		// bottom line
		DrawCan1RectInv_Fast(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan1RectInv_Fast(can, x, y, thick, h);

		// right line
		DrawCan1RectInv_Fast(can, x+w-thick, y, thick, h);
	}
}
void Draw1FrameWInv_Fast(int x, int y, int w, int h, int thick) { DrawCan1FrameWInv_Fast(pDrawCan1, x, y, w, h, thick); }

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan1FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick)
{
	if (thick < 1) return;

	// flip rectangle
	if (w < 0)
	{
		x += w;
		w = -w;
	}

	if (h < 0)
	{
		y += h;
		h = -h;
	}

	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan1RectInv(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan1RectInv(can, x, y, w, thick);

		// bottom line
		DrawCan1RectInv(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan1RectInv(can, x, y, thick, h);

		// right line
		DrawCan1RectInv(can, x+w-thick, y, thick, h);
	}
}
void Draw1FrameWInv(int x, int y, int w, int h, int thick) { DrawCan1FrameWInv(pDrawCan1, x, y, w, h, thick); }

// ----------------------------------------------------------------------------
//                           Draw line
// ----------------------------------------------------------------------------

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan1LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
{
	u8 col = (u8)col8;

	// difference of coordinates
	int dx = x2 - x1;
	int dy = y2 - y1;

	// increment X
	int sx = 1;
	if (dx < 0)
	{
		sx = -1;
		dx = -dx;
	}

	// increment Y
	int sy = 1;
	int wb = can->wb;
	int ddy = wb;
	if (dy < 0)
	{
		sy = -1;
		ddy = -ddy;
		dy = -dy;
	}

	// destination address
	u8* d = &can->buf[x1/8 + (y1 - can->basey)*wb];
	int xfrac = 7 - (x1 & 7);

	// draw first pixel
	*d = (*d & ~(0x01<<xfrac)) | (col<<xfrac);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			xfrac -= sx;
			if (xfrac < 0)
			{
				xfrac = 7;
				d++;
			}
			if (xfrac == 8)
			{
				xfrac = 0;
				d--;
			}

			if (p > 0)
			{
				if (over) *d = (*d & ~(0x01<<xfrac)) | (col<<xfrac); // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			*d = (*d & ~(0x01<<xfrac)) | (col<<xfrac);
		}
	}

	// steeply in Y direction, Y is prefered as base
	else
	{
		int m = 2*dx;
		int p = m - dy;
		dy = 2*dy;
		while (y1 != y2)
		{
			y1 += sy;
			d += ddy;
			if (p > 0)
			{
				if (over) *d = (*d & ~(0x01<<xfrac)) | (col<<xfrac); // draw overlapped pixel
				x1 += sx;
				xfrac -= sx;
				if (xfrac < 0)
				{
					xfrac = 7;
					d++;
				}
				if (xfrac == 8)
				{
					xfrac = 0;
					d--;
				}
				p -= dy;
			}
			p += m;
			*d = (*d & ~(0x01<<xfrac)) | (col<<xfrac);
		}
	}
}
void Draw1LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan1LineOver_Fast(pDrawCan1, x1, y1, x2, y2, col, over); }

// Draw line (does not check clipping limits)
void DrawCan1Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan1LineOver_Fast(pDrawCan1, x1, y1, x2, y2, col, False); }
void Draw1Line_Fast(int x1, int y1, int x2, int y2, u16 col) { Draw1LineOver_Fast(x1, y1, x2, y2, col, False); }

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan1LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
{
	u8 col = (u8)col8;

	// difference of coordinates
	int dx = x2 - x1;
	int dy = y2 - y1;

	// update dirty area
	int x0 = x1;
	int y0 = y1;
	int w = dx;
	int h = dy;
	if (w < 0)
	{
		x0 = x2;
		w = -w;
	}
	if (h < 0)
	{
		y0 = y2;
		h = -h;
	}
	DrawCanDirtyRect(can, x0, y0, w+1, h+1);

	// increment X
	int sx = 1;
	if (dx < 0)
	{
		sx = -1;
		dx = -dx;
	}

	// increment Y
	int sy = 1;
	int wb = can->wb;
	int ddy = wb;
	if (dy < 0)
	{
		sy = -1;
		ddy = -ddy;
		dy = -dy;
	}

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u8* d = &can->buf[x1/8 + (y1 - can->basey)*wb];
	int xfrac = 7 - (x1 & 7);

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~(0x01<<xfrac)) | (col<<xfrac);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			xfrac -= sx;
			if (xfrac < 0)
			{
				xfrac = 7;
				d++;
			}
			if (xfrac == 8)
			{
				xfrac = 0;
				d--;
			}

			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~(0x01<<xfrac)) | (col<<xfrac);
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~(0x01<<xfrac)) | (col<<xfrac);
		}
	}

	// steeply in Y direction, Y is prefered as base
	else
	{
		int m = 2*dx;
		int p = m - dy;
		dy = 2*dy;
		while (y1 != y2)
		{
			y1 += sy;
			d += ddy;
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~(0x01<<xfrac)) | (col<<xfrac);
				x1 += sx;
				xfrac -= sx;
				if (xfrac < 0)
				{
					xfrac = 7;
					d++;
				}
				if (xfrac == 8)
				{
					xfrac = 0;
					d--;
				}

				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~(0x01<<xfrac)) | (col<<xfrac);
		}
	}
}
void Draw1LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan1LineOver(pDrawCan1, x1, y1, x2, y2, col, over); }

// Draw line (checks clipping limits)
void DrawCan1Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan1LineOver(can, x1, y1, x2, y2, col, False); }
void Draw1Line(int x1, int y1, int x2, int y2, u16 col) { DrawCan1Line(pDrawCan1, x1, y1, x2, y2, col); }

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan1LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
{
	// difference of coordinates
	int dx = x2 - x1;
	int dy = y2 - y1;

	// increment X
	int sx = 1;
	if (dx < 0)
	{
		sx = -1;
		dx = -dx;
	}

	// increment Y
	int sy = 1;
	int wb = can->wb;
	int ddy = wb;
	if (dy < 0)
	{
		sy = -1;
		ddy = -ddy;
		dy = -dy;
	}

	// destination address
	u8* d = &can->buf[x1/8 + (y1 - can->basey)*wb];
	int xfrac = 7 - (x1 & 7);

	// draw first pixel
	*d ^= 0x01<<xfrac;

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			xfrac -= sx;
			if (xfrac < 0)
			{
				xfrac = 7;
				d++;
			}
			if (xfrac == 8)
			{
				xfrac = 0;
				d--;
			}

			if (p > 0)
			{
				if (over) *d ^= 0x01<<xfrac; // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			*d ^= 0x01<<xfrac;
		}
	}

	// steeply in Y direction, Y is prefered as base
	else
	{
		int m = 2*dx;
		int p = m - dy;
		dy = 2*dy;
		while (y1 != y2)
		{
			y1 += sy;
			d += ddy;
			if (p > 0)
			{
				if (over) *d ^= 0x01<<xfrac; // draw overlapped pixel
				x1 += sx;
				xfrac -= sx;
				if (xfrac < 0)
				{
					xfrac = 7;
					d++;
				}
				if (xfrac == 8)
				{
					xfrac = 0;
					d--;
				}
				p -= dy;
			}
			p += m;
			*d ^= 0x01<<xfrac;
		}
	}
}
void Draw1LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over) { DrawCan1LineOverInv_Fast(pDrawCan1, x1, y1, x2, y2, over); }

// Draw line inverted (does not check clipping limits)
void DrawCan1LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan1LineOverInv_Fast(can, x1, y1, x2, y2, False); }
void Draw1LineInv_Fast(int x1, int y1, int x2, int y2) { DrawCan1LineOverInv_Fast(pDrawCan1, x1, y1, x2, y2, False); }

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan1LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
{
	// difference of coordinates
	int dx = x2 - x1;
	int dy = y2 - y1;

	// update dirty area
	int x0 = x1;
	int y0 = y1;
	int w = dx;
	int h = dy;
	if (w < 0)
	{
		x0 = x2;
		w = -w;
	}
	if (h < 0)
	{
		y0 = y2;
		h = -h;
	}
	DrawCanDirtyRect(can, x0, y0, w+1, h+1);

	// increment X
	int sx = 1;
	if (dx < 0)
	{
		sx = -1;
		dx = -dx;
	}

	// increment Y
	int sy = 1;
	int wb = can->wb;
	int ddy = wb;
	if (dy < 0)
	{
		sy = -1;
		ddy = -ddy;
		dy = -dy;
	}

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u8* d = &can->buf[x1/8 + (y1 - can->basey)*wb];
	int xfrac = 7 - (x1 & 7);

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= 0x01<<xfrac;

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			xfrac -= sx;
			if (xfrac < 0)
			{
				xfrac = 7;
				d++;
			}
			if (xfrac == 8)
			{
				xfrac = 0;
				d--;
			}

			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= 0x01<<xfrac;
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= 0x01<<xfrac;
		}
	}

	// steeply in Y direction, Y is prefered as base
	else
	{
		int m = 2*dx;
		int p = m - dy;
		dy = 2*dy;
		while (y1 != y2)
		{
			y1 += sy;
			d += ddy;
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= 0x01<<xfrac;
				x1 += sx;
				xfrac -= sx;
				if (xfrac < 0)
				{
					xfrac = 7;
					d++;
				}
				if (xfrac == 8)
				{
					xfrac = 0;
					d--;
				}
				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= 0x01<<xfrac;
		}
	}
}
void Draw1LineOverInv(int x1, int y1, int x2, int y2, Bool over) { DrawCan1LineOverInv(pDrawCan1, x1, y1, x2, y2, over); }

// Draw line inverted (checks clipping limits)
void DrawCan1LineInv(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan1LineOverInv(can, x1, y1, x2, y2, False); }
void Draw1LineInv(int x1, int y1, int x2, int y2) { DrawCan1LineOverInv(pDrawCan1, x1, y1, x2, y2, False); }

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan1LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan1LineOver_Fast(can, x1, y1, x2, y2, col, False);
		return;
	}

	// difference of coordinates
	dxabs = x2 - x1;
	dyabs = y2 - y1;
	if (dxabs < 0) dxabs = -dxabs;
	if (dyabs < 0) dyabs = -dyabs;

	// draw round ends
	if (round && (thick > 2))
	{
		int thick2 = (thick-1) | 1; // a circle of odd size is needed to be placed exactly on the coordinate
		DrawCan1Round_Fast(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan1Round_Fast(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
	}

	// difference of coordinates
	//   X and Y coordinates are "swapped" because they express
	//   a direction orthogonal to the direction of the line
	dy = x2 - x1;
	dx = y2 - y1;
	Bool swap = True;

	// increment X
	int sx = 1;
	if (dx < 0)
	{
		sx = -1;
		dx = -dx;
		swap = !swap;
	}

	// increment Y
	int sy = 1;
	if (dy < 0)
	{
		sy = -1;
		dy = -dy;
		swap = !swap;
	}

	// thick correction (otherwise the diagonal lines would be thicker than the perpendicular lines)
	if (thick >= 4)
	{
		if ((dxabs >= dyabs*3/4) && (dxabs*3/4 <= dyabs))
		{
			if (thick >= 17) thick--;
			if (thick >= 14) thick--;
			if (thick >= 11) thick--;
			if (((thick & 1) == 0) || (thick >= 7)) thick--;
		}
	}

	// prepare adjust to shift to the middle of the line
	int dx2 = dx*2;
	int dy2 = dy*2;
	int adj = thick/2;

	// steeply in X direction, X is prefered as base
	if (dx >= dy)
	{
		// swapped direction
		if (swap)
		{
			adj = thick - 1 - adj;
			sy = -sy;
		}
		else
			sx = -sx;

		// shift to the middle of the line
		int err = dy2 - dx;
		for (i = adj; i > 0; i--)
		{
			x1 -= sx;
			x2 -= sx;
			if (err >= 0)
			{
				y1 -= sy;
				y2 -= sy;
				err -= dx2;
			}
			err += dy2;
		}

		// draw first line
		DrawCan1LineOver_Fast(can, x1, y1, x2, y2, col, False);

		// draw other lines
		err = dy2 - dx;
		for (i = thick; i > 1; i--)
		{
			x1 += sx;
			x2 += sx;
			over = False;
			if (err >= 0)
			{
				y1 += sy;
				y2 += sy;
				err -= dx2;
				over = True;
			}

			// draw line
			DrawCan1LineOver_Fast(can, x1, y1, x2, y2, col, over);
			err += dy2;
		}
	}

	// steeply in Y direction, Y is prefered as base
	else
	{
		// swapped direction
		if (swap)
			sx = -sx;
		else
		{
			adj = thick - 1 - adj;
			sy = -sy;
		}

		// shift to the middle of the line
		int err = dx2 - dy;
		for (i = adj; i > 0; i--)
		{
			y1 -= sy;
			y2 -= sy;
			if (err >= 0)
			{
				x1 -= sx;
				x2 -= sx;
				err -= dy2;
			}
			err += dx2;
		}

		// draw first line
		DrawCan1LineOver_Fast(can, x1, y1, x2, y2, col, False);

		// draw other lines
		err = dx2 - dy;
		for (i = thick; i > 1; i--)
		{
			y1 += sy;
			y2 += sy;
			over = False;
			if (err >= 0)
			{
				x1 += sx;
				x2 += sx;
				err -= dy2;
				over = True;
			}

			// draw line
			DrawCan1LineOver_Fast(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw1LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan1LineW_Fast(pDrawCan1, x1, y1, x2, y2, col, thick, round); }

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan1LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan1LineOver(can, x1, y1, x2, y2, col, False);
		return;
	}

	// difference of coordinates
	dxabs = x2 - x1;
	dyabs = y2 - y1;
	if (dxabs < 0) dxabs = -dxabs;
	if (dyabs < 0) dyabs = -dyabs;

	// draw round ends
	if (round && (thick > 2))
	{
		int thick2 = (thick-1) | 1; // a circle of odd size is needed to be placed exactly on the coordinate
		DrawCan1Round(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan1Round(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
	}

	// difference of coordinates
	//   X and Y coordinates are "swapped" because they express
	//   a direction orthogonal to the direction of the line
	dy = x2 - x1;
	dx = y2 - y1;
	Bool swap = True;

	// increment X
	int sx = 1;
	if (dx < 0)
	{
		sx = -1;
		dx = -dx;
		swap = !swap;
	}

	// increment Y
	int sy = 1;
	if (dy < 0)
	{
		sy = -1;
		dy = -dy;
		swap = !swap;
	}

	// thick correction (otherwise the diagonal lines would be thicker than the perpendicular lines)
	if (thick >= 4)
	{
		if ((dxabs >= dyabs*3/4) && (dxabs*3/4 <= dyabs))
		{
			if (thick >= 17) thick--;
			if (thick >= 14) thick--;
			if (thick >= 11) thick--;
			if (((thick & 1) == 0) || (thick >= 7)) thick--;
		}
	}

	// prepare adjust to shift to the middle of the line
	int dx2 = dx*2;
	int dy2 = dy*2;
	int adj = thick/2;

	// steeply in X direction, X is prefered as base
	if (dx >= dy)
	{
		// swapped direction
		if (swap)
		{
			adj = thick - 1 - adj;
			sy = -sy;
		}
		else
			sx = -sx;

		// shift to the middle of the line
		int err = dy2 - dx;
		for (i = adj; i > 0; i--)
		{
			x1 -= sx;
			x2 -= sx;
			if (err >= 0)
			{
				y1 -= sy;
				y2 -= sy;
				err -= dx2;
			}
			err += dy2;
		}

		// draw first line
		DrawCan1LineOver(can, x1, y1, x2, y2, col, False);

		// draw other lines
		err = dy2 - dx;
		for (i = thick; i > 1; i--)
		{
			x1 += sx;
			x2 += sx;
			over = False;
			if (err >= 0)
			{
				y1 += sy;
				y2 += sy;
				err -= dx2;
				over = True;
			}

			// draw line
			DrawCan1LineOver(can, x1, y1, x2, y2, col, over);
			err += dy2;
		}
	}

	// steeply in Y direction, Y is prefered as base
	else
	{
		// swapped direction
		if (swap)
			sx = -sx;
		else
		{
			adj = thick - 1 - adj;
			sy = -sy;
		}

		// shift to the middle of the line
		int err = dx2 - dy;
		for (i = adj; i > 0; i--)
		{
			y1 -= sy;
			y2 -= sy;
			if (err >= 0)
			{
				x1 -= sx;
				x2 -= sx;
				err -= dy2;
			}
			err += dx2;
		}

		// draw first line
		DrawCan1LineOver(can, x1, y1, x2, y2, col, False);

		// draw other lines
		err = dx2 - dy;
		for (i = thick; i > 1; i--)
		{
			y1 += sy;
			y2 += sy;
			over = False;
			if (err >= 0)
			{
				x1 += sx;
				x2 += sx;
				err -= dy2;
				over = True;
			}

			// draw line
			DrawCan1LineOver(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw1LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan1LineW(pDrawCan1, x1, y1, x2, y2, col, thick, round); }

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan1LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan1LineOverInv_Fast(can, x1, y1, x2, y2, False);
		return;
	}

	// difference of coordinates
	dxabs = x2 - x1;
	dyabs = y2 - y1;
	if (dxabs < 0) dxabs = -dxabs;
	if (dyabs < 0) dyabs = -dyabs;

	// difference of coordinates
	//   X and Y coordinates are "swapped" because they express
	//   a direction orthogonal to the direction of the line
	dy = x2 - x1;
	dx = y2 - y1;
	Bool swap = True;

	// increment X
	int sx = 1;
	if (dx < 0)
	{
		sx = -1;
		dx = -dx;
		swap = !swap;
	}

	// increment Y
	int sy = 1;
	if (dy < 0)
	{
		sy = -1;
		dy = -dy;
		swap = !swap;
	}

	// thick correction (otherwise the diagonal lines would be thicker than the perpendicular lines)
	if (thick >= 4)
	{
		if ((dxabs >= dyabs*3/4) && (dxabs*3/4 <= dyabs))
		{
			if (thick >= 17) thick--;
			if (thick >= 14) thick--;
			if (thick >= 11) thick--;
			if (((thick & 1) == 0) || (thick >= 7)) thick--;
		}
	}

	// prepare adjust to shift to the middle of the line
	int dx2 = dx*2;
	int dy2 = dy*2;
	int adj = thick/2;

	// steeply in X direction, X is prefered as base
	if (dx >= dy)
	{
		// swapped direction
		if (swap)
		{
			adj = thick - 1 - adj;
			sy = -sy;
		}
		else
			sx = -sx;

		// shift to the middle of the line
		int err = dy2 - dx;
		for (i = adj; i > 0; i--)
		{
			x1 -= sx;
			x2 -= sx;
			if (err >= 0)
			{
				y1 -= sy;
				y2 -= sy;
				err -= dx2;
			}
			err += dy2;
		}

		// draw first line
		DrawCan1LineOverInv_Fast(can, x1, y1, x2, y2, False);

		// draw other lines
		err = dy2 - dx;
		for (i = thick; i > 1; i--)
		{
			x1 += sx;
			x2 += sx;
			over = False;
			if (err >= 0)
			{
				y1 += sy;
				y2 += sy;
				err -= dx2;
				over = True;
			}

			// draw line
			DrawCan1LineOverInv_Fast(can, x1, y1, x2, y2, over);
			err += dy2;
		}
	}

	// steeply in Y direction, Y is prefered as base
	else
	{
		// swapped direction
		if (swap)
			sx = -sx;
		else
		{
			adj = thick - 1 - adj;
			sy = -sy;
		}

		// shift to the middle of the line
		int err = dx2 - dy;
		for (i = adj; i > 0; i--)
		{
			y1 -= sy;
			y2 -= sy;
			if (err >= 0)
			{
				x1 -= sx;
				x2 -= sx;
				err -= dy2;
			}
			err += dx2;
		}

		// draw first line
		DrawCan1LineOverInv_Fast(can, x1, y1, x2, y2, False);

		// draw other lines
		err = dx2 - dy;
		for (i = thick; i > 1; i--)
		{
			y1 += sy;
			y2 += sy;
			over = False;
			if (err >= 0)
			{
				x1 += sx;
				x2 += sx;
				err -= dy2;
				over = True;
			}

			// draw line
			DrawCan1LineOverInv_Fast(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw1LineWInv_Fast(int x1, int y1, int x2, int y2, int thick) { DrawCan1LineWInv_Fast(pDrawCan1, x1, y1, x2, y2, thick); }

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan1LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan1LineOverInv(can, x1, y1, x2, y2, False);
		return;
	}

	// difference of coordinates
	dxabs = x2 - x1;
	dyabs = y2 - y1;
	if (dxabs < 0) dxabs = -dxabs;
	if (dyabs < 0) dyabs = -dyabs;

	// difference of coordinates
	//   X and Y coordinates are "swapped" because they express
	//   a direction orthogonal to the direction of the line
	dy = x2 - x1;
	dx = y2 - y1;
	Bool swap = True;

	// increment X
	int sx = 1;
	if (dx < 0)
	{
		sx = -1;
		dx = -dx;
		swap = !swap;
	}

	// increment Y
	int sy = 1;
	if (dy < 0)
	{
		sy = -1;
		dy = -dy;
		swap = !swap;
	}

	// thick correction (otherwise the diagonal lines would be thicker than the perpendicular lines)
	if (thick >= 4)
	{
		if ((dxabs >= dyabs*3/4) && (dxabs*3/4 <= dyabs))
		{
			if (thick >= 17) thick--;
			if (thick >= 14) thick--;
			if (thick >= 11) thick--;
			if (((thick & 1) == 0) || (thick >= 7)) thick--;
		}
	}

	// prepare adjust to shift to the middle of the line
	int dx2 = dx*2;
	int dy2 = dy*2;
	int adj = thick/2;

	// steeply in X direction, X is prefered as base
	if (dx >= dy)
	{
		// swapped direction
		if (swap)
		{
			adj = thick - 1 - adj;
			sy = -sy;
		}
		else
			sx = -sx;

		// shift to the middle of the line
		int err = dy2 - dx;
		for (i = adj; i > 0; i--)
		{
			x1 -= sx;
			x2 -= sx;
			if (err >= 0)
			{
				y1 -= sy;
				y2 -= sy;
				err -= dx2;
			}
			err += dy2;
		}

		// draw first line
		DrawCan1LineOverInv(can, x1, y1, x2, y2, False);

		// draw other lines
		err = dy2 - dx;
		for (i = thick; i > 1; i--)
		{
			x1 += sx;
			x2 += sx;
			over = False;
			if (err >= 0)
			{
				y1 += sy;
				y2 += sy;
				err -= dx2;
				over = True;
			}

			// draw line
			DrawCan1LineOverInv(can, x1, y1, x2, y2, over);
			err += dy2;
		}
	}

	// steeply in Y direction, Y is prefered as base
	else
	{
		// swapped direction
		if (swap)
			sx = -sx;
		else
		{
			adj = thick - 1 - adj;
			sy = -sy;
		}

		// shift to the middle of the line
		int err = dx2 - dy;
		for (i = adj; i > 0; i--)
		{
			y1 -= sy;
			y2 -= sy;
			if (err >= 0)
			{
				x1 -= sx;
				x2 -= sx;
				err -= dy2;
			}
			err += dx2;
		}

		// draw first line
		DrawCan1LineOverInv(can, x1, y1, x2, y2, False);

		// draw other lines
		err = dx2 - dy;
		for (i = thick; i > 1; i--)
		{
			y1 += sy;
			y2 += sy;
			over = False;
			if (err >= 0)
			{
				x1 += sx;
				x2 += sx;
				err -= dy2;
				over = True;
			}

			// draw line
			DrawCan1LineOverInv(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw1LineWInv(int x1, int y1, int x2, int y2, int thick) { DrawCan1LineWInv(pDrawCan1, x1, y1, x2, y2, thick); }

// ----------------------------------------------------------------------------
//                          Draw round (Filled circle)
// ----------------------------------------------------------------------------

#define DRAWCANROUND_HIDE()	{				\
	if ((mask & DRAWCAN_ROUND_NOTOP) != 0) y1 = 0;		\
	if ((mask & DRAWCAN_ROUND_NOBOTTOM) != 0) y2 = 0;	\
	if ((mask & DRAWCAN_ROUND_NOLEFT) != 0) x1 = 0;		\
	if ((mask & DRAWCAN_ROUND_NORIGHT) != 0) x2 = 0; }

#define DRAWCANROUND_CLIP()	{			\
		int k = can->clipx1;			\
		if (x+x1 < k) x1 = k - x;		\
		k = can->clipx2;			\
		if (x+x2 >= k) x2 = k - 1 - x;		\
		k = can->clipy1;			\
		if (y+y1 < k) y1 = k - y;		\
		k = can->clipy2;			\
		if (y+y2 >= k) y2 = k - 1 - y;		\
		if ((x2 < x1) || (y2 < y1)) return; }

//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round
//		DRAWCAN_ROUND_NOTOP	= hide top part of the round
//		DRAWCAN_ROUND_NOBOTTOM	= hide bottom part of the round
//		DRAWCAN_ROUND_NOLEFT	= hide left part of the round
//		DRAWCAN_ROUND_NORIGHT	= hide right part of the round
//		DRAWCAN_ROUND_ALL	= draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCan1Round_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0, *s;
	int r2;
	u8 col = (u8)col8;

	// hide parts of the round
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		int xx = x+x1;
		int x0 = 7 - (xx & 7);
		s0 = &can->buf[xx/8 + ((y+y1) - can->basey)*wb];

		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s = (*s & ~(0x01<<xx)) | (col<<xx);
				xx--;
				if (xx < 0)
				{
					s++;
					xx = 7;
				}
			}
			s0 += wb;
		}
	}

	// even diameter (2, 4, 6, ...)
	else
	{
		// to correspond to the center of DrawCircle [x0,y0] = [x+d/2,y+d/2]
		x--;
		y--;

		// prepare buffer address
		x1++;
		y1++;

		int xx = x+x1;
		int x0 = 7 - (xx & 7);
		s0 = &can->buf[xx/8 + ((y+y1) - can->basey)*wb];

		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s = (*s & ~(0x01<<xx)) | (col<<xx);
				xx--;
				if (xx < 0)
				{
					s++;
					xx = 7;
				}
			}
			s0 += wb;
		}
	}
}
void Draw1Round_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan1Round_Fast(pDrawCan1, x, y, d, col, mask); }

// Draw round (checks clipping limits)
void DrawCan1Round(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	if (d < 1) d = 1;
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0, *s;
	int r2;
	u8 col = (u8)col8;

	// hide parts of the round
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// clipping
		DRAWCANROUND_CLIP();

		// update dirty area
		DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

		// prepare buffer address
		int xx = x+x1;
		int x0 = 7 - (xx & 7);
		s0 = &can->buf[xx/8 + ((y+y1) - can->basey)*wb];

		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s = (*s & ~(0x01<<xx)) | (col<<xx);
				xx--;
				if (xx < 0)
				{
					s++;
					xx = 7;
				}
			}
			s0 += wb;
		}
	}

	// even diameter (2, 4, 6, ...)
	else
	{
		// to correspond to the center of DrawCircle [x0,y0] = [x+d/2,y+d/2]
		x--;
		y--;
		x1++;
		y1++;

		// clipping
		DRAWCANROUND_CLIP();

		// update dirty area
		DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

		// prepare buffer address
		int xx = x+x1;
		int x0 = 7 - (xx & 7);
		s0 = &can->buf[xx/8 + ((y+y1) - can->basey)*wb];

		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s = (*s & ~(0x01<<xx)) | (col<<xx);
				xx--;
				if (xx < 0)
				{
					s++;
					xx = 7;
				}
			}
			s0 += wb;
		}
	}
}
void Draw1Round(int x, int y, int d, u16 col, u8 mask) { DrawCan1Round(pDrawCan1, x, y, d, col, mask); }

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan1RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	if (d < 1) d = 1;
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0, *s;
	int r2;

	// hide parts of the round
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		int xx = x+x1;
		int x0 = 7 - (xx & 7);
		s0 = &can->buf[xx/8 + ((y+y1) - can->basey)*wb];

		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s ^= 0x01<<xx;
				xx--;
				if (xx < 0)
				{
					s++;
					xx = 7;
				}
			}
			s0 += wb;
		}
	}

	// even diameter (2, 4, 6, ...)
	else
	{
		// to correspond to the center of DrawCircle [x0,y0] = [x+d/2,y+d/2]
		x--;
		y--;

		// prepare buffer address
		x1++;
		y1++;

		int xx = x+x1;
		int x0 = 7 - (xx & 7);
		s0 = &can->buf[xx/8 + ((y+y1) - can->basey)*wb];

		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s ^= 0x01<<xx;
				xx--;
				if (xx < 0)
				{
					s++;
					xx = 7;
				}
			}
			s0 += wb;
		}
	}
}
void Draw1RoundInv_Fast(int x, int y, int d, u8 mask) { DrawCan1RoundInv_Fast(pDrawCan1, x, y, d, mask); }

// Draw round inverted (checks clipping limits)
void DrawCan1RoundInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0, *s;
	int r2;

	// hide parts of the round
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// clipping
		DRAWCANROUND_CLIP();

		// update dirty area
		DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

		// prepare buffer address
		int xx = x+x1;
		int x0 = 7 - (xx & 7);
		s0 = &can->buf[xx/8 + ((y+y1) - can->basey)*wb];

		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s ^= 0x01<<xx;
				xx--;
				if (xx < 0)
				{
					s++;
					xx = 7;
				}
			}
			s0 += wb;
		}
	}

	// even diameter (2, 4, 6, ...)
	else
	{
		// to correspond to the center of DrawCircle [x0,y0] = [x+d/2,y+d/2]
		x--;
		y--;
		x1++;
		y1++;

		// clipping
		DRAWCANROUND_CLIP();

		// update dirty area
		DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

		// prepare buffer address
		int xx = x+x1;
		int x0 = 7 - (xx & 7);
		s0 = &can->buf[xx/8 + ((y+y1) - can->basey)*wb];

		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s ^= 0x01<<xx;
				xx--;
				if (xx < 0)
				{
					s++;
					xx = 7;
				}
			}
			s0 += wb;
		}
	}
}
void Draw1RoundInv(int x, int y, int d, u8 mask) { DrawCan1RoundInv(pDrawCan1, x, y, d, mask); }

// ----------------------------------------------------------------------------
//                               Draw circle
// ----------------------------------------------------------------------------
// Using Mid-Point Circle Drawing Algorithm

// draw circle arcs
//#define DRAWCAN_CIRCLE_ARC0	B0		// draw arc 0..45 deg
//#define DRAWCAN_CIRCLE_ARC1	B1		// draw arc 45..90 deg
//#define DRAWCAN_CIRCLE_ARC2	B2		// draw arc 90..135 deg
//#define DRAWCAN_CIRCLE_ARC3	B3		// draw arc 135..180 deg
//#define DRAWCAN_CIRCLE_ARC4	B4		// draw arc 180..225 deg
//#define DRAWCAN_CIRCLE_ARC5	B5		// draw arc 225..270 deg
//#define DRAWCAN_CIRCLE_ARC6	B6		// draw arc 270..315 deg
//#define DRAWCAN_CIRCLE_ARC7	B7		// draw arc 315..360 deg
//#define DRAWCAN_CIRCLE_ALL	0xff		// draw whole circle

#define DRAWCANCIRCLE_PIXFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xfrac = 7 - (x2 & 7);		\
		s = &s0[(x2>>3) + (y2-basey)*wb];	\
		*s = (*s & ~(0x01<<xfrac)) | (col<<xfrac); }

#define DRAWCANCIRCLE_PIX(xxx,yyy,mmm)			\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xfrac = 7 - (x2 & 7);		\
		s = &s0[(x2>>3) + (y2-basey)*wb];	\
		*s = (*s & ~(0x01<<xfrac)) | (col<<xfrac); } }

#define DRAWCANCIRCLE_PIXINVFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xfrac = 7 - (x2 & 7);		\
		s = &s0[(x2>>3) + (y2-basey)*wb];	\
		*s ^= 0x01<<xfrac; }

#define DRAWCANCIRCLE_PIXINV(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xfrac = 7 - (x2 & 7);		\
		s = &s0[(x2>>3) + (y2-basey)*wb];	\
		*s ^= 0x01<<xfrac; } }

//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan1Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan1Point_Fast(can, x, y, col);
		return;
	}

	// prepare
	int r = d/2;
	int xx = 0;
	int yy = r;
	int p = 1 - r;

	// pointer to middle of the circle
	int wb = can->wb;
	int basey = can->basey;
	u8* s0 = can->buf;
	u8* s;

	// odd diameter
	int c = -1; // even correction - on even diameter do not draw middle point
	if ((d & 1) != 0) // on odd diameter - draw middle point
	{
		// draw middle point (xx = 0)
		DRAWCANCIRCLE_PIXFAST(+yy,  0,DRAWCAN_CIRCLE_ARC0);	// 0..45 deg, 315..360 deg
		DRAWCANCIRCLE_PIXFAST(  0,-yy,DRAWCAN_CIRCLE_ARC1);	// 45..90 deg, 90..135 deg
		DRAWCANCIRCLE_PIXFAST(-yy,  0,DRAWCAN_CIRCLE_ARC3);	// 135..180 deg, 180..225 deg
		DRAWCANCIRCLE_PIXFAST(  0,+yy,DRAWCAN_CIRCLE_ARC5);	// 225..270 deg, 270..315 deg
		c = 0;
	}

	while (True)
	{
		// shift to next point
		xx++;
		if (p > 0)
		{
			yy--;
			p -= 2*yy;
		}
		p += 2*xx + 1;

		// stop drawing
		if (xx >= yy)
		{
			if (xx == yy)
			{
				// draw last point (xx == yy)
				DRAWCANCIRCLE_PIXFAST(+xx+c,-xx  ,DRAWCAN_CIRCLE_ARC0);	// 0..45 deg, 45..90 deg
				DRAWCANCIRCLE_PIXFAST(-xx  ,-xx  ,DRAWCAN_CIRCLE_ARC2);	// 90..135 deg, 135..180 deg
				DRAWCANCIRCLE_PIXFAST(-xx  ,+xx+c,DRAWCAN_CIRCLE_ARC4);	// 180..225 deg, 225..270 deg
				DRAWCANCIRCLE_PIXFAST(+xx+c,+xx+c,DRAWCAN_CIRCLE_ARC6); // 270..315 deg, 315..360 deg
			}
			break;
		}

		// draw points
		DRAWCANCIRCLE_PIXFAST(+yy+c,-xx  ,DRAWCAN_CIRCLE_ARC0);		// 0..45 deg
		DRAWCANCIRCLE_PIXFAST(+xx+c,-yy  ,DRAWCAN_CIRCLE_ARC1);		// 45..90 deg
		DRAWCANCIRCLE_PIXFAST(-xx  ,-yy  ,DRAWCAN_CIRCLE_ARC2);		// 90..135 deg
		DRAWCANCIRCLE_PIXFAST(-yy  ,-xx  ,DRAWCAN_CIRCLE_ARC3);		// 135..180 deg
		DRAWCANCIRCLE_PIXFAST(-yy  ,+xx+c,DRAWCAN_CIRCLE_ARC4);		// 180..225 deg
		DRAWCANCIRCLE_PIXFAST(-xx  ,+yy+c,DRAWCAN_CIRCLE_ARC5);		// 225..270 deg
		DRAWCANCIRCLE_PIXFAST(+xx+c,+yy+c,DRAWCAN_CIRCLE_ARC6);		// 270..315 deg
		DRAWCANCIRCLE_PIXFAST(+yy+c,+xx+c,DRAWCAN_CIRCLE_ARC7);		// 315..360 deg
	}
}
void Draw1Circle_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan1Circle_Fast(pDrawCan1, x, y, d, col, mask); }

// Draw circle or arc (checks clipping limits)
void DrawCan1Circle(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan1Point(can, x, y, col);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan1Circle_Fast(can, x, y, d, col, mask);
		return;
	}

	// prepare
	int xx = 0;
	int yy = r;
	int p = 1 - r;
	int clipx1 = can->clipx1;
	int clipx2 = can->clipx2;
	int clipy1 = can->clipy1;
	int clipy2 = can->clipy2;

	// pointer to middle of the circle
	int wb = can->wb;
	int basey = can->basey;
	u8* s0 = can->buf;
	u8* s;

	// odd diameter
	int c = -1; // even correction - on even diameter do not draw middle point
	if ((d & 1) != 0) // on odd diameter - draw middle point
	{
		// draw middle point (xx = 0)
		DRAWCANCIRCLE_PIX(+yy,  0,DRAWCAN_CIRCLE_ARC0);	// 0..45 deg, 315..360 deg
		DRAWCANCIRCLE_PIX(  0,-yy,DRAWCAN_CIRCLE_ARC1);	// 45..90 deg, 90..135 deg
		DRAWCANCIRCLE_PIX(-yy,  0,DRAWCAN_CIRCLE_ARC3);	// 135..180 deg, 180..225 deg
		DRAWCANCIRCLE_PIX(  0,+yy,DRAWCAN_CIRCLE_ARC5);	// 225..270 deg, 270..315 deg
		c = 0;
	}

	while (True)
	{
		// shift to next point
		xx++;
		if (p > 0)
		{
			yy--;
			p -= 2*yy;
		}
		p += 2*xx + 1;

		// stop drawing
		if (xx >= yy)
		{
			if (xx == yy)
			{
				// draw last point (xx == yy)
				DRAWCANCIRCLE_PIX(+xx+c,-xx  ,DRAWCAN_CIRCLE_ARC0);	// 0..45 deg, 45..90 deg
				DRAWCANCIRCLE_PIX(-xx  ,-xx  ,DRAWCAN_CIRCLE_ARC2);	// 90..135 deg, 135..180 deg
				DRAWCANCIRCLE_PIX(-xx  ,+xx+c,DRAWCAN_CIRCLE_ARC4);	// 180..225 deg, 225..270 deg
				DRAWCANCIRCLE_PIX(+xx+c,+xx+c,DRAWCAN_CIRCLE_ARC6); // 270..315 deg, 315..360 deg
			}
			break;
		}

		// draw points
		DRAWCANCIRCLE_PIX(+yy+c,-xx  ,DRAWCAN_CIRCLE_ARC0);		// 0..45 deg
		DRAWCANCIRCLE_PIX(+xx+c,-yy  ,DRAWCAN_CIRCLE_ARC1);		// 45..90 deg
		DRAWCANCIRCLE_PIX(-xx  ,-yy  ,DRAWCAN_CIRCLE_ARC2);		// 90..135 deg
		DRAWCANCIRCLE_PIX(-yy  ,-xx  ,DRAWCAN_CIRCLE_ARC3);		// 135..180 deg
		DRAWCANCIRCLE_PIX(-yy  ,+xx+c,DRAWCAN_CIRCLE_ARC4);		// 180..225 deg
		DRAWCANCIRCLE_PIX(-xx  ,+yy+c,DRAWCAN_CIRCLE_ARC5);		// 225..270 deg
		DRAWCANCIRCLE_PIX(+xx+c,+yy+c,DRAWCAN_CIRCLE_ARC6);		// 270..315 deg
		DRAWCANCIRCLE_PIX(+yy+c,+xx+c,DRAWCAN_CIRCLE_ARC7);		// 315..360 deg
	}
}
void Draw1Circle(int x, int y, int d, u16 col, u8 mask) { DrawCan1Circle(pDrawCan1, x, y, d, col, mask); }

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan1CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan1PointInv_Fast(can, x, y);
		return;
	}

	// prepare
	int r = d/2;
	int xx = 0;
	int yy = r;
	int p = 1 - r;

	// pointer to middle of the circle
	int wb = can->wb;
	int basey = can->basey;
	u8* s0 = can->buf;
	u8* s;

	// odd diameter
	int c = -1; // even correction - on even diameter do not draw middle point
	if ((d & 1) != 0) // on odd diameter - draw middle point
	{
		// draw middle point (xx = 0)
		DRAWCANCIRCLE_PIXINVFAST(+yy,  0,DRAWCAN_CIRCLE_ARC0);	// 0..45 deg, 315..360 deg
		DRAWCANCIRCLE_PIXINVFAST(  0,-yy,DRAWCAN_CIRCLE_ARC1);	// 45..90 deg, 90..135 deg
		DRAWCANCIRCLE_PIXINVFAST(-yy,  0,DRAWCAN_CIRCLE_ARC3);	// 135..180 deg, 180..225 deg
		DRAWCANCIRCLE_PIXINVFAST(  0,+yy,DRAWCAN_CIRCLE_ARC5);	// 225..270 deg, 270..315 deg
		c = 0;
	}

	while (True)
	{
		// shift to next point
		xx++;
		if (p > 0)
		{
			yy--;
			p -= 2*yy;
		}
		p += 2*xx + 1;

		// stop drawing
		if (xx >= yy)
		{
			if (xx == yy)
			{
				// draw last point (xx == yy)
				DRAWCANCIRCLE_PIXINVFAST(+xx+c,-xx  ,DRAWCAN_CIRCLE_ARC0);	// 0..45 deg, 45..90 deg
				DRAWCANCIRCLE_PIXINVFAST(-xx  ,-xx  ,DRAWCAN_CIRCLE_ARC2);	// 90..135 deg, 135..180 deg
				DRAWCANCIRCLE_PIXINVFAST(-xx  ,+xx+c,DRAWCAN_CIRCLE_ARC4);	// 180..225 deg, 225..270 deg
				DRAWCANCIRCLE_PIXINVFAST(+xx+c,+xx+c,DRAWCAN_CIRCLE_ARC6);	// 270..315 deg, 315..360 deg
			}
			break;
		}

		// draw points
		DRAWCANCIRCLE_PIXINVFAST(+yy+c,-xx  ,DRAWCAN_CIRCLE_ARC0);		// 0..45 deg
		DRAWCANCIRCLE_PIXINVFAST(+xx+c,-yy  ,DRAWCAN_CIRCLE_ARC1);		// 45..90 deg
		DRAWCANCIRCLE_PIXINVFAST(-xx  ,-yy  ,DRAWCAN_CIRCLE_ARC2);		// 90..135 deg
		DRAWCANCIRCLE_PIXINVFAST(-yy  ,-xx  ,DRAWCAN_CIRCLE_ARC3);		// 135..180 deg
		DRAWCANCIRCLE_PIXINVFAST(-yy  ,+xx+c,DRAWCAN_CIRCLE_ARC4);		// 180..225 deg
		DRAWCANCIRCLE_PIXINVFAST(-xx  ,+yy+c,DRAWCAN_CIRCLE_ARC5);		// 225..270 deg
		DRAWCANCIRCLE_PIXINVFAST(+xx+c,+yy+c,DRAWCAN_CIRCLE_ARC6);		// 270..315 deg
		DRAWCANCIRCLE_PIXINVFAST(+yy+c,+xx+c,DRAWCAN_CIRCLE_ARC7);		// 315..360 deg
	}
}
void Draw1CircleInv_Fast(int x, int y, int d, u8 mask) { DrawCan1CircleInv_Fast(pDrawCan1, x, y, d, mask); }

// Draw circle or arc inverted (checks clipping limits)
void DrawCan1CircleInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan1PointInv(can, x, y);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan1CircleInv_Fast(can, x, y, d, mask);
		return;
	}

	// prepare
	int xx = 0;
	int yy = r;
	int p = 1 - r;
	int clipx1 = can->clipx1;
	int clipx2 = can->clipx2;
	int clipy1 = can->clipy1;
	int clipy2 = can->clipy2;

	// pointer to middle of the circle
	int wb = can->wb;
	int basey = can->basey;
	u8* s0 = can->buf;
	u8* s;

	// odd diameter
	int c = -1; // even correction - on even diameter do not draw middle point
	if ((d & 1) != 0) // on odd diameter - draw middle point
	{
		// draw middle point (xx = 0)
		DRAWCANCIRCLE_PIXINV(+yy,  0,DRAWCAN_CIRCLE_ARC0);	// 0..45 deg, 315..360 deg
		DRAWCANCIRCLE_PIXINV(  0,-yy,DRAWCAN_CIRCLE_ARC1);	// 45..90 deg, 90..135 deg
		DRAWCANCIRCLE_PIXINV(-yy,  0,DRAWCAN_CIRCLE_ARC3);	// 135..180 deg, 180..225 deg
		DRAWCANCIRCLE_PIXINV(  0,+yy,DRAWCAN_CIRCLE_ARC5);	// 225..270 deg, 270..315 deg
		c = 0;
	}

	while (True)
	{
		// shift to next point
		xx++;
		if (p > 0)
		{
			yy--;
			p -= 2*yy;
		}
		p += 2*xx + 1;

		// stop drawing
		if (xx >= yy)
		{
			if (xx == yy)
			{
				// draw last point (xx == yy)
				DRAWCANCIRCLE_PIXINV(+xx+c,-xx  ,DRAWCAN_CIRCLE_ARC0);	// 0..45 deg, 45..90 deg
				DRAWCANCIRCLE_PIXINV(-xx  ,-xx  ,DRAWCAN_CIRCLE_ARC2);	// 90..135 deg, 135..180 deg
				DRAWCANCIRCLE_PIXINV(-xx  ,+xx+c,DRAWCAN_CIRCLE_ARC4);	// 180..225 deg, 225..270 deg
				DRAWCANCIRCLE_PIXINV(+xx+c,+xx+c,DRAWCAN_CIRCLE_ARC6);	// 270..315 deg, 315..360 deg
			}
			break;
		}

		// draw points
		DRAWCANCIRCLE_PIXINV(+yy+c,-xx  ,DRAWCAN_CIRCLE_ARC0);		// 0..45 deg
		DRAWCANCIRCLE_PIXINV(+xx+c,-yy  ,DRAWCAN_CIRCLE_ARC1);		// 45..90 deg
		DRAWCANCIRCLE_PIXINV(-xx  ,-yy  ,DRAWCAN_CIRCLE_ARC2);		// 90..135 deg
		DRAWCANCIRCLE_PIXINV(-yy  ,-xx  ,DRAWCAN_CIRCLE_ARC3);		// 135..180 deg
		DRAWCANCIRCLE_PIXINV(-yy  ,+xx+c,DRAWCAN_CIRCLE_ARC4);		// 180..225 deg
		DRAWCANCIRCLE_PIXINV(-xx  ,+yy+c,DRAWCAN_CIRCLE_ARC5);		// 225..270 deg
		DRAWCANCIRCLE_PIXINV(+xx+c,+yy+c,DRAWCAN_CIRCLE_ARC6);		// 270..315 deg
		DRAWCANCIRCLE_PIXINV(+yy+c,+xx+c,DRAWCAN_CIRCLE_ARC7);		// 315..360 deg
	}
}
void Draw1CircleInv(int x, int y, int d, u8 mask) { DrawCan1CircleInv(pDrawCan1, x, y, d, mask); }

#undef DRAWCANCIRCLE_PIXFAST
#undef DRAWCANCIRCLE_PIX
#undef DRAWCANCIRCLE_PIXINVFAST
#undef DRAWCANCIRCLE_PIXINV

// ----------------------------------------------------------------------------
//                              Draw ring
// ----------------------------------------------------------------------------

//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring
//		DRAWCAN_ROUND_NOTOP	= hide top part of the ring
//		DRAWCAN_ROUND_NOBOTTOM	= hide bottom part of the ring
//		DRAWCAN_ROUND_NOLEFT	= hide left part of the ring
//		DRAWCAN_ROUND_NORIGHT	= hide right part of the ring
//		DRAWCAN_ROUND_ALL	= draw whole ring

// Draw ring (does not check clipping limits)
void DrawCan1Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan1Round_Fast(can, x, y, d, col, mask);
		return;
	}

	// draw circle
	if (din == d-1)
	{
		u8 mask2 = DRAWCAN_CIRCLE_ALL;
		if ((mask & DRAWCAN_ROUND_NOTOP) != 0) mask2 &= DRAWCAN_CIRCLE_BOTTOM;
		if ((mask & DRAWCAN_ROUND_NOBOTTOM) != 0) mask2 &= DRAWCAN_CIRCLE_TOP;
		if ((mask & DRAWCAN_ROUND_NOLEFT) != 0) mask2 &= DRAWCAN_CIRCLE_RIGHT;
		if ((mask & DRAWCAN_ROUND_NORIGHT) != 0) mask2 &= DRAWCAN_CIRCLE_LEFT;
		DrawCan1Circle_Fast(can, x, y, d, col, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0, *s;
	int r2, rin2, xx, xxx, xfrac;

	// hide parts of the ring
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		s0 = &can->buf[((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		rin2 = rin*(rin-1);
		if (d == 3) r2--;
		if (din == 3) rin2--;

		// draw ring
		for (y = y1; y <= y2; y++)
		{
			for (xx = x1; xx <= x2; xx++)
			{
				r = xx*xx + y*y;
				if ((r <= r2) && (r > rin2))
				{
					xxx = xx+x;
					xfrac = 7 - (xxx & 7);
					s = &s0[xxx>>3];
					*s = (*s & ~(0x01<<xfrac)) | (col<<xfrac);
				}
			}
			s0 += wb;
		}
	}

	// even diameter (2, 4, 6, ...)
	else
	{
		// to correspond to the center of DrawCircle [x0,y0] = [x+d/2,y+d/2]
		x--;
		y--;

		// prepare buffer address
		x1++;
		y1++;
		s0 = &can->buf[((y+y1) - can->basey)*wb];
		r2 = r*r;
		rin2 = rin*rin;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			for (xx = x1; xx <= x2; xx++)
			{
				r = xx*(xx-1) + y*(y-1);
				if ((r < r2) && (r >= rin2))
				{
					xxx = xx+x;
					xfrac = 7 - (xxx & 7);
					s = &s0[xxx>>3];
					*s = (*s & ~(0x01<<xfrac)) | (col<<xfrac);
				}
			}
			s0 += wb;
		}
	}
}
void Draw1Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan1Ring_Fast(pDrawCan1, x, y, d, din, col, mask); }

// Draw ring (checks clipping limits)
void DrawCan1Ring(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	u8 col = (u8)col8;
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan1Round(can, x, y, d, col, mask);
		return;
	}

	// draw circle
	if (din == d-1)
	{
		u8 mask2 = DRAWCAN_CIRCLE_ALL;
		if ((mask & DRAWCAN_ROUND_NOTOP) != 0) mask2 &= DRAWCAN_CIRCLE_BOTTOM;
		if ((mask & DRAWCAN_ROUND_NOBOTTOM) != 0) mask2 &= DRAWCAN_CIRCLE_TOP;
		if ((mask & DRAWCAN_ROUND_NOLEFT) != 0) mask2 &= DRAWCAN_CIRCLE_RIGHT;
		if ((mask & DRAWCAN_ROUND_NORIGHT) != 0) mask2 &= DRAWCAN_CIRCLE_LEFT;
		DrawCan1Circle(can, x, y, d, col, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0, *s;
	int r2, rin2, xx, xxx, xfrac;

	// hide parts of the ring
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// clipping
		DRAWCANROUND_CLIP();

		// update dirty area
		DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

		// prepare buffer address
		s0 = &can->buf[((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		rin2 = rin*(rin-1);
		if (d == 3) r2--;
		if (din == 3) rin2--;

		// draw ring
		for (y = y1; y <= y2; y++)
		{
			for (xx = x1; xx <= x2; xx++)
			{
				r = xx*xx + y*y;
				if ((r <= r2) && (r > rin2))
				{
					xxx = xx+x;
					xfrac = 7 - (xxx & 7);
					s = &s0[xxx>>3];
					*s = (*s & ~(0x01<<xfrac)) | (col<<xfrac);
				}
			}
			s0 += wb;
		}
	}

	// even diameter (2, 4, 6, ...)
	else
	{
		// to correspond to the center of DrawCircle [x0,y0] = [x+d/2,y+d/2]
		x--;
		y--;
		x1++;
		y1++;

		// clipping
		DRAWCANROUND_CLIP();

		// update dirty area
		DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

		// prepare buffer address
		s0 = &can->buf[((y+y1) - can->basey)*wb];
		r2 = r*r;
		rin2 = rin*rin;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			for (xx = x1; xx <= x2; xx++)
			{
				r = xx*(xx-1) + y*(y-1);
				if ((r < r2) && (r >= rin2))
				{
					xxx = xx+x;
					xfrac = 7 - (xxx & 7);
					s = &s0[xxx>>3];
					*s = (*s & ~(0x01<<xfrac)) | (col<<xfrac);
				}
			}
			s0 += wb;
		}
	}
}
void Draw1Ring(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan1Ring(pDrawCan1, x, y, d, din, col, mask); }

// Draw ring inverted (does not check clipping limits)
void DrawCan1RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan1RoundInv_Fast(can, x, y, d, mask);
		return;
	}

	// draw circle
	if (din == d-1)
	{
		u8 mask2 = DRAWCAN_CIRCLE_ALL;
		if ((mask & DRAWCAN_ROUND_NOTOP) != 0) mask2 &= DRAWCAN_CIRCLE_BOTTOM;
		if ((mask & DRAWCAN_ROUND_NOBOTTOM) != 0) mask2 &= DRAWCAN_CIRCLE_TOP;
		if ((mask & DRAWCAN_ROUND_NOLEFT) != 0) mask2 &= DRAWCAN_CIRCLE_RIGHT;
		if ((mask & DRAWCAN_ROUND_NORIGHT) != 0) mask2 &= DRAWCAN_CIRCLE_LEFT;
		DrawCan1CircleInv_Fast(can, x, y, d, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0, *s;
	int r2, rin2, xx, xxx, xfrac;

	// hide parts of the ring
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		s0 = &can->buf[((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		rin2 = rin*(rin-1);
		if (d == 3) r2--;
		if (din == 3) rin2--;

		// draw ring
		for (y = y1; y <= y2; y++)
		{
			for (xx = x1; xx <= x2; xx++)
			{
				r = xx*xx + y*y;
				if ((r <= r2) && (r > rin2))
				{
					xxx = xx+x;
					xfrac = 7 - (xxx & 7);
					s = &s0[xxx>>3];
					*s ^= 0x01<<xfrac;
				}
			}
			s0 += wb;
		}
	}

	// even diameter (2, 4, 6, ...)
	else
	{
		// to correspond to the center of DrawCircle [x0,y0] = [x+d/2,y+d/2]
		x--;
		y--;

		// prepare buffer address
		x1++;
		y1++;
		s0 = &can->buf[((y+y1) - can->basey)*wb];
		r2 = r*r;
		rin2 = rin*rin;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			for (xx = x1; xx <= x2; xx++)
			{
				r = xx*(xx-1) + y*(y-1);
				if ((r < r2) && (r >= rin2))
				{
					xxx = xx+x;
					xfrac = 7 - (xxx & 7);
					s = &s0[xxx>>3];
					*s ^= 0x01<<xfrac;
				}
			}
			s0 += wb;
		}
	}
}
void Draw1RingInv_Fast(int x, int y, int d, int din, u8 mask) { DrawCan1RingInv_Fast(pDrawCan1, x, y, d, din, mask); }

// Draw ring inverted (checks clipping limits)
void DrawCan1RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan1RoundInv(can, x, y, d, mask);
		return;
	}

	// draw circle
	if (din == d-1)
	{
		u8 mask2 = DRAWCAN_CIRCLE_ALL;
		if ((mask & DRAWCAN_ROUND_NOTOP) != 0) mask2 &= DRAWCAN_CIRCLE_BOTTOM;
		if ((mask & DRAWCAN_ROUND_NOBOTTOM) != 0) mask2 &= DRAWCAN_CIRCLE_TOP;
		if ((mask & DRAWCAN_ROUND_NOLEFT) != 0) mask2 &= DRAWCAN_CIRCLE_RIGHT;
		if ((mask & DRAWCAN_ROUND_NORIGHT) != 0) mask2 &= DRAWCAN_CIRCLE_LEFT;
		DrawCan1CircleInv(can, x, y, d, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0, *s;
	int r2, rin2, xx, xxx, xfrac;

	// hide parts of the ring
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// clipping
		DRAWCANROUND_CLIP();

		// update dirty area
		DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

		// prepare buffer address
		s0 = &can->buf[((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		rin2 = rin*(rin-1);
		if (d == 3) r2--;
		if (din == 3) rin2--;

		// draw ring
		for (y = y1; y <= y2; y++)
		{
			for (xx = x1; xx <= x2; xx++)
			{
				r = xx*xx + y*y;
				if ((r <= r2) && (r > rin2))
				{
					xxx = xx+x;
					xfrac = 7 - (xxx & 7);
					s = &s0[xxx>>3];
					*s ^= 0x01<<xfrac;
				}
			}
			s0 += wb;
		}
	}

	// even diameter (2, 4, 6, ...)
	else
	{
		// to correspond to the center of DrawCircle [x0,y0] = [x+d/2,y+d/2]
		x--;
		y--;
		x1++;
		y1++;

		// clipping
		DRAWCANROUND_CLIP();

		// update dirty area
		DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

		// prepare buffer address
		s0 = &can->buf[((y+y1) - can->basey)*wb];
		r2 = r*r;
		rin2 = rin*rin;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			for (xx = x1; xx <= x2; xx++)
			{
				r = xx*(xx-1) + y*(y-1);
				if ((r < r2) && (r >= rin2))
				{
					xxx = xx+x;
					xfrac = 7 - (xxx & 7);
					s = &s0[xxx>>3];
					*s ^= 0x01<<xfrac;
				}
			}
			s0 += wb;
		}
	}
}
void Draw1RingInv(int x, int y, int d, int din, u8 mask) { DrawCan1RingInv(pDrawCan1, x, y, d, din, mask); }

// ----------------------------------------------------------------------------
//                              Draw triangle
// ----------------------------------------------------------------------------

// Draw triangle (does not check clipping limits)
void DrawCan1Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
{
	int x, y, k, xmin, xmax;
	u8 col = (u8)col8;

	// sort coordinates by Y (order from top to bottom: 1 / 2 / 3)
	if (y2 < y1)
	{
		k = x2; x2 = x1; x1 = k;
		k = y2; y2 = y1; y1 = k;
	}

	if (y3 < y1)
	{
		k = x3; x3 = x1; x1 = k;
		k = y3; y3 = y1; y1 = k;
	}
			
	if (y3 < y2)
	{
		k = x3; x3 = x2; x2 = k;
		k = y3; y3 = y2; y2 = k;
	}

	// top sub-triangle y1 <= y < y2 (without bottom y2)
	for (y = y1; y < y2; y++)
	{
		xmin = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		xmax = x1 + (x3 - x1) * (y - y1) / (y3 - y1);

		if (xmax < xmin)
		{
			k = xmin - xmax + 1;
			if (k > 0) DrawCan1HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan1HLine_Fast(can, xmin, y, k, col);
		}
	}

	// bottom y2 == y3
	if (y2 == y3)
	{
		xmin = x2;
		xmax = x3;

		if (xmax < xmin)
		{
			k = xmin - xmax + 1;
			if (k > 0) DrawCan1HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan1HLine_Fast(can, xmin, y, k, col);
		}
	}

	// bottom sub-triangle y2 <= y <= y3
	else
	{
		for (; y <= y3; y++)
		{
			xmin = x2 + (x3 - x2) * (y - y2) / (y3 - y2);
			xmax = x1 + (x3 - x1) * (y - y1) / (y3 - y1);

			if (xmax < xmin)
			{
				k = xmin - xmax + 1;
				if (k > 0) DrawCan1HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan1HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw1Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan1Triangle_Fast(pDrawCan1, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle (checks clipping limits)
void DrawCan1Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
{
	int x, y, k, xmin, xmax;
	u8 col = (u8)col8;

	// sort coordinates by Y (order from top to bottom: 1 / 2 / 3)
	if (y2 < y1)
	{
		k = x2; x2 = x1; x1 = k;
		k = y2; y2 = y1; y1 = k;
	}

	if (y3 < y1)
	{
		k = x3; x3 = x1; x1 = k;
		k = y3; y3 = y1; y1 = k;
	}
			
	if (y3 < y2)
	{
		k = x3; x3 = x2; x2 = k;
		k = y3; y3 = y2; y2 = k;
	}

	// update dirty area
	xmin = x1;
	if (x2 < xmin) xmin = x2;
	if (x3 < xmin) xmin = x3;
	xmax = x1;
	if (x2 > xmax) xmax = x2;
	if (x3 > xmax) xmax = x3;
	DrawCanDirtyRect(can, xmin, y1, xmax-xmin+1, y3-y1+1);

	// use fast version
	if (DrawCanRectClipped(can, xmin, y1, xmax-xmin+1, y3-y1+1))
	{
		DrawCan1Triangle_Fast(can, x1, y1, x2, y2, x3, y3, col);
		return;
	}

	// prepare clipping
	int clipx1 = can->clipx1;
	int clipx2 = can->clipx2;
	int clipy1 = can->clipy1;
	int clipy2 = can->clipy2;

	// top sub-triangle y1 <= y < y2 (without bottom y2)
	y = y1;
	if (y < clipy1) y = clipy1;
	int ymax = y2;
	if (ymax > clipy2) ymax = clipy2;
	for (; y < ymax; y++)
	{
		xmin = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		xmax = x1 + (x3 - x1) * (y - y1) / (y3 - y1);

		if (xmax < xmin)
		{
			if (xmax < clipx1) xmax = clipx1;
			if (xmin >= clipx2) xmin = clipx2-1;
			k = xmin - xmax + 1;
			if (k > 0) DrawCan1HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan1HLine_Fast(can, xmin, y, k, col);
		}
	}

	// bottom y2 == y3
	if (y2 == y3)
	{
		if ((y2 >= clipy1) && (y2 < clipy2))
		{
			xmin = x2;
			xmax = x3;

			if (xmax < xmin)
			{
				if (xmax < clipx1) xmax = clipx1;
				if (xmin >= clipx2) xmin = clipx2-1;
				k = xmin - xmax + 1;
				if (k > 0) DrawCan1HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan1HLine_Fast(can, xmin, y, k, col);
			}
		}
	}

	// bottom sub-triangle y2 <= y <= y3
	else
	{
		y = y2;
		if (y < clipy1) y = clipy1;
		ymax = y3;
		if (ymax >= clipy2) ymax = clipy2-1;
		for (; y <= ymax; y++)
		{
			xmin = x2 + (x3 - x2) * (y - y2) / (y3 - y2);
			xmax = x1 + (x3 - x1) * (y - y1) / (y3 - y1);

			if (xmax < xmin)
			{
				if (xmax < clipx1) xmax = clipx1;
				if (xmin >= clipx2) xmin = clipx2-1;
				k = xmin - xmax + 1;
				if (k > 0) DrawCan1HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan1HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw1Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan1Triangle(pDrawCan1, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle inverted (does not check clipping limits)
void DrawCan1TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
{
	int x, y, k, xmin, xmax;

	// sort coordinates by Y (order from top to bottom: 1 / 2 / 3)
	if (y2 < y1)
	{
		k = x2; x2 = x1; x1 = k;
		k = y2; y2 = y1; y1 = k;
	}

	if (y3 < y1)
	{
		k = x3; x3 = x1; x1 = k;
		k = y3; y3 = y1; y1 = k;
	}
			
	if (y3 < y2)
	{
		k = x3; x3 = x2; x2 = k;
		k = y3; y3 = y2; y2 = k;
	}

	// top sub-triangle y1 <= y < y2 (without bottom y2)
	for (y = y1; y < y2; y++)
	{
		xmin = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		xmax = x1 + (x3 - x1) * (y - y1) / (y3 - y1);

		if (xmax < xmin)
		{
			k = xmin - xmax + 1;
			if (k > 0) DrawCan1HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan1HLineInv_Fast(can, xmin, y, k);
		}
	}

	// bottom y2 == y3
	if (y2 == y3)
	{
		xmin = x2;
		xmax = x3;

		if (xmax < xmin)
		{
			k = xmin - xmax + 1;
			if (k > 0) DrawCan1HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan1HLineInv_Fast(can, xmin, y, k);
		}
	}

	// bottom sub-triangle y2 <= y <= y3
	else
	{
		for (; y <= y3; y++)
		{
			xmin = x2 + (x3 - x2) * (y - y2) / (y3 - y2);
			xmax = x1 + (x3 - x1) * (y - y1) / (y3 - y1);

			if (xmax < xmin)
			{
				k = xmin - xmax + 1;
				if (k > 0) DrawCan1HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan1HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw1TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan1TriangleInv_Fast(pDrawCan1, x1, y1, x2, y2, x3, y3); }

// Draw triangle inverted (checks clipping limits)
void DrawCan1TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
{
	int x, y, k, xmin, xmax;

	// sort coordinates by Y (order from top to bottom: 1 / 2 / 3)
	if (y2 < y1)
	{
		k = x2; x2 = x1; x1 = k;
		k = y2; y2 = y1; y1 = k;
	}

	if (y3 < y1)
	{
		k = x3; x3 = x1; x1 = k;
		k = y3; y3 = y1; y1 = k;
	}
			
	if (y3 < y2)
	{
		k = x3; x3 = x2; x2 = k;
		k = y3; y3 = y2; y2 = k;
	}

	// update dirty area
	xmin = x1;
	if (x2 < xmin) xmin = x2;
	if (x3 < xmin) xmin = x3;
	xmax = x1;
	if (x2 > xmax) xmax = x2;
	if (x3 > xmax) xmax = x3;
	DrawCanDirtyRect(can, xmin, y1, xmax-xmin+1, y3-y1+1);

	// use fast version
	if (DrawCanRectClipped(can, xmin, y1, xmax-xmin+1, y3-y1+1))
	{
		DrawCan1TriangleInv_Fast(can, x1, y1, x2, y2, x3, y3);
		return;
	}

	// prepare clipping
	int clipx1 = can->clipx1;
	int clipx2 = can->clipx2;
	int clipy1 = can->clipy1;
	int clipy2 = can->clipy2;

	// top sub-triangle y1 <= y < y2 (without bottom y2)
	int wb = can->wb;
	int basey = can->basey;
	y = y1;
	if (y < clipy1) y = clipy1;
	int ymax = y2;
	if (ymax > clipy2) ymax = clipy2;
	for (; y < ymax; y++)
	{
		xmin = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		xmax = x1 + (x3 - x1) * (y - y1) / (y3 - y1);

		if (xmax < xmin)
		{
			if (xmax < clipx1) xmax = clipx1;
			if (xmin >= clipx2) xmin = clipx2-1;
			k = xmin - xmax + 1;
			if (k > 0) DrawCan1HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan1HLineInv_Fast(can, xmin, y, k);
		}
	}

	// bottom y2 == y3
	if (y2 == y3)
	{
		if ((y2 >= clipy1) && (y2 < clipy2))
		{
			xmin = x2;
			xmax = x3;

			if (xmax < xmin)
			{
				if (xmax < clipx1) xmax = clipx1;
				if (xmin >= clipx2) xmin = clipx2-1;
				k = xmin - xmax + 1;
				if (k > 0) DrawCan1HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan1HLineInv_Fast(can, xmin, y, k);
			}
		}
	}

	// bottom sub-triangle y2 <= y <= y3
	else
	{
		y = y2;
		if (y < clipy1) y = clipy1;
		ymax = y3;
		if (ymax >= clipy2) ymax = clipy2-1;
		for (; y <= ymax; y++)
		{
			xmin = x2 + (x3 - x2) * (y - y2) / (y3 - y2);
			xmax = x1 + (x3 - x1) * (y - y1) / (y3 - y1);

			if (xmax < xmin)
			{
				if (xmax < clipx1) xmax = clipx1;
				if (xmin >= clipx2) xmin = clipx2-1;
				k = xmin - xmax + 1;
				if (k > 0) DrawCan1HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan1HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw1TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan1TriangleInv(pDrawCan1, x1, y1, x2, y2, x3, y3); }

// ----------------------------------------------------------------------------
//                              Draw fill area
// ----------------------------------------------------------------------------

// Draw fill sub-area (internal function)
void _DrawCan1SubFill(sDrawCan* can, int x, int y, u8 fnd, u8 col)
{
	// save start position -> x2
	int x2 = x;

	// fill start point
	DrawCan1Point_Fast(can, x, y, col);

	// update dirty area in Y direction
	if (y < can->dirtyy1) can->dirtyy1 = y;
	if (y >= can->dirtyy2) can->dirtyy2 = y+1;

	// search start of segment in LEFT-X direction -> x1
	int clipx1 = can->clipx1;
	while ((x > clipx1) && ((u8)DrawCan1GetPoint_Fast(can, x-1, y) == fnd))
	{
		x--;
		DrawCan1Point_Fast(can, x, y, col);
	}
	int x1 = x;

	// search end of segment in RIGHT-X direction -> x2
	int clipx2 = can->clipx2-1;
	while ((x2 < clipx2) && ((u8)DrawCan1GetPoint_Fast(can, x2+1, y) == fnd))
	{
		x2++;
		DrawCan1Point_Fast(can, x2, y, col);
	}

	// search segments in UP-Y direction
	if (y > can->clipy1)
	{
		x = x1;
		y--;
		while (x <= x2)
		{
			if ((u8)DrawCan1GetPoint_Fast(can, x, y) == fnd) _DrawCan1SubFill(can, x, y, fnd, col);
			x++;
		}
		y++;
	}

	// search segments in DOWN-Y direction
	if (y < can->clipy2-1)
	{
		x = x1;
		y++;
		while (x <= x2)
		{
			if ((u8)DrawCan1GetPoint_Fast(can, x, y) == fnd) _DrawCan1SubFill(can, x, y, fnd, col);
			x++;
		}
	}

	// update dirty area in X direction
	if (x1 < can->dirtyx1) can->dirtyx1 = x1;
	if (x2 >= can->dirtyx2) can->dirtyx2 = x2+1;
}

// Draw fill area
void DrawCan1Fill(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check clipping
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// get color to search
	u8 fnd = (u8)DrawCan1GetPoint_Fast(can, x, y);

	// fill if color is different
	if (fnd != col) _DrawCan1SubFill(can, x, y, fnd, col);
}
void Draw1Fill(int x, int y, u16 col) { DrawCan1Fill(pDrawCan1, x, y, col); }

// ----------------------------------------------------------------------------
//                             Draw charactter
// ----------------------------------------------------------------------------
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw1SelFont(const u8* font, u8 fontw, u8 fonth) { DrawCanSelFont(pDrawCan1, font, fontw, fonth); }

void Draw1SelFont8x8()		{ DrawCanSelFont8x8(pDrawCan1); }		// sans-serif bold, height 8 lines
void Draw1SelFont8x14()		{ DrawCanSelFont8x14(pDrawCan1); }		// sans-serif bold, height 14 lines
void Draw1SelFont8x16()		{ DrawCanSelFont8x16(pDrawCan1); }		// sans-serif bold, height 16 lines
void Draw1SelFont8x14Serif()	{ DrawCanSelFont8x14Serif(pDrawCan1); }		// serif bold, height 14 lines
void Draw1SelFont8x16Serif()	{ DrawCanSelFont8x16Serif(pDrawCan1); }		// serif bold, height 16 lines
void Draw1SelFont6x8()		{ DrawCanSelFont6x8(pDrawCan1); }		// condensed font, width 6 pixels, height 8 lines
void Draw1SelFont8x8Game()	{ DrawCanSelFont8x8Game(pDrawCan1); }		// game font, height 8 lines
void Draw1SelFont8x8Ibm()	{ DrawCanSelFont8x8Ibm(pDrawCan1); }		// IBM charset font, height 8 lines
void Draw1SelFont8x14Ibm()	{ DrawCanSelFont8x14Ibm(pDrawCan1); }		// IBM charset font, height 14 lines
void Draw1SelFont8x16Ibm()	{ DrawCanSelFont8x16Ibm(pDrawCan1); }		// IBM charset font, height 16 lines
void Draw1SelFont8x8IbmThin()	{ DrawCanSelFont8x8IbmThin(pDrawCan1); }	// IBM charset thin font, height 8 lines
void Draw1SelFont8x8Italic()	{ DrawCanSelFont8x8Italic(pDrawCan1); }		// italic, height 8 lines
void Draw1SelFont8x8Thin()	{ DrawCanSelFont8x8Thin(pDrawCan1); }		// thin font, height 8 lines
void Draw1SelFont5x8()		{ DrawCanSelFont5x8(pDrawCan1); }		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan1Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
{
	int i, j, wb, scalex2, scaley2;
	u8 b;
	u8 col = (u8)col8;

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// loop through lines of one character
	int x0 = x;
	scaley2 = scaley;
	for (i = can->fonth*scaley; i > 0; i--)
	{
		// get one font sample
		b = *s;

		// loop through pixels of one character line
		x = x0;
		if (scalex == 1)
		{
			for (j = can->fontw; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan1Point_Fast(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan1Point_Fast(can, x, y, col);
				x++;
				scalex2--;
				if (scalex2 <= 0)
				{
					b <<= 1;
					scalex2 = scalex;
				}
			}
		}

		// scale Y
		y++;
		scaley2--;
		if (scaley2 <= 0)
		{
			s += 256;
			scaley2 = scaley;
		}
	}
}
void Draw1Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan1Char_Fast(pDrawCan1, ch, x, y, col, scalex, scaley); }

// Draw character with transparent background (checks clipping limits)
void DrawCan1Char(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
{
	int i, j, scaley2, scalex2;
	u8 b;
	u8 col = (u8)col8;

	// invalid size
	if ((scalex < 1) || (scaley < 1)) return;

	// character size
	int w = can->fontw*scalex;
	int h = can->fonth*scaley;

	// use fast version
	if (DrawCanRectClipped(can, x, y, w, h))
	{
		DrawCanDirtyRect_Fast(can, x, y, w, h);
		DrawCan1Char_Fast(can, ch, x, y, col, scalex, scaley);
		return;
	}

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// loop through lines of one character
	int x0 = x;
	scaley2 = scaley;
	for (i = h; i > 0; i--)
	{
		// get one font sample
		b = *s;

		// loop through pixels of one character line
		x = x0;
		if (scalex == 1)
		{
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan1Point(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan1Point(can, x, y, col);
				x++;
				scalex2--;
				if (scalex2 <= 0)
				{
					b <<= 1;
					scalex2 = scalex;
				}
			}
		}

		// scale Y
		y++;
		scaley2--;
		if (scaley2 <= 0)
		{
			s += 256;
			scaley2 = scaley;
		}
	}
}
void Draw1Char(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan1Char(pDrawCan1, ch, x, y, col, scalex, scaley); }

// Draw character with background (does not check clipping limits)
void DrawCan1CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
{
	int i, j, wb, scaley2, scalex2;
	u8 b;
	u8 col = (u8)col8;
	u8 bgcol = (u8)bgcol8;

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// loop through lines of one character
	int x0 = x;
	scaley2 = scaley;
	for (i = can->fonth*scaley; i > 0; i--)
	{
		// get one font sample
		b = *s;

		// loop through pixels of one character line
		x = x0;
		if (scalex == 1)
		{
			for (j = can->fontw; j > 0; j--)
			{
				DrawCan1Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				DrawCan1Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				scalex2--;
				if (scalex2 <= 0)
				{
					b <<= 1;
					scalex2 = scalex;
				}
			}
		}

		// scale Y
		y++;
		scaley2--;
		if (scaley2 <= 0)
		{
			s += 256;
			scaley2 = scaley;
		}
	}
}
void Draw1CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan1CharBg_Fast(pDrawCan1, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character with background (checks clipping limits)
void DrawCan1CharBg(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
{
	int i, j, scaley2, scalex2;
	u8 b;
	u8 col = (u8)col8;
	u8 bgcol = (u8)bgcol8;

	// invalid size
	if ((scalex < 1) || (scaley < 1)) return;

	// character size
	int w = can->fontw*scalex;
	int h = can->fonth*scaley;

	// use fast version
	if (DrawCanRectClipped(can, x, y, w, h))
	{
		DrawCanDirtyRect_Fast(can, x, y, w, h);
		DrawCan1CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);
		return;
	}

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// loop through lines of one character
	int x0 = x;
	scaley2 = scaley;
	for (i = h; i > 0; i--)
	{
		// get one font sample
		b = *s;

		// loop through pixels of one character line
		x = x0;
		if (scalex == 1)
		{
			for (j = w; j > 0; j--)
			{
				DrawCan1Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				DrawCan1Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				scalex2--;
				if (scalex2 <= 0)
				{
					b <<= 1;
					scalex2 = scalex;
				}
			}
		}

		// scale Y
		y++;
		scaley2--;
		if (scaley2 <= 0)
		{
			s += 256;
			scaley2 = scaley;
		}
	}
}
void Draw1CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan1CharBg(pDrawCan1, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character inverted (does not check clipping limits)
void DrawCan1CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
{
	int i, j, wb, scaley2, scalex2;
	u8 b;

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// loop through lines of one character
	int x0 = x;
	scaley2 = scaley;
	for (i = can->fonth*scaley; i > 0; i--)
	{
		// get one font sample
		b = *s;

		// loop through pixels of one character line
		x = x0;
		if (scalex == 1)
		{
			for (j = can->fontw; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan1PointInv_Fast(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan1PointInv_Fast(can, x, y);
				x++;
				scalex2--;
				if (scalex2 <= 0)
				{
					b <<= 1;
					scalex2 = scalex;
				}
			}
		}

		// scale Y
		y++;
		scaley2--;
		if (scaley2 <= 0)
		{
			s += 256;
			scaley2 = scaley;
		}
	}
}
void Draw1CharInv_Fast(char ch, int x, int y, int scalex, int scaley) { DrawCan1CharInv_Fast(pDrawCan1, ch, x, y, scalex, scaley); }

// Draw character inverted (checks clipping limits)
void DrawCan1CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
{
	int i, j, scaley2, scalex2;
	u8 b;

	// invalid size
	if ((scalex < 1) || (scaley < 1)) return;

	// character size
	int w = can->fontw*scalex;
	int h = can->fonth*scaley;

	// use fast version
	if (DrawCanRectClipped(can, x, y, w, h))
	{
		DrawCanDirtyRect_Fast(can, x, y, w, h);
		DrawCan1CharInv_Fast(can, ch, x, y, scalex, scaley);
		return;
	}

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// loop through lines of one character
	int x0 = x;
	scaley2 = scaley;
	for (i = h; i > 0; i--)
	{
		// get one font sample
		b = *s;

		// loop through pixels of one character line
		x = x0;
		if (scalex == 1)
		{
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan1PointInv(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan1PointInv(can, x, y);
				x++;
				scalex2--;
				if (scalex2 <= 0)
				{
					b <<= 1;
					scalex2 = scalex;
				}
			}
		}

		// scale Y
		y++;
		scaley2--;
		if (scaley2 <= 0)
		{
			s += 256;
			scaley2 = scaley;
		}
	}
}
void Draw1CharInv(char ch, int x, int y, int scalex, int scaley) { DrawCan1CharInv(pDrawCan1, ch, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                             Draw text
// ----------------------------------------------------------------------------
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan1Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
{
	// get text length
	if (len < 0) len = StrLen(text);

	// loop through characters of text
	char ch;
	int dx = can->fontw*scalex;
	for (; len > 0; len--)
	{
		// get next character of the text
		ch = *text++;

		// draw character
		DrawCan1Char_Fast(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw1Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan1Text_Fast(pDrawCan1, text, len, x, y, col, scalex, scaley); }

// Draw text with transparent background (checks clipping limits)
void DrawCan1Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
{
	// get text length
	if (len < 0) len = StrLen(text);

	// loop through characters of text
	char ch;
	int dx = can->fontw*scalex;
	for (; len > 0; len--)
	{
		// get next character of the text
		ch = *text++;

		// draw character
		DrawCan1Char(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw1Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan1Text(pDrawCan1, text, len, x, y, col, scalex, scaley); }

// Draw text with background (does not check clipping limits)
void DrawCan1TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
{
	// get text length
	if (len < 0) len = StrLen(text);

	// loop through characters of text
	char ch;
	int dx = can->fontw*scalex;
	for (; len > 0; len--)
	{
		// get next character of the text
		ch = *text++;

		// draw character
		DrawCan1CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw1TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan1TextBg_Fast(pDrawCan1, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text with background (checks clipping limits)
void DrawCan1TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
{
	// get text length
	if (len < 0) len = StrLen(text);

	// loop through characters of text
	char ch;
	int dx = can->fontw*scalex;
	for (; len > 0; len--)
	{
		// get next character of the text
		ch = *text++;

		// draw character
		DrawCan1CharBg(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw1TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan1TextBg(pDrawCan1, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text inverted (does not check clipping limits)
void DrawCan1TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
{
	// get text length
	if (len < 0) len = StrLen(text);

	// loop through characters of text
	char ch;
	int dx = can->fontw*scalex;
	for (; len > 0; len--)
	{
		// get next character of the text
		ch = *text++;

		// draw character
		DrawCan1CharInv_Fast(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw1TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan1TextInv_Fast(pDrawCan1, text, len, x, y, scalex, scaley); }

// Draw text inverted (checks clipping limits)
void DrawCan1TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
{
	// get text length
	if (len < 0) len = StrLen(text);

	// loop through characters of text
	char ch;
	int dx = can->fontw*scalex;
	for (; len > 0; len--)
	{
		// get next character of the text
		ch = *text++;

		// draw character
		DrawCan1CharInv(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw1TextInv(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan1TextInv(pDrawCan1, text, len, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                               Draw ellipse
// ----------------------------------------------------------------------------
// Using mid-point ellipse drawing algorithm

#define DRAWCANELLIPSE_PIXFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xfrac = 7 - (x2 & 7);		\
		s = &s0[(x2>>3) + (y2-basey)*wb];	\
		*s = (*s & ~(0x01<<xfrac)) | (col<<xfrac); }

#define DRAWCANELLIPSE_PIX(xxx,yyy,mmm)			\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xfrac = 7 - (x2 & 7);		\
		s = &s0[(x2>>3) + (y2-basey)*wb];	\
		*s = (*s & ~(0x01<<xfrac)) | (col<<xfrac); } }

#define DRAWCANELLIPSE_PIXINVFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xfrac = 7 - (x2 & 7);		\
		s = &s0[(x2>>3) + (y2-basey)*wb];	\
		*s ^= 0x01<<xfrac; }

#define DRAWCANELLIPSE_PIXINV(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xfrac = 7 - (x2 & 7);		\
		s = &s0[(x2>>3) + (y2-basey)*wb];	\
		*s ^= 0x01<<xfrac; } }

//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse
//		DRAWCAN_ELLIPSE_ARC0	= draw arc 0..90 deg
//		DRAWCAN_ELLIPSE_ARC1	= draw arc 90..180 deg
//		DRAWCAN_ELLIPSE_ARC2	= draw arc 180..270 deg
//		DRAWCAN_ELLIPSE_ARC3	= draw arc 270..360 deg
//		DRAWCAN_ELLIPSE_ALL	= draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan1Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// limit range
	if (dx < 1) dx = 1;
	if (dx > DRAWCAN_ELLIPSE_MAXD) dx = DRAWCAN_ELLIPSE_MAXD;
	if (dy < 1) dy = 1;
	if (dy > DRAWCAN_ELLIPSE_MAXD) dy = DRAWCAN_ELLIPSE_MAXD;

	// prepare
	int rx = dx/2;
	int ry = dy/2;
	int xx = 0;
	int yy = ry;

	// pointer to middle of the ellipse
	int wb = can->wb;
	int basey = can->basey;
	u8* s0 = can->buf;
	u8* s;

	// odd diameters X and Y
	int cx = -1; // even correction X (on even diameter X do not draw middle point X)
	int cy = -1; // even correction Y (on even diameter Y do not draw middle point Y)
	if ((dy & 1) != 0) cy = 0;
	if ((dx & 1) != 0)
	{
		DRAWCANELLIPSE_PIXFAST(0, -ry   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIXFAST(0, +ry+cy, DRAWCAN_ELLIPSE_ARC2);
		cx = 0;
	}

	// odd diameter Y
	if ((dy & 1) != 0)
	{
		DRAWCANELLIPSE_PIXFAST(-rx   , 0, DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIXFAST(+rx+cx, 0, DRAWCAN_ELLIPSE_ARC3);
	}

	// draw top and bottom arcs
	int p = ry*ry - rx*rx*ry + rx*rx/4;
	int ddx = 0;
	int ddy = 2*rx*rx*ry;
	while (True)
	{
		xx++;
		ddx += 2*ry*ry;
		if (p < 0)
			p += ddx + ry*ry;
		else
		{
			yy--;
			ddy -= 2*rx*rx;
			p += ddx - ddy + ry*ry;
		}

		if (ddx > ddy) break;

		DRAWCANELLIPSE_PIXFAST(+xx+cx, -yy   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIXFAST(-xx   , -yy   , DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIXFAST(-xx   , +yy+cy, DRAWCAN_ELLIPSE_ARC2);
		DRAWCANELLIPSE_PIXFAST(+xx+cx, +yy+cy, DRAWCAN_ELLIPSE_ARC3);
	}

	// draw left and right arcs
	xx = rx;
	yy = 0;
	p = rx*rx - ry*ry*rx + ry*ry/4;
	ddy = 0;
	ddx = 2*ry*ry*rx;
	while (True)
	{
		yy++;
		ddy += 2*rx*rx;
		if (p < 0)
			p += ddy + rx*rx;
		else
		{
			xx--;
			ddx -= 2*ry*ry;
			p += ddy - ddx + rx*rx;
		}

		if (ddy > ddx) break;

		DRAWCANELLIPSE_PIXFAST(+xx+cx, -yy   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIXFAST(-xx   , -yy   , DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIXFAST(-xx   , +yy+cy, DRAWCAN_ELLIPSE_ARC2);
		DRAWCANELLIPSE_PIXFAST(+xx+cx, +yy+cy, DRAWCAN_ELLIPSE_ARC3);
	}
}
void Draw1Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan1Ellipse_Fast(pDrawCan1, x, y, dx, dy, col, mask); }

// Draw ellipse (checks clipping limits)
void DrawCan1Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// limit range
	if (dx < 1) dx = 1;
	if (dx > DRAWCAN_ELLIPSE_MAXD) dx = DRAWCAN_ELLIPSE_MAXD;
	if (dy < 1) dy = 1;
	if (dy > DRAWCAN_ELLIPSE_MAXD) dy = DRAWCAN_ELLIPSE_MAXD;

	// update dirty area (we ignore undrawn arcs)
	int rx = dx/2;
	int ry = dy/2;
	DrawCanDirtyRect(can, x - rx, y - ry, dx, dy);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - rx, y - ry, dx, dy))
	{
		DrawCan1Ellipse_Fast(can, x, y, dx, dy, col, mask);
		return;
	}

	// prepare
	int xx = 0;
	int yy = ry;
	int clipx1 = can->clipx1;
	int clipx2 = can->clipx2;
	int clipy1 = can->clipy1;
	int clipy2 = can->clipy2;

	// pointer to middle of the ellipse
	int wb = can->wb;
	int basey = can->basey;
	u8* s0 = can->buf;
	u8* s;

	// odd diameters X and Y
	int cx = -1; // even correction X (on even diameter X do not draw middle point X)
	int cy = -1; // even correction Y (on even diameter Y do not draw middle point Y)
	if ((dy & 1) != 0) cy = 0;
	if ((dx & 1) != 0)
	{
		DRAWCANELLIPSE_PIX(0, -ry   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIX(0, +ry+cy, DRAWCAN_ELLIPSE_ARC2);
		cx = 0;
	}

	// odd diameter Y
	if ((dy & 1) != 0)
	{
		DRAWCANELLIPSE_PIX(-rx   , 0, DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIX(+rx+cx, 0, DRAWCAN_ELLIPSE_ARC3);
	}

	// draw top and bottom arcs
	int p = ry*ry - rx*rx*ry + rx*rx/4;
	int ddx = 0;
	int ddy = 2*rx*rx*ry;
	while (True)
	{
		xx++;
		ddx += 2*ry*ry;
		if (p < 0)
			p += ddx + ry*ry;
		else
		{
			yy--;
			ddy -= 2*rx*rx;
			p += ddx - ddy + ry*ry;
		}

		if (ddx > ddy) break;

		DRAWCANELLIPSE_PIX(+xx+cx, -yy   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIX(-xx   , -yy   , DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIX(-xx   , +yy+cy, DRAWCAN_ELLIPSE_ARC2);
		DRAWCANELLIPSE_PIX(+xx+cx, +yy+cy, DRAWCAN_ELLIPSE_ARC3);
	}

	// draw left and right arcs
	xx = rx;
	yy = 0;
	p = rx*rx - ry*ry*rx + ry*ry/4;
	ddy = 0;
	ddx = 2*ry*ry*rx;
	while (True)
	{
		yy++;
		ddy += 2*rx*rx;
		if (p < 0)
			p += ddy + rx*rx;
		else
		{
			xx--;
			ddx -= 2*ry*ry;
			p += ddy - ddx + rx*rx;
		}

		if (ddy > ddx) break;

		DRAWCANELLIPSE_PIX(+xx+cx, -yy   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIX(-xx   , -yy   , DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIX(-xx   , +yy+cy, DRAWCAN_ELLIPSE_ARC2);
		DRAWCANELLIPSE_PIX(+xx+cx, +yy+cy, DRAWCAN_ELLIPSE_ARC3);
	}
}
void Draw1Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan1Ellipse(pDrawCan1, x, y, dx, dy, col, mask); }

// Draw ellipse inverted (does not check clipping limits)
void DrawCan1EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
{
	// limit range
	if (dx < 1) dx = 1;
	if (dx > DRAWCAN_ELLIPSE_MAXD) dx = DRAWCAN_ELLIPSE_MAXD;
	if (dy < 1) dy = 1;
	if (dy > DRAWCAN_ELLIPSE_MAXD) dy = DRAWCAN_ELLIPSE_MAXD;

	// prepare
	int rx = dx/2;
	int ry = dy/2;
	int xx = 0;
	int yy = ry;

	// pointer to middle of the ellipse
	int wb = can->wb;
	int basey = can->basey;
	u8* s0 = can->buf;
	u8* s;

	// odd diameters X and Y
	int cx = -1; // even correction X (on even diameter X do not draw middle point X)
	int cy = -1; // even correction Y (on even diameter Y do not draw middle point Y)
	if ((dy & 1) != 0) cy = 0;
	if ((dx & 1) != 0)
	{
		DRAWCANELLIPSE_PIXINVFAST(0, -ry   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIXINVFAST(0, +ry+cy, DRAWCAN_ELLIPSE_ARC2);
		cx = 0;
	}

	// odd diameter Y
	if ((dy & 1) != 0)
	{
		DRAWCANELLIPSE_PIXINVFAST(-rx   , 0, DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIXINVFAST(+rx+cx, 0, DRAWCAN_ELLIPSE_ARC3);
	}

	// draw top and bottom arcs
	int p = ry*ry - rx*rx*ry + rx*rx/4;
	int ddx = 0;
	int ddy = 2*rx*rx*ry;
	while (True)
	{
		xx++;
		ddx += 2*ry*ry;
		if (p < 0)
			p += ddx + ry*ry;
		else
		{
			yy--;
			ddy -= 2*rx*rx;
			p += ddx - ddy + ry*ry;
		}

		if (ddx > ddy) break;

		DRAWCANELLIPSE_PIXINVFAST(+xx+cx, -yy   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIXINVFAST(-xx   , -yy   , DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIXINVFAST(-xx   , +yy+cy, DRAWCAN_ELLIPSE_ARC2);
		DRAWCANELLIPSE_PIXINVFAST(+xx+cx, +yy+cy, DRAWCAN_ELLIPSE_ARC3);
	}

	// draw left and right arcs
	xx = rx;
	yy = 0;
	p = rx*rx - ry*ry*rx + ry*ry/4;
	ddy = 0;
	ddx = 2*ry*ry*rx;
	while (True)
	{
		yy++;
		ddy += 2*rx*rx;
		if (p < 0)
			p += ddy + rx*rx;
		else
		{
			xx--;
			ddx -= 2*ry*ry;
			p += ddy - ddx + rx*rx;
		}

		if (ddy > ddx) break;

		DRAWCANELLIPSE_PIXINVFAST(+xx+cx, -yy   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIXINVFAST(-xx   , -yy   , DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIXINVFAST(-xx   , +yy+cy, DRAWCAN_ELLIPSE_ARC2);
		DRAWCANELLIPSE_PIXINVFAST(+xx+cx, +yy+cy, DRAWCAN_ELLIPSE_ARC3);
	}
}
void Draw1EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan1EllipseInv_Fast(pDrawCan1, x, y, dx, dy, mask); }

// Draw ellipse inverted (checks clipping limits)
void DrawCan1EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
{
	// limit range
	if (dx < 1) dx = 1;
	if (dx > DRAWCAN_ELLIPSE_MAXD) dx = DRAWCAN_ELLIPSE_MAXD;
	if (dy < 1) dy = 1;
	if (dy > DRAWCAN_ELLIPSE_MAXD) dy = DRAWCAN_ELLIPSE_MAXD;

	// update dirty area (we ignore undrawn arcs)
	int rx = dx/2;
	int ry = dy/2;
	DrawCanDirtyRect(can, x - rx, y - ry, dx, dy);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - rx, y - ry, dx, dy))
	{
		DrawCan1EllipseInv_Fast(can, x, y, dx, dy, mask);
		return;
	}

	// prepare
	int xx = 0;
	int yy = ry;
	int clipx1 = can->clipx1;
	int clipx2 = can->clipx2;
	int clipy1 = can->clipy1;
	int clipy2 = can->clipy2;

	// pointer to middle of the ellipse
	int wb = can->wb;
	int basey = can->basey;
	u8* s0 = can->buf;
	u8* s;

	// odd diameters X and Y
	int cx = -1; // even correction X (on even diameter X do not draw middle point X)
	int cy = -1; // even correction Y (on even diameter Y do not draw middle point Y)
	if ((dy & 1) != 0) cy = 0;
	if ((dx & 1) != 0)
	{
		DRAWCANELLIPSE_PIXINV(0, -ry   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIXINV(0, +ry+cy, DRAWCAN_ELLIPSE_ARC2);
		cx = 0;
	}

	// odd diameter Y
	if ((dy & 1) != 0)
	{
		DRAWCANELLIPSE_PIXINV(-rx   , 0, DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIXINV(+rx+cx, 0, DRAWCAN_ELLIPSE_ARC3);
	}

	// draw top and bottom arcs
	int p = ry*ry - rx*rx*ry + rx*rx/4;
	int ddx = 0;
	int ddy = 2*rx*rx*ry;
	while (True)
	{
		xx++;
		ddx += 2*ry*ry;
		if (p < 0)
			p += ddx + ry*ry;
		else
		{
			yy--;
			ddy -= 2*rx*rx;
			p += ddx - ddy + ry*ry;
		}

		if (ddx > ddy) break;

		DRAWCANELLIPSE_PIXINV(+xx+cx, -yy   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIXINV(-xx   , -yy   , DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIXINV(-xx   , +yy+cy, DRAWCAN_ELLIPSE_ARC2);
		DRAWCANELLIPSE_PIXINV(+xx+cx, +yy+cy, DRAWCAN_ELLIPSE_ARC3);
	}

	// draw left and right arcs
	xx = rx;
	yy = 0;
	p = rx*rx - ry*ry*rx + ry*ry/4;
	ddy = 0;
	ddx = 2*ry*ry*rx;
	while (True)
	{
		yy++;
		ddy += 2*rx*rx;
		if (p < 0)
			p += ddy + rx*rx;
		else
		{
			xx--;
			ddx -= 2*ry*ry;
			p += ddy - ddx + rx*rx;
		}

		if (ddy > ddx) break;

		DRAWCANELLIPSE_PIXINV(+xx+cx, -yy   , DRAWCAN_ELLIPSE_ARC0);
		DRAWCANELLIPSE_PIXINV(-xx   , -yy   , DRAWCAN_ELLIPSE_ARC1);
		DRAWCANELLIPSE_PIXINV(-xx   , +yy+cy, DRAWCAN_ELLIPSE_ARC2);
		DRAWCANELLIPSE_PIXINV(+xx+cx, +yy+cy, DRAWCAN_ELLIPSE_ARC3);
	}
}
void Draw1EllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan1EllipseInv(pDrawCan1, x, y, dx, dy, mask); }

#undef DRAWCANELLIPSE_PIXFAST
#undef DRAWCANELLIPSE_PIX
#undef DRAWCANELLIPSE_PIXINVFAST
#undef DRAWCANELLIPSE_PIXINV

// ----------------------------------------------------------------------------
//                          Draw filled ellipse
// ----------------------------------------------------------------------------

//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse
//		DRAWCAN_ROUND_NOTOP	= hide top part of the round
//		DRAWCAN_ROUND_NOBOTTOM	= hide bottom part of the round
//		DRAWCAN_ROUND_NOLEFT	= hide left part of the round
//		DRAWCAN_ROUND_NORIGHT	= hide right part of the round
//		DRAWCAN_ROUND_ALL	= draw whole round

// Draw filled ellipse (does not check clipping limits)
void DrawCan1FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// limit range
	if (dx < 1) dx = 1;
	if (dx > DRAWCAN_ELLIPSE_MAXD) dx = DRAWCAN_ELLIPSE_MAXD;
	if (dy < 1) dy = 1;
	if (dy > DRAWCAN_ELLIPSE_MAXD) dy = DRAWCAN_ELLIPSE_MAXD;

	// prepare
	int rx = (dx+1)/2;
	int ry = (dy+1)/2;
	int x1 = -rx;
	int x2 = +rx;
	int y1 = -ry;
	int y2 = +ry;
	int wb = can->wb;
	u8 *s0, *s;
	int r2, xx, yy;
	int rx2 = rx*rx;
	int ry2 = ry*ry;

	// hide parts of the ellipse
	DRAWCANROUND_HIDE();

	// even diameter correction
	int cx = 1 - (dx & 1);
	int cy = 1 - (dy & 1);
	x -= cx;
	y -= cy;
	x1 += cx;
	y1 += cy;	

	// prepare buffer address
	int xxx = x+x1;
	int x0 = 7 - (xxx & 7);
	s0 = &can->buf[xxx/8 + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	int xx0 = x+x1;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		xxx = x0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = (*s & ~(0x01<<xxx)) | (col<<xxx);
			xxx--;
			if (xxx < 0)
			{
				s++;
				xxx = 7;
			}
		}
		s0 += wb;
	}
}
void Draw1FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan1FillEllipse_Fast(pDrawCan1, x, y, dx, dy, col, mask); }

// Draw filled ellipse (checks clipping limits)
void DrawCan1FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// limit range
	if (dx < 1) dx = 1;
	if (dx > DRAWCAN_ELLIPSE_MAXD) dx = DRAWCAN_ELLIPSE_MAXD;
	if (dy < 1) dy = 1;
	if (dy > DRAWCAN_ELLIPSE_MAXD) dy = DRAWCAN_ELLIPSE_MAXD;

	// prepare
	int rx = (dx+1)/2;
	int ry = (dy+1)/2;
	int x1 = -rx;
	int x2 = +rx;
	int y1 = -ry;
	int y2 = +ry;
	int wb = can->wb;
	u8 *s0, *s;
	int r2, xx, yy;
	int rx2 = rx*rx;
	int ry2 = ry*ry;

	// hide parts of the ellipse
	DRAWCANROUND_HIDE();

	// even diameter correction
	int cx = 1 - (dx & 1);
	int cy = 1 - (dy & 1);
	x -= cx;
	y -= cy;
	x1 += cx;
	y1 += cy;	

	// clipping
	DRAWCANROUND_CLIP();

	// update dirty area
	DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

	// prepare buffer address
	int xxx = x+x1;
	int x0 = 7 - (xxx & 7);
	s0 = &can->buf[xxx/8 + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	int xx0 = x+x1;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		xxx = x0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = (*s & ~(0x01<<xxx)) | (col<<xxx);
			xxx--;
			if (xxx < 0)
			{
				s++;
				xxx = 7;
			}
		}
		s0 += wb;
	}
}
void Draw1FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan1FillEllipse(pDrawCan1, x, y, dx, dy, col, mask); }

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan1FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
{
	// limit range
	if (dx < 1) dx = 1;
	if (dx > DRAWCAN_ELLIPSE_MAXD) dx = DRAWCAN_ELLIPSE_MAXD;
	if (dy < 1) dy = 1;
	if (dy > DRAWCAN_ELLIPSE_MAXD) dy = DRAWCAN_ELLIPSE_MAXD;

	// prepare
	int rx = (dx+1)/2;
	int ry = (dy+1)/2;
	int x1 = -rx;
	int x2 = +rx;
	int y1 = -ry;
	int y2 = +ry;
	int wb = can->wb;
	u8 *s0, *s;
	int r2, xx, yy;
	int rx2 = rx*rx;
	int ry2 = ry*ry;

	// hide parts of the ellipse
	DRAWCANROUND_HIDE();

	// even diameter correction
	int cx = 1 - (dx & 1);
	int cy = 1 - (dy & 1);
	x -= cx;
	y -= cy;
	x1 += cx;
	y1 += cy;	

	// prepare buffer address
	int xxx = x+x1;
	int x0 = 7 - (xxx & 7);
	s0 = &can->buf[xxx/8 + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	int xx0 = x+x1;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		xxx = x0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s ^= 0x01<<xxx;
			xxx--;
			if (xxx < 0)
			{
				s++;
				xxx = 7;
			}
		}
		s0 += wb;
	}
}
void Draw1FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan1FillEllipseInv_Fast(pDrawCan1, x, y, dx, dy, mask); }

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan1FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
{
	// limit range
	if (dx < 1) dx = 1;
	if (dx > DRAWCAN_ELLIPSE_MAXD) dx = DRAWCAN_ELLIPSE_MAXD;
	if (dy < 1) dy = 1;
	if (dy > DRAWCAN_ELLIPSE_MAXD) dy = DRAWCAN_ELLIPSE_MAXD;

	// prepare
	int rx = (dx+1)/2;
	int ry = (dy+1)/2;
	int x1 = -rx;
	int x2 = +rx;
	int y1 = -ry;
	int y2 = +ry;
	int wb = can->wb;
	u8 *s0, *s;
	int r2, xx, yy;
	int rx2 = rx*rx;
	int ry2 = ry*ry;

	// hide parts of the ellipse
	DRAWCANROUND_HIDE();

	// even diameter correction
	int cx = 1 - (dx & 1);
	int cy = 1 - (dy & 1);
	x -= cx;
	y -= cy;
	x1 += cx;
	y1 += cy;	

	// clipping
	DRAWCANROUND_CLIP();

	// update dirty area
	DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

	// prepare buffer address
	int xxx = x+x1;
	int x0 = 7 - (xxx & 7);
	s0 = &can->buf[xxx/8 + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		xxx = x0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s ^= 0x01<<xxx;
			xxx--;
			if (xxx < 0)
			{
				s++;
				xxx = 7;
			}
		}
		s0 += wb;
	}
}
void Draw1FillEllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan1FillEllipseInv(pDrawCan1, x, y, dx, dy, mask); }

#undef DRAWCANROUND_HIDE
#undef DRAWCANROUND_CLIP

// ----------------------------------------------------------------------------
//                            Draw image
// ----------------------------------------------------------------------------

// limit image coordinates
#define DRAWCAN_IMGLIMIT() {				\
	/* limit coordinate X */			\
	if (xs < 0) { w += xs; xd -= xs; xs = 0; }	\
	int k = can->clipx1 - xd;			\
	if (k > 0) { w -= k; xs += k; xd += k; }	\
	/* limit width W */				\
	k = can->clipx2 - xd; if (w > k) w = k;		\
	k = ws - xs; if (w > k) w = k;			\
	if (w <= 0) return;				\
	/* limit coordinate Y */			\
	if (ys < 0) { h += ys; yd -= ys; ys = 0; }	\
	k = can->clipy1 - yd;				\
	if (k > 0) { h -= k; ys += k; yd += k; }	\
	/* limit height H */				\
	k = can->clipy2 - yd; if (h > k) h = k;		\
	if (h <= 0) return; }

//  can ... destination canvas
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  src ... source image
//  xs ... source X coordinate
//  ys ... source Y coordinate
//  w ... width to draw
//  h ... height to draw
//  wbs ... pitch of source image (length of line in bytes)
//  col ... key transparent color

// Draw image with the same format as destination
void DrawCan1Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*8;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/8;
	u8* d0 = &can->buf[xdint + (yd - can->basey)*wbd];
	int xdfrac = 7 - (xd & 7);

	// source address
	int xsint = xs/8;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = 7 - (xs & 7);

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x01);
			xxs--;
			if (xxs < 0)
			{
				xxs = 7;
				s++;
			}

			*d = (*d & ~(0x01<<xxd)) | (c<<xxd);
			xxd--;
			if (xxd < 0)
			{
				xxd = 7;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw1Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan1Img(pDrawCan1, xd, yd, src, xs, ys, w, h, wbs); }

// Draw image inverted with the same format as destination
void DrawCan1ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*8;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/8;
	u8* d0 = &can->buf[xdint + (yd - can->basey)*wbd];
	int xdfrac = 7 - (xd & 7);

	// source address
	int xsint = xs/8;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = 7 - (xs & 7);

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x01);
			xxs--;
			if (xxs < 0)
			{
				xxs = 7;
				s++;
			}

			*d ^= (u8)c<<xxd;
			xxd--;
			if (xxd < 0)
			{
				xxd = 7;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw1ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan1ImgInv(pDrawCan1, xd, yd, src, xs, ys, w, h, wbs); }

// Draw transparent image with the same format as destination
void DrawCan1Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs*8;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/8;
	u8* d0 = &can->buf[xdint + (yd - can->basey)*wbd];
	int xdfrac = 7 - (xd & 7);

	// source address
	int xsint = xs/8;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = 7 - (xs & 7);

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x01);
			xxs--;
			if (xxs < 0)
			{
				xxs = 7;
				s++;
			}

			if (c != col) *d = (*d & ~(0x01<<xxd)) | (c<<xxd);
			xxd--;
			if (xxd < 0)
			{
				xxd = 7;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw1Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan1Blit(pDrawCan1, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image with the same format as destination, with substitute color
// - This function is here only for compatibility, and is replaced by the DrawCan1Blit/Draw1Blit function
void DrawCan1BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { DrawCan1Blit(can, xd, yd, src, xs, ys, w, h, wbs, col); }
void Draw1BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { DrawCan1Blit(pDrawCan1, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image inverted with the same format as destination
void DrawCan1BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs*8;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/8;
	u8* d0 = &can->buf[xdint + (yd - can->basey)*wbd];
	int xdfrac = 7 - (xd & 7);

	// source address
	int xsint = xs/8;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = 7 - (xs & 7);

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x01);
			xxs--;
			if (xxs < 0)
			{
				xxs = 7;
				s++;
			}

			if (c != col) *d ^= (u8)c<<xxd;
			xxd--;
			if (xxd < 0)
			{
				xxd = 7;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw1BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan1BlitInv(pDrawCan1, xd, yd, src, xs, ys, w, h, wbs, col); }

// Get image from canvas to buffer
//  can ... source canvas
//  xs ... source X coordinate
//  ys ... source Y coordinate
//  w ... width
//  h ... height
//  dst ... destination buffer
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  wbd ... pitch of destination buffer (length of line in bytes)
void DrawCan1GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd)
{
	// width of destination image
	int wd = wbd*8;

	// limit coordinate X
	if (xd < 0)
	{
		w += xd;
		xs -= xd;
		xd = 0;
	}

	int k = can->clipx1 - xs;
	if (k > 0)
	{
		w -= k;
		xd += k;
		xs += k;
	}

	// limit w
	k = can->clipx2 - xs;
	if (w > k) w = k;
	k = wd - xd;
	if (w > k) w = k;
	if (w <= 0) return;

	// limit coordinate Y
	if (yd < 0)
	{
		h += yd;
		ys -= yd;
		yd = 0;
	}

	k = can->clipy1 - ys;
	if (k > 0)
	{
		h -= k;
		yd += k;
		ys += k;
	}

	// limit h
	k = can->clipy2 - ys;
	if (h > k) h = k;
	if (h <= 0) return;

	// source address
	int wbs = can->wb;
	int xsint = xs/8;
	const u8* s0 = (const u8*)&can->buf[xsint + (ys - can->basey)*wbs];
	int xsfrac = 7 - (xs & 7);

	// destination address
	int xdint = xd/8;
	u8* d0 = &((u8*)dst)[xdint + yd*wbd];
	int xdfrac = 7 - (xd & 7);

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x01);
			xxs--;
			if (xxs < 0)
			{
				xxs = 7;
				s++;
			}

			*d = (*d & ~(0x01<<xxd)) | (c<<xxd);
			xxd--;
			if (xxd < 0)
			{
				xxd = 7;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw1GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { DrawCan1GetImg(pDrawCan1, xs, ys, w, h, dst, xd, yd, wbd); }

// Draw image with 2D transformation matrix
//  can ... destination canvas
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  wd ... destination width
//  hd ... destination height
//  src ... source image
//  ws ... source image width
//  hs ... source image height
//  wbs ... pitch of source image (length of line in bytes)
//  m ... transformation matrix (should be prepared using PrepDrawImg function)
//  mode ... draw mode DRAWIMG_*
//  color ... key or border color (DRAWIMG_PERSP mode: horizon offset)
// Note to wrap and perspective mode: Width and height of source image should be power of 2, or it will render slower.
void DrawCan1ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src_img, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
{
	const u8* src = (const u8*)src_img;

	// limit xd
	int x0 = -wd/2; // start X coordinate
	int k = can->clipx1 - xd;
	if (k > 0)
	{
		wd -= k;
		x0 += k;
		xd += k;
	}
	k = can->clipx2;
	if (xd + wd > k) wd = k - xd;
	if (wd <= 0) return;

	// limit yd
	k = can->clipy1 - yd;
	int h0 = hd;
	int y0 = (mode == DRAWIMG_PERSP) ? (-hd) : (-hd/2); // start Y coordinate
	if (k > 0)
	{
		hd -= k;
		y0 += k;
		yd += k;
	}
	k = can->clipy2;
	if (yd + hd > k) hd = k - yd;
	if (hd <= 0) return;

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, wd, hd);

	// load transformation matrix and convert to integer fractional number
	int m11 = TOFRACT(m->m11);
	int m12 = TOFRACT(m->m12);
	int m13 = TOFRACT(m->m13);
	int m21 = TOFRACT(m->m21);
	int m22 = TOFRACT(m->m22);
	int m23 = TOFRACT(m->m23);

	// check invalid zero matrix
	Bool zero = (m11 | m12 | m13 | m21 | m22 | m23) == 0;

	// prepare variables
	int xy0m, yy0m; // temporary Y members
	int wbd = can->wb;
	u8* d0 = &((u8*)can->buf)[wbd*(yd - can->basey)]; // destination image
	int i, x2, y2, xx;
	const u8* s;
	u8* d;
	u8 pix;
	int xd0 = xd;

	// 0: wrap image
	if (mode == DRAWIMG_WRAP)
	{
		// image dimension is power of 2
		if (IsPow2(ws) && IsPow2(hs))
		{
			// coordinate mask
			int xmask = ws - 1;
			int ymask = hs - 1;

			for (; hd > 0; hd--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) & xmask;
					y2 = (yy0m>>FRACT) & ymask;

					s = &src[(x2>>3) + y2*wbs];
					xx = 7 - (x2 & 7);
					pix = (*s >> xx) & 1;
					d = &d0[xd>>3];
					xx = 7 - (xd & 7);
					*d = (*d & ~(0x01<<xx)) | (pix<<xx);
					xd++;

					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}

				y0++;
				d0 += wbd;
			}
		}

		// image dimension is not power of 2
		else
		{
			for (; hd > 0; hd--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) % ws;
					if (x2 < 0) x2 += ws;
					y2 = (yy0m>>FRACT) % hs;
					if (y2 < 0) y2 += hs;

					s = &src[(x2>>3) + y2*wbs];
					xx = 7 - (x2 & 7);
					pix = (*s >> xx) & 1;
					d = &d0[xd>>3];
					xx = 7 - (xd & 7);
					*d = (*d & ~(0x01<<xx)) | (pix<<xx);
					xd++;

					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}

				y0++;
				d0 += wbd;
			}
		}
	}

	// 1: no border
	else if (mode == DRAWIMG_NOBORDER)
	{
		// if matrix is valid
		if (!zero)
		{
			// source image dimension
			u32 ww = ws;
			u32 hh = hs;

			for (; hd > 0; hd--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;

				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (((u32)x2 < ww) && ((u32)y2 < hh))
					{
						s = &src[(x2>>3) + y2*wbs];
						xx = 7 - (x2 & 7);
						pix = (*s >> xx) & 1;
						d = &d0[xd>>3];
						xx = 7 - (xd & 7);
						*d = (*d & ~(0x01<<xx)) | (pix<<xx);
					}
					xd++;
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d0 += wbd;
			}
		}
	}

	// 2: clamp image
	else if (mode == DRAWIMG_CLAMP)
	{
		// invalid matrix
		if (zero)
		{
			u16 col = *src;
			for (; hd > 0; hd--)
			{
				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					d = &d0[xd>>3];
					xx = 7 - (xd & 7);
					*d = (*d & ~(0x01<<xx)) | (col<<xx);
					xd++;
				}
				d0 += wbd;
			}
		}
		else
		{
			// source image dimension
			u32 ww = ws - 1;
			u32 hh = hs - 1;

			for (; hd > 0; hd--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (x2 < 0) x2 = 0;
					if (x2 > ww) x2 = ww;
					if (y2 < 0) y2 = 0;
					if (y2 > hh) y2 = hh;

					s = &src[(x2>>3) + y2*wbs];
					xx = 7 - (x2 & 7);
					pix = (*s >> xx) & 1;
					d = &d0[xd>>3];
					xx = 7 - (xd & 7);
					*d = (*d & ~(0x01<<xx)) | (pix<<xx);
					xd++;

					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d0 += wbd;
			}
		}
	}

	// 3: color border
	else if (mode == DRAWIMG_COLOR)
	{
		// invalid matrix
		if (zero)
		{
			for (; hd > 0; hd--)
			{
				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					d = &d0[xd>>3];
					xx = 7 - (xd & 7);
					*d = (*d & ~(0x01<<xx)) | (color<<xx);
					xd++;
				}
				d0 += wbd;
			}
		}
		else
		{
			// source image dimension
			u32 ww = ws;
			u32 hh = hs;

			for (; hd > 0; hd--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (((u32)x2 < ww) && ((u32)y2 < hh))
					{
						s = &src[(x2>>3) + y2*wbs];
						xx = 7 - (x2 & 7);
						pix = (*s >> xx) & 1;
					}
					else
						pix = color;
					d = &d0[xd>>3];
					xx = 7 - (xd & 7);
					*d = (*d & ~(0x01<<xx)) | (pix<<xx);
					xd++;

					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}

				y0++;
				d0 += wbd;
			}
		}
	}

	// 4: transparency
	else if (mode == DRAWIMG_TRANSP)
	{
		// if matrix is valid
		if (!zero)
		{
			u32 ww = ws;
			u32 hh = hs;
			u16 c;

			for (; hd > 0; hd--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (((u32)x2 < ww) && ((u32)y2 < hh))
					{
						s = &src[(x2>>3) + y2*wbs];
						xx = 7 - (x2 & 7);
						pix = (*s >> xx) & 1;
						if (pix != color)
						{
							d = &d0[xd>>3];
							xx = 7 - (xd & 7);
							*d = (*d & ~(0x01<<xx)) | (pix<<xx);
						}
					}
					xd++;

					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d0 += wbd;
			}
		}
	}

	// 5: perspective
	else if (mode == DRAWIMG_PERSP)
	{
		// image dimension is power of 2
		if (IsPow2(ws) && IsPow2(hs))
		{
			// coordinate mask
			int xmask = ws - 1;
			int ymask = hs - 1;

			for (; hd > 0; hd--)
			{
				int dist = FRACTMUL*h0/(h0 + y0 + color + 1);
				int m11b = (m11*dist)>>FRACT;
				int m21b = (m21*dist)>>FRACT;
				int m12b = (m12*dist)>>FRACT;
				int m22b = (m22*dist)>>FRACT;

				xy0m = x0*m11b + y0*m12b + m13;
				yy0m = x0*m21b + y0*m22b + m23;
		
				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) & xmask;
					xy0m += m11b; // x0*m11

					y2 = (yy0m>>FRACT) & ymask;
					yy0m += m21b; // x0*m21

					s = &src[(x2>>3) + y2*wbs];
					xx = 7 - (x2 & 7);
					pix = (*s >> xx) & 1;
					d = &d0[xd>>3];
					xx = 7 - (xd & 7);
					*d = (*d & ~(0x01<<xx)) | (pix<<xx);
					xd++;
				}
				y0++;
				d0 += wbd;
			}
		}

		// image dimension is not power of 2
		else
		{
			for (; hd > 0; hd--)
			{
				int dist = FRACTMUL*h0/(h0 + y0 + color + 1);
				int m11b = (m11*dist)>>FRACT;
				int m21b = (m21*dist)>>FRACT;
				int m12b = (m12*dist)>>FRACT;
				int m22b = (m22*dist)>>FRACT;

				xy0m = x0*m11b + y0*m12b + m13;
				yy0m = x0*m21b + y0*m22b + m23;
		
				xd = xd0;
				for (i = wd; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) % ws;
					if (x2 < 0) x2 += ws;
					xy0m += m11b; // x0*m11

					y2 = (yy0m>>FRACT) % hs;
					if (y2 < 0) y2 += hs;
					yy0m += m21b; // x0*m21

					s = &src[(x2>>3) + y2*wbs];
					xx = 7 - (x2 & 7);
					pix = (*s >> xx) & 1;
					d = &d0[xd>>3];
					xx = 7 - (xd & 7);
					*d = (*d & ~(0x01<<xx)) | (pix<<xx);
					xd++;
				}
				y0++;
				d0 += wbd;
			}
		}
	}
}
void Draw1ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan1ImgMat(pDrawCan1, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

#undef DRAWCAN_IMGLIMIT

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// drawing function interface
const sDrawCanFnc DrawCan1Fnc = {
	.pDrawPitch		= Draw1Pitch,			// calculate pitch of graphics line from image width
	.pDrawMaxWidth		= Draw1MaxWidth,		// calculate max. width of image from the pitch
	// Clear canvas
	.pDrawCanClearCol	= DrawCan1ClearCol,		// Clear canvas with color
	.pDrawCanClear		= DrawCan1Clear,		// Clear canvas with black color
	// Draw point
	.pDrawCanPoint_Fast	= DrawCan1Point_Fast,		// Draw point (does not check clipping limits)
	.pDrawCanPoint		= DrawCan1Point,		// Draw point (checks clipping limits)
	.pDrawCanPointInv_Fast	= DrawCan1PointInv_Fast,	// Draw point inverted (does not check clipping limits)
	.pDrawCanPointInv	= DrawCan1PointInv,		// Draw point inverted (checks clipping limits)
	// Get point
	.pDrawCanGetPoint_Fast	= DrawCan1GetPoint_Fast,	// Get point (does not check clipping limits)
	.pDrawCanGetPoint	= DrawCan1GetPoint,		// Get point (checks clipping limits)
	// Draw rectangle
	.pDrawCanRect_Fast	= DrawCan1Rect_Fast,		// Draw rectangle (does not check clipping limits)
	.pDrawCanRect		= DrawCan1Rect,			// Draw rectangle (checks clipping limits)
	.pDrawCanRectInv_Fast	= DrawCan1RectInv_Fast,		// Draw rectangle inverted (does not check clipping limits)
	.pDrawCanRectInv	= DrawCan1RectInv,		// Draw rectangle inverted (checks clipping limits
	// Draw horizontal line
	.pDrawCanHLine_Fast	= DrawCan1HLine_Fast,		// Draw horizontal line (does not check clipping limits)
	.pDrawCanHLine		= DrawCan1HLine,		// Draw horizontal line (checks clipping limits; negative width flip line)
	.pDrawCanHLineInv_Fast	= DrawCan1HLineInv_Fast,	// Draw horizontal line inverted (does not check clipping limits)
	.pDrawCanHLineInv	= DrawCan1HLineInv,		// Draw horizontal line inverted (checks clipping limits; negative width flip line)
	// Draw vertical line
	.pDrawCanVLine_Fast	= DrawCan1VLine_Fast,		// Draw vertical line (does not check clipping limits)
	.pDrawCanVLine		= DrawCan1VLine,		// Draw vertical line (checks clipping limits; negative height flip line)
	.pDrawCanVLineInv_Fast	= DrawCan1VLineInv_Fast,	// Draw vertical line inverted (does not check clipping limits)
	.pDrawCanVLineInv	= DrawCan1VLineInv,		// Draw vertical line inverted (checks clipping limits; negative height flip line)
	// Draw frame
	.pDrawCanFrame_Fast	= DrawCan1Frame_Fast,		// Draw 1-pixel frame (does not check clipping limits)
	.pDrawCanFrame		= DrawCan1Frame,		// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameInv_Fast	= DrawCan1FrameInv_Fast,	// Draw 1-pixel frame inverted (does not check clipping limits)
	.pDrawCanFrameInv	= DrawCan1FrameInv,		// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameW_Fast	= DrawCan1FrameW_Fast,		// Draw thick frame (does not check clipping limits)
	.pDrawCanFrameW		= DrawCan1FrameW,		// Draw thick frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameWInv_Fast	= DrawCan1FrameWInv_Fast,	// Draw thick frame inverted (does not check clipping limits)
	.pDrawCanFrameWInv	= DrawCan1FrameWInv,		// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
	// Draw line
	.pDrawCanLineOver_Fast	= DrawCan1LineOver_Fast,	// Draw line with overlapped pixels (does not check clipping limits)
	.pDrawCanLine_Fast	= DrawCan1Line_Fast,		// Draw line (does not check clipping limits)
	.pDrawCanLineOver	= DrawCan1LineOver,		// Draw line with overlapped pixels (checks clipping limits)
	.pDrawCanLine		= DrawCan1Line,			// Draw line (checks clipping limits)
	.pDrawCanLineOverInv_Fast = DrawCan1LineOverInv_Fast,	// Draw line inverted with overlapped pixels (does not check clipping limits)
	.pDrawCanLineInv_Fast	= DrawCan1LineInv_Fast,		// Draw line inverted (does not check clipping limits)
	.pDrawCanLineOverInv	= DrawCan1LineOverInv,		// Draw line inverted with overlapped pixels (checks clipping limits)
	.pDrawCanLineInv	= DrawCan1LineInv,		// Draw line inverted (checks clipping limits)
	.pDrawCanLineW_Fast	= DrawCan1LineW_Fast,		// Draw thick line (does not check clipping limits)
	.pDrawCanLineW		= DrawCan1LineW,		// Draw thick line (checks clipping limits)
	.pDrawCanLineWInv_Fast	= DrawCan1LineWInv_Fast,	// Draw thick line inverted (does not check clipping limits)
	.pDrawCanLineWInv	= DrawCan1LineWInv,		// Draw thick line inverted (checks clipping limits)
	// Draw round (filled circle)
	.pDrawCanRound_Fast	= DrawCan1Round_Fast,		// Draw round (does not check clipping limits)
	.pDrawCanRound		= DrawCan1Round,		// Draw round (checks clipping limits)
	.pDrawCanRoundInv_Fast	= DrawCan1RoundInv_Fast,	// Draw round inverted (does not check clipping limits)
	.pDrawCanRoundInv	= DrawCan1RoundInv,		// Draw round inverted (checks clipping limits)
	// Draw circle
	.pDrawCanCircle_Fast	= DrawCan1Circle_Fast,		// Draw circle or arc (does not check clipping limits)
	.pDrawCanCircle		= DrawCan1Circle,		// Draw circle or arc (checks clipping limits)
	.pDrawCanCircleInv_Fast	= DrawCan1CircleInv_Fast,	// Draw circle or arc inverted (does not check clipping limits)
	.pDrawCanCircleInv	= DrawCan1CircleInv,		// Draw circle or arc inverted (checks clipping limits)
	// Draw ring
	.pDrawCanRing_Fast	= DrawCan1Ring_Fast,		// Draw ring (does not check clipping limits)
	.pDrawCanRing		= DrawCan1Ring,			// Draw ring (checks clipping limits)
	.pDrawCanRingInv_Fast	= DrawCan1RingInv_Fast,		// Draw ring inverted (does not check clipping limits)
	.pDrawCanRingInv	= DrawCan1RingInv,		// Draw ring inverted (checks clipping limits)
	// Draw triangle
	.pDrawCanTriangle_Fast	= DrawCan1Triangle_Fast,	// Draw triangle (does not check clipping limits)
	.pDrawCanTriangle	= DrawCan1Triangle,		// Draw triangle (checks clipping limits)
	.pDrawCanTriangleInv_Fast = DrawCan1TriangleInv_Fast,	// Draw triangle inverted (does not check clipping limits)
	.pDrawCanTriangleInv	= DrawCan1TriangleInv,		// Draw triangle inverted (checks clipping limits)
	// Draw fill area
	.pDrawCanFill		= DrawCan1Fill,			// Draw fill area
	// Draw character
	.pDrawCanChar_Fast	= DrawCan1Char_Fast,		// Draw character with transparent background (does not check clipping limits)
	.pDrawCanChar		= DrawCan1Char,			// Draw character with transparent background (checks clipping limits)
	.pDrawCanCharBg_Fast	= DrawCan1CharBg_Fast,		// Draw character with background (does not check clipping limits)
	.pDrawCanCharBg		= DrawCan1CharBg,		// Draw character with background (checks clipping limits)
	.pDrawCanCharInv_Fast	= DrawCan1CharInv_Fast,		// Draw character inverted (does not check clipping limits)
	.pDrawCanCharInv	= DrawCan1CharInv,		// Draw character inverted (checks clipping limits)
	// Draw text
	.pDrawCanText_Fast	= DrawCan1Text_Fast,		// Draw text with transparent background (does not check clipping limits)
	.pDrawCanText		= DrawCan1Text,			// Draw text with transparent background (checks clipping limits)
	.pDrawCanTextBg_Fast	= DrawCan1TextBg_Fast,		// Draw text with background (does not check clipping limits)
	.pDrawCanTextBg		= DrawCan1TextBg,		// Draw text with background (checks clipping limits)
	.pDrawCanTextInv_Fast	= DrawCan1TextInv_Fast,		// Draw text inverted (does not check clipping limits)
	.pDrawCanTextInv	= DrawCan1TextInv,		// Draw text inverted (checks clipping limits)
	// Draw ellipse
	.pDrawCanEllipse_Fast	= DrawCan1Ellipse_Fast,		// Draw ellipse (does not check clipping limits)
	.pDrawCanEllipse	= DrawCan1Ellipse,		// Draw ellipse (checks clipping limits)
	.pDrawCanEllipseInv_Fast = DrawCan1EllipseInv_Fast,	// Draw ellipse inverted (does not check clipping limits)
	.pDrawCanEllipseInv	= DrawCan1EllipseInv,		// Draw ellipse inverted (checks clipping limits)
	// Draw filled ellipse
	.pDrawCanFillEllipse_Fast = DrawCan1FillEllipse_Fast,	// Draw filled ellipse (does not check clipping limits)
	.pDrawCanFillEllipse	= DrawCan1FillEllipse,		// Draw filled ellipse (checks clipping limits)
	.pDrawCanFillEllipseInv_Fast = DrawCan1FillEllipseInv_Fast, // Draw filled ellipse inverted (does not check clipping limits)
	.pDrawCanFillEllipseInv = DrawCan1FillEllipseInv,	// Draw filled ellipse inverte (checks clipping limits)
	// Draw image
	.pDrawCanImg		= DrawCan1Img,			// Draw image with the same format as destination
	.pDrawCanImgInv		= DrawCan1ImgInv,		// Draw image inverted with the same format as destination
	.pDrawCanBlit		= DrawCan1Blit,			// Draw transparent image with the same format as destination
	.pDrawCanBlitInv	= DrawCan1BlitInv,		// Draw transparent image inverted with the same format as destination
	.pDrawCanBlitSubs	= DrawCan1BlitSubs,		// Draw transparent image with the same format as destination, with substitute color
	.pDrawCanGetImg		= DrawCan1GetImg,		// Get image from canvas to buffer
	.pDrawCanImgMat		= DrawCan1ImgMat,		// Draw image with 2D transformation matrix
	// colors
	.pColRgb		= Draw1ColRgb,			// convert RGB888 color to pixel color
#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
	.pColRand		= Draw1ColRand,			// random color
#endif
	.col_black		= COL1_BLACK,			// black color
	.col_blue		= COL1_BLACK,			// blue color
	.col_green		= COL1_WHITE,			// green color
	.col_cyan		= COL1_WHITE,			// cyan color
	.col_red		= COL1_WHITE,			// red color
	.col_magenta		= COL1_WHITE,			// magenta color
	.col_yellow		= COL1_WHITE,			// yellow color
	.col_white		= COL1_WHITE,			// white color
	.col_gray		= COL1_BLACK,			// gray color
	.col_dkblue		= COL1_BLACK,			// dark blue color
	.col_dkgreen		= COL1_BLACK,			// dark green color
	.col_dkcyan		= COL1_BLACK,			// dark cyan color
	.col_dkred		= COL1_BLACK,			// dark red color
	.col_dkmagenta		= COL1_BLACK,			// dark magenta color
	.col_dkyellow		= COL1_WHITE,			// dark yellow color
	.col_dkwhite		= COL1_WHITE,			// dark white color
	.col_dkgray		= COL1_BLACK,			// dark gray color
	.col_ltblue		= COL1_WHITE,			// light blue color
	.col_ltgreen		= COL1_WHITE,			// light green color
	.col_ltcyan		= COL1_WHITE,			// light cyan color
	.col_ltred		= COL1_WHITE,			// light red color
	.col_ltmagenta		= COL1_WHITE,			// light magenta color
	.col_ltyellow		= COL1_WHITE,			// light yellow color
	.col_ltgray		= COL1_WHITE,			// light gray color
	.col_azure		= COL1_WHITE,			// azure blue color
	.col_orange		= COL1_WHITE,			// orange color
	.col_brown		= COL1_WHITE,			// brown color
};

#endif // USE_DRAWCAN0

#endif // USE_DRAWCAN
