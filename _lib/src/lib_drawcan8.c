
// ****************************************************************************
//
//                      Drawing to canvas at 8-bit format
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

#if USE_DRAWCAN	&& USE_DRAWCAN8		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#include "../../_font/_include.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_float.h"
#include "../inc/lib_text.h"
#include "../inc/lib_rand.h"
#include "../inc/lib_drawcan.h"
#include "../inc/lib_drawcan8.h"

#ifndef WIDTH
#define WIDTH		320		// display width
#endif

#ifndef HEIGHT
#define HEIGHT		240		// display height
#endif

// default drawing canvas for 8-bit format
sDrawCan DrawCan8 = {
	// format
	.format = DRAWCAN_FORMAT_8,	// u8	format;		// canvas format CANVAS_*
	.colbit = 8,			// u8	colbit;		// number of bits per pixel
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
	.wb = (WIDTH+3)&~3,		// s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	.striph = HEIGHT,		// s16	striph;		// strip height (number of lines)

	// print
	.printposnum = WIDTH/FONTW,	// u16	printposnum;	// number of text positions per row (= w / fontw)
	.printrownum = HEIGHT/FONTH,	// u16	printrownum;	// number of text rows (= h / fonth)
	.printpos = 0,			// u16	printpos;	// console print character position
	.printrow = 0,			// u16	printrow;	// console print character row
	.printcol = COL8_WHITE,		// u16	printcol;	// console print color

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
	.drawfnc = NULL, // &DrawCan8Fnc,	// const struct sDrawCanFnc_* drawfnc; // drawing functions
};

// current drawing canvas for 8-bit format
sDrawCan* pDrawCan8 = &DrawCan8;

// set default drawing canvas for 8-bit format
void SetDrawCan8(sDrawCan* can) { pDrawCan8 = can; }

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw8Pitch(int w) { return (w + 3) & ~3; }

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw8MaxWidth(int pitch) { return pitch; }

// convert RGB888 color to 8-bit pixel color RGB332
u16 Draw8ColRgb(u8 r, u8 g, u8 b) { return COLOR8(r, g, b); }

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 8-bit pixel color RGB332
u16 Draw8ColRand() { return COL8_RANDOM; }
#endif

// ----------------------------------------------------------------------------
//                            Clear canvas
// ----------------------------------------------------------------------------

// Clear canvas with color
void DrawCan8ClearCol(sDrawCan* can, u16 col8)
{
	u8 col = (u8)col8;
	int x = can->clipx1;
	int y = can->clipy1;
	int w = can->clipx2 - x;
	int h = can->clipy2 - y;
	DrawCan8Rect(can, x, y, w, h, col);
}
void Draw8ClearCol(u16 col) { DrawCan8ClearCol(pDrawCan8, col); }

// Clear canvas with black color
void DrawCan8Clear(sDrawCan* can) { DrawCan8ClearCol(can, COL8_BLACK); }
void Draw8Clear() { DrawCan8Clear(pDrawCan8); }

// ----------------------------------------------------------------------------
//                               Draw point
// ----------------------------------------------------------------------------

// Draw point (does not check clipping limits)
void DrawCan8Point_Fast(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// draw pixel
	can->buf[x + (y - can->basey)*can->wb] = col;
}
void Draw8Point_Fast(int x, int y, u16 col) { DrawCan8Point_Fast(pDrawCan8, x, y, col); }

// Draw point (checks clipping limits)
void DrawCan8Point(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// draw pixel
	can->buf[x + (y - can->basey)*can->wb] = col;
}
void Draw8Point(int x, int y, u16 col) { DrawCan8Point(pDrawCan8, x, y, col); }

// Draw point inverted (does not check clipping limits)
void DrawCan8PointInv_Fast(sDrawCan* can, int x, int y)
{
	// invert pixel
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	d[0] = ~d[0];
}
void Draw8PointInv_Fast(int x, int y) { DrawCan8PointInv_Fast(pDrawCan8, x, y); }

// Draw point inverted (checks clipping limits)
void DrawCan8PointInv(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// invert pixel
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	d[0] = ~d[0];
}
void Draw8PointInv(int x, int y) { DrawCan8PointInv(pDrawCan8, x, y); }

// ----------------------------------------------------------------------------
//                               Get point
// ----------------------------------------------------------------------------

// Get point (does not check clipping limits)
u16 DrawCan8GetPoint_Fast(sDrawCan* can, int x, int y)
{
	return can->buf[x + (y - can->basey)*can->wb];
}
u16 Draw8GetPoint_Fast(int x, int y) { return DrawCan8GetPoint_Fast(pDrawCan8, x, y); }

// Get point (checks clipping limits)
u16 DrawCan8GetPoint(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return COL8_BLACK;

	// get pixel
	return can->buf[x + (y - can->basey)*can->wb];
}
u16 Draw8GetPoint(int x, int y) { return DrawCan8GetPoint(pDrawCan8, x, y); }

// ----------------------------------------------------------------------------
//                            Draw rectangle
// ----------------------------------------------------------------------------

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan8Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col8)
{
	u8 col = (u8)col8;

	// draw
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	int wb = can->wb - w;
	int i;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--) *d++ = col;
		d += wb;
	}
}
void Draw8Rect_Fast(int x, int y, int w, int h, u16 col) { DrawCan8Rect_Fast(pDrawCan8, x, y, w, h, col); }

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan8Rect(sDrawCan* can, int x, int y, int w, int h, u16 col8)
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
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	int wb = can->wb - w;
	int i;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--) *d++ = col;
		d += wb;
	}
}
void Draw8Rect(int x, int y, int w, int h, u16 col) { DrawCan8Rect(pDrawCan8, x, y, w, h, col); }

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan8RectInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// invert
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	int wb = can->wb - w;
	int i;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--) { *d = ~*d; d++; }
		d += wb;
	}
}
void Draw8RectInv_Fast(int x, int y, int w, int h) { DrawCan8RectInv_Fast(pDrawCan8, x, y, w, h); }

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan8RectInv(sDrawCan* can, int x, int y, int w, int h)
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
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	int wb = can->wb - w;
	int i;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--) { *d = ~*d; d++; }
		d += wb;
	}
}
void Draw8RectInv(int x, int y, int w, int h) { DrawCan8RectInv(pDrawCan8, x, y, w, h); }

// ----------------------------------------------------------------------------
//                          Draw horizontal line
// ----------------------------------------------------------------------------

// Draw horizontal line (does not check clipping limits; width must be > 0))
void DrawCan8HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col8)
{
	u8 col = (u8)col8;

	// draw
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	int i;
	for (i = w; i > 0; i--) *d++ = col;
}
void Draw8HLine_Fast(int x, int y, int w, u16 col) { DrawCan8HLine_Fast(pDrawCan8, x, y, w, col); }

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan8HLine(sDrawCan* can, int x, int y, int w, u16 col8)
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
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	int i;
	for (i = w; i > 0; i--) *d++ = col;
}
void Draw8HLine(int x, int y, int w, u16 col) { DrawCan8HLine(pDrawCan8, x, y, w, col); }

// Draw horizontal line inverted (does not check clipping limits; width must be > 0))
void DrawCan8HLineInv_Fast(sDrawCan* can, int x, int y, int w)
{
	// invert
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	int i;
	for (i = w; i > 0; i--) { *d = ~*d; d++; }
}
void Draw8HLineInv_Fast(int x, int y, int w) { DrawCan8HLineInv_Fast(pDrawCan8, x, y, w); }

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan8HLineInv(sDrawCan* can, int x, int y, int w)
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
	u8* d = &can->buf[x + (y - can->basey)*can->wb];
	int i;
	for (i = w; i > 0; i--) { *d = ~*d; d++; }
}
void Draw8HLineInv(int x, int y, int w) { DrawCan8HLineInv(pDrawCan8, x, y, w); }

// ----------------------------------------------------------------------------
//                           Draw vertical line
// ----------------------------------------------------------------------------

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan8VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col8)
{
	u8 col = (u8)col8;

	// draw
	int wb = can->wb;
	u8* d = &can->buf[x + (y - can->basey)*wb];
	for (; h > 0; h--)
	{
		*d = col;
		d += wb;
	}
}
void Draw8VLine_Fast(int x, int y, int h, u16 col) { DrawCan8VLine_Fast(pDrawCan8, x, y, h, col); }

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan8VLine(sDrawCan* can, int x, int y, int h, u16 col8)
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
	int wb = can->wb;
	u8* d = &can->buf[x + (y - can->basey)*wb];
	for (; h > 0; h--)
	{
		*d = col;
		d += wb;
	}
}
void Draw8VLine(int x, int y, int h, u16 col) { DrawCan8VLine(pDrawCan8, x, y, h, col); }

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan8VLineInv_Fast(sDrawCan* can, int x, int y, int h)
{
	// invert
	int wb = can->wb;
	u8* d = &can->buf[x + (y - can->basey)*wb];
	for (; h > 0; h--)
	{
		*d = ~*d;
		d += wb;
	}
}
void Draw8VLineInv_Fast(int x, int y, int h) { DrawCan8VLineInv_Fast(pDrawCan8, x, y, h); }

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan8VLineInv(sDrawCan* can, int x, int y, int h)
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
	int wb = can->wb;
	u8* d = &can->buf[x + (y - can->basey)*wb];
	for (; h > 0; h--)
	{
		*d = ~*d;
		d += wb;
	}
}
void Draw8VLineInv(int x, int y, int h) { DrawCan8VLineInv(pDrawCan8, x, y, h); }

// ----------------------------------------------------------------------------
//                           Draw frame
// ----------------------------------------------------------------------------

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan8Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
{
	// bottom line (dark)
	DrawCan8HLine_Fast(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan8VLine_Fast(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan8HLine_Fast(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan8VLine_Fast(can, x, y+1, h-2, col_light);
	}
}
void Draw8Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan8Frame_Fast(pDrawCan8, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan8Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
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
	DrawCan8HLine(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan8VLine(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan8HLine(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan8VLine(can, x, y+1, h-2, col_light);
	}
}
void Draw8Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan8Frame(pDrawCan8, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan8FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// bottom line
	DrawCan8HLineInv_Fast(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan8VLineInv_Fast(can, x+w-1, y, h-1);

		// top line
		DrawCan8HLineInv_Fast(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan8VLineInv_Fast(can, x, y+1, h-2);
	}
}
void Draw8FrameInv_Fast(int x, int y, int w, int h) { DrawCan8FrameInv_Fast(pDrawCan8, x, y, w, h); }

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan8FrameInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan8HLineInv(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan8VLineInv(can, x+w-1, y, h-1);

		// top line
		DrawCan8HLineInv(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan8VLineInv(can, x, y+1, h-2);
	}
}
void Draw8FrameInv(int x, int y, int w, int h) { DrawCan8FrameInv(pDrawCan8, x, y, w, h); }

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan8FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan8Rect_Fast(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan8Rect_Fast(can, x, y, w, thick, col);

		// bottom line
		DrawCan8Rect_Fast(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan8Rect_Fast(can, x, y, thick, h, col);

		// right line
		DrawCan8Rect_Fast(can, x+w-thick, y, thick, h, col);
	}
}
void Draw8FrameW_Fast(int x, int y, int w, int h, u16 col, int thick) { DrawCan8FrameW_Fast(pDrawCan8, x, y, w, h, col, thick); }

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan8FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
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
		DrawCan8Rect(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan8Rect(can, x, y, w, thick, col);

		// bottom line
		DrawCan8Rect(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan8Rect(can, x, y, thick, h, col);

		// right line
		DrawCan8Rect(can, x+w-thick, y, thick, h, col);
	}
}
void Draw8FrameW(int x, int y, int w, int h, u16 col, int thick) { DrawCan8Frame(pDrawCan8, x, y, w, h, col, thick); }

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan8FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan8RectInv_Fast(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan8RectInv_Fast(can, x, y, w, thick);

		// bottom line
		DrawCan8RectInv_Fast(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan8RectInv_Fast(can, x, y, thick, h);

		// right line
		DrawCan8RectInv_Fast(can, x+w-thick, y, thick, h);
	}
}
void Draw8FrameWInv_Fast(int x, int y, int w, int h, int thick) { DrawCan8FrameWInv_Fast(pDrawCan8, x, y, w, h, thick); }

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan8FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick)
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
		DrawCan8RectInv(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan8RectInv(can, x, y, w, thick);

		// bottom line
		DrawCan8RectInv(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan8RectInv(can, x, y, thick, h);

		// right line
		DrawCan8RectInv(can, x+w-thick, y, thick, h);
	}
}
void Draw8FrameWInv(int x, int y, int w, int h, int thick) { DrawCan8FrameWInv(pDrawCan8, x, y, w, h, thick); }

// ----------------------------------------------------------------------------
//                           Draw line
// ----------------------------------------------------------------------------

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan8LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
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
	u8* d = &can->buf[x1 + (y1 - can->basey)*wb];

	// draw first pixel
	*d = col;

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			d += sx;
			if (p > 0)
			{
				if (over) *d = col; // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			*d = col;
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
				if (over) *d = col; // draw overlapped pixel
				d += sx;
				p -= dy;
			}
			p += m;
			*d = col;
		}
	}
}
void Draw8LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan8LineOver_Fast(pDrawCan8, x1, y1, x2, y2, col, over); }

// Draw line (does not check clipping limits)
void DrawCan8Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan8LineOver_Fast(pDrawCan8, x1, y1, x2, y2, col, False); }
void Draw8Line_Fast(int x1, int y1, int x2, int y2, u16 col) { Draw8LineOver_Fast(x1, y1, x2, y2, col, False); }

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan8LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
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
	u8* d = &can->buf[x1 + (y1 - can->basey)*wb];

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = col;

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			d += sx;
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = col;
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = col;
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = col;
				x1 += sx;
				d += sx;
				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = col;
		}
	}
}
void Draw8LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan8LineOver(pDrawCan8, x1, y1, x2, y2, col, over); }

// Draw line (checks clipping limits)
void DrawCan8Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan8LineOver(can, x1, y1, x2, y2, col, False); }
void Draw8Line(int x1, int y1, int x2, int y2, u16 col) { DrawCan8Line(pDrawCan8, x1, y1, x2, y2, col); }

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan8LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	u8* d = &can->buf[x1 + (y1 - can->basey)*wb];

	// draw first pixel
	*d = ~*d;

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			d += sx;
			if (p > 0)
			{
				if (over) *d = ~*d; // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			*d = ~*d;
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
				if (over) *d = ~*d; // draw overlapped pixel
				d += sx;
				p -= dy;
			}
			p += m;
			*d = ~*d;
		}
	}
}
void Draw8LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over) { DrawCan8LineOverInv_Fast(pDrawCan8, x1, y1, x2, y2, over); }

// Draw line inverted (does not check clipping limits)
void DrawCan8LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan8LineOverInv_Fast(can, x1, y1, x2, y2, False); }
void Draw8LineInv_Fast(int x1, int y1, int x2, int y2) { DrawCan8LineOverInv_Fast(pDrawCan8, x1, y1, x2, y2, False); }

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan8LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	u8* d = &can->buf[x1 + (y1 - can->basey)*wb];

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = ~*d;

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			d += sx;
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = ~*d;
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = ~*d;
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = ~*d;
				x1 += sx;
				d += sx;
				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) *d = ~*d;
		}
	}
}
void Draw8LineOverInv(int x1, int y1, int x2, int y2, Bool over) { DrawCan8LineOverInv(pDrawCan8, x1, y1, x2, y2, over); }

// Draw line inverted (checks clipping limits)
void DrawCan8LineInv(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan8LineOverInv(can, x1, y1, x2, y2, False); }
void Draw8LineInv(int x1, int y1, int x2, int y2) { DrawCan8LineOverInv(pDrawCan8, x1, y1, x2, y2, False); }

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan8LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan8LineOver_Fast(can, x1, y1, x2, y2, col, False);
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
		DrawCan8Round_Fast(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan8Round_Fast(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan8LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan8LineOver_Fast(can, x1, y1, x2, y2, col, over);
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
		DrawCan8LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan8LineOver_Fast(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw8LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan8LineW_Fast(pDrawCan8, x1, y1, x2, y2, col, thick, round); }

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan8LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan8LineOver(can, x1, y1, x2, y2, col, False);
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
		DrawCan8Round(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan8Round(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan8LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan8LineOver(can, x1, y1, x2, y2, col, over);
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
		DrawCan8LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan8LineOver(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw8LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan8LineW(pDrawCan8, x1, y1, x2, y2, col, thick, round); }

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan8LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan8LineOverInv_Fast(can, x1, y1, x2, y2, False);
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
		DrawCan8LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan8LineOverInv_Fast(can, x1, y1, x2, y2, over);
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
		DrawCan8LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan8LineOverInv_Fast(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw8LineWInv_Fast(int x1, int y1, int x2, int y2, int thick) { DrawCan8LineWInv_Fast(pDrawCan8, x1, y1, x2, y2, thick); }

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan8LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan8LineOverInv(can, x1, y1, x2, y2, False);
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
		DrawCan8LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan8LineOverInv(can, x1, y1, x2, y2, over);
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
		DrawCan8LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan8LineOverInv(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw8LineWInv(int x1, int y1, int x2, int y2, int thick) { DrawCan8LineWInv(pDrawCan8, x1, y1, x2, y2, thick); }

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
void DrawCan8Round_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s = col;
				s++;
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s = col;
				s++;
			}
			s0 += wb;
		}
	}
}
void Draw8Round_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan8Round_Fast(pDrawCan8, x, y, d, col, mask); }

// Draw round (checks clipping limits)
void DrawCan8Round(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s = col;
				s++;
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s = col;
				s++;
			}
			s0 += wb;
		}
	}
}
void Draw8Round(int x, int y, int d, u16 col, u8 mask) { DrawCan8Round(pDrawCan8, x, y, d, col, mask); }

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan8RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s = ~*s;
				s++;
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s = ~*s;
				s++;
			}
			s0 += wb;
		}
	}
}
void Draw8RoundInv_Fast(int x, int y, int d, u8 mask) { DrawCan8RoundInv_Fast(pDrawCan8, x, y, d, mask); }

// Draw round inverted (checks clipping limits)
void DrawCan8RoundInv(sDrawCan* can, int x, int y, int d, u8 mask)
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) *s = ~*s;
				s++;
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) *s = ~*s;
				s++;
			}
			s0 += wb;
		}
	}
}
void Draw8RoundInv(int x, int y, int d, u8 mask) { DrawCan8RoundInv(pDrawCan8, x, y, d, mask); }

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

#define DRAWCANCIRCLE_PIXFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	s = &s0[(xxx) + (yyy)*wb];	\
		*s = col; }

#define DRAWCANCIRCLE_PIX(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2 + (y2-basey)*wb];	\
		*s = col; } }

#define DRAWCANCIRCLE_PIXINVFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	s = &s0[(xxx) + (yyy)*wb];	\
		*s = ~*s; }

#define DRAWCANCIRCLE_PIXINV(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2 + (y2-basey)*wb];	\
		*s = ~*s; } }

//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan8Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan8Point_Fast(can, x, y, col);
		return;
	}

	// prepare
	int r = d/2;
	int xx = 0;
	int yy = r;
	int p = 1 - r;

	// pointer to middle of the circle
	int wb = can->wb;
	u8* s0 = &can->buf[x + (y - can->basey)*wb];
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
void Draw8Circle_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan8Circle_Fast(pDrawCan8, x, y, d, col, mask); }

// Draw circle or arc (checks clipping limits)
void DrawCan8Circle(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan8Point(can, x, y, col);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan8Circle_Fast(can, x, y, d, col, mask);
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
void Draw8Circle(int x, int y, int d, u16 col, u8 mask) { DrawCan8Circle(pDrawCan8, x, y, d, col, mask); }

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan8CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan8PointInv_Fast(can, x, y);
		return;
	}

	// prepare
	int r = d/2;
	int xx = 0;
	int yy = r;
	int p = 1 - r;

	// pointer to middle of the circle
	int wb = can->wb;
	u8* s0 = &can->buf[x + (y - can->basey)*wb];
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
void Draw8CircleInv_Fast(int x, int y, int d, u8 mask) { DrawCan8CircleInv_Fast(pDrawCan8, x, y, d, mask); }

// Draw circle or arc inverted (checks clipping limits)
void DrawCan8CircleInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan8PointInv(can, x, y);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan8CircleInv_Fast(can, x, y, d, mask);
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
void Draw8CircleInv(int x, int y, int d, u8 mask) { DrawCan8CircleInv(pDrawCan8, x, y, d, mask); }

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
void DrawCan8Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	// nothing to draw
	if (din >= d) return;
	u8 col = (u8)col8;

	// draw round
	if (din <= 0)
	{
		DrawCan8Round_Fast(can, x, y, d, col, mask);
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
		DrawCan8Circle_Fast(can, x, y, d, col, mask2);
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
	int r2, rin2;

	// hide parts of the ring
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		rin2 = rin*(rin-1);
		if (d == 3) r2--;
		if (din == 3) rin2--;

		// draw ring
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				r = x*x + y*y;
				if ((r <= r2) && (r > rin2)) *s = col;
				s++;
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*r;
		rin2 = rin*rin;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				r = x*(x-1) + y*(y-1);
				if ((r < r2) && (r >= rin2)) *s = col;
				s++;
			}
			s0 += wb;
		}
	}
}
void Draw8Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan8Ring_Fast(pDrawCan8, x, y, d, din, col, mask); }

// Draw ring (checks clipping limits)
void DrawCan8Ring(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	u8 col = (u8)col8;
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan8Round(can, x, y, d, col, mask);
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
		DrawCan8Circle(can, x, y, d, col, mask2);
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
	int r2, rin2;

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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		rin2 = rin*(rin-1);
		if (d == 3) r2--;
		if (din == 3) rin2--;

		// draw ring
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				r = x*x + y*y;
				if ((r <= r2) && (r > rin2)) *s = col;
				s++;
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*r;
		rin2 = rin*rin;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				r = x*(x-1) + y*(y-1);
				if ((r < r2) && (r >= rin2)) *s = col;
				s++;
			}
			s0 += wb;
		}
	}
}
void Draw8Ring(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan8Ring(pDrawCan8, x, y, d, din, col, mask); }

// Draw ring inverted (does not check clipping limits)
void DrawCan8RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan8RoundInv_Fast(can, x, y, d, mask);
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
		DrawCan8CircleInv_Fast(can, x, y, d, mask2);
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
	int r2, rin2;

	// hide parts of the ring
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		rin2 = rin*(rin-1);
		if (d == 3) r2--;
		if (din == 3) rin2--;

		// draw ring
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				r = x*x + y*y;
				if ((r <= r2) && (r > rin2)) *s = ~*s;
				s++;
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*r;
		rin2 = rin*rin;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				r = x*(x-1) + y*(y-1);
				if ((r < r2) && (r >= rin2)) *s = ~*s;
				s++;
			}
			s0 += wb;
		}
	}
}
void Draw8RingInv_Fast(int x, int y, int d, int din, u8 mask) { DrawCan8RingInv_Fast(pDrawCan8, x, y, d, din, mask); }

// Draw ring inverted (checks clipping limits)
void DrawCan8RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan8RoundInv(can, x, y, d, mask);
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
		DrawCan8CircleInv(can, x, y, d, mask2);
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
	int r2, rin2;

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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		rin2 = rin*(rin-1);
		if (d == 3) r2--;
		if (din == 3) rin2--;

		// draw ring
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				r = x*x + y*y;
				if ((r <= r2) && (r > rin2)) *s = ~*s;
				s++;
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
		s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
		r2 = r*r;
		rin2 = rin*rin;

		// draw round
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			for (x = x1; x <= x2; x++)
			{
				r = x*(x-1) + y*(y-1);
				if ((r < r2) && (r >= rin2)) *s = ~*s;
				s++;
			}
			s0 += wb;
		}
	}
}
void Draw8RingInv(int x, int y, int d, int din, u8 mask) { DrawCan8RingInv(pDrawCan8, x, y, d, din, mask); }

// ----------------------------------------------------------------------------
//                              Draw triangle
// ----------------------------------------------------------------------------

// Draw triangle (does not check clipping limits)
void DrawCan8Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
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
			if (k > 0) DrawCan8HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan8HLine_Fast(can, xmin, y, k, col);
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
			if (k > 0) DrawCan8HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan8HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan8HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan8HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw8Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan8Triangle_Fast(pDrawCan8, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle (checks clipping limits)
void DrawCan8Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
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
		DrawCan8Triangle_Fast(can, x1, y1, x2, y2, x3, y3, col);
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
			if (k > 0) DrawCan8HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan8HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan8HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan8HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan8HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan8HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw8Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan8Triangle(pDrawCan8, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle inverted (does not check clipping limits)
void DrawCan8TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
			if (k > 0) DrawCan8HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan8HLineInv_Fast(can, xmin, y, k);
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
			if (k > 0) DrawCan8HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan8HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan8HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan8HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw8TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan8TriangleInv_Fast(pDrawCan8, x1, y1, x2, y2, x3, y3); }

// Draw triangle inverted (checks clipping limits)
void DrawCan8TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
		DrawCan8TriangleInv_Fast(can, x1, y1, x2, y2, x3, y3);
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
			if (k > 0) DrawCan8HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan8HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan8HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan8HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan8HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan8HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw8TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan8TriangleInv(pDrawCan8, x1, y1, x2, y2, x3, y3); }

// ----------------------------------------------------------------------------
//                              Draw fill area
// ----------------------------------------------------------------------------

// Draw fill sub-area (internal function)
void _DrawCan8SubFill(sDrawCan* can, int x, int y, u8 fnd, u8 col)
{
	// prepare buffer address
	u8* s = &can->buf[x + (y - can->basey)*can->wb];

	// save start position -> x2, s2
	int x2 = x;
	u8* s2 = s;

	// fill start point
	*s = col;

	// update dirty area in Y direction
	if (y < can->dirtyy1) can->dirtyy1 = y;
	if (y >= can->dirtyy2) can->dirtyy2 = y+1;

	// search start of segment in LEFT-X direction -> x1, s1
	int clipx1 = can->clipx1;
	while ((x > clipx1) && (s[-1] == fnd))
	{
		x--;
		s--;
		*s = col;
	}
	int x1 = x;
	u8* s1 = s;

	// search end of segment in RIGHT-X direction -> x2, s2
	int clipx2 = can->clipx2-1;
	while ((x2 < clipx2) && (s2[1] == fnd))
	{
		x2++;
		s2++;
		*s2 = col;
	}

	// search segments in UP-Y direction
	if (y > can->clipy1)
	{
		x = x1;
		s = s1 - can->wb;
		y--;
		while (x <= x2)
		{
			if (*s == fnd) _DrawCan8SubFill(can, x, y, fnd, col);
			x++;
			s++;
		}
		y++;
	}

	// search segments in DOWN-Y direction
	if (y < can->clipy2-1)
	{
		x = x1;
		s = s1 + can->wb;
		y++;
		while (x <= x2)
		{
			if (*s == fnd) _DrawCan8SubFill(can, x, y, fnd, col);
			x++;
			s++;
		}
	}

	// update dirty area in X direction
	if (x1 < can->dirtyx1) can->dirtyx1 = x1;
	if (x2 >= can->dirtyx2) can->dirtyx2 = x2+1;
}

// Draw fill area
void DrawCan8Fill(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check clipping
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// get color to search
	u8 fnd = (u8)DrawCan8GetPoint_Fast(can, x, y);

	// fill if color is different
	if (fnd != col) _DrawCan8SubFill(can, x, y, fnd, col);
}
void Draw8Fill(int x, int y, u16 col) { DrawCan8Fill(pDrawCan8, x, y, col); }

// ----------------------------------------------------------------------------
//                             Draw charactter
// ----------------------------------------------------------------------------
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw8SelFont(const u8* font, u8 fontw, u8 fonth) { DrawCanSelFont(pDrawCan8, font, fontw, fonth); }

void Draw8SelFont8x8()		{ DrawCanSelFont8x8(pDrawCan8); }		// sans-serif bold, height 8 lines
void Draw8SelFont8x14()		{ DrawCanSelFont8x14(pDrawCan8); }		// sans-serif bold, height 14 lines
void Draw8SelFont8x16()		{ DrawCanSelFont8x16(pDrawCan8); }		// sans-serif bold, height 16 lines
void Draw8SelFont8x14Serif()	{ DrawCanSelFont8x14Serif(pDrawCan8); }		// serif bold, height 14 lines
void Draw8SelFont8x16Serif()	{ DrawCanSelFont8x16Serif(pDrawCan8); }		// serif bold, height 16 lines
void Draw8SelFont6x8()		{ DrawCanSelFont6x8(pDrawCan8); }		// condensed font, width 6 pixels, height 8 lines
void Draw8SelFont8x8Game()	{ DrawCanSelFont8x8Game(pDrawCan8); }		// game font, height 8 lines
void Draw8SelFont8x8Ibm()	{ DrawCanSelFont8x8Ibm(pDrawCan8); }		// IBM charset font, height 8 lines
void Draw8SelFont8x14Ibm()	{ DrawCanSelFont8x14Ibm(pDrawCan8); }		// IBM charset font, height 14 lines
void Draw8SelFont8x16Ibm()	{ DrawCanSelFont8x16Ibm(pDrawCan8); }		// IBM charset font, height 16 lines
void Draw8SelFont8x8IbmThin()	{ DrawCanSelFont8x8IbmThin(pDrawCan8); }	// IBM charset thin font, height 8 lines
void Draw8SelFont8x8Italic()	{ DrawCanSelFont8x8Italic(pDrawCan8); }		// italic, height 8 lines
void Draw8SelFont8x8Thin()	{ DrawCanSelFont8x8Thin(pDrawCan8); }		// thin font, height 8 lines
void Draw8SelFont5x8()		{ DrawCanSelFont5x8(pDrawCan8); }		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan8Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
{
	int i, j, wb, scalex2, scaley2;
	u8 b;
	u8 col = (u8)col8;

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// destination address
	wb = can->wb;
	u8* d0 = &can->buf[x + (y - can->basey)*wb];
	u8* d;

	// loop through lines of one character
	scaley2 = scaley;
	for (i = can->fonth*scaley; i > 0; i--)
	{
		// get one font sample
		b = *s;

		// destination address
		d = d0;

		// loop through pixels of one character line
		if (scalex == 1)
		{
			for (j = can->fontw; j > 0; j--)
			{
				if ((b & 0x80) != 0) *d = col;
				d++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) *d = col;
				d++;
				scalex2--;
				if (scalex2 <= 0)
				{
					b <<= 1;
					scalex2 = scalex;
				}
			}
		}

		// scale Y
		d0 += wb;
		scaley2--;
		if (scaley2 <= 0)
		{
			s += 256;
			scaley2 = scaley;
		}
	}
}
void Draw8Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan8Char_Fast(pDrawCan8, ch, x, y, col, scalex, scaley); }

// Draw character with transparent background (checks clipping limits)
void DrawCan8Char(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
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
		DrawCan8Char_Fast(can, ch, x, y, col, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan8Point(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan8Point(can, x, y, col);
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
void Draw8Char(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan8Char(pDrawCan8, ch, x, y, col, scalex, scaley); }

// Draw character with background (does not check clipping limits)
void DrawCan8CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
{
	int i, j, wb, scaley2, scalex2;
	u8 b;
	u8 col = (u8)col8;
	u8 bgcol = (u8)bgcol8;

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// destination address
	wb = can->wb;
	u8* d0 = &can->buf[x + (y - can->basey)*wb];
	u8* d;

	// loop through lines of one character
	scaley2 = scaley;
	for (i = can->fonth*scaley; i > 0; i--)
	{
		// get one font sample
		b = *s;

		// destination address
		d = d0;

		// loop through pixels of one character line
		if (scalex == 1)
		{
			for (j = can->fontw; j > 0; j--)
			{
				*d = ((b & 0x80) != 0) ? col : bgcol;
				d++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				*d = ((b & 0x80) != 0) ? col : bgcol;
				d++;
				scalex2--;
				if (scalex2 <= 0)
				{
					b <<= 1;
					scalex2 = scalex;
				}
			}
		}

		// scale Y
		d0 += wb;
		scaley2--;
		if (scaley2 <= 0)
		{
			s += 256;
			scaley2 = scaley;
		}
	}
}
void Draw8CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan8CharBg_Fast(pDrawCan8, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character with background (checks clipping limits)
void DrawCan8CharBg(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
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
		DrawCan8CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);
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
				DrawCan8Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				DrawCan8Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw8CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan8CharBg(pDrawCan8, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character inverted (does not check clipping limits)
void DrawCan8CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
{
	int i, j, wb, scaley2, scalex2;
	u8 b;

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// destination address
	wb = can->wb;
	u8* d0 = &can->buf[x + (y - can->basey)*wb];
	u8* d;

	// loop through lines of one character
	scaley2 = scaley;
	for (i = can->fonth*scaley; i > 0; i--)
	{
		// get one font sample
		b = *s;

		// destination address
		d = d0;

		// loop through pixels of one character line
		if (scalex == 1)
		{
			for (j = can->fontw; j > 0; j--)
			{
				if ((b & 0x80) != 0) *d = ~*d;
				d++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) *d = ~*d;
				d++;
				scalex2--;
				if (scalex2 <= 0)
				{
					b <<= 1;
					scalex2 = scalex;
				}
			}
		}

		// scale Y
		d0 += wb;
		scaley2--;
		if (scaley2 <= 0)
		{
			s += 256;
			scaley2 = scaley;
		}
	}
}
void Draw8CharInv_Fast(char ch, int x, int y, int scalex, int scaley) { DrawCan8CharInv_Fast(pDrawCan8, ch, x, y, scalex, scaley); }

// Draw character inverted (checks clipping limits)
void DrawCan8CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
		DrawCan8CharInv_Fast(can, ch, x, y, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan8PointInv(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan8PointInv(can, x, y);
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
void Draw8CharInv(char ch, int x, int y, int scalex, int scaley) { DrawCan8CharInv(pDrawCan8, ch, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                             Draw text
// ----------------------------------------------------------------------------
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan8Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan8Char_Fast(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw8Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan8Text_Fast(pDrawCan8, text, len, x, y, col, scalex, scaley); }

// Draw text with transparent background (checks clipping limits)
void DrawCan8Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan8Char(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw8Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan8Text(pDrawCan8, text, len, x, y, col, scalex, scaley); }

// Draw text with background (does not check clipping limits)
void DrawCan8TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan8CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw8TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan8TextBg_Fast(pDrawCan8, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text with background (checks clipping limits)
void DrawCan8TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan8CharBg(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw8TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan8TextBg(pDrawCan8, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text inverted (does not check clipping limits)
void DrawCan8TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan8CharInv_Fast(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw8TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan8TextInv_Fast(pDrawCan8, text, len, x, y, scalex, scaley); }

// Draw text inverted (checks clipping limits)
void DrawCan8TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan8CharInv(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw8TextInv(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan8TextInv(pDrawCan8, text, len, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                               Draw ellipse
// ----------------------------------------------------------------------------
// Using mid-point ellipse drawing algorithm

#define DRAWCANELLIPSE_PIXFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	s = &s0[(xxx) + (yyy)*wb];	\
		*s = col; }

#define DRAWCANELLIPSE_PIX(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2 + (y2-basey)*wb];	\
		*s = col; } }

#define DRAWCANELLIPSE_PIXINVFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	s = &s0[(xxx) + (yyy)*wb];	\
		*s = ~*s; }

#define DRAWCANELLIPSE_PIXINV(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2 + (y2-basey)*wb];	\
		*s = ~*s; } }

//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse
//		DRAWCAN_ELLIPSE_ARC0	= draw arc 0..90 deg
//		DRAWCAN_ELLIPSE_ARC1	= draw arc 90..180 deg
//		DRAWCAN_ELLIPSE_ARC2	= draw arc 180..270 deg
//		DRAWCAN_ELLIPSE_ARC3	= draw arc 270..360 deg
//		DRAWCAN_ELLIPSE_ALL	= draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan8Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	u8* s0 = &can->buf[x + (y - can->basey)*wb];
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
void Draw8Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan8Ellipse_Fast(pDrawCan8, x, y, dx, dy, col, mask); }

// Draw ellipse (checks clipping limits)
void DrawCan8Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
		DrawCan8Ellipse_Fast(can, x, y, dx, dy, col, mask);
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
void Draw8Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan8Ellipse(pDrawCan8, x, y, dx, dy, col, mask); }

// Draw ellipse inverted (does not check clipping limits)
void DrawCan8EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	u8* s0 = &can->buf[x + (y - can->basey)*wb];
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
void Draw8EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan8EllipseInv_Fast(pDrawCan8, x, y, dx, dy, mask); }

// Draw ellipse inverted (checks clipping limits)
void DrawCan8EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
		DrawCan8EllipseInv_Fast(can, x, y, dx, dy, mask);
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
void Draw8EllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan8EllipseInv(pDrawCan8, x, y, dx, dy, mask); }

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
void DrawCan8FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = col;
			s++;
		}
		s0 += wb;
	}
}
void Draw8FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan8FillEllipse_Fast(pDrawCan8, x, y, dx, dy, col, mask); }

// Draw filled ellipse (checks clipping limits)
void DrawCan8FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = col;
			s++;
		}
		s0 += wb;
	}
}
void Draw8FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan8FillEllipse(pDrawCan8, x, y, dx, dy, col, mask); }

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan8FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = ~*s;
			s++;
		}
		s0 += wb;
	}
}
void Draw8FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan8FillEllipseInv_Fast(pDrawCan8, x, y, dx, dy, mask); }

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan8FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	s0 = &can->buf[(x+x1) + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) *s = ~*s;
			s++;
		}
		s0 += wb;
	}
}
void Draw8FillEllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan8FillEllipseInv(pDrawCan8, x, y, dx, dy, mask); }

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
void DrawCan8Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d = &can->buf[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s = &((const u8*)src)[xs + ys*wbs];

	// copy image
	for (; h > 0; h--)
	{
		memcpy(d, s, w);
		d += wbd;
		s += wbs;
	}
}
void Draw8Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan8Img(pDrawCan8, xd, yd, src, xs, ys, w, h, wbs); }

// Draw image inverted with the same format as destination
void DrawCan8ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d = &can->buf[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s = &((const u8*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			*d ^= *s;
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw8ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan8ImgInv(pDrawCan8, xd, yd, src, xs, ys, w, h, wbs); }

// Draw transparent image with the same format as destination
void DrawCan8Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d = &can->buf[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s = &((const u8*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u8 c;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			c = *s;
			if (c != col) *d = c;
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw8Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan8Blit(pDrawCan8, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image inverted with the same format as destination
void DrawCan8BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d = &can->buf[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s = &((const u8*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u8 c;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			c = *s;
			if (c != col) *d ^= c;
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw8BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan8BlitInv(pDrawCan8, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan8BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8, u16 fnd8, u16 subs8)
{
	u8 col = (u8)col8;
	u8 fnd = (u8)fnd8;
	u8 subs = (u8)subs8;

	// width of source image
	int ws = wbs;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d = &can->buf[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s = &((const u8*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u8 c;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			c = *s;
			if (c != col)
			{
				if (c == fnd) c = subs;
				*d = c;
			}
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw8BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { DrawCan8BlitSubs(pDrawCan8, xd, yd, src, xs, ys, w, h, wbs, col, fnd, subs); }

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
void DrawCan8GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd)
{
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
	k = wbd - xd;
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
	const u8* s = &can->buf[xs + (ys - can->basey)*wbs];

	// destination address
	u8* d = &((u8*)dst)[xd + yd*wbd];

	// copy image
	for (; h > 0; h--)
	{
		memcpy(d, s, w);
		d += wbd;
		s += wbs;
	}
}
void Draw8GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { DrawCan8GetImg(pDrawCan8, xs, ys, w, h, dst, xd, yd, wbd); }

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
void DrawCan8ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src_img, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
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
	u8* d = &((u8*)can->buf)[xd + can->wb*(yd - can->basey)]; // destination image
	int wbd = can->wb - wd;
	int i, x2, y2;

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
		
				for (i = wd; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) & xmask;
					y2 = (yy0m>>FRACT) & ymask;
					*d++ = src[x2 + y2*wbs];
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}

				y0++;
				d += wbd;
			}
		}

		// image dimension is not power of 2
		else
		{
			for (; hd > 0; hd--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				for (i = wd; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) % ws;
					if (x2 < 0) x2 += ws;
					y2 = (yy0m>>FRACT) % hs;
					if (y2 < 0) y2 += hs;
					*d++ = src[x2 + y2*wbs];
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}

				y0++;
				d += wbd;
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

				for (i = wd; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (((u32)x2 < ww) && ((u32)y2 < hh)) *d = src[x2 + y2*wbs];
					d++;
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d += wbd;
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
				for (i = wd; i > 0; i--)
				{
					*d++ = col;
				}
				d += wbd;
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
		
				for (i = wd; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (x2 < 0) x2 = 0;
					if (x2 > ww) x2 = ww;
					if (y2 < 0) y2 = 0;
					if (y2 > hh) y2 = hh;
					*d++ = src[x2 + y2*wbs];
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d += wbd;
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
				for (i = wd; i > 0; i--)
				{
					*d++ = color;
				}
				d += wbd;
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
		
				for (i = wd; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (((u32)x2 < ww) && ((u32)y2 < hh))
						*d = src[x2 + y2*wbs];
					else
						*d = color;
					d++;
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}

				y0++;
				d += wbd;
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
		
				for (i = wd; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (((u32)x2 < ww) && ((u32)y2 < hh))
					{
						c = src[x2 + y2*wbs];
						if (c != color) *d = c;
					}
					d++;
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d += wbd;
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
		
				for (i = wd; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) & xmask;
					xy0m += m11b; // x0*m11

					y2 = (yy0m>>FRACT) & ymask;
					yy0m += m21b; // x0*m21

					*d++ = src[x2 + y2*wbs];
				}
				y0++;
				d += wbd;
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
		
				for (i = wd; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) % ws;
					if (x2 < 0) x2 += ws;
					xy0m += m11b; // x0*m11

					y2 = (yy0m>>FRACT) % hs;
					if (y2 < 0) y2 += hs;
					yy0m += m21b; // x0*m21

					*d++ = src[x2 + y2*wbs];
				}
				y0++;
				d += wbd;
			}
		}
	}
}
void Draw8ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan8ImgMat(pDrawCan8, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

#undef DRAWCAN_IMGLIMIT

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// drawing function interface
const sDrawCanFnc DrawCan8Fnc = {
	.pDrawPitch		= Draw8Pitch,			// calculate pitch of graphics line from image width
	.pDrawMaxWidth		= Draw8MaxWidth,		// calculate max. width of image from the pitch
	// Clear canvas
	.pDrawCanClearCol	= DrawCan8ClearCol,		// Clear canvas with color
	.pDrawCanClear		= DrawCan8Clear,		// Clear canvas with black color
	// Draw point
	.pDrawCanPoint_Fast	= DrawCan8Point_Fast,		// Draw point (does not check clipping limits)
	.pDrawCanPoint		= DrawCan8Point,		// Draw point (checks clipping limits)
	.pDrawCanPointInv_Fast	= DrawCan8PointInv_Fast,	// Draw point inverted (does not check clipping limits)
	.pDrawCanPointInv	= DrawCan8PointInv,		// Draw point inverted (checks clipping limits)
	// Get point
	.pDrawCanGetPoint_Fast	= DrawCan8GetPoint_Fast,	// Get point (does not check clipping limits)
	.pDrawCanGetPoint	= DrawCan8GetPoint,		// Get point (checks clipping limits)
	// Draw rectangle
	.pDrawCanRect_Fast	= DrawCan8Rect_Fast,		// Draw rectangle (does not check clipping limits)
	.pDrawCanRect		= DrawCan8Rect,			// Draw rectangle (checks clipping limits)
	.pDrawCanRectInv_Fast	= DrawCan8RectInv_Fast,		// Draw rectangle inverted (does not check clipping limits)
	.pDrawCanRectInv	= DrawCan8RectInv,		// Draw rectangle inverted (checks clipping limits
	// Draw horizontal line
	.pDrawCanHLine_Fast	= DrawCan8HLine_Fast,		// Draw horizontal line (does not check clipping limits)
	.pDrawCanHLine		= DrawCan8HLine,		// Draw horizontal line (checks clipping limits; negative width flip line)
	.pDrawCanHLineInv_Fast	= DrawCan8HLineInv_Fast,	// Draw horizontal line inverted (does not check clipping limits)
	.pDrawCanHLineInv	= DrawCan8HLineInv,		// Draw horizontal line inverted (checks clipping limits; negative width flip line)
	// Draw vertical line
	.pDrawCanVLine_Fast	= DrawCan8VLine_Fast,		// Draw vertical line (does not check clipping limits)
	.pDrawCanVLine		= DrawCan8VLine,		// Draw vertical line (checks clipping limits; negative height flip line)
	.pDrawCanVLineInv_Fast	= DrawCan8VLineInv_Fast,	// Draw vertical line inverted (does not check clipping limits)
	.pDrawCanVLineInv	= DrawCan8VLineInv,		// Draw vertical line inverted (checks clipping limits; negative height flip line)
	// Draw frame
	.pDrawCanFrame_Fast	= DrawCan8Frame_Fast,		// Draw 1-pixel frame (does not check clipping limits)
	.pDrawCanFrame		= DrawCan8Frame,		// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameInv_Fast	= DrawCan8FrameInv_Fast,	// Draw 1-pixel frame inverted (does not check clipping limits)
	.pDrawCanFrameInv	= DrawCan8FrameInv,		// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameW_Fast	= DrawCan8FrameW_Fast,		// Draw thick frame (does not check clipping limits)
	.pDrawCanFrameW		= DrawCan8FrameW,		// Draw thick frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameWInv_Fast	= DrawCan8FrameWInv_Fast,	// Draw thick frame inverted (does not check clipping limits)
	.pDrawCanFrameWInv	= DrawCan8FrameWInv,		// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
	// Draw line
	.pDrawCanLineOver_Fast	= DrawCan8LineOver_Fast,	// Draw line with overlapped pixels (does not check clipping limits)
	.pDrawCanLine_Fast	= DrawCan8Line_Fast,		// Draw line (does not check clipping limits)
	.pDrawCanLineOver	= DrawCan8LineOver,		// Draw line with overlapped pixels (checks clipping limits)
	.pDrawCanLine		= DrawCan8Line,			// Draw line (checks clipping limits)
	.pDrawCanLineOverInv_Fast = DrawCan8LineOverInv_Fast,	// Draw line inverted with overlapped pixels (does not check clipping limits)
	.pDrawCanLineInv_Fast	= DrawCan8LineInv_Fast,		// Draw line inverted (does not check clipping limits)
	.pDrawCanLineOverInv	= DrawCan8LineOverInv,		// Draw line inverted with overlapped pixels (checks clipping limits)
	.pDrawCanLineInv	= DrawCan8LineInv,		// Draw line inverted (checks clipping limits)
	.pDrawCanLineW_Fast	= DrawCan8LineW_Fast,		// Draw thick line (does not check clipping limits)
	.pDrawCanLineW		= DrawCan8LineW,		// Draw thick line (checks clipping limits)
	.pDrawCanLineWInv_Fast	= DrawCan8LineWInv_Fast,	// Draw thick line inverted (does not check clipping limits)
	.pDrawCanLineWInv	= DrawCan8LineWInv,		// Draw thick line inverted (checks clipping limits)
	// Draw round (filled circle)
	.pDrawCanRound_Fast	= DrawCan8Round_Fast,		// Draw round (does not check clipping limits)
	.pDrawCanRound		= DrawCan8Round,		// Draw round (checks clipping limits)
	.pDrawCanRoundInv_Fast	= DrawCan8RoundInv_Fast,	// Draw round inverted (does not check clipping limits)
	.pDrawCanRoundInv	= DrawCan8RoundInv,		// Draw round inverted (checks clipping limits)
	// Draw circle
	.pDrawCanCircle_Fast	= DrawCan8Circle_Fast,		// Draw circle or arc (does not check clipping limits)
	.pDrawCanCircle		= DrawCan8Circle,		// Draw circle or arc (checks clipping limits)
	.pDrawCanCircleInv_Fast	= DrawCan8CircleInv_Fast,	// Draw circle or arc inverted (does not check clipping limits)
	.pDrawCanCircleInv	= DrawCan8CircleInv,		// Draw circle or arc inverted (checks clipping limits)
	// Draw ring
	.pDrawCanRing_Fast	= DrawCan8Ring_Fast,		// Draw ring (does not check clipping limits)
	.pDrawCanRing		= DrawCan8Ring,			// Draw ring (checks clipping limits)
	.pDrawCanRingInv_Fast	= DrawCan8RingInv_Fast,		// Draw ring inverted (does not check clipping limits)
	.pDrawCanRingInv	= DrawCan8RingInv,		// Draw ring inverted (checks clipping limits)
	// Draw triangle
	.pDrawCanTriangle_Fast	= DrawCan8Triangle_Fast,	// Draw triangle (does not check clipping limits)
	.pDrawCanTriangle	= DrawCan8Triangle,		// Draw triangle (checks clipping limits)
	.pDrawCanTriangleInv_Fast = DrawCan8TriangleInv_Fast,	// Draw triangle inverted (does not check clipping limits)
	.pDrawCanTriangleInv	= DrawCan8TriangleInv,		// Draw triangle inverted (checks clipping limits)
	// Draw fill area
	.pDrawCanFill		= DrawCan8Fill,			// Draw fill area
	// Draw character
	.pDrawCanChar_Fast	= DrawCan8Char_Fast,		// Draw character with transparent background (does not check clipping limits)
	.pDrawCanChar		= DrawCan8Char,			// Draw character with transparent background (checks clipping limits)
	.pDrawCanCharBg_Fast	= DrawCan8CharBg_Fast,		// Draw character with background (does not check clipping limits)
	.pDrawCanCharBg		= DrawCan8CharBg,		// Draw character with background (checks clipping limits)
	.pDrawCanCharInv_Fast	= DrawCan8CharInv_Fast,		// Draw character inverted (does not check clipping limits)
	.pDrawCanCharInv	= DrawCan8CharInv,		// Draw character inverted (checks clipping limits)
	// Draw text
	.pDrawCanText_Fast	= DrawCan8Text_Fast,		// Draw text with transparent background (does not check clipping limits)
	.pDrawCanText		= DrawCan8Text,			// Draw text with transparent background (checks clipping limits)
	.pDrawCanTextBg_Fast	= DrawCan8TextBg_Fast,		// Draw text with background (does not check clipping limits)
	.pDrawCanTextBg		= DrawCan8TextBg,		// Draw text with background (checks clipping limits)
	.pDrawCanTextInv_Fast	= DrawCan8TextInv_Fast,		// Draw text inverted (does not check clipping limits)
	.pDrawCanTextInv	= DrawCan8TextInv,		// Draw text inverted (checks clipping limits)
	// Draw ellipse
	.pDrawCanEllipse_Fast	= DrawCan8Ellipse_Fast,		// Draw ellipse (does not check clipping limits)
	.pDrawCanEllipse	= DrawCan8Ellipse,		// Draw ellipse (checks clipping limits)
	.pDrawCanEllipseInv_Fast = DrawCan8EllipseInv_Fast,	// Draw ellipse inverted (does not check clipping limits)
	.pDrawCanEllipseInv	= DrawCan8EllipseInv,		// Draw ellipse inverted (checks clipping limits)
	// Draw filled ellipse
	.pDrawCanFillEllipse_Fast = DrawCan8FillEllipse_Fast,	// Draw filled ellipse (does not check clipping limits)
	.pDrawCanFillEllipse	= DrawCan8FillEllipse,		// Draw filled ellipse (checks clipping limits)
	.pDrawCanFillEllipseInv_Fast = DrawCan8FillEllipseInv_Fast, // Draw filled ellipse inverted (does not check clipping limits)
	.pDrawCanFillEllipseInv = DrawCan8FillEllipseInv,	// Draw filled ellipse inverte (checks clipping limits)
	// Draw image
	.pDrawCanImg		= DrawCan8Img,			// Draw image with the same format as destination
	.pDrawCanImgInv		= DrawCan8ImgInv,		// Draw image inverted with the same format as destination
	.pDrawCanBlit		= DrawCan8Blit,			// Draw transparent image with the same format as destination
	.pDrawCanBlitInv	= DrawCan8BlitInv,		// Draw transparent image inverted with the same format as destination
	.pDrawCanBlitSubs	= DrawCan8BlitSubs,		// Draw transparent image with the same format as destination, with substitute color
	.pDrawCanGetImg		= DrawCan8GetImg,		// Get image from canvas to buffer
	.pDrawCanImgMat		= DrawCan8ImgMat,		// Draw image with 2D transformation matrix
	// colors
	.pColRgb		= Draw8ColRgb,			// convert RGB888 color to pixel color
#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
	.pColRand		= Draw8ColRand,			// random color
#endif
	.col_black		= COL8_BLACK,			// black color
	.col_blue		= COL8_BLUE,			// blue color
	.col_green		= COL8_GREEN,			// green color
	.col_cyan		= COL8_CYAN,			// cyan color
	.col_red		= COL8_RED,			// red color
	.col_magenta		= COL8_MAGENTA,			// magenta color
	.col_yellow		= COL8_YELLOW,			// yellow color
	.col_white		= COL8_WHITE,			// white color
	.col_gray		= COL8_GRAY,			// gray color
	.col_dkblue		= COL8_DKBLUE,			// dark blue color
	.col_dkgreen		= COL8_DKGREEN,			// dark green color
	.col_dkcyan		= COL8_DKCYAN,			// dark cyan color
	.col_dkred		= COL8_DKRED,			// dark red color
	.col_dkmagenta		= COL8_DKMAGENTA,		// dark magenta color
	.col_dkyellow		= COL8_DKYELLOW,		// dark yellow color
	.col_dkwhite		= COL8_DKWHITE,			// dark white color
	.col_dkgray		= COL8_DKGRAY,			// dark gray color
	.col_ltblue		= COL8_LTBLUE,			// light blue color
	.col_ltgreen		= COL8_LTGREEN,			// light green color
	.col_ltcyan		= COL8_LTCYAN,			// light cyan color
	.col_ltred		= COL8_LTRED,			// light red color
	.col_ltmagenta		= COL8_LTMAGENTA,		// light magenta color
	.col_ltyellow		= COL8_LTYELLOW,		// light yellow color
	.col_ltgray		= COL8_LTGRAY,			// light gray color
	.col_azure		= COL8_AZURE,			// azure blue color
	.col_orange		= COL8_ORANGE,			// orange color
	.col_brown		= COL8_BROWN,			// brown color
};

#endif // USE_DRAWCAN0

#endif // USE_DRAWCAN
