
// ****************************************************************************
//
//                      Drawing to canvas at 3-bit format
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

#if USE_DRAWCAN	&& USE_DRAWCAN3		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#include "../../_font/_include.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_float.h"
#include "../inc/lib_text.h"
#include "../inc/lib_rand.h"
#include "../inc/lib_drawcan.h"
#include "../inc/lib_drawcan3.h"

#ifndef WIDTH
#define WIDTH		320		// display width
#endif

#ifndef HEIGHT
#define HEIGHT		240		// display height
#endif

// default drawing canvas for 3-bit format
sDrawCan DrawCan3 = {
	// format
	.format = DRAWCAN_FORMAT_3,	// u8	format;		// canvas format CANVAS_*
	.colbit = 3,			// u8	colbit;		// number of bits per pixel
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
	.wb = (WIDTH+9)/10*4,		// s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	.striph = HEIGHT,		// s16	striph;		// strip height (number of lines)

	// print
	.printposnum = WIDTH/FONTW,	// u16	printposnum;	// number of text positions per row (= w / fontw)
	.printrownum = HEIGHT/FONTH,	// u16	printrownum;	// number of text rows (= h / fonth)
	.printpos = 0,			// u16	printpos;	// console print character position
	.printrow = 0,			// u16	printrow;	// console print character row
	.printcol = COL3_WHITE,		// u16	printcol;	// console print color

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
	.drawfnc = NULL, //&DrawCan3Fnc,	// const struct sDrawCanFnc_* drawfnc; // drawing functions
};

// current drawing canvas for 3-bit format
sDrawCan* pDrawCan3 = &DrawCan3;

// set default drawing canvas for 3-bit format
void SetDrawCan3(sDrawCan* can) { pDrawCan3 = can; }

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw3Pitch(int w) { return (w + 9)/10*4; }

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw3MaxWidth(int pitch) { return pitch*10/4; }

// convert RGB888 color to 3-bit pixel color RGB111
u16 Draw3ColRgb(u8 r, u8 g, u8 b) { return COLOR3(r,g,b); }

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 3-bit pixel color RGB111
u16 Draw3ColRand() { return COL3_RANDOM; }
#endif

// ----------------------------------------------------------------------------
//                            Clear canvas
// ----------------------------------------------------------------------------

// Clear canvas with color
void DrawCan3ClearCol(sDrawCan* can, u16 col8)
{
	u8 col = (u8)col8;
	int x = can->clipx1;
	int y = can->clipy1;
	int w = can->clipx2 - x;
	int h = can->clipy2 - y;
	DrawCan3Rect(can, x, y, w, h, col);
}
void Draw3ClearCol(u16 col) { DrawCan3ClearCol(pDrawCan3, col); }

// Clear canvas with black color
void DrawCan3Clear(sDrawCan* can) { DrawCan3ClearCol(can, COL3_BLACK); }
void Draw3Clear() { DrawCan3Clear(pDrawCan3); }

// ----------------------------------------------------------------------------
//                               Draw point
// ----------------------------------------------------------------------------

// Draw point (does not check clipping limits)
void DrawCan3Point_Fast(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// draw pixel
	int x2 = x/10;
	x -= x2*10;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	x *= 3;
	*d = (*d & ~((u32)0x07<<x)) | ((u32)col<<x);
}
void Draw3Point_Fast(int x, int y, u16 col) { DrawCan3Point_Fast(pDrawCan3, x, y, col); }

// Draw point (checks clipping limits)
void DrawCan3Point(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// draw pixel
	int x2 = x/10;
	x -= x2*10;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	x *= 3;
	*d = (*d & ~((u32)0x07<<x)) | ((u32)col<<x);
}
void Draw3Point(int x, int y, u16 col) { DrawCan3Point(pDrawCan3, x, y, col); }

// Draw point inverted (does not check clipping limits)
void DrawCan3PointInv_Fast(sDrawCan* can, int x, int y)
{
	// invert pixel
	int x2 = x/10;
	x -= x2*10;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	*d ^= (u32)0x07<<(x*3);
}
void Draw3PointInv_Fast(int x, int y) { DrawCan3PointInv_Fast(pDrawCan3, x, y); }

// Draw point inverted (checks clipping limits)
void DrawCan3PointInv(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// invert pixel
	int x2 = x/10;
	x -= x2*10;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	*d ^= (u32)0x07<<(x*3);
}
void Draw3PointInv(int x, int y) { DrawCan3PointInv(pDrawCan3, x, y); }

// ----------------------------------------------------------------------------
//                               Get point
// ----------------------------------------------------------------------------

// Get point (does not check clipping limits)
u16 DrawCan3GetPoint_Fast(sDrawCan* can, int x, int y)
{
	int x2 = x/10;
	x -= x2*10;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	x *= 3;
	return (u8)((*d >> x) & 0x07);
}
u16 Draw3GetPoint_Fast(int x, int y) { return DrawCan3GetPoint_Fast(pDrawCan3, x, y); }

// Get point (checks clipping limits)
u16 DrawCan3GetPoint(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return COL3_BLACK;

	// get pixel
	int x2 = x/10;
	x -= x2*10;
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	x *= 3;
	return (u8)((*d >> x) & 0x07);
}
u16 Draw3GetPoint(int x, int y) { return DrawCan3GetPoint(pDrawCan3, x, y); }

// ----------------------------------------------------------------------------
//                            Draw rectangle
// ----------------------------------------------------------------------------

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan3Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col8)
{
	u8 col = (u8)col8;

	// split width to whole and fraction part
	int x2 = x/10;
	x -= x2*10; // start X relative 0..4

	// prepare destination address
	int wb = can->wb;
	u32* d0 = (u32*)&can->buf[x2*4 + (y - can->basey)*wb];
	u32* d;
	wb /= 4;

	// prepare multiply color
	u32 colcol = col|(col<<3)|(col<<6)|(col<<9)|(col<<12);
	colcol = colcol | (colcol << 15);

	// mask of first pixels
	int x3 = x*3;
	u32 m1 = ~((u32)-1 << x3); // mask of first pixels
	u32 col1 = colcol << x3; // color of first pixels

	// width of inner pixels
	if (x > 0) w -= 10 - x;
	int w2 = w/10;

	// mask of last pixels
	u32 m2 = (u32)-1;
	u32 col2 = 0;
	if (w >= 0) // width is large enough
	{
		w -= w2*10;
		w *= 3;
		m2 = (u32)-1 << w;
		col2 = colcol >> (30 - w);
	}
	else // last pixels are not valid
	{
		w += 10; // end of first pixels
		w *= 3;
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
void Draw3Rect_Fast(int x, int y, int w, int h, u16 col) { DrawCan3Rect_Fast(pDrawCan3, x, y, w, h, col); }

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan3Rect(sDrawCan* can, int x, int y, int w, int h, u16 col8)
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
	DrawCan3Rect_Fast(can, x, y, w, h, col);
}
void Draw3Rect(int x, int y, int w, int h, u16 col) { DrawCan3Rect(pDrawCan3, x, y, w, h, col); }

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan3RectInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// split width to whole and fraction part
	int x2 = x/10;
	x -= x2*10; // start X relative 0..4

	// prepare destination address
	int wb = can->wb;
	u32* d0 = (u32*)&can->buf[x2*4 + (y - can->basey)*wb];
	u32* d;
	wb /= 4;

	// mask of first pixels
	u32 m1 = (u32)-1 << (x*3); // mask of first pixels

	// width of inner pixels
	if (x > 0) w -= 10 - x;
	int w2 = w/10;

	// mask of last pixels
	u32 m2 = (u32)-1;
	if (w >= 0) // width is large enough
	{
		w -= w2*10;
		m2 = ~((u32)-1 << (w*3));
	}
	else // last pixels are not valid
	{
		w += 10; // end of first pixels
		u32 m3 = (u32)-1 << (w*3);
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
void Draw3RectInv_Fast(int x, int y, int w, int h) { DrawCan3RectInv_Fast(pDrawCan3, x, y, w, h); }

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan3RectInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan3RectInv_Fast(can, x, y, w, h);
}
void Draw3RectInv(int x, int y, int w, int h) { DrawCan3RectInv(pDrawCan3, x, y, w, h); }

// ----------------------------------------------------------------------------
//                          Draw horizontal line
// ----------------------------------------------------------------------------

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan3HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col8)
{
	u8 col = (u8)col8;

	// split width to whole and fraction part
	int x2 = x/10;
	x -= x2*10; // start X relative 0..4

	// prepare destination address
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];

	// prepare multiply color
	u32 colcol = col|(col<<3)|(col<<6)|(col<<9)|(col<<12);
	colcol = colcol | (colcol << 15);

	// mask of first pixels
	int x3 = x*3;
	u32 m1 = ~((u32)-1 << x3); // mask of first pixels
	u32 col1 = colcol << x3; // color of first pixels

	// width of inner pixels
	if (x > 0) w -= 10 - x;
	int w2 = w/10;

	// mask of last pixels
	u32 m2 = (u32)-1;
	u32 col2 = 0;
	if (w >= 0) // width is large enough
	{
		w -= w2*10;
		w *= 3;
		m2 = (u32)-1 << w;
		col2 = colcol >> (30 - w);
	}
	else // last pixels are not valid
	{
		w += 10; // end of first pixels
		w *= 3;
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
void Draw3HLine_Fast(int x, int y, int w, u16 col) { DrawCan3HLine_Fast(pDrawCan3, x, y, w, col); }

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan3HLine(sDrawCan* can, int x, int y, int w, u16 col)
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
	DrawCan3HLine_Fast(can, x, y, w, col);
}
void Draw3HLine(int x, int y, int w, u16 col) { DrawCan3HLine(pDrawCan3, x, y, w, col); }

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan3HLineInv_Fast(sDrawCan* can, int x, int y, int w)
{
	// split width to whole and fraction part
	int x2 = x/10;
	x -= x2*10; // start X relative 0..4

	// prepare destination address
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];

	// mask of first pixels
	u32 m1 = (u32)-1 << (x*3); // mask of first pixels

	// width of inner pixels
	if (x > 0) w -= 10 - x;
	int w2 = w/10;

	// mask of last pixels
	u32 m2 = (u32)-1;
	if (w >= 0) // width is large enough
	{
		w -= w2*10;
		m2 = ~((u32)-1 << (w*3));
	}
	else // last pixels are not valid
	{
		w += 10; // end of first pixels
		u32 m3 = (u32)-1 << (w*3);
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
void Draw3HLineInv_Fast(int x, int y, int w) { DrawCan3HLineInv_Fast(pDrawCan3, x, y, w); }

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan3HLineInv(sDrawCan* can, int x, int y, int w)
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
	DrawCan3HLineInv_Fast(can, x, y, w);
}
void Draw3HLineInv(int x, int y, int w) { DrawCan3HLineInv(pDrawCan3, x, y, w); }

// ----------------------------------------------------------------------------
//                           Draw vertical line
// ----------------------------------------------------------------------------

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan3VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col8)
{
	u8 col = (u8)col8;

	// draw
	int x2 = x/10;
	x -= x2*10; // start X relative 0..4
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	int wb = can->wb/4;
	x *= 3;
	u32 m = ~((u32)0x07 << x);
	u32 col2 = (u32)col << x;
	for (; h > 0; h--)
	{
		*d = (*d & m) | col2;
		d += wb;
	}
}
void Draw3VLine_Fast(int x, int y, int h, u16 col) { DrawCan3VLine_Fast(pDrawCan3, x, y, h, col); }

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan3VLine(sDrawCan* can, int x, int y, int h, u16 col)
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
	DrawCan3VLine_Fast(can, x, y, h, col);
}
void Draw3VLine(int x, int y, int h, u16 col) { DrawCan3VLine(pDrawCan3, x, y, h, col); }

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan3VLineInv_Fast(sDrawCan* can, int x, int y, int h)
{
	// invert
	int x2 = x/10;
	x -= x2*10; // start X relative 0..4
	u32* d = (u32*)&can->buf[x2*4 + (y - can->basey)*can->wb];
	int wb = can->wb/4;
	x *= 3;
	u32 m = (u32)0x07 << x;
	for (; h > 0; h--)
	{
		*d ^= m;
		d += wb;
	}
}
void Draw3VLineInv_Fast(int x, int y, int h) { DrawCan3VLineInv_Fast(pDrawCan3, x, y, h); }

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan3VLineInv(sDrawCan* can, int x, int y, int h)
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
	DrawCan3VLineInv_Fast(can, x, y, h);
}
void Draw3VLineInv(int x, int y, int h) { DrawCan3VLineInv(pDrawCan3, x, y, h); }

// ----------------------------------------------------------------------------
//                           Draw frame
// ----------------------------------------------------------------------------

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan3Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
{
	// bottom line (dark)
	DrawCan3HLine_Fast(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan3VLine_Fast(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan3HLine_Fast(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan3VLine_Fast(can, x, y+1, h-2, col_light);
	}
}
void Draw3Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan3Frame_Fast(pDrawCan3, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan3Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
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
	DrawCan3HLine(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan3VLine(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan3HLine(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan3VLine(can, x, y+1, h-2, col_light);
	}
}
void Draw3Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan3Frame(pDrawCan3, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan3FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// bottom line
	DrawCan3HLineInv_Fast(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan3VLineInv_Fast(can, x+w-1, y, h-1);

		// top line
		DrawCan3HLineInv_Fast(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan3VLineInv_Fast(can, x, y+1, h-2);
	}
}
void Draw3FrameInv_Fast(int x, int y, int w, int h) { DrawCan3FrameInv_Fast(pDrawCan3, x, y, w, h); }

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan3FrameInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan3HLineInv(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan3VLineInv(can, x+w-1, y, h-1);

		// top line
		DrawCan3HLineInv(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan3VLineInv(can, x, y+1, h-2);
	}
}
void Draw3FrameInv(int x, int y, int w, int h) { DrawCan3FrameInv(pDrawCan3, x, y, w, h); }

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan3FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan3Rect_Fast(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan3Rect_Fast(can, x, y, w, thick, col);

		// bottom line
		DrawCan3Rect_Fast(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan3Rect_Fast(can, x, y, thick, h, col);

		// right line
		DrawCan3Rect_Fast(can, x+w-thick, y, thick, h, col);
	}
}
void Draw3FrameW_Fast(int x, int y, int w, int h, u16 col, int thick) { DrawCan3FrameW_Fast(pDrawCan3, x, y, w, h, col, thick); }

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan3FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
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
		DrawCan3Rect(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan3Rect(can, x, y, w, thick, col);

		// bottom line
		DrawCan3Rect(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan3Rect(can, x, y, thick, h, col);

		// right line
		DrawCan3Rect(can, x+w-thick, y, thick, h, col);
	}
}
void Draw3FrameW(int x, int y, int w, int h, u16 col, int thick) { DrawCan3Frame(pDrawCan3, x, y, w, h, col, thick); }

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan3FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan3RectInv_Fast(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan3RectInv_Fast(can, x, y, w, thick);

		// bottom line
		DrawCan3RectInv_Fast(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan3RectInv_Fast(can, x, y, thick, h);

		// right line
		DrawCan3RectInv_Fast(can, x+w-thick, y, thick, h);
	}
}
void Draw3FrameWInv_Fast(int x, int y, int w, int h, int thick) { DrawCan3FrameWInv_Fast(pDrawCan3, x, y, w, h, thick); }

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan3FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick)
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
		DrawCan3RectInv(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan3RectInv(can, x, y, w, thick);

		// bottom line
		DrawCan3RectInv(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan3RectInv(can, x, y, thick, h);

		// right line
		DrawCan3RectInv(can, x+w-thick, y, thick, h);
	}
}
void Draw3FrameWInv(int x, int y, int w, int h, int thick) { DrawCan3FrameWInv(pDrawCan3, x, y, w, h, thick); }

// ----------------------------------------------------------------------------
//                           Draw line
// ----------------------------------------------------------------------------

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan3LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
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
	int xint = x1/10;
	int xfrac = x1 - xint*10; // start X relative 0..9

	// destination address
	u32* d = (u32*)&can->buf[xint*4 + (y1 - can->basey)*wb];
	ddy /= 4;

	// draw first pixel
	*d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3));

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
			if (xfrac == 10)
			{
				xfrac = 0;
				d++;
			}
			if (xfrac == -1)
			{
				xfrac = 9;
				d--;
			}

			if (p > 0)
			{
				if (over) *d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3)); // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			*d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3));
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
				if (over) *d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3)); // draw overlapped pixel
				x1 += sx;
				xfrac += sx;
				if (xfrac == 10)
				{
					xfrac = 0;
					d++;
				}
				if (xfrac == -1)
				{
					xfrac = 9;
					d--;
				}
				p -= dy;
			}
			p += m;
			*d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3));
		}
	}
}
void Draw3LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan3LineOver_Fast(pDrawCan3, x1, y1, x2, y2, col, over); }

// Draw line (does not check clipping limits)
void DrawCan3Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan3LineOver_Fast(pDrawCan3, x1, y1, x2, y2, col, False); }
void Draw3Line_Fast(int x1, int y1, int x2, int y2, u16 col) { Draw3LineOver_Fast(x1, y1, x2, y2, col, False); }

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan3LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
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
	int xint = x1/10;
	int xfrac = x1 - xint*10; // start X relative 0..9

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u32* d = (u32*)&can->buf[xint*4 + (y1 - can->basey)*wb];
	ddy /= 4;

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3));

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
			if (xfrac == 10)
			{
				xfrac = 0;
				d++;
			}
			if (xfrac == -1)
			{
				xfrac = 9;
				d--;
			}

			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3));
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3));
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3));
				x1 += sx;
				xfrac += sx;
				if (xfrac == 10)
				{
					xfrac = 0;
					d++;
				}
				if (xfrac == -1)
				{
					xfrac = 9;
					d--;
				}

				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = (*d & ~((u32)0x07<<(xfrac*3))) | ((u32)col<<(xfrac*3));
		}
	}
}
void Draw3LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan3LineOver(pDrawCan3, x1, y1, x2, y2, col, over); }

// Draw line (checks clipping limits)
void DrawCan3Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan3LineOver(can, x1, y1, x2, y2, col, False); }
void Draw3Line(int x1, int y1, int x2, int y2, u16 col) { DrawCan3Line(pDrawCan3, x1, y1, x2, y2, col); }

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan3LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	int xint = x1/10;
	int xfrac = x1 - xint*10; // start X relative 0..9

	// destination address
	u32* d = (u32*)&can->buf[xint*4 + (y1 - can->basey)*wb];
	ddy /= 4;

	// draw first pixel
	*d ^= (u32)0x07<<(xfrac*3);

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
			if (xfrac == 10)
			{
				xfrac = 0;
				d++;
			}
			if (xfrac == -1)
			{
				xfrac = 9;
				d--;
			}

			if (p > 0)
			{
				if (over) *d ^= (u32)0x07<<(xfrac*3); // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			*d ^= (u32)0x07<<(xfrac*3);
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
				if (over) *d ^= (u32)0x07<<(xfrac*3); // draw overlapped pixel
				x1 += sx;
				xfrac += sx;
				if (xfrac == 10)
				{
					xfrac = 0;
					d++;
				}
				if (xfrac == -1)
				{
					xfrac = 9;
					d--;
				}
				p -= dy;
			}
			p += m;
			*d ^= (u32)0x07<<(xfrac*3);
		}
	}
}
void Draw3LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over) { DrawCan3LineOverInv_Fast(pDrawCan3, x1, y1, x2, y2, over); }

// Draw line inverted (does not check clipping limits)
void DrawCan3LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan3LineOverInv_Fast(can, x1, y1, x2, y2, False); }
void Draw3LineInv_Fast(int x1, int y1, int x2, int y2) { DrawCan3LineOverInv_Fast(pDrawCan3, x1, y1, x2, y2, False); }

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan3LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	int xint = x1/10;
	int xfrac = x1 - xint*10; // start X relative 0..9

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u32* d = (u32*)&can->buf[xint*4 + (y1 - can->basey)*wb];
	ddy /= 4;

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x07<<(xfrac*3);

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
			if (xfrac == 10)
			{
				xfrac = 0;
				d++;
			}
			if (xfrac == -1)
			{
				xfrac = 9;
				d--;
			}
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x07<<(xfrac*3);
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x07<<(xfrac*3);
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x07<<(xfrac*3);
				x1 += sx;
				xfrac += sx;
				if (xfrac == 10)
				{
					xfrac = 0;
					d++;
				}
				if (xfrac == -1)
				{
					xfrac = 9;
					d--;
				}
				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d ^= (u32)0x07<<(xfrac*3);
		}
	}
}
void Draw3LineOverInv(int x1, int y1, int x2, int y2, Bool over) { DrawCan3LineOverInv(pDrawCan3, x1, y1, x2, y2, over); }

// Draw line inverted (checks clipping limits)
void DrawCan3LineInv(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan3LineOverInv(can, x1, y1, x2, y2, False); }
void Draw3LineInv(int x1, int y1, int x2, int y2) { DrawCan3LineOverInv(pDrawCan3, x1, y1, x2, y2, False); }

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan3LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan3LineOver_Fast(can, x1, y1, x2, y2, col, False);
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
		DrawCan3Round_Fast(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan3Round_Fast(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan3LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan3LineOver_Fast(can, x1, y1, x2, y2, col, over);
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
		DrawCan3LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan3LineOver_Fast(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw3LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan3LineW_Fast(pDrawCan3, x1, y1, x2, y2, col, thick, round); }

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan3LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan3LineOver(can, x1, y1, x2, y2, col, False);
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
		DrawCan3Round(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan3Round(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan3LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan3LineOver(can, x1, y1, x2, y2, col, over);
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
		DrawCan3LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan3LineOver(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw3LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan3LineW(pDrawCan3, x1, y1, x2, y2, col, thick, round); }

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan3LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan3LineOverInv_Fast(can, x1, y1, x2, y2, False);
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
		DrawCan3LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan3LineOverInv_Fast(can, x1, y1, x2, y2, over);
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
		DrawCan3LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan3LineOverInv_Fast(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw3LineWInv_Fast(int x1, int y1, int x2, int y2, int thick) { DrawCan3LineWInv_Fast(pDrawCan3, x1, y1, x2, y2, thick); }

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan3LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan3LineOverInv(can, x1, y1, x2, y2, False);
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
		DrawCan3LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan3LineOverInv(can, x1, y1, x2, y2, over);
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
		DrawCan3LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan3LineOverInv(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw3LineWInv(int x1, int y1, int x2, int y2, int thick) { DrawCan3LineWInv(pDrawCan3, x1, y1, x2, y2, thick); }

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
void DrawCan3Round_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
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
		int xx = (x+x1)/10;
		int x0 = ((x+x1) - xx*10)*3;
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
				if ((x*x + y*y) <= r2) *s = (*s & ~((u32)0x07<<xx)) | ((u32)col<<xx);
				xx += 3;
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

		int xx = (x+x1)/10;
		int x0 = ((x+x1) - xx*10)*3;
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
				if ((x*(x-1) + y*(y-1)) < r2) *s = (*s & ~((u32)0x07<<xx)) | ((u32)col<<xx);
				xx += 3;
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
void Draw3Round_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan3Round_Fast(pDrawCan3, x, y, d, col, mask); }

// Draw round (checks clipping limits)
void DrawCan3Round(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
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
		int xx = (x+x1)/10;
		int x0 = ((x+x1) - xx*10)*3;
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
				if ((x*x + y*y) <= r2) *s = (*s & ~((u32)0x07<<xx)) | ((u32)col<<xx);
				xx += 3;
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
		int xx = (x+x1)/10;
		int x0 = ((x+x1) - xx*10)*3;
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
				if ((x*(x-1) + y*(y-1)) < r2) *s = (*s & ~((u32)0x07<<xx)) | ((u32)col<<xx);
				xx += 3;
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
void Draw3Round(int x, int y, int d, u16 col, u8 mask) { DrawCan3Round(pDrawCan3, x, y, d, col, mask); }

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan3RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
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
		int xx = (x+x1)/10;
		int x0 = ((x+x1) - xx*10)*3;
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
				if ((x*x + y*y) <= r2) *s ^= (u32)0x07<<xx;
				xx += 3;
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

		int xx = (x+x1)/10;
		int x0 = ((x+x1) - xx*10)*3;
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
				if ((x*(x-1) + y*(y-1)) < r2) *s ^= (u32)0x07<<xx;
				xx += 3;
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
void Draw3RoundInv_Fast(int x, int y, int d, u8 mask) { DrawCan3RoundInv_Fast(pDrawCan3, x, y, d, mask); }

// Draw round inverted (checks clipping limits)
void DrawCan3RoundInv(sDrawCan* can, int x, int y, int d, u8 mask)
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
		int xx = (x+x1)/10;
		int x0 = ((x+x1) - xx*10)*3;
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
				if ((x*x + y*y) <= r2) *s ^= (u32)0x07<<xx;
				xx += 3;
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
		int xx = (x+x1)/10;
		int x0 = ((x+x1) - xx*10)*3;
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
				if ((x*(x-1) + y*(y-1)) < r2) *s ^= (u32)0x07<<xx;
				xx += 3;
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
void Draw3RoundInv(int x, int y, int d, u8 mask) { DrawCan3RoundInv(pDrawCan3, x, y, d, mask); }

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
		int xint = x2/10;			\
		int xfrac = (x2 - xint*10)*3;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s = (*s & ~((u32)0x07<<xfrac)) | ((u32)col<<xfrac); }

#define DRAWCANCIRCLE_PIX(xxx,yyy,mmm)			\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xint = x2/10;			\
		int xfrac = (x2 - xint*10)*3;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s = (*s & ~((u32)0x07<<xfrac)) | ((u32)col<<xfrac); } }

#define DRAWCANCIRCLE_PIXINVFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xint = x2/10;			\
		int xfrac = (x2 - xint*10)*3;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s ^= (u32)0x07<<xfrac; }

#define DRAWCANCIRCLE_PIXINV(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xint = x2/10;			\
		int xfrac = (x2 - xint*10)*3;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s ^= (u32)0x07<<xfrac; } }

//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan3Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan3Point_Fast(can, x, y, col);
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
void Draw3Circle_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan3Circle_Fast(pDrawCan3, x, y, d, col, mask); }

// Draw circle or arc (checks clipping limits)
void DrawCan3Circle(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan3Point(can, x, y, col);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan3Circle_Fast(can, x, y, d, col, mask);
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
void Draw3Circle(int x, int y, int d, u16 col, u8 mask) { DrawCan3Circle(pDrawCan3, x, y, d, col, mask); }

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan3CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan3PointInv_Fast(can, x, y);
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
void Draw3CircleInv_Fast(int x, int y, int d, u8 mask) { DrawCan3CircleInv_Fast(pDrawCan3, x, y, d, mask); }

// Draw circle or arc inverted (checks clipping limits)
void DrawCan3CircleInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan3PointInv(can, x, y);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan3CircleInv_Fast(can, x, y, d, mask);
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
void Draw3CircleInv(int x, int y, int d, u8 mask) { DrawCan3CircleInv(pDrawCan3, x, y, d, mask); }

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
void DrawCan3Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan3Round_Fast(can, x, y, d, col, mask);
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
		DrawCan3Circle_Fast(can, x, y, d, col, mask2);
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
					xint = xxx/10;
					xfrac = (xxx - xint*10)*3;
					s = (u32*)&s0[xint*4];
					*s = (*s & ~((u32)0x07<<xfrac)) | ((u32)col<<xfrac);
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
					xint = xxx/10;
					xfrac = (xxx - xint*10)*3;
					s = (u32*)&s0[xint*4];
					*s = (*s & ~((u32)0x07<<xfrac)) | ((u32)col<<xfrac);
				}
			}
			s0 += wb;
		}
	}
}
void Draw3Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan3Ring_Fast(pDrawCan3, x, y, d, din, col, mask); }

// Draw ring (checks clipping limits)
void DrawCan3Ring(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	u8 col = (u8)col8;
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan3Round(can, x, y, d, col, mask);
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
		DrawCan3Circle(can, x, y, d, col, mask2);
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
					xint = xxx/10;
					xfrac = (xxx - xint*10)*3;
					s = (u32*)&s0[xint*4];
					*s = (*s & ~((u32)0x07<<xfrac)) | ((u32)col<<xfrac);
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
					xint = xxx/10;
					xfrac = (xxx - xint*10)*3;
					s = (u32*)&s0[xint*4];
					*s = (*s & ~((u32)0x07<<xfrac)) | ((u32)col<<xfrac);
				}
			}
			s0 += wb;
		}
	}
}
void Draw3Ring(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan3Ring(pDrawCan3, x, y, d, din, col, mask); }

// Draw ring inverted (does not check clipping limits)
void DrawCan3RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan3RoundInv_Fast(can, x, y, d, mask);
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
		DrawCan3CircleInv_Fast(can, x, y, d, mask2);
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
					xint = xxx/10;
					xfrac = (xxx - xint*10)*3;
					s = (u32*)&s0[xint*4];
					*s ^= (u32)0x07<<xfrac;
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
					xint = xxx/10;
					xfrac = (xxx - xint*10)*3;
					s = (u32*)&s0[xint*4];
					*s ^= (u32)0x07<<xfrac;
				}
			}
			s0 += wb;
		}
	}
}
void Draw3RingInv_Fast(int x, int y, int d, int din, u8 mask) { DrawCan3RingInv_Fast(pDrawCan3, x, y, d, din, mask); }

// Draw ring inverted (checks clipping limits)
void DrawCan3RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan3RoundInv(can, x, y, d, mask);
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
		DrawCan3CircleInv(can, x, y, d, mask2);
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
					xint = xxx/10;
					xfrac = (xxx - xint*10)*3;
					s = (u32*)&s0[xint*4];
					*s ^= (u32)0x07<<xfrac;
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
					xint = xxx/10;
					xfrac = (xxx - xint*10)*3;
					s = (u32*)&s0[xint*4];
					*s ^= (u32)0x07<<xfrac;
				}
			}
			s0 += wb;
		}
	}
}
void Draw3RingInv(int x, int y, int d, int din, u8 mask) { DrawCan3RingInv(pDrawCan3, x, y, d, din, mask); }

// ----------------------------------------------------------------------------
//                              Draw triangle
// ----------------------------------------------------------------------------

// Draw triangle (does not check clipping limits)
void DrawCan3Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
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
			if (k > 0) DrawCan3HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan3HLine_Fast(can, xmin, y, k, col);
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
			if (k > 0) DrawCan3HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan3HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan3HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan3HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw3Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan3Triangle_Fast(pDrawCan3, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle (checks clipping limits)
void DrawCan3Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
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
		DrawCan3Triangle_Fast(can, x1, y1, x2, y2, x3, y3, col);
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
			if (k > 0) DrawCan3HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan3HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan3HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan3HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan3HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan3HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw3Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan3Triangle(pDrawCan3, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle inverted (does not check clipping limits)
void DrawCan3TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
			if (k > 0) DrawCan3HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan3HLineInv_Fast(can, xmin, y, k);
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
			if (k > 0) DrawCan3HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan3HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan3HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan3HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw3TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan3TriangleInv_Fast(pDrawCan3, x1, y1, x2, y2, x3, y3); }

// Draw triangle inverted (checks clipping limits)
void DrawCan3TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
		DrawCan3TriangleInv_Fast(can, x1, y1, x2, y2, x3, y3);
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
			if (k > 0) DrawCan3HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan3HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan3HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan3HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan3HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan3HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw3TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan3TriangleInv(pDrawCan3, x1, y1, x2, y2, x3, y3); }

// ----------------------------------------------------------------------------
//                              Draw fill area
// ----------------------------------------------------------------------------

// Draw fill sub-area (internal function)
void _DrawCan3SubFill(sDrawCan* can, int x, int y, u8 fnd, u8 col)
{
	// save start position -> x2
	int x2 = x;

	// fill start point
	DrawCan3Point_Fast(can, x, y, col);

	// update dirty area in Y direction
	if (y < can->dirtyy1) can->dirtyy1 = y;
	if (y >= can->dirtyy2) can->dirtyy2 = y+1;

	// search start of segment in LEFT-X direction -> x1
	int clipx1 = can->clipx1;
	while ((x > clipx1) && ((u8)DrawCan3GetPoint_Fast(can, x-1, y) == fnd))
	{
		x--;
		DrawCan3Point_Fast(can, x, y, col);
	}
	int x1 = x;

	// search end of segment in RIGHT-X direction -> x2
	int clipx2 = can->clipx2-1;
	while ((x2 < clipx2) && ((u8)DrawCan3GetPoint_Fast(can, x2+1, y) == fnd))
	{
		x2++;
		DrawCan3Point_Fast(can, x2, y, col);
	}

	// search segments in UP-Y direction
	if (y > can->clipy1)
	{
		x = x1;
		y--;
		while (x <= x2)
		{
			if ((u8)DrawCan3GetPoint_Fast(can, x, y) == fnd) _DrawCan3SubFill(can, x, y, fnd, col);
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
			if ((u8)DrawCan3GetPoint_Fast(can, x, y) == fnd) _DrawCan3SubFill(can, x, y, fnd, col);
			x++;
		}
	}

	// update dirty area in X direction
	if (x1 < can->dirtyx1) can->dirtyx1 = x1;
	if (x2 >= can->dirtyx2) can->dirtyx2 = x2+1;
}

// Draw fill area
void DrawCan3Fill(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check clipping
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// get color to search
	u8 fnd = (u8)DrawCan3GetPoint_Fast(can, x, y);

	// fill if color is different
	if (fnd != col) _DrawCan3SubFill(can, x, y, fnd, col);
}
void Draw3Fill(int x, int y, u16 col) { DrawCan3Fill(pDrawCan3, x, y, col); }

// ----------------------------------------------------------------------------
//                             Draw charactter
// ----------------------------------------------------------------------------
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw3SelFont(const u8* font, u8 fontw, u8 fonth) { DrawCanSelFont(pDrawCan3, font, fontw, fonth); }

void Draw3SelFont8x8()		{ DrawCanSelFont8x8(pDrawCan3); }		// sans-serif bold, height 8 lines
void Draw3SelFont8x14()		{ DrawCanSelFont8x14(pDrawCan3); }		// sans-serif bold, height 14 lines
void Draw3SelFont8x16()		{ DrawCanSelFont8x16(pDrawCan3); }		// sans-serif bold, height 16 lines
void Draw3SelFont8x14Serif()	{ DrawCanSelFont8x14Serif(pDrawCan3); }		// serif bold, height 14 lines
void Draw3SelFont8x16Serif()	{ DrawCanSelFont8x16Serif(pDrawCan3); }		// serif bold, height 16 lines
void Draw3SelFont6x8()		{ DrawCanSelFont6x8(pDrawCan3); }		// condensed font, width 6 pixels, height 8 lines
void Draw3SelFont8x8Game()	{ DrawCanSelFont8x8Game(pDrawCan3); }		// game font, height 8 lines
void Draw3SelFont8x8Ibm()	{ DrawCanSelFont8x8Ibm(pDrawCan3); }		// IBM charset font, height 8 lines
void Draw3SelFont8x14Ibm()	{ DrawCanSelFont8x14Ibm(pDrawCan3); }		// IBM charset font, height 14 lines
void Draw3SelFont8x16Ibm()	{ DrawCanSelFont8x16Ibm(pDrawCan3); }		// IBM charset font, height 16 lines
void Draw3SelFont8x8IbmThin()	{ DrawCanSelFont8x8IbmThin(pDrawCan3); }	// IBM charset thin font, height 8 lines
void Draw3SelFont8x8Italic()	{ DrawCanSelFont8x8Italic(pDrawCan3); }		// italic, height 8 lines
void Draw3SelFont8x8Thin()	{ DrawCanSelFont8x8Thin(pDrawCan3); }		// thin font, height 8 lines
void Draw3SelFont5x8()		{ DrawCanSelFont5x8(pDrawCan3); }		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan3Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
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
				if ((b & 0x80) != 0) DrawCan3Point_Fast(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan3Point_Fast(can, x, y, col);
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
void Draw3Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan3Char_Fast(pDrawCan3, ch, x, y, col, scalex, scaley); }

// Draw character with transparent background (checks clipping limits)
void DrawCan3Char(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
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
		DrawCan3Char_Fast(can, ch, x, y, col, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan3Point(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan3Point(can, x, y, col);
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
void Draw3Char(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan3Char(pDrawCan3, ch, x, y, col, scalex, scaley); }

// Draw character with background (does not check clipping limits)
void DrawCan3CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
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
				DrawCan3Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				DrawCan3Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw3CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan3CharBg_Fast(pDrawCan3, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character with background (checks clipping limits)
void DrawCan3CharBg(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
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
		DrawCan3CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);
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
				DrawCan3Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				DrawCan3Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw3CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan3CharBg(pDrawCan3, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character inverted (does not check clipping limits)
void DrawCan3CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
				if ((b & 0x80) != 0) DrawCan3PointInv_Fast(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan3PointInv_Fast(can, x, y);
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
void Draw3CharInv_Fast(char ch, int x, int y, int scalex, int scaley) { DrawCan3CharInv_Fast(pDrawCan3, ch, x, y, scalex, scaley); }

// Draw character inverted (checks clipping limits)
void DrawCan3CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
		DrawCan3CharInv_Fast(can, ch, x, y, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan3PointInv(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan3PointInv(can, x, y);
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
void Draw3CharInv(char ch, int x, int y, int scalex, int scaley) { DrawCan3CharInv(pDrawCan3, ch, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                             Draw text
// ----------------------------------------------------------------------------
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan3Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan3Char_Fast(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw3Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan3Text_Fast(pDrawCan3, text, len, x, y, col, scalex, scaley); }

// Draw text with transparent background (checks clipping limits)
void DrawCan3Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan3Char(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw3Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan3Text(pDrawCan3, text, len, x, y, col, scalex, scaley); }

// Draw text with background (does not check clipping limits)
void DrawCan3TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan3CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw3TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan3TextBg_Fast(pDrawCan3, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text with background (checks clipping limits)
void DrawCan3TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan3CharBg(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw3TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan3TextBg(pDrawCan3, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text inverted (does not check clipping limits)
void DrawCan3TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan3CharInv_Fast(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw3TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan3TextInv_Fast(pDrawCan3, text, len, x, y, scalex, scaley); }

// Draw text inverted (checks clipping limits)
void DrawCan3TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan3CharInv(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw3TextInv(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan3TextInv(pDrawCan3, text, len, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                               Draw ellipse
// ----------------------------------------------------------------------------
// Using mid-point ellipse drawing algorithm

#define DRAWCANELLIPSE_PIXFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xint = x2/10;			\
		int xfrac = (x2 - xint*10)*3;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s = (*s & ~((u32)0x07<<xfrac)) | ((u32)col<<xfrac); }

#define DRAWCANELLIPSE_PIX(xxx,yyy,mmm)			\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xint = x2/10;			\
		int xfrac = (x2 - xint*10)*3;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s = (*s & ~((u32)0x07<<xfrac)) | ((u32)col<<xfrac); } }

#define DRAWCANELLIPSE_PIXINVFAST(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		int xint = x2/10;			\
		int xfrac = (x2 - xint*10)*3;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s ^= (u32)0x07<<xfrac; }

#define DRAWCANELLIPSE_PIXINV(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)				\
	{	int x2 = x+(xxx);			\
		int y2 = y+(yyy);			\
		if ((x2 >= clipx1) &&			\
			(x2 < clipx2) &&		\
			(y2 >= clipy1) &&		\
			(y2 < clipy2)) {		\
		int xint = x2/10;			\
		int xfrac = (x2 - xint*10)*3;		\
		s = (u32*)&s0[xint*4 + (y2-basey)*wb];	\
		*s ^= (u32)0x07<<xfrac; } }

//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse
//		DRAWCAN_ELLIPSE_ARC0	= draw arc 0..90 deg
//		DRAWCAN_ELLIPSE_ARC1	= draw arc 90..180 deg
//		DRAWCAN_ELLIPSE_ARC2	= draw arc 180..270 deg
//		DRAWCAN_ELLIPSE_ARC3	= draw arc 270..360 deg
//		DRAWCAN_ELLIPSE_ALL	= draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan3Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
void Draw3Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan3Ellipse_Fast(pDrawCan3, x, y, dx, dy, col, mask); }

// Draw ellipse (checks clipping limits)
void DrawCan3Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
		DrawCan3Ellipse_Fast(can, x, y, dx, dy, col, mask);
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
void Draw3Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan3Ellipse(pDrawCan3, x, y, dx, dy, col, mask); }

// Draw ellipse inverted (does not check clipping limits)
void DrawCan3EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
void Draw3EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan3EllipseInv_Fast(pDrawCan3, x, y, dx, dy, mask); }

// Draw ellipse inverted (checks clipping limits)
void DrawCan3EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
		DrawCan3EllipseInv_Fast(can, x, y, dx, dy, mask);
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
void Draw3EllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan3EllipseInv(pDrawCan3, x, y, dx, dy, mask); }

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
void DrawCan3FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	int xxx = (x+x1)/10;
	int x0 = ((x+x1) - xxx*10)*3;
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
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = (*s & ~((u32)0x07<<xxx)) | ((u32)col<<xxx);
			xxx += 3;
			if (xxx == 30)
			{
				s++;
				xxx = 0;
			}
		}
		s0 += wb;
	}
}
void Draw3FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan3FillEllipse_Fast(pDrawCan3, x, y, dx, dy, col, mask); }

// Draw filled ellipse (checks clipping limits)
void DrawCan3FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	int xxx = (x+x1)/10;
	int x0 = ((x+x1) - xxx*10)*3;
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
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = (*s & ~((u32)0x07<<xxx)) | ((u32)col<<xxx);
			xxx += 3;
			if (xxx == 30)
			{
				s++;
				xxx = 0;
			}
		}
		s0 += wb;
	}
}
void Draw3FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan3FillEllipse(pDrawCan3, x, y, dx, dy, col, mask); }

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan3FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	int xxx = (x+x1)/10;
	int x0 = ((x+x1) - xxx*10)*3;
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
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s ^= (u32)0x07<<xxx;
			xxx += 3;
			if (xxx == 30)
			{
				s++;
				xxx = 0;
			}
		}
		s0 += wb;
	}
}
void Draw3FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan3FillEllipseInv_Fast(pDrawCan3, x, y, dx, dy, mask); }

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan3FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	int xxx = (x+x1)/10;
	int x0 = ((x+x1) - xxx*10)*3;
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
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s ^= (u32)0x07<<xxx;
			xxx += 3;
			if (xxx == 30)
			{
				s++;
				xxx = 0;
			}
		}
		s0 += wb;
	}
}
void Draw3FillEllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan3FillEllipseInv(pDrawCan3, x, y, dx, dy, mask); }

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
void DrawCan3Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*10/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/10;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*10)*3;

	// source address
	int xsint = xs/10;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*10)*3;

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
			c = (u8)((*s >> xxs) & 0x07);
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			*d = (*d & ~((u32)0x07<<xxd)) | ((u32)c<<xxd);
			xxd += 3;
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
void Draw3Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan3Img(pDrawCan3, xd, yd, src, xs, ys, w, h, wbs); }

// Draw image inverted with the same format as destination
void DrawCan3ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*10/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/10;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*10)*3;

	// source address
	int xsint = xs/10;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*10)*3;

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
			c = (u8)((*s >> xxs) & 0x07);
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			*d ^= (u32)c<<xxd;
			xxd += 3;
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
void Draw3ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan3ImgInv(pDrawCan3, xd, yd, src, xs, ys, w, h, wbs); }

// Draw transparent image with the same format as destination
void DrawCan3Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs*10/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/10;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*10)*3;

	// source address
	int xsint = xs/10;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*10)*3;

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
			c = (u8)((*s >> xxs) & 0x07);
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			if (c != col) *d = (*d & ~((u32)0x07<<xxd)) | ((u32)c<<xxd);
			xxd += 3;
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
void Draw3Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan3Blit(pDrawCan3, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan3BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8, u16 fnd8, u16 subs8)
{
	u8 col = (u8)col8;
	u8 fnd = (u8)fnd8;
	u8 subs = (u8)subs8;

	// width of source image
	int ws = wbs*10/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/10;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*10)*3;

	// source address
	int xsint = xs/10;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*10)*3;

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
			c = (u8)((*s >> xxs) & 0x07);
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			if (c != col)
			{
				if (c == fnd) c = subs;
				*d = (*d & ~((u32)0x07<<xxd)) | ((u32)c<<xxd);
			}
			xxd += 3;
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
void Draw3BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { DrawCan3BlitSubs(pDrawCan3, xd, yd, src, xs, ys, w, h, wbs, col, fnd, subs); }

// Draw transparent image inverted with the same format as destination
void DrawCan3BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs*10/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	int xdint = xd/10;
	u32* d0 = (u32*)&can->buf[xdint*4 + (yd - can->basey)*wbd];
	int xdfrac = (xd - xdint*10)*3;

	// source address
	int xsint = xs/10;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*10)*3;

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
			c = (u8)((*s >> xxs) & 0x07);
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			if (c != col) *d ^= (u32)c<<xxd;
			xxd += 3;
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
void Draw3BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan3BlitInv(pDrawCan3, xd, yd, src, xs, ys, w, h, wbs, col); }

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
void DrawCan3GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd)
{
	// width of destination image
	int wd = wbd*10/4;

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
	int xsint = xs/10;
	const u32* s0 = (const u32*)&can->buf[xsint*4 + (ys - can->basey)*wbs];
	int xsfrac = (xs - xsint*10)*3;

	// destination address
	int xdint = xd/10;
	u32* d0 = (u32*)&((u8*)dst)[xdint*4 + yd*wbd];
	int xdfrac = (xd - xdint*10)*3;

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
			c = (u8)((*s >> xxs) & 0x07);
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}

			*d = (*d & ~((u32)0x07<<xxd)) | ((u32)c<<xxd);
			xxd += 3;
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
void Draw3GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { DrawCan3GetImg(pDrawCan3, xs, ys, w, h, dst, xd, yd, wbd); }

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
void DrawCan3ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src_img, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
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

					xb = x2/10;
					xc = (x2 - xb*10)*3;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x07;
					xb = xd/10;
					xc = (xd - xb*10)*3;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x07<<xc)) | ((u32)pix<<xc);
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

					xb = x2/10;
					xc = (x2 - xb*10)*3;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x07;
					xb = xd/10;
					xc = (xd - xb*10)*3;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x07<<xc)) | ((u32)pix<<xc);
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
						xb = x2/10;
						xc = (x2 - xb*10)*3;
						s = (const u32*)&src[xb*4 + y2*wbs];
						pix = (*s >> xc) & 0x07;
						xb = xd/10;
						xc = (xd - xb*10)*3;
						d = (u32*)&d0[xb*4];
						*d = (*d & ~((u32)0x07<<xc)) | ((u32)pix<<xc);
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
					xb = xd/10;
					xc = (xd - xb*10)*3;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x07<<xc)) | ((u32)col<<xc);
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

					xb = x2/10;
					xc = (x2 - xb*10)*3;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x07;
					xb = xd/10;
					xc = (xd - xb*10)*3;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x07<<xc)) | ((u32)pix<<xc);
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
					xb = xd/10;
					xc = (xd - xb*10)*3;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x07<<xc)) | ((u32)color<<xc);
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
						xb = x2/10;
						xc = (x2 - xb*10)*3;
						s = (const u32*)&src[xb*4 + y2*wbs];
						pix = (*s >> xc) & 0x07;
					}
					else
						pix = color;
					xb = xd/10;
					xc = (xd - xb*10)*3;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x07<<xc)) | ((u32)pix<<xc);
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
						xb = x2/10;
						xc = (x2 - xb*10)*3;
						s = (const u32*)&src[xb*4 + y2*wbs];
						pix = (*s >> xc) & 0x07;

						if (pix != color)
						{
							xb = xd/10;
							xc = (xd - xb*10)*3;
							d = (u32*)&d0[xb*4];
							*d = (*d & ~((u32)0x07<<xc)) | ((u32)pix<<xc);
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

					xb = x2/10;
					xc = (x2 - xb*10)*3;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x07;
					xb = xd/10;
					xc = (xd - xb*10)*3;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x07<<xc)) | ((u32)pix<<xc);
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

					xb = x2/10;
					xc = (x2 - xb*10)*3;
					s = (const u32*)&src[xb*4 + y2*wbs];
					pix = (*s >> xc) & 0x07;
					xb = xd/10;
					xc = (xd - xb*10)*3;
					d = (u32*)&d0[xb*4];
					*d = (*d & ~((u32)0x07<<xc)) | ((u32)pix<<xc);
					xd++;
				}
				y0++;
				d0 += wbd;
			}
		}
	}
}
void Draw3ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan3ImgMat(pDrawCan3, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

#undef DRAWCAN_IMGLIMIT

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// drawing function interface
const sDrawCanFnc DrawCan3Fnc = {
	.pDrawPitch		= Draw3Pitch,			// calculate pitch of graphics line from image width
	.pDrawMaxWidth		= Draw3MaxWidth,		// calculate max. width of image from the pitch
	// Clear canvas
	.pDrawCanClearCol	= DrawCan3ClearCol,		// Clear canvas with color
	.pDrawCanClear		= DrawCan3Clear,		// Clear canvas with black color
	// Draw point
	.pDrawCanPoint_Fast	= DrawCan3Point_Fast,		// Draw point (does not check clipping limits)
	.pDrawCanPoint		= DrawCan3Point,		// Draw point (checks clipping limits)
	.pDrawCanPointInv_Fast	= DrawCan3PointInv_Fast,	// Draw point inverted (does not check clipping limits)
	.pDrawCanPointInv	= DrawCan3PointInv,		// Draw point inverted (checks clipping limits)
	// Get point
	.pDrawCanGetPoint_Fast	= DrawCan3GetPoint_Fast,	// Get point (does not check clipping limits)
	.pDrawCanGetPoint	= DrawCan3GetPoint,		// Get point (checks clipping limits)
	// Draw rectangle
	.pDrawCanRect_Fast	= DrawCan3Rect_Fast,		// Draw rectangle (does not check clipping limits)
	.pDrawCanRect		= DrawCan3Rect,			// Draw rectangle (checks clipping limits)
	.pDrawCanRectInv_Fast	= DrawCan3RectInv_Fast,		// Draw rectangle inverted (does not check clipping limits)
	.pDrawCanRectInv	= DrawCan3RectInv,		// Draw rectangle inverted (checks clipping limits
	// Draw horizontal line
	.pDrawCanHLine_Fast	= DrawCan3HLine_Fast,		// Draw horizontal line (does not check clipping limits)
	.pDrawCanHLine		= DrawCan3HLine,		// Draw horizontal line (checks clipping limits; negative width flip line)
	.pDrawCanHLineInv_Fast	= DrawCan3HLineInv_Fast,	// Draw horizontal line inverted (does not check clipping limits)
	.pDrawCanHLineInv	= DrawCan3HLineInv,		// Draw horizontal line inverted (checks clipping limits; negative width flip line)
	// Draw vertical line
	.pDrawCanVLine_Fast	= DrawCan3VLine_Fast,		// Draw vertical line (does not check clipping limits)
	.pDrawCanVLine		= DrawCan3VLine,		// Draw vertical line (checks clipping limits; negative height flip line)
	.pDrawCanVLineInv_Fast	= DrawCan3VLineInv_Fast,	// Draw vertical line inverted (does not check clipping limits)
	.pDrawCanVLineInv	= DrawCan3VLineInv,		// Draw vertical line inverted (checks clipping limits; negative height flip line)
	// Draw frame
	.pDrawCanFrame_Fast	= DrawCan3Frame_Fast,		// Draw 1-pixel frame (does not check clipping limits)
	.pDrawCanFrame		= DrawCan3Frame,		// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameInv_Fast	= DrawCan3FrameInv_Fast,	// Draw 1-pixel frame inverted (does not check clipping limits)
	.pDrawCanFrameInv	= DrawCan3FrameInv,		// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameW_Fast	= DrawCan3FrameW_Fast,		// Draw thick frame (does not check clipping limits)
	.pDrawCanFrameW		= DrawCan3FrameW,		// Draw thick frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameWInv_Fast	= DrawCan3FrameWInv_Fast,	// Draw thick frame inverted (does not check clipping limits)
	.pDrawCanFrameWInv	= DrawCan3FrameWInv,		// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
	// Draw line
	.pDrawCanLineOver_Fast	= DrawCan3LineOver_Fast,	// Draw line with overlapped pixels (does not check clipping limits)
	.pDrawCanLine_Fast	= DrawCan3Line_Fast,		// Draw line (does not check clipping limits)
	.pDrawCanLineOver	= DrawCan3LineOver,		// Draw line with overlapped pixels (checks clipping limits)
	.pDrawCanLine		= DrawCan3Line,			// Draw line (checks clipping limits)
	.pDrawCanLineOverInv_Fast = DrawCan3LineOverInv_Fast,	// Draw line inverted with overlapped pixels (does not check clipping limits)
	.pDrawCanLineInv_Fast	= DrawCan3LineInv_Fast,		// Draw line inverted (does not check clipping limits)
	.pDrawCanLineOverInv	= DrawCan3LineOverInv,		// Draw line inverted with overlapped pixels (checks clipping limits)
	.pDrawCanLineInv	= DrawCan3LineInv,		// Draw line inverted (checks clipping limits)
	.pDrawCanLineW_Fast	= DrawCan3LineW_Fast,		// Draw thick line (does not check clipping limits)
	.pDrawCanLineW		= DrawCan3LineW,		// Draw thick line (checks clipping limits)
	.pDrawCanLineWInv_Fast	= DrawCan3LineWInv_Fast,	// Draw thick line inverted (does not check clipping limits)
	.pDrawCanLineWInv	= DrawCan3LineWInv,		// Draw thick line inverted (checks clipping limits)
	// Draw round (filled circle)
	.pDrawCanRound_Fast	= DrawCan3Round_Fast,		// Draw round (does not check clipping limits)
	.pDrawCanRound		= DrawCan3Round,		// Draw round (checks clipping limits)
	.pDrawCanRoundInv_Fast	= DrawCan3RoundInv_Fast,	// Draw round inverted (does not check clipping limits)
	.pDrawCanRoundInv	= DrawCan3RoundInv,		// Draw round inverted (checks clipping limits)
	// Draw circle
	.pDrawCanCircle_Fast	= DrawCan3Circle_Fast,		// Draw circle or arc (does not check clipping limits)
	.pDrawCanCircle		= DrawCan3Circle,		// Draw circle or arc (checks clipping limits)
	.pDrawCanCircleInv_Fast	= DrawCan3CircleInv_Fast,	// Draw circle or arc inverted (does not check clipping limits)
	.pDrawCanCircleInv	= DrawCan3CircleInv,		// Draw circle or arc inverted (checks clipping limits)
	// Draw ring
	.pDrawCanRing_Fast	= DrawCan3Ring_Fast,		// Draw ring (does not check clipping limits)
	.pDrawCanRing		= DrawCan3Ring,			// Draw ring (checks clipping limits)
	.pDrawCanRingInv_Fast	= DrawCan3RingInv_Fast,		// Draw ring inverted (does not check clipping limits)
	.pDrawCanRingInv	= DrawCan3RingInv,		// Draw ring inverted (checks clipping limits)
	// Draw triangle
	.pDrawCanTriangle_Fast	= DrawCan3Triangle_Fast,	// Draw triangle (does not check clipping limits)
	.pDrawCanTriangle	= DrawCan3Triangle,		// Draw triangle (checks clipping limits)
	.pDrawCanTriangleInv_Fast = DrawCan3TriangleInv_Fast,	// Draw triangle inverted (does not check clipping limits)
	.pDrawCanTriangleInv	= DrawCan3TriangleInv,		// Draw triangle inverted (checks clipping limits)
	// Draw fill area
	.pDrawCanFill		= DrawCan3Fill,			// Draw fill area
	// Draw character
	.pDrawCanChar_Fast	= DrawCan3Char_Fast,		// Draw character with transparent background (does not check clipping limits)
	.pDrawCanChar		= DrawCan3Char,			// Draw character with transparent background (checks clipping limits)
	.pDrawCanCharBg_Fast	= DrawCan3CharBg_Fast,		// Draw character with background (does not check clipping limits)
	.pDrawCanCharBg		= DrawCan3CharBg,		// Draw character with background (checks clipping limits)
	.pDrawCanCharInv_Fast	= DrawCan3CharInv_Fast,		// Draw character inverted (does not check clipping limits)
	.pDrawCanCharInv	= DrawCan3CharInv,		// Draw character inverted (checks clipping limits)
	// Draw text
	.pDrawCanText_Fast	= DrawCan3Text_Fast,		// Draw text with transparent background (does not check clipping limits)
	.pDrawCanText		= DrawCan3Text,			// Draw text with transparent background (checks clipping limits)
	.pDrawCanTextBg_Fast	= DrawCan3TextBg_Fast,		// Draw text with background (does not check clipping limits)
	.pDrawCanTextBg		= DrawCan3TextBg,		// Draw text with background (checks clipping limits)
	.pDrawCanTextInv_Fast	= DrawCan3TextInv_Fast,		// Draw text inverted (does not check clipping limits)
	.pDrawCanTextInv	= DrawCan3TextInv,		// Draw text inverted (checks clipping limits)
	// Draw ellipse
	.pDrawCanEllipse_Fast	= DrawCan3Ellipse_Fast,		// Draw ellipse (does not check clipping limits)
	.pDrawCanEllipse	= DrawCan3Ellipse,		// Draw ellipse (checks clipping limits)
	.pDrawCanEllipseInv_Fast = DrawCan3EllipseInv_Fast,	// Draw ellipse inverted (does not check clipping limits)
	.pDrawCanEllipseInv	= DrawCan3EllipseInv,		// Draw ellipse inverted (checks clipping limits)
	// Draw filled ellipse
	.pDrawCanFillEllipse_Fast = DrawCan3FillEllipse_Fast,	// Draw filled ellipse (does not check clipping limits)
	.pDrawCanFillEllipse	= DrawCan3FillEllipse,		// Draw filled ellipse (checks clipping limits)
	.pDrawCanFillEllipseInv_Fast = DrawCan3FillEllipseInv_Fast, // Draw filled ellipse inverted (does not check clipping limits)
	.pDrawCanFillEllipseInv = DrawCan3FillEllipseInv,	// Draw filled ellipse inverte (checks clipping limits)
	// Draw image
	.pDrawCanImg		= DrawCan3Img,			// Draw image with the same format as destination
	.pDrawCanImgInv		= DrawCan3ImgInv,		// Draw image inverted with the same format as destination
	.pDrawCanBlit		= DrawCan3Blit,			// Draw transparent image with the same format as destination
	.pDrawCanBlitInv	= DrawCan3BlitInv,		// Draw transparent image inverted with the same format as destination
	.pDrawCanBlitSubs	= DrawCan3BlitSubs,		// Draw transparent image with the same format as destination, with substitute color
	.pDrawCanGetImg		= DrawCan3GetImg,		// Get image from canvas to buffer
	.pDrawCanImgMat		= DrawCan3ImgMat,		// Draw image with 2D transformation matrix
	// colors
	.pColRgb		= Draw3ColRgb,			// convert RGB888 color to pixel color
#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
	.pColRand		= Draw3ColRand,			// random color
#endif
	.col_black		= COL3_BLACK,			// black color
	.col_blue		= COL3_BLUE,			// blue color
	.col_green		= COL3_GREEN,			// green color
	.col_cyan		= COL3_CYAN,			// cyan color
	.col_red		= COL3_RED,			// red color
	.col_magenta		= COL3_MAGENTA,			// magenta color
	.col_yellow		= COL3_YELLOW,			// yellow color
	.col_white		= COL3_WHITE,			// white color
	.col_gray		= COL3_GRAY,			// gray color
	.col_dkblue		= COL3_BLUE,			// dark blue color
	.col_dkgreen		= COL3_GREEN,			// dark green color
	.col_dkcyan		= COL3_CYAN,			// dark cyan color
	.col_dkred		= COL3_RED,			// dark red color
	.col_dkmagenta		= COL3_MAGENTA,			// dark magenta color
	.col_dkyellow		= COL3_YELLOW,			// dark yellow color
	.col_dkwhite		= COL3_WHITE,			// dark white color
	.col_dkgray		= COL3_GRAY,			// dark gray color
	.col_ltblue		= COL3_BLUE,			// light blue color
	.col_ltgreen		= COL3_GREEN,			// light green color
	.col_ltcyan		= COL3_CYAN,			// light cyan color
	.col_ltred		= COL3_RED,			// light red color
	.col_ltmagenta		= COL3_MAGENTA,			// light magenta color
	.col_ltyellow		= COL3_YELLOW,			// light yellow color
	.col_ltgray		= COL3_WHITE,			// light gray color
	.col_azure		= COL3_CYAN,			// azure blue color
	.col_orange		= COL3_YELLOW,			// orange color
	.col_brown		= COL3_YELLOW,			// brown color
};

#endif // USE_DRAWCAN0

#endif // USE_DRAWCAN
