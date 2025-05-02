
// ****************************************************************************
//
//                      Drawing to canvas at 6-bit format
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

#if USE_DRAWCAN	&& USE_DRAWCAN6		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#include "../../_font/_include.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_float.h"
#include "../inc/lib_text.h"
#include "../inc/lib_rand.h"
#include "../inc/lib_drawcan.h"
#include "../inc/lib_drawcan6.h"

#ifndef WIDTH
#define WIDTH		320		// display width
#endif

#ifndef HEIGHT
#define HEIGHT		240		// display height
#endif

// default drawing canvas for 6-bit format
sDrawCan DrawCan6 = {
	// format
	.format = DRAWCAN_FORMAT_6,	// u8	format;		// canvas format CANVAS_*
	.colbit = 6,			// u8	colbit;		// number of bits per pixel
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
	.wb = (WIDTH+4)/5*4,		// s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	.striph = HEIGHT,		// s16	striph;		// strip height (number of lines)

	// print
	.printposnum = WIDTH/FONTW,	// u16	printposnum;	// number of text positions per row (= w / fontw)
	.printrownum = HEIGHT/FONTH,	// u16	printrownum;	// number of text rows (= h / fonth)
	.printpos = 0,			// u16	printpos;	// console print character position
	.printrow = 0,			// u16	printrow;	// console print character row
	.printcol = COL6_WHITE,		// u16	printcol;	// console print color

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
	.drawfnc = NULL, //&DrawCan6Fnc,	// const struct sDrawCanFnc_* drawfnc; // drawing functions
};

// current drawing canvas for 6-bit format
sDrawCan* pDrawCan6 = &DrawCan6;

// set default drawing canvas for 6-bit format
void SetDrawCan6(sDrawCan* can) { pDrawCan6 = can; }

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw6Pitch(int w) { return (w + 4)/5*4; }

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw6MaxWidth(int pitch) { return pitch*5/4; }

// convert RGB888 color to 6-bit pixel color RGB222
u16 Draw6ColRgb(u8 r, u8 g, u8 b) { return COLOR6(r, g, b); }

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 6-bit pixel color RGB222
u16 Draw6ColRand() { return COL6_RANDOM; }
#endif

// ----------------------------------------------------------------------------
//                            Clear canvas
// ----------------------------------------------------------------------------

// Clear canvas with color
void DrawCan6ClearCol(sDrawCan* can, u16 col8)
{
	u8 col = (u8)col8;
	int x = can->clipx1;
	int y = can->clipy1;
	int w = can->clipx2 - x;
	int h = can->clipy2 - y;
	DrawCan6Rect(can, x, y, w, h, col);
}
void Draw6ClearCol(u16 col) { DrawCan6ClearCol(pDrawCan6, col); }

// Clear canvas with black color
void DrawCan6Clear(sDrawCan* can) { DrawCan6ClearCol(can, COL6_BLACK); }
void Draw6Clear() { DrawCan6Clear(pDrawCan6); }

// ----------------------------------------------------------------------------
//                               Draw point
// ----------------------------------------------------------------------------

// Draw point (does not check clipping limits)
void DrawCan6Point_Fast(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// draw pixel
	int x2 = x/5;
	x -= x2*5;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	x *= 6;
	*d = (*d & ~((u32)0x3f<<x)) | ((u32)col<<x);
}
void Draw6Point_Fast(int x, int y, u16 col) { DrawCan6Point_Fast(pDrawCan6, x, y, col); }

// Draw point (checks clipping limits)
void DrawCan6Point(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// draw pixel
	int x2 = x/5;
	x -= x2*5;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	x *= 6;
	*d = (*d & ~((u32)0x3f<<x)) | ((u32)col<<x);
}
void Draw6Point(int x, int y, u16 col) { DrawCan6Point(pDrawCan6, x, y, col); }

// Draw point inverted (does not check clipping limits)
void DrawCan6PointInv_Fast(sDrawCan* can, int x, int y)
{
	// invert pixel
	int x2 = x/5;
	x -= x2*5;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	*d ^= (u32)0x3f<<(x*6);
}
void Draw6PointInv_Fast(int x, int y) { DrawCan6PointInv_Fast(pDrawCan6, x, y); }

// Draw point inverted (checks clipping limits)
void DrawCan6PointInv(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// invert pixel
	int x2 = x/5;
	x -= x2*5;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	*d ^= (u32)0x3f<<(x*6);
}
void Draw6PointInv(int x, int y) { DrawCan6PointInv(pDrawCan6, x, y); }

// ----------------------------------------------------------------------------
//                               Get point
// ----------------------------------------------------------------------------

// Get point (does not check clipping limits)
u16 DrawCan6GetPoint_Fast(sDrawCan* can, int x, int y)
{
	int x2 = x/5;
	x -= x2*5;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	x *= 6;
	return (u8)((*d >> x) & 0x3f);
}
u16 Draw6GetPoint_Fast(int x, int y) { return DrawCan6GetPoint_Fast(pDrawCan6, x, y); }

// Get point (checks clipping limits)
u16 DrawCan6GetPoint(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return COL6_BLACK;

	// get pixel
	int x2 = x/5;
	x -= x2*5;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	x *= 6;
	return (u8)((*d >> x) & 0x3f);
}
u16 Draw6GetPoint(int x, int y) { return DrawCan6GetPoint(pDrawCan6, x, y); }

// ----------------------------------------------------------------------------
//                            Draw rectangle
// ----------------------------------------------------------------------------

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan6Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col8)
{
	u8 col = (u8)col8;

	// split width to whole and fraction part
	int x2 = x/5;
	x -= x2*5; // start X relative 0..4

	// prepare destination address
	int wb = can->wb;
	u32* d0 = (u32*)&can->buf[x2*4 + (y - can->basey)*wb];
	u32* d;
	wb /= 4;

	// prepare multiply color
	u32 colcol = (u32)col | (col << 6) | (col << 12) | (col << 18) | (col << 24);

	// mask of first pixels
	int x6 = x*6;
	u32 m1 = ~((u32)-1 << x6); // mask of first pixels
	u32 col1 = colcol << x6; // color of first pixels

	// width of inner pixels
	if (x > 0) w -= 5 - x;
	int w2 = w/5;

	// mask of last pixels
	u32 m2 = (u32)-1;
	u32 col2 = 0;
	if (w >= 0) // width is large enough
	{
		w -= w2*5;
		w *= 6;
		m2 = (u32)-1 << w;
		col2 = colcol >> (30 - w);
	}
	else // last pixels are not valid
	{
		w += 5; // end of first pixels
		w *= 6;
		u32 m3 = (u32)-1 << w; // mask 2 of first pixels
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
void Draw6Rect_Fast(int x, int y, int w, int h, u16 col) { DrawCan6Rect_Fast(pDrawCan6, x, y, w, h, col); }

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan6Rect(sDrawCan* can, int x, int y, int w, int h, u16 col8)
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
	DrawCan6Rect_Fast(can, x, y, w, h, col);
}
void Draw6Rect(int x, int y, int w, int h, u16 col) { DrawCan6Rect(pDrawCan6, x, y, w, h, col); }

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan6RectInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// split width to whole and fraction part
	int x2 = x/5;
	x -= x2*5; // start X relative 0..4

	// prepare destination address
	int wb = can->wb;
	u32* d0 = (u32*)&can->buf[x2*4 + (y - can->basey)*wb];
	u32* d;
	wb /= 4;

	// mask of first pixels
	u32 m1 = (u32)-1 << (x*6); // mask of first pixels

	// width of inner pixels
	if (x > 0) w -= 5 - x;
	int w2 = w/5;

	// mask of last pixels
	u32 m2 = (u32)-1;
	if (w >= 0) // width is large enough
	{
		w -= w2*5;
		m2 = ~((u32)-1 << (w*6));
	}
	else // last pixels are not valid
	{
		w += 5; // end of first pixels
		u32 m3 = (u32)-1 << (w*6);
		m1 &= ~m3;
		w = 0; // no pixels left as last ones
	}

	int i;
	for (; h > 0; h--)
	{
		d = d0;

		// invert first pixels
		if (x > 0)
		{
			*d ^= m1;
			d++;
		}

		// invert inner pixels
		i = w2;
		for (; i > 0; i--)
		{
			*d = ~*d;
			d++;
		}

		// invert last pixels
		if (w > 0) *d ^= m2;

		// shift buffer address
		d0 += wb;
	}
}
void Draw6RectInv_Fast(int x, int y, int w, int h) { DrawCan6RectInv_Fast(pDrawCan6, x, y, w, h); }

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan6RectInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan6RectInv_Fast(can, x, y, w, h);
}
void Draw6RectInv(int x, int y, int w, int h) { DrawCan6RectInv(pDrawCan6, x, y, w, h); }

// ----------------------------------------------------------------------------
//                          Draw horizontal line
// ----------------------------------------------------------------------------

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan6HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col8)
{
	u8 col = (u8)col8;

	// split width to whole and fraction part
	int x2 = x/5;
	x -= x2*5; // start X relative 0..4

	// prepare destination address
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];

	// prepare multiply color
	u32 colcol = (u32)col | (col << 6) | (col << 12) | (col << 18) | (col << 24);

	// mask of first pixels
	int x6 = x*6;
	u32 m1 = ~((u32)-1 << x6); // mask of first pixels
	u32 col1 = colcol << x6; // color of first pixels

	// width of inner pixels
	if (x > 0) w -= 5 - x;
	int w2 = w/5;

	// mask of last pixels
	u32 m2 = (u32)-1;
	u32 col2 = 0;
	if (w >= 0) // width is large enough
	{
		w -= w2*5;
		w *= 6;
		m2 = (u32)-1 << w;
		col2 = colcol >> (30 - w);
	}
	else // last pixels are not valid
	{
		w += 5; // end of first pixels
		w *= 6;
		u32 m3 = (u32)-1 << w; // mask 2 of first pixels
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
void Draw6HLine_Fast(int x, int y, int w, u16 col) { DrawCan6HLine_Fast(pDrawCan6, x, y, w, col); }

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan6HLine(sDrawCan* can, int x, int y, int w, u16 col)
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

	// draw
	DrawCan6HLine_Fast(can, x, y, w, col);
}
void Draw6HLine(int x, int y, int w, u16 col) { DrawCan6HLine(pDrawCan6, x, y, w, col); }

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan6HLineInv_Fast(sDrawCan* can, int x, int y, int w)
{
	// split width to whole and fraction part
	int x2 = x/5;
	x -= x2*5; // start X relative 0..4

	// prepare destination address
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];

	// mask of first pixels
	u32 m1 = (u32)-1 << (x*6); // mask of first pixels

	// width of inner pixels
	if (x > 0) w -= 5 - x;
	int w2 = w/5;

	// mask of last pixels
	u32 m2 = (u32)-1;
	if (w >= 0) // width is large enough
	{
		w -= w2*5;
		m2 = ~((u32)-1 << (w*6));
	}
	else // last pixels are not valid
	{
		w += 5; // end of first pixels
		u32 m3 = (u32)-1 << (w*6);
		m1 &= ~m3;
		w = 0; // no pixels left as last ones
	}

	// invert first pixels
	if (x > 0)
	{
		*d ^= m1;
		d++;
	}

	// invert inner pixels
	int i = w2;
	for (; i > 0; i--)
	{
		*d = ~*d;
		d++;
	}

	// invert last pixels
	if (w > 0) *d ^= m2;
}
void Draw6HLineInv_Fast(int x, int y, int w) { DrawCan6HLineInv_Fast(pDrawCan6, x, y, w); }

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan6HLineInv(sDrawCan* can, int x, int y, int w)
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
	DrawCan6HLineInv_Fast(can, x, y, w);
}
void Draw6HLineInv(int x, int y, int w) { DrawCan6HLineInv(pDrawCan6, x, y, w); }

// ----------------------------------------------------------------------------
//                           Draw vertical line
// ----------------------------------------------------------------------------

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan6VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col8)
{
	u8 col = (u8)col8;

	// draw
	int x2 = x/5;
	x -= x2*5; // start X relative 0..4
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	int wb = can->wb/4;
	x *= 6;
	u32 m = ~((u32)0x3f << x);
	u32 col2 = (u32)col << x;
	for (; h > 0; h--)
	{
		*d = (*d & m) | col2;
		d += wb;
	}
}
void Draw6VLine_Fast(int x, int y, int h, u16 col) { DrawCan6VLine_Fast(pDrawCan6, x, y, h, col); }

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan6VLine(sDrawCan* can, int x, int y, int h, u16 col)
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

	// draw
	DrawCan6VLine_Fast(can, x, y, h, col);
}
void Draw6VLine(int x, int y, int h, u16 col) { DrawCan6VLine(pDrawCan6, x, y, h, col); }

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan6VLineInv_Fast(sDrawCan* can, int x, int y, int h)
{
	// invert
	int x2 = x/5;
	x -= x2*5; // start X relative 0..4
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	int wb = can->wb/4;
	x *= 6;
	u32 m = (u32)0x3f << x;
	for (; h > 0; h--)
	{
		*d ^= m;
		d += wb;
	}
}
void Draw6VLineInv_Fast(int x, int y, int h) { DrawCan6VLineInv_Fast(pDrawCan6, x, y, h); }

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan6VLineInv(sDrawCan* can, int x, int y, int h)
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
	DrawCan6VLineInv_Fast(can, x, y, h);
}
void Draw6VLineInv(int x, int y, int h) { DrawCan6VLineInv(pDrawCan6, x, y, h); }

// ----------------------------------------------------------------------------
//                           Draw frame
// ----------------------------------------------------------------------------

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan6Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
{
	// bottom line (dark)
	DrawCan6HLine_Fast(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan6VLine_Fast(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan6HLine_Fast(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan6VLine_Fast(can, x, y+1, h-2, col_light);
	}
}
void Draw6Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan6Frame_Fast(pDrawCan6, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan6Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
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
	DrawCan6HLine(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan6VLine(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan6HLine(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan6VLine(can, x, y+1, h-2, col_light);
	}
}
void Draw6Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan6Frame(pDrawCan6, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan6FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// bottom line
	DrawCan6HLineInv_Fast(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan6VLineInv_Fast(can, x+w-1, y, h-1);

		// top line
		DrawCan6HLineInv_Fast(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan6VLineInv_Fast(can, x, y+1, h-2);
	}
}
void Draw6FrameInv_Fast(int x, int y, int w, int h) { DrawCan6FrameInv_Fast(pDrawCan6, x, y, w, h); }

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan6FrameInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan6HLineInv(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan6VLineInv(can, x+w-1, y, h-1);

		// top line
		DrawCan6HLineInv(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan6VLineInv(can, x, y+1, h-2);
	}
}
void Draw6FrameInv(int x, int y, int w, int h) { DrawCan6FrameInv(pDrawCan6, x, y, w, h); }

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan6FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan6Rect_Fast(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan6Rect_Fast(can, x, y, w, thick, col);

		// bottom line
		DrawCan6Rect_Fast(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan6Rect_Fast(can, x, y, thick, h, col);

		// right line
		DrawCan6Rect_Fast(can, x+w-thick, y, thick, h, col);
	}
}
void Draw6FrameW_Fast(int x, int y, int w, int h, u16 col, int thick) { DrawCan6FrameW_Fast(pDrawCan6, x, y, w, h, col, thick); }

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan6FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
{
	if (thick < 1) return;

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
		DrawCan6Rect(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan6Rect(can, x, y, w, thick, col);

		// bottom line
		DrawCan6Rect(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan6Rect(can, x, y, thick, h, col);

		// right line
		DrawCan6Rect(can, x+w-thick, y, thick, h, col);
	}
}
void Draw6FrameW(int x, int y, int w, int h, u16 col, int thick) { DrawCan6Frame(pDrawCan6, x, y, w, h, col, thick); }

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan6FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan6RectInv_Fast(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan6RectInv_Fast(can, x, y, w, thick);

		// bottom line
		DrawCan6RectInv_Fast(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan6RectInv_Fast(can, x, y, thick, h);

		// right line
		DrawCan6RectInv_Fast(can, x+w-thick, y, thick, h);
	}
}
void Draw6FrameWInv_Fast(int x, int y, int w, int h, int thick) { DrawCan6FrameWInv_Fast(pDrawCan6, x, y, w, h, thick); }

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan6FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick)
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
		DrawCan6RectInv(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan6RectInv(can, x, y, w, thick);

		// bottom line
		DrawCan6RectInv(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan6RectInv(can, x, y, thick, h);

		// right line
		DrawCan6RectInv(can, x+w-thick, y, thick, h);
	}
}
void Draw6FrameWInv(int x, int y, int w, int h, int thick) { DrawCan6FrameWInv(pDrawCan6, x, y, w, h, thick); }

// ----------------------------------------------------------------------------
//                           Draw line
// ----------------------------------------------------------------------------

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan6LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
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

	// split width to whole and fraction part
	int xint = x1/5;
	int xfrac = x1 - xint*5; // start X relative 0..4

	// destination address
	u32* d = (u32*)&can->buf[xint*4 + (y1 - can->basey)*wb];
	ddy /= 4;

	// draw first pixel
	*d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6));

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			xfrac += sx;
			if (xfrac == 5)
			{
				xfrac = 0;
				d++;
			}
			if (xfrac == -1)
			{
				xfrac = 4;
				d--;
			}

			if (p > 0)
			{
				if (over) *d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6)); // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			*d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6));
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
				if (over) *d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6)); // draw overlapped pixel
				x1 += sx;
				xfrac += sx;
				if (xfrac == 5)
				{
					xfrac = 0;
					d++;
				}
				if (xfrac == -1)
				{
					xfrac = 4;
					d--;
				}
				p -= dy;
			}
			p += m;
			*d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6));
		}
	}
}
void Draw6LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan6LineOver_Fast(pDrawCan6, x1, y1, x2, y2, col, over); }

// Draw line (does not check clipping limits)
void DrawCan6Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan6LineOver_Fast(pDrawCan6, x1, y1, x2, y2, col, False); }
void Draw6Line_Fast(int x1, int y1, int x2, int y2, u16 col) { Draw6LineOver_Fast(x1, y1, x2, y2, col, False); }

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan6LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
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

	// split width to whole and fraction part
	int xint = x1/5;
	int xfrac = x1 - xint*5; // start X relative 0..4

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u32* d = (u32*)&can->buf[xint*4 + (y1 - can->basey)*wb];
	ddy /= 4;

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6));

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			xfrac += sx;
			if (xfrac == 5)
			{
				xfrac = 0;
				d++;
			}
			if (xfrac == -1)
			{
				xfrac = 4;
				d--;
			}

			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6));
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6));
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6));
				x1 += sx;
				xfrac += sx;
				if (xfrac == 5)
				{
					xfrac = 0;
					d++;
				}
				if (xfrac == -1)
				{
					xfrac = 4;
					d--;
				}

				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x3f<<(xfrac*6))) | ((u32)col<<(xfrac*6));
		}
	}
}
void Draw6LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan6LineOver(pDrawCan6, x1, y1, x2, y2, col, over); }

// Draw line (checks clipping limits)
void DrawCan6Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan6LineOver(can, x1, y1, x2, y2, col, False); }
void Draw6Line(int x1, int y1, int x2, int y2, u16 col) { DrawCan6Line(pDrawCan6, x1, y1, x2, y2, col); }

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan6LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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

	// split width to whole and fraction part
	int xint = x1/5;
	int xfrac = x1 - xint*5; // start X relative 0..4

	// destination address
	u32* d = (u32*)&can->buf[xint*4 + (y1 - can->basey)*wb];
	ddy /= 4;

	// draw first pixel
	*d ^= (u32)0x3f<<(xfrac*6);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			xfrac += sx;
			if (xfrac == 5)
			{
				xfrac = 0;
				d++;
			}
			if (xfrac == -1)
			{
				xfrac = 4;
				d--;
			}

			if (p > 0)
			{
				if (over) *d ^= (u32)0x3f<<(xfrac*6); // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			*d ^= (u32)0x3f<<(xfrac*6);
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
				if (over) *d ^= (u32)0x3f<<(xfrac*6); // draw overlapped pixel
				x1 += sx;
				xfrac += sx;
				if (xfrac == 5)
				{
					xfrac = 0;
					d++;
				}
				if (xfrac == -1)
				{
					xfrac = 4;
					d--;
				}
				p -= dy;
			}
			p += m;
			*d ^= (u32)0x3f<<(xfrac*6);
		}
	}
}
void Draw6LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over) { DrawCan6LineOverInv_Fast(pDrawCan6, x1, y1, x2, y2, over); }

// Draw line inverted (does not check clipping limits)
void DrawCan6LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan6LineOverInv_Fast(can, x1, y1, x2, y2, False); }
void Draw6LineInv_Fast(int x1, int y1, int x2, int y2) { DrawCan6LineOverInv_Fast(pDrawCan6, x1, y1, x2, y2, False); }

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan6LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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

	// split width to whole and fraction part
	int xint = x1/5;
	int xfrac = x1 - xint*5; // start X relative 0..4

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u32* d = (u32*)&can->buf[xint*4 + (y1 - can->basey)*wb];
	ddy /= 4;

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x3f<<(xfrac*6);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			xfrac += sx;
			if (xfrac == 5)
			{
				xfrac = 0;
				d++;
			}
			if (xfrac == -1)
			{
				xfrac = 4;
				d--;
			}
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x3f<<(xfrac*6);
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x3f<<(xfrac*6);
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x3f<<(xfrac*6);
				x1 += sx;
				xfrac += sx;
				if (xfrac == 5)
				{
					xfrac = 0;
					d++;
				}
				if (xfrac == -1)
				{
					xfrac = 4;
					d--;
				}
				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x3f<<(xfrac*6);
		}
	}
}
void Draw6LineOverInv(int x1, int y1, int x2, int y2, Bool over) { DrawCan6LineOverInv(pDrawCan6, x1, y1, x2, y2, over); }

// Draw line inverted (checks clipping limits)
void DrawCan6LineInv(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan6LineOverInv(can, x1, y1, x2, y2, False); }
void Draw6LineInv(int x1, int y1, int x2, int y2) { DrawCan6LineOverInv(pDrawCan6, x1, y1, x2, y2, False); }

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan6LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan6LineOver_Fast(can, x1, y1, x2, y2, col, False);
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
		DrawCan6Round_Fast(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan6Round_Fast(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan6LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan6LineOver_Fast(can, x1, y1, x2, y2, col, over);
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
		DrawCan6LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan6LineOver_Fast(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw6LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan6LineW_Fast(pDrawCan6, x1, y1, x2, y2, col, thick, round); }

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan6LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan6LineOver(can, x1, y1, x2, y2, col, False);
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
		DrawCan6Round(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan6Round(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan6LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan6LineOver(can, x1, y1, x2, y2, col, over);
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
		DrawCan6LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan6LineOver(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw6LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan6LineW(pDrawCan6, x1, y1, x2, y2, col, thick, round); }

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan6LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan6LineOverInv_Fast(can, x1, y1, x2, y2, False);
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
		DrawCan6LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan6LineOverInv_Fast(can, x1, y1, x2, y2, over);
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
		DrawCan6LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan6LineOverInv_Fast(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw6LineWInv_Fast(int x1, int y1, int x2, int y2, int thick) { DrawCan6LineWInv_Fast(pDrawCan6, x1, y1, x2, y2, thick); }

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan6LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan6LineOverInv(can, x1, y1, x2, y2, False);
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
		DrawCan6LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan6LineOverInv(can, x1, y1, x2, y2, over);
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
		DrawCan6LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan6LineOverInv(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw6LineWInv(int x1, int y1, int x2, int y2, int thick) { DrawCan6LineWInv(pDrawCan6, x1, y1, x2, y2, thick); }

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
void DrawCan6Round_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u32 *s0, *s;
	int r2;
	u8 col = (u8)col8;

	// hide parts of the round
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		int xx = (x+x1)/5;
		int x0 = ((x+x1) - xx*5)*6;
		s0 = (u32*)&can->buf[xx*4 + ((y+y1) - can->basey)*wb];
		wb /= 4;

		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s = (*s & ~((u32)0x3f<<xx)) | ((u32)col<<xx);
				xx += 6;
				if (xx == 30)
				{
					s++;
					xx = 0;
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

		int xx = (x+x1)/5;
		int x0 = ((x+x1) - xx*5)*6;
		s0 = (u32*)&can->buf[xx*4 + ((y+y1) - can->basey)*wb];
		wb /= 4;
		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s = (*s & ~((u32)0x3f<<xx)) | ((u32)col<<xx);
				xx += 6;
				if (xx == 30)
				{
					s++;
					xx = 0;
				}
			}
			s0 += wb;
		}
	}
}
void Draw6Round_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan6Round_Fast(pDrawCan6, x, y, d, col, mask); }

// Draw round (checks clipping limits)
void DrawCan6Round(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	if (d < 1) d = 1;
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u32 *s0, *s;
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
		int xx = (x+x1)/5;
		int x0 = ((x+x1) - xx*5)*6;
		s0 = (u32*)&can->buf[xx*4 + ((y+y1) - can->basey)*wb];
		wb /= 4;

		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s = (*s & ~((u32)0x3f<<xx)) | ((u32)col<<xx);
				xx += 6;
				if (xx == 30)
				{
					s++;
					xx = 0;
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
		int xx = (x+x1)/5;
		int x0 = ((x+x1) - xx*5)*6;
		s0 = (u32*)&can->buf[xx*4 + ((y+y1) - can->basey)*wb];
		wb /= 4;

		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s = (*s & ~((u32)0x3f<<xx)) | ((u32)col<<xx);
				xx += 6;
				if (xx == 30)
				{
					s++;
					xx = 0;
				}
			}
			s0 += wb;
		}
	}
}
void Draw6Round(int x, int y, int d, u16 col, u8 mask) { DrawCan6Round(pDrawCan6, x, y, d, col, mask); }

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan6RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	if (d < 1) d = 1;
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u32 *s0, *s;
	int r2;

	// hide parts of the round
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		int xx = (x+x1)/5;
		int x0 = ((x+x1) - xx*5)*6;
		s0 = (u32*)&can->buf[xx*4 + ((y+y1) - can->basey)*wb];
		wb /= 4;

		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s ^= (u32)0x3f<<xx;
				xx += 6;
				if (xx == 30)
				{
					s++;
					xx = 0;
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

		int xx = (x+x1)/5;
		int x0 = ((x+x1) - xx*5)*6;
		s0 = (u32*)&can->buf[xx*4 + ((y+y1) - can->basey)*wb];
		wb /= 4;

		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s ^= (u32)0x3f<<xx;
				xx += 6;
				if (xx == 30)
				{
					s++;
					xx = 0;
				}
			}
			s0 += wb;
		}
	}
}
void Draw6RoundInv_Fast(int x, int y, int d, u8 mask) { DrawCan6RoundInv_Fast(pDrawCan6, x, y, d, mask); }

// Draw round inverted (checks clipping limits)
void DrawCan6RoundInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u32 *s0, *s;
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
		int xx = (x+x1)/5;
		int x0 = ((x+x1) - xx*5)*6;
		s0 = (u32*)&can->buf[xx*4 + ((y+y1) - can->basey)*wb];
		wb /= 4;

		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s ^= (u32)0x3f<<xx;
				xx += 6;
				if (xx == 30)
				{
					s++;
					xx = 0;
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
		int xx = (x+x1)/5;
		int x0 = ((x+x1) - xx*5)*6;
		s0 = (u32*)&can->buf[xx*4 + ((y+y1) - can->basey)*wb];
		wb /= 4;

		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			xx = x0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s ^= (u32)0x3f<<xx;
				xx += 6;
				if (xx == 30)
				{
					s++;
					xx = 0;
				}
			}
			s0 += wb;
		}
	}
}
void Draw6RoundInv(int x, int y, int d, u8 mask) { DrawCan6RoundInv(pDrawCan6, x, y, d, mask); }

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
		int xint = x2/5;			\
		int xfrac = (x2 - xint*5)*6;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s = (*s & ~((u32)0x3f<<xfrac)) | ((u32)col<<xfrac); }

#define DRAWCANCIRCLE_PIX(xxx,yyy,mmm)			\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xint = x2/5;			\
		int xfrac = (x2 - xint*5)*6;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s = (*s & ~((u32)0x3f<<xfrac)) | ((u32)col<<xfrac); } }

#define DRAWCANCIRCLE_PIXINVFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xint = x2/5;			\
		int xfrac = (x2 - xint*5)*6;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s ^= (u32)0x3f<<xfrac; }

#define DRAWCANCIRCLE_PIXINV(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xint = x2/5;			\
		int xfrac = (x2 - xint*5)*6;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s ^= (u32)0x3f<<xfrac; } }

//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan6Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan6Point_Fast(can, x, y, col);
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
	u32* s;

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
void Draw6Circle_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan6Circle_Fast(pDrawCan6, x, y, d, col, mask); }

// Draw circle or arc (checks clipping limits)
void DrawCan6Circle(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan6Point(can, x, y, col);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan6Circle_Fast(can, x, y, d, col, mask);
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
	u32* s;

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
void Draw6Circle(int x, int y, int d, u16 col, u8 mask) { DrawCan6Circle(pDrawCan6, x, y, d, col, mask); }

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan6CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan6PointInv_Fast(can, x, y);
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
	u32* s;

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
void Draw6CircleInv_Fast(int x, int y, int d, u8 mask) { DrawCan6CircleInv_Fast(pDrawCan6, x, y, d, mask); }

// Draw circle or arc inverted (checks clipping limits)
void DrawCan6CircleInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan6PointInv(can, x, y);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan6CircleInv_Fast(can, x, y, d, mask);
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
	u32* s;

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
void Draw6CircleInv(int x, int y, int d, u8 mask) { DrawCan6CircleInv(pDrawCan6, x, y, d, mask); }

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
void DrawCan6Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	// nothing to draw
	if (din >= d) return;
	u8 col = (u8)col8;

	// draw round
	if (din <= 0)
	{
		DrawCan6Round_Fast(can, x, y, d, col, mask);
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
		DrawCan6Circle_Fast(can, x, y, d, col, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0;
	u32 *s;
	int r2, rin2, xx, xxx, xint, xfrac;

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
					xint = xxx/5;
					xfrac = (xxx - xint*5)*6;
					s = (u32*)&s0[xint*4];
					*s = (*s & ~((u32)0x3f<<xfrac)) | ((u32)col<<xfrac);
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
					xint = xxx/5;
					xfrac = (xxx - xint*5)*6;
					s = (u32*)&s0[xint*4];
					*s = (*s & ~((u32)0x3f<<xfrac)) | ((u32)col<<xfrac);
				}
			}
			s0 += wb;
		}
	}
}
void Draw6Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan6Ring_Fast(pDrawCan6, x, y, d, din, col, mask); }

// Draw ring (checks clipping limits)
void DrawCan6Ring(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	u8 col = (u8)col8;
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan6Round(can, x, y, d, col, mask);
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
		DrawCan6Circle(can, x, y, d, col, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0;
	u32 *s;
	int r2, rin2, xx, xxx, xint, xfrac;

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
					xint = xxx/5;
					xfrac = (xxx - xint*5)*6;
					s = (u32*)&s0[xint*4];
					*s = (*s & ~((u32)0x3f<<xfrac)) | ((u32)col<<xfrac);
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
					xint = xxx/5;
					xfrac = (xxx - xint*5)*6;
					s = (u32*)&s0[xint*4];
					*s = (*s & ~((u32)0x3f<<xfrac)) | ((u32)col<<xfrac);
				}
			}
			s0 += wb;
		}
	}
}
void Draw6Ring(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan6Ring(pDrawCan6, x, y, d, din, col, mask); }

// Draw ring inverted (does not check clipping limits)
void DrawCan6RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan6RoundInv_Fast(can, x, y, d, mask);
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
		DrawCan6CircleInv_Fast(can, x, y, d, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0;
	u32 *s;
	int r2, rin2, xx, xxx, xint, xfrac;

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
					xint = xxx/5;
					xfrac = (xxx - xint*5)*6;
					s = (u32*)&s0[xint*4];
					*s ^= (u32)0x3f<<xfrac;
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
					xint = xxx/5;
					xfrac = (xxx - xint*5)*6;
					s = (u32*)&s0[xint*4];
					*s ^= (u32)0x3f<<xfrac;
				}
			}
			s0 += wb;
		}
	}
}
void Draw6RingInv_Fast(int x, int y, int d, int din, u8 mask) { DrawCan6RingInv_Fast(pDrawCan6, x, y, d, din, mask); }

// Draw ring inverted (checks clipping limits)
void DrawCan6RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan6RoundInv(can, x, y, d, mask);
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
		DrawCan6CircleInv(can, x, y, d, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb;
	u8 *s0;
	u32 *s;
	int r2, rin2, xx, xxx, xint, xfrac;

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
					xint = xxx/5;
					xfrac = (xxx - xint*5)*6;
					s = (u32*)&s0[xint*4];
					*s ^= (u32)0x3f<<xfrac;
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
					xint = xxx/5;
					xfrac = (xxx - xint*5)*6;
					s = (u32*)&s0[xint*4];
					*s ^= (u32)0x3f<<xfrac;
				}
			}
			s0 += wb;
		}
	}
}
void Draw6RingInv(int x, int y, int d, int din, u8 mask) { DrawCan6RingInv(pDrawCan6, x, y, d, din, mask); }

// ----------------------------------------------------------------------------
//                              Draw triangle
// ----------------------------------------------------------------------------

// Draw triangle (does not check clipping limits)
void DrawCan6Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
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
			if (k > 0) DrawCan6HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan6HLine_Fast(can, xmin, y, k, col);
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
			if (k > 0) DrawCan6HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan6HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan6HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan6HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw6Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan6Triangle_Fast(pDrawCan6, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle (checks clipping limits)
void DrawCan6Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
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
		DrawCan6Triangle_Fast(can, x1, y1, x2, y2, x3, y3, col);
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
			if (k > 0) DrawCan6HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan6HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan6HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan6HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan6HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan6HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw6Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan6Triangle(pDrawCan6, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle inverted (does not check clipping limits)
void DrawCan6TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
			if (k > 0) DrawCan6HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan6HLineInv_Fast(can, xmin, y, k);
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
			if (k > 0) DrawCan6HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan6HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan6HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan6HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw6TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan6TriangleInv_Fast(pDrawCan6, x1, y1, x2, y2, x3, y3); }

// Draw triangle inverted (checks clipping limits)
void DrawCan6TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
		DrawCan6TriangleInv_Fast(can, x1, y1, x2, y2, x3, y3);
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
			if (k > 0) DrawCan6HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan6HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan6HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan6HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan6HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan6HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw6TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan6TriangleInv(pDrawCan6, x1, y1, x2, y2, x3, y3); }

// ----------------------------------------------------------------------------
//                              Draw fill area
// ----------------------------------------------------------------------------

// Draw fill sub-area (internal function)
void _DrawCan6SubFill(sDrawCan* can, int x, int y, u8 fnd, u8 col)
{
	// save start position -> x2
	int x2 = x;

	// fill start point
	DrawCan6Point_Fast(can, x, y, col);

	// update dirty area in Y direction
	if (y < can->dirtyy1) can->dirtyy1 = y;
	if (y >= can->dirtyy2) can->dirtyy2 = y+1;

	// search start of segment in LEFT-X direction -> x1
	int clipx1 = can->clipx1;
	while ((x > clipx1) && ((u8)DrawCan6GetPoint_Fast(can, x-1, y) == fnd))
	{
		x--;
		DrawCan6Point_Fast(can, x, y, col);
	}
	int x1 = x;

	// search end of segment in RIGHT-X direction -> x2
	int clipx2 = can->clipx2-1;
	while ((x2 < clipx2) && ((u8)DrawCan6GetPoint_Fast(can, x2+1, y) == fnd))
	{
		x2++;
		DrawCan6Point_Fast(can, x2, y, col);
	}

	// search segments in UP-Y direction
	if (y > can->clipy1)
	{
		x = x1;
		y--;
		while (x <= x2)
		{
			if ((u8)DrawCan6GetPoint_Fast(can, x, y) == fnd) _DrawCan6SubFill(can, x, y, fnd, col);
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
			if ((u8)DrawCan6GetPoint_Fast(can, x, y) == fnd) _DrawCan6SubFill(can, x, y, fnd, col);
			x++;
		}
	}

	// update dirty area in X direction
	if (x1 < can->dirtyx1) can->dirtyx1 = x1;
	if (x2 >= can->dirtyx2) can->dirtyx2 = x2+1;
}

// Draw fill area
void DrawCan6Fill(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check clipping
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// get color to search
	u8 fnd = (u8)DrawCan6GetPoint_Fast(can, x, y);

	// fill if color is different
	if (fnd != col) _DrawCan6SubFill(can, x, y, fnd, col);
}
void Draw6Fill(int x, int y, u16 col) { DrawCan6Fill(pDrawCan6, x, y, col); }

// ----------------------------------------------------------------------------
//                             Draw charactter
// ----------------------------------------------------------------------------
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw6SelFont(const u8* font, u8 fontw, u8 fonth) { DrawCanSelFont(pDrawCan6, font, fontw, fonth); }

void Draw6SelFont8x8()		{ DrawCanSelFont8x8(pDrawCan6); }		// sans-serif bold, height 8 lines
void Draw6SelFont8x14()		{ DrawCanSelFont8x14(pDrawCan6); }		// sans-serif bold, height 14 lines
void Draw6SelFont8x16()		{ DrawCanSelFont8x16(pDrawCan6); }		// sans-serif bold, height 16 lines
void Draw6SelFont8x14Serif()	{ DrawCanSelFont8x14Serif(pDrawCan6); }		// serif bold, height 14 lines
void Draw6SelFont8x16Serif()	{ DrawCanSelFont8x16Serif(pDrawCan6); }		// serif bold, height 16 lines
void Draw6SelFont6x8()		{ DrawCanSelFont6x8(pDrawCan6); }		// condensed font, width 6 pixels, height 8 lines
void Draw6SelFont8x8Game()	{ DrawCanSelFont8x8Game(pDrawCan6); }		// game font, height 8 lines
void Draw6SelFont8x8Ibm()	{ DrawCanSelFont8x8Ibm(pDrawCan6); }		// IBM charset font, height 8 lines
void Draw6SelFont8x14Ibm()	{ DrawCanSelFont8x14Ibm(pDrawCan6); }		// IBM charset font, height 14 lines
void Draw6SelFont8x16Ibm()	{ DrawCanSelFont8x16Ibm(pDrawCan6); }		// IBM charset font, height 16 lines
void Draw6SelFont8x8IbmThin()	{ DrawCanSelFont8x8IbmThin(pDrawCan6); }	// IBM charset thin font, height 8 lines
void Draw6SelFont8x8Italic()	{ DrawCanSelFont8x8Italic(pDrawCan6); }		// italic, height 8 lines
void Draw6SelFont8x8Thin()	{ DrawCanSelFont8x8Thin(pDrawCan6); }		// thin font, height 8 lines
void Draw6SelFont5x8()		{ DrawCanSelFont5x8(pDrawCan6); }		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan6Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
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
				if ((b & 0x80) != 0) DrawCan6Point_Fast(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan6Point_Fast(can, x, y, col);
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
void Draw6Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan6Char_Fast(pDrawCan6, ch, x, y, col, scalex, scaley); }

// Draw character with transparent background (checks clipping limits)
void DrawCan6Char(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
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
		DrawCan6Char_Fast(can, ch, x, y, col, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan6Point(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan6Point(can, x, y, col);
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
void Draw6Char(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan6Char(pDrawCan6, ch, x, y, col, scalex, scaley); }

// Draw character with background (does not check clipping limits)
void DrawCan6CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
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
				DrawCan6Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				DrawCan6Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw6CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan6CharBg_Fast(pDrawCan6, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character with background (checks clipping limits)
void DrawCan6CharBg(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
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
		DrawCan6CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);
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
				DrawCan6Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				DrawCan6Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw6CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan6CharBg(pDrawCan6, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character inverted (does not check clipping limits)
void DrawCan6CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
				if ((b & 0x80) != 0) DrawCan6PointInv_Fast(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan6PointInv_Fast(can, x, y);
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
void Draw6CharInv_Fast(char ch, int x, int y, int scalex, int scaley) { DrawCan6CharInv_Fast(pDrawCan6, ch, x, y, scalex, scaley); }

// Draw character inverted (checks clipping limits)
void DrawCan6CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
		DrawCan6CharInv_Fast(can, ch, x, y, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan6PointInv(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan6PointInv(can, x, y);
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
void Draw6CharInv(char ch, int x, int y, int scalex, int scaley) { DrawCan6CharInv(pDrawCan6, ch, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                             Draw text
// ----------------------------------------------------------------------------
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan6Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan6Char_Fast(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw6Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan6Text_Fast(pDrawCan6, text, len, x, y, col, scalex, scaley); }

// Draw text with transparent background (checks clipping limits)
void DrawCan6Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan6Char(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw6Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan6Text(pDrawCan6, text, len, x, y, col, scalex, scaley); }

// Draw text with background (does not check clipping limits)
void DrawCan6TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan6CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw6TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan6TextBg_Fast(pDrawCan6, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text with background (checks clipping limits)
void DrawCan6TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan6CharBg(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw6TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan6TextBg(pDrawCan6, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text inverted (does not check clipping limits)
void DrawCan6TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan6CharInv_Fast(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw6TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan6TextInv_Fast(pDrawCan6, text, len, x, y, scalex, scaley); }

// Draw text inverted (checks clipping limits)
void DrawCan6TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan6CharInv(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw6TextInv(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan6TextInv(pDrawCan6, text, len, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                               Draw ellipse
// ----------------------------------------------------------------------------
// Using mid-point ellipse drawing algorithm

#define DRAWCANELLIPSE_PIXFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xint = x2/5;			\
		int xfrac = (x2 - xint*5)*6;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s = (*s & ~((u32)0x3f<<xfrac)) | ((u32)col<<xfrac); }

#define DRAWCANELLIPSE_PIX(xxx,yyy,mmm)			\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xint = x2/5;			\
		int xfrac = (x2 - xint*5)*6;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s = (*s & ~((u32)0x3f<<xfrac)) | ((u32)col<<xfrac); } }

#define DRAWCANELLIPSE_PIXINVFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xint = x2/5;			\
		int xfrac = (x2 - xint*5)*6;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s ^= (u32)0x3f<<xfrac; }

#define DRAWCANELLIPSE_PIXINV(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xint = x2/5;			\
		int xfrac = (x2 - xint*5)*6;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s ^= (u32)0x3f<<xfrac; } }

//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse
//		DRAWCAN_ELLIPSE_ARC0	= draw arc 0..90 deg
//		DRAWCAN_ELLIPSE_ARC1	= draw arc 90..180 deg
//		DRAWCAN_ELLIPSE_ARC2	= draw arc 180..270 deg
//		DRAWCAN_ELLIPSE_ARC3	= draw arc 270..360 deg
//		DRAWCAN_ELLIPSE_ALL	= draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan6Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	u32* s;

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
void Draw6Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan6Ellipse_Fast(pDrawCan6, x, y, dx, dy, col, mask); }

// Draw ellipse (checks clipping limits)
void DrawCan6Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
		DrawCan6Ellipse_Fast(can, x, y, dx, dy, col, mask);
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
	u32* s;

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
void Draw6Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan6Ellipse(pDrawCan6, x, y, dx, dy, col, mask); }

// Draw ellipse inverted (does not check clipping limits)
void DrawCan6EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	u32* s;

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
void Draw6EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan6EllipseInv_Fast(pDrawCan6, x, y, dx, dy, mask); }

// Draw ellipse inverted (checks clipping limits)
void DrawCan6EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
		DrawCan6EllipseInv_Fast(can, x, y, dx, dy, mask);
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
	u32* s;

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
void Draw6EllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan6EllipseInv(pDrawCan6, x, y, dx, dy, mask); }

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
void DrawCan6FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	u32 *s0, *s;
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
	int xxx = (x+x1)/5;
	int x0 = ((x+x1) - xxx*5)*6;
	s0 = (u32*)&can->buf[xxx*4 + ((y+y1) - can->basey)*wb];
	wb /= 4;
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		xxx = x0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = (*s & ~((u32)0x3f<<xxx)) | ((u32)col<<xxx);
			xxx += 6;
			if (xxx == 30)
			{
				s++;
				xxx = 0;
			}
		}
		s0 += wb;
	}
}
void Draw6FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan6FillEllipse_Fast(pDrawCan6, x, y, dx, dy, col, mask); }

// Draw filled ellipse (checks clipping limits)
void DrawCan6FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	u32 *s0, *s;
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
	int xxx = (x+x1)/5;
	int x0 = ((x+x1) - xxx*5)*6;
	s0 = (u32*)&can->buf[xxx*4 + ((y+y1) - can->basey)*wb];
	wb /= 4;
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		xxx = x0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = (*s & ~((u32)0x3f<<xxx)) | ((u32)col<<xxx);
			xxx += 6;
			if (xxx == 30)
			{
				s++;
				xxx = 0;
			}
		}
		s0 += wb;
	}
}
void Draw6FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan6FillEllipse(pDrawCan6, x, y, dx, dy, col, mask); }

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan6FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	u32 *s0, *s;
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
	int xxx = (x+x1)/5;
	int x0 = ((x+x1) - xxx*5)*6;
	s0 = (u32*)&can->buf[xxx*4 + ((y+y1) - can->basey)*wb];
	wb /= 4;
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		xxx = x0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s ^= (u32)0x3f<<xxx;
			xxx += 6;
			if (xxx == 30)
			{
				s++;
				xxx = 0;
			}
		}
		s0 += wb;
	}
}
void Draw6FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan6FillEllipseInv_Fast(pDrawCan6, x, y, dx, dy, mask); }

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan6FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	u32 *s0, *s;
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
	int xxx = (x+x1)/5;
	int x0 = ((x+x1) - xxx*5)*6;
	s0 = (u32*)&can->buf[xxx*4 + ((y+y1) - can->basey)*wb];
	wb /= 4;
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		xxx = x0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s ^= (u32)0x3f<<xxx;
			xxx += 6;
			if (xxx == 30)
			{
				s++;
				xxx = 0;
			}
		}
		s0 += wb;
	}
}
void Draw6FillEllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan6FillEllipseInv(pDrawCan6, x, y, dx, dy, mask); }

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
void DrawCan6Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*5/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/5;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*5)*6;

	// source address
	int xsint = xs/5;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*5)*6;

	// copy image
	int i;
	u32* d;
	const u32* s;
	int xxs, xxd;
	wbd /= 4;
	wbs /= 4;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x3f);
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			*d = (*d & ~((u32)0x3f<<xxd)) | ((u32)c<<xxd);
			xxd += 6;
			if (xxd == 30)
			{
				xxd = 0;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw6Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan6Img(pDrawCan6, xd, yd, src, xs, ys, w, h, wbs); }

// Draw image inverted with the same format as destination
void DrawCan6ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*5/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/5;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*5)*6;

	// source address
	int xsint = xs/5;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*5)*6;

	// copy image
	int i;
	u32* d;
	const u32* s;
	int xxs, xxd;
	wbd /= 4;
	wbs /= 4;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x3f);
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			*d ^= (u32)c<<xxd;
			xxd += 6;
			if (xxd == 30)
			{
				xxd = 0;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw6ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan6ImgInv(pDrawCan6, xd, yd, src, xs, ys, w, h, wbs); }

// Draw transparent image with the same format as destination
void DrawCan6Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs*5/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/5;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*5)*6;

	// source address
	int xsint = xs/5;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*5)*6;

	// copy image
	int i;
	u32* d;
	const u32* s;
	int xxs, xxd;
	wbd /= 4;
	wbs /= 4;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x3f);
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			if (c != col) *d = (*d & ~((u32)0x3f<<xxd)) | ((u32)c<<xxd);
			xxd += 6;
			if (xxd == 30)
			{
				xxd = 0;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw6Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan6Blit(pDrawCan6, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan6BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8, u16 fnd8, u16 subs8)
{
	u8 col = (u8)col8;
	u8 fnd = (u8)fnd8;
	u8 subs = (u8)subs8;

	// width of source image
	int ws = wbs*5/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/5;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*5)*6;

	// source address
	int xsint = xs/5;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*5)*6;

	// copy image
	int i;
	u32* d;
	const u32* s;
	int xxs, xxd;
	wbd /= 4;
	wbs /= 4;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x3f);
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			if (c != col)
			{
				if (c == fnd) c = subs;
				*d = (*d & ~((u32)0x3f<<xxd)) | ((u32)c<<xxd);
			}

			xxd += 6;
			if (xxd == 30)
			{
				xxd = 0;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw6BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { DrawCan6BlitSubs(pDrawCan6, xd, yd, src, xs, ys, w, h, wbs, col, fnd, subs); }

// Draw transparent image inverted with the same format as destination
void DrawCan6BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs*5/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/5;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*5)*6;

	// source address
	int xsint = xs/5;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*5)*6;

	// copy image
	int i;
	u32* d;
	const u32* s;
	int xxs, xxd;
	wbd /= 4;
	wbs /= 4;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x3f);
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			if (c != col) *d ^= (u32)c<<xxd;
			xxd += 6;
			if (xxd == 30)
			{
				xxd = 0;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw6BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan6BlitInv(pDrawCan6, xd, yd, src, xs, ys, w, h, wbs, col); }

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
void DrawCan6GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd)
{
	// width of destination image
	int wd = wbd*5/4;

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
	int xsint = xs/5;
	const u32* s0 = (const u32*)&can->buf[xsint*4 + (ys - can->basey)*wbs];
	int xsfrac = (xs - xsint*5)*6;

	// destination address
	int xdint = xd/5;
	u32* d0 = (u32*)&((u8*)dst)[xdint*4 + yd*wbd];
	int xdfrac = (xd - xdint*5)*6;

	// copy image
	int i;
	u32* d;
	const u32* s;
	int xxs, xxd;
	wbd /= 4;
	wbs /= 4;
	u8 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xsfrac;
		xxd = xdfrac;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x3f);
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			*d = (*d & ~((u32)0x3f<<xxd)) | ((u32)c<<xxd);
			xxd += 6;
			if (xxd == 30)
			{
				xxd = 0;
				d++;
			}
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw6GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { DrawCan6GetImg(pDrawCan6, xs, ys, w, h, dst, xd, yd, wbd); }

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
void DrawCan6ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src_img, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
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
	int i, x2, y2, xb, xc;
	const u32* s;
	u32* d;
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

					xb = x2/5;
					xc = (x2 - xb*5)*6;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x3f;
					xb = xd/5;
					xc = (xd - xb*5)*6;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x3f<<xc)) | ((u32)pix<<xc);
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

					xb = x2/5;
					xc = (x2 - xb*5)*6;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x3f;
					xb = xd/5;
					xc = (xd - xb*5)*6;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x3f<<xc)) | ((u32)pix<<xc);
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
						xb = x2/5;
						xc = (x2 - xb*5)*6;
						s = (const u32*)&src[xb*4 + y2*wbs];
						pix = (*s >> xc) & 0x3f;
						xb = xd/5;
						xc = (xd - xb*5)*6;
						d = (u32*)&d0[xb*4];
						*d = (*d & ~((u32)0x3f<<xc)) | ((u32)pix<<xc);
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
					xb = xd/5;
					xc = (xd - xb*5)*6;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x3f<<xc)) | ((u32)col<<xc);
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

					xb = x2/5;
					xc = (x2 - xb*5)*6;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x3f;
					xb = xd/5;
					xc = (xd - xb*5)*6;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x3f<<xc)) | ((u32)pix<<xc);
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
					xb = xd/5;
					xc = (xd - xb*5)*6;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x3f<<xc)) | ((u32)color<<xc);
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
						xb = x2/5;
						xc = (x2 - xb*5)*6;
						s = (const u32*)&src[xb*4 + y2*wbs];
						pix = (*s >> xc) & 0x3f;
					}
					else
						pix = color;
					xb = xd/5;
					xc = (xd - xb*5)*6;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x3f<<xc)) | ((u32)pix<<xc);
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
						xb = x2/5;
						xc = (x2 - xb*5)*6;
						s = (const u32*)&src[xb*4 + y2*wbs];
						pix = (*s >> xc) & 0x3f;

						if (pix != color)
						{
							xb = xd/5;
							xc = (xd - xb*5)*6;
							d = (u32*)&d0[xb*4];
							*d = (*d & ~((u32)0x3f<<xc)) | ((u32)pix<<xc);
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

					xb = x2/5;
					xc = (x2 - xb*5)*6;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x3f;
					xb = xd/5;
					xc = (xd - xb*5)*6;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x3f<<xc)) | ((u32)pix<<xc);
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

					xb = x2/5;
					xc = (x2 - xb*5)*6;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x3f;
					xb = xd/5;
					xc = (xd - xb*5)*6;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x3f<<xc)) | ((u32)pix<<xc);
					xd++;
				}
				y0++;
				d0 += wbd;
			}
		}
	}
}
void Draw6ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan6ImgMat(pDrawCan6, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

#undef DRAWCAN_IMGLIMIT

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// drawing function interface
const sDrawCanFnc DrawCan6Fnc = {
	.pDrawPitch		= Draw6Pitch,			// calculate pitch of graphics line from image width
	.pDrawMaxWidth		= Draw6MaxWidth,		// calculate max. width of image from the pitch
	// Clear canvas
	.pDrawCanClearCol	= DrawCan6ClearCol,		// Clear canvas with color
	.pDrawCanClear		= DrawCan6Clear,		// Clear canvas with black color
	// Draw point
	.pDrawCanPoint_Fast	= DrawCan6Point_Fast,		// Draw point (does not check clipping limits)
	.pDrawCanPoint		= DrawCan6Point,		// Draw point (checks clipping limits)
	.pDrawCanPointInv_Fast	= DrawCan6PointInv_Fast,	// Draw point inverted (does not check clipping limits)
	.pDrawCanPointInv	= DrawCan6PointInv,		// Draw point inverted (checks clipping limits)
	// Get point
	.pDrawCanGetPoint_Fast	= DrawCan6GetPoint_Fast,	// Get point (does not check clipping limits)
	.pDrawCanGetPoint	= DrawCan6GetPoint,		// Get point (checks clipping limits)
	// Draw rectangle
	.pDrawCanRect_Fast	= DrawCan6Rect_Fast,		// Draw rectangle (does not check clipping limits)
	.pDrawCanRect		= DrawCan6Rect,			// Draw rectangle (checks clipping limits)
	.pDrawCanRectInv_Fast	= DrawCan6RectInv_Fast,		// Draw rectangle inverted (does not check clipping limits)
	.pDrawCanRectInv	= DrawCan6RectInv,		// Draw rectangle inverted (checks clipping limits
	// Draw horizontal line
	.pDrawCanHLine_Fast	= DrawCan6HLine_Fast,		// Draw horizontal line (does not check clipping limits)
	.pDrawCanHLine		= DrawCan6HLine,		// Draw horizontal line (checks clipping limits; negative width flip line)
	.pDrawCanHLineInv_Fast	= DrawCan6HLineInv_Fast,	// Draw horizontal line inverted (does not check clipping limits)
	.pDrawCanHLineInv	= DrawCan6HLineInv,		// Draw horizontal line inverted (checks clipping limits; negative width flip line)
	// Draw vertical line
	.pDrawCanVLine_Fast	= DrawCan6VLine_Fast,		// Draw vertical line (does not check clipping limits)
	.pDrawCanVLine		= DrawCan6VLine,		// Draw vertical line (checks clipping limits; negative height flip line)
	.pDrawCanVLineInv_Fast	= DrawCan6VLineInv_Fast,	// Draw vertical line inverted (does not check clipping limits)
	.pDrawCanVLineInv	= DrawCan6VLineInv,		// Draw vertical line inverted (checks clipping limits; negative height flip line)
	// Draw frame
	.pDrawCanFrame_Fast	= DrawCan6Frame_Fast,		// Draw 1-pixel frame (does not check clipping limits)
	.pDrawCanFrame		= DrawCan6Frame,		// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameInv_Fast	= DrawCan6FrameInv_Fast,	// Draw 1-pixel frame inverted (does not check clipping limits)
	.pDrawCanFrameInv	= DrawCan6FrameInv,		// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameW_Fast	= DrawCan6FrameW_Fast,		// Draw thick frame (does not check clipping limits)
	.pDrawCanFrameW		= DrawCan6FrameW,		// Draw thick frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameWInv_Fast	= DrawCan6FrameWInv_Fast,	// Draw thick frame inverted (does not check clipping limits)
	.pDrawCanFrameWInv	= DrawCan6FrameWInv,		// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
	// Draw line
	.pDrawCanLineOver_Fast	= DrawCan6LineOver_Fast,	// Draw line with overlapped pixels (does not check clipping limits)
	.pDrawCanLine_Fast	= DrawCan6Line_Fast,		// Draw line (does not check clipping limits)
	.pDrawCanLineOver	= DrawCan6LineOver,		// Draw line with overlapped pixels (checks clipping limits)
	.pDrawCanLine		= DrawCan6Line,			// Draw line (checks clipping limits)
	.pDrawCanLineOverInv_Fast = DrawCan6LineOverInv_Fast,	// Draw line inverted with overlapped pixels (does not check clipping limits)
	.pDrawCanLineInv_Fast	= DrawCan6LineInv_Fast,		// Draw line inverted (does not check clipping limits)
	.pDrawCanLineOverInv	= DrawCan6LineOverInv,		// Draw line inverted with overlapped pixels (checks clipping limits)
	.pDrawCanLineInv	= DrawCan6LineInv,		// Draw line inverted (checks clipping limits)
	.pDrawCanLineW_Fast	= DrawCan6LineW_Fast,		// Draw thick line (does not check clipping limits)
	.pDrawCanLineW		= DrawCan6LineW,		// Draw thick line (checks clipping limits)
	.pDrawCanLineWInv_Fast	= DrawCan6LineWInv_Fast,	// Draw thick line inverted (does not check clipping limits)
	.pDrawCanLineWInv	= DrawCan6LineWInv,		// Draw thick line inverted (checks clipping limits)
	// Draw round (filled circle)
	.pDrawCanRound_Fast	= DrawCan6Round_Fast,		// Draw round (does not check clipping limits)
	.pDrawCanRound		= DrawCan6Round,		// Draw round (checks clipping limits)
	.pDrawCanRoundInv_Fast	= DrawCan6RoundInv_Fast,	// Draw round inverted (does not check clipping limits)
	.pDrawCanRoundInv	= DrawCan6RoundInv,		// Draw round inverted (checks clipping limits)
	// Draw circle
	.pDrawCanCircle_Fast	= DrawCan6Circle_Fast,		// Draw circle or arc (does not check clipping limits)
	.pDrawCanCircle		= DrawCan6Circle,		// Draw circle or arc (checks clipping limits)
	.pDrawCanCircleInv_Fast	= DrawCan6CircleInv_Fast,	// Draw circle or arc inverted (does not check clipping limits)
	.pDrawCanCircleInv	= DrawCan6CircleInv,		// Draw circle or arc inverted (checks clipping limits)
	// Draw ring
	.pDrawCanRing_Fast	= DrawCan6Ring_Fast,		// Draw ring (does not check clipping limits)
	.pDrawCanRing		= DrawCan6Ring,			// Draw ring (checks clipping limits)
	.pDrawCanRingInv_Fast	= DrawCan6RingInv_Fast,		// Draw ring inverted (does not check clipping limits)
	.pDrawCanRingInv	= DrawCan6RingInv,		// Draw ring inverted (checks clipping limits)
	// Draw triangle
	.pDrawCanTriangle_Fast	= DrawCan6Triangle_Fast,	// Draw triangle (does not check clipping limits)
	.pDrawCanTriangle	= DrawCan6Triangle,		// Draw triangle (checks clipping limits)
	.pDrawCanTriangleInv_Fast = DrawCan6TriangleInv_Fast,	// Draw triangle inverted (does not check clipping limits)
	.pDrawCanTriangleInv	= DrawCan6TriangleInv,		// Draw triangle inverted (checks clipping limits)
	// Draw fill area
	.pDrawCanFill		= DrawCan6Fill,			// Draw fill area
	// Draw character
	.pDrawCanChar_Fast	= DrawCan6Char_Fast,		// Draw character with transparent background (does not check clipping limits)
	.pDrawCanChar		= DrawCan6Char,			// Draw character with transparent background (checks clipping limits)
	.pDrawCanCharBg_Fast	= DrawCan6CharBg_Fast,		// Draw character with background (does not check clipping limits)
	.pDrawCanCharBg		= DrawCan6CharBg,		// Draw character with background (checks clipping limits)
	.pDrawCanCharInv_Fast	= DrawCan6CharInv_Fast,		// Draw character inverted (does not check clipping limits)
	.pDrawCanCharInv	= DrawCan6CharInv,		// Draw character inverted (checks clipping limits)
	// Draw text
	.pDrawCanText_Fast	= DrawCan6Text_Fast,		// Draw text with transparent background (does not check clipping limits)
	.pDrawCanText		= DrawCan6Text,			// Draw text with transparent background (checks clipping limits)
	.pDrawCanTextBg_Fast	= DrawCan6TextBg_Fast,		// Draw text with background (does not check clipping limits)
	.pDrawCanTextBg		= DrawCan6TextBg,		// Draw text with background (checks clipping limits)
	.pDrawCanTextInv_Fast	= DrawCan6TextInv_Fast,		// Draw text inverted (does not check clipping limits)
	.pDrawCanTextInv	= DrawCan6TextInv,		// Draw text inverted (checks clipping limits)
	// Draw ellipse
	.pDrawCanEllipse_Fast	= DrawCan6Ellipse_Fast,		// Draw ellipse (does not check clipping limits)
	.pDrawCanEllipse	= DrawCan6Ellipse,		// Draw ellipse (checks clipping limits)
	.pDrawCanEllipseInv_Fast = DrawCan6EllipseInv_Fast,	// Draw ellipse inverted (does not check clipping limits)
	.pDrawCanEllipseInv	= DrawCan6EllipseInv,		// Draw ellipse inverted (checks clipping limits)
	// Draw filled ellipse
	.pDrawCanFillEllipse_Fast = DrawCan6FillEllipse_Fast,	// Draw filled ellipse (does not check clipping limits)
	.pDrawCanFillEllipse	= DrawCan6FillEllipse,		// Draw filled ellipse (checks clipping limits)
	.pDrawCanFillEllipseInv_Fast = DrawCan6FillEllipseInv_Fast, // Draw filled ellipse inverted (does not check clipping limits)
	.pDrawCanFillEllipseInv = DrawCan6FillEllipseInv,	// Draw filled ellipse inverte (checks clipping limits)
	// Draw image
	.pDrawCanImg		= DrawCan6Img,			// Draw image with the same format as destination
	.pDrawCanImgInv		= DrawCan6ImgInv,		// Draw image inverted with the same format as destination
	.pDrawCanBlit		= DrawCan6Blit,			// Draw transparent image with the same format as destination
	.pDrawCanBlitInv	= DrawCan6BlitInv,		// Draw transparent image inverted with the same format as destination
	.pDrawCanBlitSubs	= DrawCan6BlitSubs,		// Draw transparent image with the same format as destination, with substitute color
	.pDrawCanGetImg		= DrawCan6GetImg,		// Get image from canvas to buffer
	.pDrawCanImgMat		= DrawCan6ImgMat,		// Draw image with 2D transformation matrix
	// colors
	.pColRgb		= Draw6ColRgb,			// convert RGB888 color to pixel color
#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
	.pColRand		= Draw6ColRand,			// random color
#endif
	.col_black		= COL6_BLACK,			// black color
	.col_blue		= COL6_BLUE,			// blue color
	.col_green		= COL6_GREEN,			// green color
	.col_cyan		= COL6_CYAN,			// cyan color
	.col_red		= COL6_RED,			// red color
	.col_magenta		= COL6_MAGENTA,			// magenta color
	.col_yellow		= COL6_YELLOW,			// yellow color
	.col_white		= COL6_WHITE,			// white color
	.col_gray		= COL6_GRAY,			// gray color
	.col_dkblue		= COL6_DKBLUE,			// dark blue color
	.col_dkgreen		= COL6_DKGREEN,			// dark green color
	.col_dkcyan		= COL6_DKCYAN,			// dark cyan color
	.col_dkred		= COL6_DKRED,			// dark red color
	.col_dkmagenta		= COL6_DKMAGENTA,		// dark magenta color
	.col_dkyellow		= COL6_DKYELLOW,		// dark yellow color
	.col_dkwhite		= COL6_DKWHITE,			// dark white color
	.col_dkgray		= COL6_DKGRAY,			// dark gray color
	.col_ltblue		= COL6_LTBLUE,			// light blue color
	.col_ltgreen		= COL6_LTGREEN,			// light green color
	.col_ltcyan		= COL6_LTCYAN,			// light cyan color
	.col_ltred		= COL6_LTRED,			// light red color
	.col_ltmagenta		= COL6_LTMAGENTA,		// light magenta color
	.col_ltyellow		= COL6_LTYELLOW,		// light yellow color
	.col_ltgray		= COL6_LTGRAY,			// light gray color
	.col_azure		= COL6_AZURE,			// azure blue color
	.col_orange		= COL6_ORANGE,			// orange color
	.col_brown		= COL6_BROWN,			// brown color
};

#endif // USE_DRAWCAN0

#endif // USE_DRAWCAN
