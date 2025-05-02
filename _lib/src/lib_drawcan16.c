
// ****************************************************************************
//
//                 Drawing to canvas at 15-bit and 16-bit format
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

#if USE_DRAWCAN	&& USE_DRAWCAN16	// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#include "../../_font/_include.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_float.h"
#include "../inc/lib_text.h"
#include "../inc/lib_rand.h"
#include "../inc/lib_drawcan.h"
#include "../inc/lib_drawcan16.h"

#ifndef WIDTH
#define WIDTH		320		// display width
#endif

#ifndef HEIGHT
#define HEIGHT		240		// display height
#endif

// default drawing canvas for 15-bit format
sDrawCan DrawCan15 = {
	// format
	.format = DRAWCAN_FORMAT_15,	// u8	format;		// canvas format CANVAS_*
	.colbit = 15,			// u8	colbit;		// number of bits per pixel
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
	.wb = (WIDTH*2+3)&~3,		// s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	.striph = HEIGHT,		// s16	striph;		// strip height (number of lines)

	// print
	.printposnum = WIDTH/FONTW,	// u16	printposnum;	// number of text positions per row (= w / fontw)
	.printrownum = HEIGHT/FONTH,	// u16	printrownum;	// number of text rows (= h / fonth)
	.printpos = 0,			// u16	printpos;	// console print character position
	.printrow = 0,			// u16	printrow;	// console print character row
	.printcol = COL15_WHITE,	// u16	printcol;	// console print color

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
	.drawfnc = NULL, //&DrawCan15Fnc,	// const struct sDrawCanFnc_* drawfnc; // drawing functions
};

// default drawing canvas for 16-bit format
sDrawCan DrawCan16 = {
	// format
	.format = DRAWCAN_FORMAT_16,	// u8	format;		// canvas format CANVAS_*
	.colbit = 16,			// u8	colbit;		// number of bits per pixel
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
	.wb = (WIDTH*2+3)&~3,		// s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	.striph = HEIGHT,		// s16	striph;		// strip height (number of lines)

	// print
	.printposnum = WIDTH/FONTW,	// u16	printposnum;	// number of text positions per row (= w / fontw)
	.printrownum = HEIGHT/FONTH,	// u16	printrownum;	// number of text rows (= h / fonth)
	.printpos = 0,			// u16	printpos;	// console print character position
	.printrow = 0,			// u16	printrow;	// console print character row
	.printcol = COL16_WHITE,	// u16	printcol;	// console print color

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
	.drawfnc = NULL, //&DrawCan16Fnc,	// const struct sDrawCanFnc_* drawfnc; // drawing functions
};

// current drawing canvas for 15-bit format
sDrawCan* pDrawCan15 = &DrawCan15;

// current drawing canvas for 16-bit format
sDrawCan* pDrawCan16 = &DrawCan16;

// set default drawing canvas for 15-bit format
void SetDrawCan15(sDrawCan* can) { pDrawCan15 = can; }

// set default drawing canvas for 16-bit format
void SetDrawCan16(sDrawCan* can) { pDrawCan16 = can; }

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw16Pitch(int w) { return (w*2 + 3) & ~3; }

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw16MaxWidth(int pitch) { return pitch/2; }

// get even 4-bit pixel
#define GETPIXEL4_EVEN(ddd) ( *(ddd) >> 4 )

// get odd 4-bit pixel
#define GETPIXEL4_ODD(ddd) ( *(ddd) & 0x0f )

// get 4-bit pixel by x coordinate
#define GETPIXEL4(ddd,xxx) ( (((xxx) & 1) == 0) ? GETPIXEL4_EVEN(ddd) : GETPIXEL4_ODD(ddd) )

// get even 12-bit pixel
#define GETPIXEL12_EVEN(ddd) ( (ddd)[0] | (((ddd)[1] & 0x0f) << 8) )

// get odd 12-bit pixel
#define GETPIXEL12_ODD(ddd) ( ((ddd)[1] >> 4) | ((ddd)[2] << 4) )

// get 12-bit pixel by x coordinate
#define GETPIXEL12(ddd,xxx) ( (((xxx) & 1) == 0) ? GETPIXEL12_EVEN(ddd) : GETPIXEL12_ODD(ddd) )

// convert RGB888 color to 15-bit pixel color RGB555
u16 Draw15ColRgb(u8 r, u8 g, u8 b) { return COLOR15(r, g, b); }

// convert RGB888 color to 16-bit pixel color RGB565
u16 Draw16ColRgb(u8 r, u8 g, u8 b) { return COLOR16(r, g, b); }

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 15-bit pixel color RGB555
u16 Draw15ColRand() { return COL15_RANDOM; }

// random 16-bit pixel color RGB565
u16 Draw16ColRand() { return COL16_RANDOM; }
#endif

// ----------------------------------------------------------------------------
//                            Clear canvas
// ----------------------------------------------------------------------------

// Clear canvas with color
void DrawCan16ClearCol(sDrawCan* can, u16 col)
{
	int x = can->clipx1;
	int y = can->clipy1;
	int w = can->clipx2 - x;
	int h = can->clipy2 - y;
	DrawCan16Rect(can, x, y, w, h, col);
}
void Draw16ClearCol(u16 col) { DrawCan16ClearCol(pDrawCan16, col); }
void Draw15ClearCol(u16 col) { DrawCan15ClearCol(pDrawCan15, col); }

// Clear canvas with black color
void DrawCan16Clear(sDrawCan* can) { DrawCan16ClearCol(can, COL16_BLACK); }
void DrawCan15Clear(sDrawCan* can) { DrawCan15ClearCol(can, COL15_BLACK); }
void Draw16Clear() { DrawCan16Clear(pDrawCan16); }
void Draw15Clear() { DrawCan15Clear(pDrawCan15); }

// ----------------------------------------------------------------------------
//                               Draw point
// ----------------------------------------------------------------------------

// Draw point (does not check clipping limits)
void DrawCan16Point_Fast(sDrawCan* can, int x, int y, u16 col)
{
	// draw pixel
	((u16*)can->buf)[x + (y - can->basey)*(can->wb/2)] = col;
}
void Draw16Point_Fast(int x, int y, u16 col) { DrawCan16Point_Fast(pDrawCan16, x, y, col); }
void Draw15Point_Fast(int x, int y, u16 col) { DrawCan15Point_Fast(pDrawCan15, x, y, col); }

// Draw point (checks clipping limits)
void DrawCan16Point(sDrawCan* can, int x, int y, u16 col)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// draw pixel
	((u16*)can->buf)[x + (y - can->basey)*(can->wb/2)] = col;
}
void Draw16Point(int x, int y, u16 col) { DrawCan16Point(pDrawCan16, x, y, col); }
void Draw15Point(int x, int y, u16 col) { DrawCan15Point(pDrawCan15, x, y, col); }

// Draw point inverted (does not check clipping limits)
void DrawCan16PointInv_Fast(sDrawCan* can, int x, int y)
{
	// invert pixel
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*(can->wb/2)];
	d[0] = ~d[0];
}
void Draw16PointInv_Fast(int x, int y) { DrawCan16PointInv_Fast(pDrawCan16, x, y); }
void Draw15PointInv_Fast(int x, int y) { DrawCan15PointInv_Fast(pDrawCan15, x, y); }

// Draw point inverted (checks clipping limits)
void DrawCan16PointInv(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// invert pixel
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*(can->wb/2)];
	d[0] = ~d[0];
}
void Draw16PointInv(int x, int y) { DrawCan16PointInv(pDrawCan16, x, y); }
void Draw15PointInv(int x, int y) { DrawCan15PointInv(pDrawCan15, x, y); }

// ----------------------------------------------------------------------------
//                               Get point
// ----------------------------------------------------------------------------

// Get point (does not check clipping limits)
u16 DrawCan16GetPoint_Fast(sDrawCan* can, int x, int y)
{
	return ((u16*)can->buf)[x + (y - can->basey)*(can->wb/2)];
}

u16 DrawCan15GetPoint_Fast(sDrawCan* can, int x, int y)
{
	return ((u16*)can->buf)[x + (y - can->basey)*(can->wb/2)] & 0x7fff;
}

u16 Draw16GetPoint_Fast(int x, int y) { return DrawCan16GetPoint_Fast(pDrawCan16, x, y); }
u16 Draw15GetPoint_Fast(int x, int y) { return DrawCan15GetPoint_Fast(pDrawCan15, x, y); }

// Get point (checks clipping limits)
u16 DrawCan16GetPoint(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return COL16_BLACK;

	// get pixel
	return ((u16*)can->buf)[x + (y - can->basey)*(can->wb/2)];
}

u16 DrawCan15GetPoint(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return COL15_BLACK;

	// get pixel
	return ((u16*)can->buf)[x + (y - can->basey)*(can->wb/2)] & 0x7fff;
}

u16 Draw16GetPoint(int x, int y) { return DrawCan16GetPoint(pDrawCan16, x, y); }
u16 Draw15GetPoint(int x, int y) { return DrawCan15GetPoint(pDrawCan15, x, y); }

// ----------------------------------------------------------------------------
//                            Draw rectangle
// ----------------------------------------------------------------------------

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan16Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col)
{
	// draw
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*can->wb/2];
	int wb = can->wb/2 - w;
	int i;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--) *d++ = col;
		d += wb;
	}
}
void Draw16Rect_Fast(int x, int y, int w, int h, u16 col) { DrawCan16Rect_Fast(pDrawCan16, x, y, w, h, col); }
void Draw15Rect_Fast(int x, int y, int w, int h, u16 col) { DrawCan15Rect_Fast(pDrawCan15, x, y, w, h, col); }

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan16Rect(sDrawCan* can, int x, int y, int w, int h, u16 col)
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

	// draw
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*can->wb/2];
	int wb = can->wb/2 - w;
	int i;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--) *d++ = col;
		d += wb;
	}
}
void Draw16Rect(int x, int y, int w, int h, u16 col) { DrawCan16Rect(pDrawCan16, x, y, w, h, col); }
void Draw15Rect(int x, int y, int w, int h, u16 col) { DrawCan15Rect(pDrawCan15, x, y, w, h, col); }

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan16RectInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// invert
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*can->wb/2];
	int wb = can->wb/2 - w;
	int i;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--) { *d = ~*d; d++; }
		d += wb;
	}
}
void Draw16RectInv_Fast(int x, int y, int w, int h) { DrawCan16RectInv_Fast(pDrawCan16, x, y, w, h); }
void Draw15RectInv_Fast(int x, int y, int w, int h) { DrawCan15RectInv_Fast(pDrawCan15, x, y, w, h); }

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan16RectInv(sDrawCan* can, int x, int y, int w, int h)
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
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*can->wb/2];
	int wb = can->wb/2 - w;
	int i;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--) { *d = ~*d; d++; }
		d += wb;
	}
}
void Draw16RectInv(int x, int y, int w, int h) { DrawCan16RectInv(pDrawCan16, x, y, w, h); }
void Draw15RectInv(int x, int y, int w, int h) { DrawCan15RectInv(pDrawCan15, x, y, w, h); }

// ----------------------------------------------------------------------------
//                          Draw horizontal line
// ----------------------------------------------------------------------------

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan16HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col)
{
	// draw
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*can->wb/2];
	int i;
	for (i = w; i > 0; i--) *d++ = col;
}
void Draw16HLine_Fast(int x, int y, int w, u16 col) { DrawCan16HLine_Fast(pDrawCan16, x, y, w, col); }
void Draw15HLine_Fast(int x, int y, int w, u16 col) { DrawCan15HLine_Fast(pDrawCan15, x, y, w, col); }

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan16HLine(sDrawCan* can, int x, int y, int w, u16 col)
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
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*can->wb/2];
	int i;
	for (i = w; i > 0; i--) *d++ = col;
}
void Draw16HLine(int x, int y, int w, u16 col) { DrawCan16HLine(pDrawCan16, x, y, w, col); }
void Draw15HLine(int x, int y, int w, u16 col) { DrawCan15HLine(pDrawCan15, x, y, w, col); }

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan16HLineInv_Fast(sDrawCan* can, int x, int y, int w)
{
	// invert
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*can->wb/2];
	int i;
	for (i = w; i > 0; i--) { *d = ~*d; d++; }
}
void Draw16HLineInv_Fast(int x, int y, int w) { DrawCan16HLineInv_Fast(pDrawCan16, x, y, w); }
void Draw15HLineInv_Fast(int x, int y, int w) { DrawCan15HLineInv_Fast(pDrawCan15, x, y, w); }

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan16HLineInv(sDrawCan* can, int x, int y, int w)
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
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*can->wb/2];
	int i;
	for (i = w; i > 0; i--) { *d = ~*d; d++; }
}
void Draw16HLineInv(int x, int y, int w) { DrawCan16HLineInv(pDrawCan16, x, y, w); }
void Draw15HLineInv(int x, int y, int w) { DrawCan15HLineInv(pDrawCan15, x, y, w); }

// ----------------------------------------------------------------------------
//                           Draw vertical line
// ----------------------------------------------------------------------------

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan16VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col)
{
	// draw
	int wb = can->wb/2;
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*wb];
	for (; h > 0; h--)
	{
		*d = col;
		d += wb;
	}
}
void Draw16VLine_Fast(int x, int y, int h, u16 col) { DrawCan16VLine_Fast(pDrawCan16, x, y, h, col); }
void Draw15VLine_Fast(int x, int y, int h, u16 col) { DrawCan15VLine_Fast(pDrawCan15, x, y, h, col); }

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan16VLine(sDrawCan* can, int x, int y, int h, u16 col)
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
	int wb = can->wb/2;
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*wb];
	for (; h > 0; h--)
	{
		*d = col;
		d += wb;
	}
}
void Draw16VLine(int x, int y, int h, u16 col) { DrawCan16VLine(pDrawCan16, x, y, h, col); }
void Draw15VLine(int x, int y, int h, u16 col) { DrawCan15VLine(pDrawCan15, x, y, h, col); }

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan16VLineInv_Fast(sDrawCan* can, int x, int y, int h)
{
	// invert
	int wb = can->wb/2;
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*wb];
	for (; h > 0; h--)
	{
		*d = ~*d;
		d += wb;
	}
}
void Draw16VLineInv_Fast(int x, int y, int h) { DrawCan16VLineInv_Fast(pDrawCan16, x, y, h); }
void Draw15VLineInv_Fast(int x, int y, int h) { DrawCan15VLineInv_Fast(pDrawCan15, x, y, h); }

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan16VLineInv(sDrawCan* can, int x, int y, int h)
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
	int wb = can->wb/2;
	u16* d = &((u16*)can->buf)[x + (y - can->basey)*wb];
	for (; h > 0; h--)
	{
		*d = ~*d;
		d += wb;
	}
}
void Draw16VLineInv(int x, int y, int h) { DrawCan16VLineInv(pDrawCan16, x, y, h); }
void Draw15VLineInv(int x, int y, int h) { DrawCan15VLineInv(pDrawCan15, x, y, h); }

// ----------------------------------------------------------------------------
//                           Draw frame
// ----------------------------------------------------------------------------

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan16Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
{
	// bottom line (dark)
	DrawCan16HLine_Fast(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan16VLine_Fast(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan16HLine_Fast(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan16VLine_Fast(can, x, y+1, h-2, col_light);
	}
}
void Draw16Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan16Frame_Fast(pDrawCan16, x, y, w, h, col_light, col_dark); }
void Draw15Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan15Frame_Fast(pDrawCan15, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan16Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
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
	DrawCan16HLine(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan16VLine(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan16HLine(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan16VLine(can, x, y+1, h-2, col_light);
	}
}
void Draw16Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan16Frame(pDrawCan16, x, y, w, h, col_light, col_dark); }
void Draw15Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan15Frame(pDrawCan15, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan16FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// bottom line
	DrawCan16HLineInv_Fast(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan16VLineInv_Fast(can, x+w-1, y, h-1);

		// top line
		DrawCan16HLineInv_Fast(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan16VLineInv_Fast(can, x, y+1, h-2);
	}
}
void Draw16FrameInv_Fast(int x, int y, int w, int h) { DrawCan16FrameInv_Fast(pDrawCan16, x, y, w, h); }
void Draw15FrameInv_Fast(int x, int y, int w, int h) { DrawCan15FrameInv_Fast(pDrawCan15, x, y, w, h); }

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan16FrameInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan16HLineInv(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan16VLineInv(can, x+w-1, y, h-1);

		// top line
		DrawCan16HLineInv(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan16VLineInv(can, x, y+1, h-2);
	}
}
void Draw16FrameInv(int x, int y, int w, int h) { DrawCan16FrameInv(pDrawCan16, x, y, w, h); }
void Draw15FrameInv(int x, int y, int w, int h) { DrawCan15FrameInv(pDrawCan15, x, y, w, h); }

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan16FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan16Rect_Fast(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan16Rect_Fast(can, x, y, w, thick, col);

		// bottom line
		DrawCan16Rect_Fast(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan16Rect_Fast(can, x, y, thick, h, col);

		// right line
		DrawCan16Rect_Fast(can, x+w-thick, y, thick, h, col);
	}
}
void Draw16FrameW_Fast(int x, int y, int w, int h, u16 col, int thick) { DrawCan16FrameW_Fast(pDrawCan16, x, y, w, h, col, thick); }
void Draw15FrameW_Fast(int x, int y, int w, int h, u16 col, int thick) { DrawCan15FrameW_Fast(pDrawCan15, x, y, w, h, col, thick); }

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan16FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
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
		DrawCan16Rect(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan16Rect(can, x, y, w, thick, col);

		// bottom line
		DrawCan16Rect(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan16Rect(can, x, y, thick, h, col);

		// right line
		DrawCan16Rect(can, x+w-thick, y, thick, h, col);
	}
}
void Draw16FrameW(int x, int y, int w, int h, u16 col, int thick) { DrawCan16Frame(pDrawCan16, x, y, w, h, col, thick); }
void Draw15FrameW(int x, int y, int w, int h, u16 col, int thick) { DrawCan15Frame(pDrawCan15, x, y, w, h, col, thick); }

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan16FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan16RectInv_Fast(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan16RectInv_Fast(can, x, y, w, thick);

		// bottom line
		DrawCan16RectInv_Fast(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan16RectInv_Fast(can, x, y, thick, h);

		// right line
		DrawCan16RectInv_Fast(can, x+w-thick, y, thick, h);
	}
}
void Draw16FrameWInv_Fast(int x, int y, int w, int h, int thick) { DrawCan16FrameWInv_Fast(pDrawCan16, x, y, w, h, thick); }
void Draw15FrameWInv_Fast(int x, int y, int w, int h, int thick) { DrawCan15FrameWInv_Fast(pDrawCan15, x, y, w, h, thick); }

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan16FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick)
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
		DrawCan16RectInv(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan16RectInv(can, x, y, w, thick);

		// bottom line
		DrawCan16RectInv(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan16RectInv(can, x, y, thick, h);

		// right line
		DrawCan16RectInv(can, x+w-thick, y, thick, h);
	}
}
void Draw16FrameWInv(int x, int y, int w, int h, int thick) { DrawCan16FrameWInv(pDrawCan16, x, y, w, h, thick); }
void Draw15FrameWInv(int x, int y, int w, int h, int thick) { DrawCan15FrameWInv(pDrawCan15, x, y, w, h, thick); }

// ----------------------------------------------------------------------------
//                           Draw line
// ----------------------------------------------------------------------------

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan16LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over)
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
	int wb = can->wb/2;
	int ddy = wb;
	if (dy < 0)
	{
		sy = -1;
		ddy = -ddy;
		dy = -dy;
	}

	// destination address
	u16* d = &((u16*)can->buf)[x1 + (y1 - can->basey)*wb];

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
void Draw16LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan16LineOver_Fast(pDrawCan16, x1, y1, x2, y2, col, over); }
void Draw15LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan15LineOver_Fast(pDrawCan15, x1, y1, x2, y2, col, over); }

// Draw line (does not check clipping limits)
void DrawCan16Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan16LineOver_Fast(pDrawCan16, x1, y1, x2, y2, col, False); }
void DrawCan15Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan15LineOver_Fast(pDrawCan15, x1, y1, x2, y2, col, False); }
void Draw16Line_Fast(int x1, int y1, int x2, int y2, u16 col) { Draw16LineOver_Fast(x1, y1, x2, y2, col, False); }
void Draw15Line_Fast(int x1, int y1, int x2, int y2, u16 col) { Draw15LineOver_Fast(x1, y1, x2, y2, col, False); }

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan16LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over)
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
	int wb = can->wb/2;
	int ddy = wb;
	if (dy < 0)
	{
		sy = -1;
		ddy = -ddy;
		dy = -dy;
	}

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u16* d = &((u16*)can->buf)[x1 + (y1 - can->basey)*wb];

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
void Draw16LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan16LineOver(pDrawCan16, x1, y1, x2, y2, col, over); }
void Draw15LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan15LineOver(pDrawCan15, x1, y1, x2, y2, col, over); }

// Draw line (checks clipping limits)
void DrawCan16Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan16LineOver(can, x1, y1, x2, y2, col, False); }
void DrawCan15Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan15LineOver(can, x1, y1, x2, y2, col, False); }
void Draw16Line(int x1, int y1, int x2, int y2, u16 col) { DrawCan16Line(pDrawCan16, x1, y1, x2, y2, col); }
void Draw15Line(int x1, int y1, int x2, int y2, u16 col) { DrawCan15Line(pDrawCan15, x1, y1, x2, y2, col); }

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan16LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	int wb = can->wb/2;
	int ddy = wb;
	if (dy < 0)
	{
		sy = -1;
		ddy = -ddy;
		dy = -dy;
	}

	// destination address
	u16* d = &((u16*)can->buf)[x1 + (y1 - can->basey)*wb];

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
void Draw16LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over) { DrawCan16LineOverInv_Fast(pDrawCan16, x1, y1, x2, y2, over); }
void Draw15LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over) { DrawCan15LineOverInv_Fast(pDrawCan15, x1, y1, x2, y2, over); }

// Draw line inverted (does not check clipping limits)
void DrawCan16LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan16LineOverInv_Fast(can, x1, y1, x2, y2, False); }
void DrawCan15LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan15LineOverInv_Fast(can, x1, y1, x2, y2, False); }
void Draw16LineInv_Fast(int x1, int y1, int x2, int y2) { DrawCan16LineOverInv_Fast(pDrawCan16, x1, y1, x2, y2, False); }
void Draw15LineInv_Fast(int x1, int y1, int x2, int y2) { DrawCan15LineOverInv_Fast(pDrawCan15, x1, y1, x2, y2, False); }

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan16LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	int wb = can->wb/2;
	int ddy = wb;
	if (dy < 0)
	{
		sy = -1;
		ddy = -ddy;
		dy = -dy;
	}

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u16* d = &((u16*)can->buf)[x1 + (y1 - can->basey)*wb];

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
void Draw16LineOverInv(int x1, int y1, int x2, int y2, Bool over) { DrawCan16LineOverInv(pDrawCan16, x1, y1, x2, y2, over); }
void Draw15LineOverInv(int x1, int y1, int x2, int y2, Bool over) { DrawCan15LineOverInv(pDrawCan15, x1, y1, x2, y2, over); }

// Draw line inverted (checks clipping limits)
void DrawCan16LineInv(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan16LineOverInv(can, x1, y1, x2, y2, False); }
void DrawCan15LineInv(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan15LineOverInv(can, x1, y1, x2, y2, False); }
void Draw16LineInv(int x1, int y1, int x2, int y2) { DrawCan16LineOverInv(pDrawCan16, x1, y1, x2, y2, False); }
void Draw15LineInv(int x1, int y1, int x2, int y2) { DrawCan15LineOverInv(pDrawCan15, x1, y1, x2, y2, False); }

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan16LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan16LineOver_Fast(can, x1, y1, x2, y2, col, False);
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
		DrawCan16Round_Fast(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan16Round_Fast(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan16LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan16LineOver_Fast(can, x1, y1, x2, y2, col, over);
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
		DrawCan16LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan16LineOver_Fast(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw16LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan16LineW_Fast(pDrawCan16, x1, y1, x2, y2, col, thick, round); }
void Draw15LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan15LineW_Fast(pDrawCan15, x1, y1, x2, y2, col, thick, round); }

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan16LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan16LineOver(can, x1, y1, x2, y2, col, False);
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
		DrawCan16Round(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan16Round(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan16LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan16LineOver(can, x1, y1, x2, y2, col, over);
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
		DrawCan16LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan16LineOver(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw16LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan16LineW(pDrawCan16, x1, y1, x2, y2, col, thick, round); }
void Draw15LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan15LineW(pDrawCan15, x1, y1, x2, y2, col, thick, round); }

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan16LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan16LineOverInv_Fast(can, x1, y1, x2, y2, False);
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
		DrawCan16LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan16LineOverInv_Fast(can, x1, y1, x2, y2, over);
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
		DrawCan16LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan16LineOverInv_Fast(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw16LineWInv_Fast(int x1, int y1, int x2, int y2, int thick) { DrawCan16LineWInv_Fast(pDrawCan16, x1, y1, x2, y2, thick); }
void Draw15LineWInv_Fast(int x1, int y1, int x2, int y2, int thick) { DrawCan15LineWInv_Fast(pDrawCan15, x1, y1, x2, y2, thick); }

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan16LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan16LineOverInv(can, x1, y1, x2, y2, False);
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
		DrawCan16LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan16LineOverInv(can, x1, y1, x2, y2, over);
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
		DrawCan16LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan16LineOverInv(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw16LineWInv(int x1, int y1, int x2, int y2, int thick) { DrawCan16LineWInv(pDrawCan16, x1, y1, x2, y2, thick); }
void Draw15LineWInv(int x1, int y1, int x2, int y2, int thick) { DrawCan15LineWInv(pDrawCan15, x1, y1, x2, y2, thick); }

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
void DrawCan16Round_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask)
{
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb/2;
	u16 *s0, *s;
	int r2;

	// hide parts of the round
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16Round_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan16Round_Fast(pDrawCan16, x, y, d, col, mask); }
void Draw15Round_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan15Round_Fast(pDrawCan15, x, y, d, col, mask); }

// Draw round (checks clipping limits)
void DrawCan16Round(sDrawCan* can, int x, int y, int d, u16 col, u8 mask)
{
	if (d < 1) d = 1;
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb/2;
	u16 *s0, *s;
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16Round(int x, int y, int d, u16 col, u8 mask) { DrawCan16Round(pDrawCan16, x, y, d, col, mask); }
void Draw15Round(int x, int y, int d, u16 col, u8 mask) { DrawCan15Round(pDrawCan15, x, y, d, col, mask); }

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan16RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	if (d < 1) d = 1;
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb/2;
	u16 *s0, *s;
	int r2;

	// hide parts of the round
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16RoundInv_Fast(int x, int y, int d, u8 mask) { DrawCan16RoundInv_Fast(pDrawCan16, x, y, d, mask); }
void Draw15RoundInv_Fast(int x, int y, int d, u8 mask) { DrawCan15RoundInv_Fast(pDrawCan15, x, y, d, mask); }

// Draw round inverted (checks clipping limits)
void DrawCan16RoundInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	int r = (d+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb/2;
	u16 *s0, *s;
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16RoundInv(int x, int y, int d, u8 mask) { DrawCan16RoundInv(pDrawCan16, x, y, d, mask); }
void Draw15RoundInv(int x, int y, int d, u8 mask) { DrawCan15RoundInv(pDrawCan15, x, y, d, mask); }

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
void DrawCan16Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan16Point_Fast(can, x, y, col);
		return;
	}

	// prepare
	int r = d/2;
	int xx = 0;
	int yy = r;
	int p = 1 - r;

	// pointer to middle of the circle
	int wb = can->wb/2;
	u16* s0 = &((u16*)can->buf)[x + (y - can->basey)*wb];
	u16* s;

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
void Draw16Circle_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan16Circle_Fast(pDrawCan16, x, y, d, col, mask); }
void Draw15Circle_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan15Circle_Fast(pDrawCan15, x, y, d, col, mask); }

// Draw circle or arc (checks clipping limits)
void DrawCan16Circle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan16Point(can, x, y, col);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan16Circle_Fast(can, x, y, d, col, mask);
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
	int wb = can->wb/2;
	int basey = can->basey;
	u16* s0 = (u16*)can->buf;
	u16* s;

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
void Draw16Circle(int x, int y, int d, u16 col, u8 mask) { DrawCan16Circle(pDrawCan16, x, y, d, col, mask); }
void Draw15Circle(int x, int y, int d, u16 col, u8 mask) { DrawCan15Circle(pDrawCan15, x, y, d, col, mask); }

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan16CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan16PointInv_Fast(can, x, y);
		return;
	}

	// prepare
	int r = d/2;
	int xx = 0;
	int yy = r;
	int p = 1 - r;

	// pointer to middle of the circle
	int wb = can->wb/2;
	u16* s0 = &((u16*)can->buf)[x + (y - can->basey)*wb];
	u16* s;

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
void Draw16CircleInv_Fast(int x, int y, int d, u8 mask) { DrawCan16CircleInv_Fast(pDrawCan16, x, y, d, mask); }
void Draw15CircleInv_Fast(int x, int y, int d, u8 mask) { DrawCan15CircleInv_Fast(pDrawCan15, x, y, d, mask); }

// Draw circle or arc inverted (checks clipping limits)
void DrawCan16CircleInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan16PointInv(can, x, y);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan16CircleInv_Fast(can, x, y, d, mask);
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
	int wb = can->wb/2;
	int basey = can->basey;
	u16* s0 = (u16*)can->buf;
	u16* s;

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
void Draw16CircleInv(int x, int y, int d, u8 mask) { DrawCan16CircleInv(pDrawCan16, x, y, d, mask); }
void Draw15CircleInv(int x, int y, int d, u8 mask) { DrawCan15CircleInv(pDrawCan15, x, y, d, mask); }

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
void DrawCan16Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask)
{
	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan16Round_Fast(can, x, y, d, col, mask);
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
		DrawCan16Circle_Fast(can, x, y, d, col, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb/2;
	u16 *s0, *s;
	int r2, rin2;

	// hide parts of the ring
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan16Ring_Fast(pDrawCan16, x, y, d, din, col, mask); }
void Draw15Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan15Ring_Fast(pDrawCan15, x, y, d, din, col, mask); }

// Draw ring (checks clipping limits)
void DrawCan16Ring(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask)
{
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan16Round(can, x, y, d, col, mask);
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
		DrawCan16Circle(can, x, y, d, col, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb/2;
	u16 *s0, *s;
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16Ring(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan16Ring(pDrawCan16, x, y, d, din, col, mask); }
void Draw15Ring(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan15Ring(pDrawCan15, x, y, d, din, col, mask); }

// Draw ring inverted (does not check clipping limits)
void DrawCan16RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan16RoundInv_Fast(can, x, y, d, mask);
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
		DrawCan16CircleInv_Fast(can, x, y, d, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb/2;
	u16 *s0, *s;
	int r2, rin2;

	// hide parts of the ring
	DRAWCANROUND_HIDE();

	// odd diameter (1, 3, 5, ...)
	if ((d & 1) != 0)
	{
		// prepare buffer address
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16RingInv_Fast(int x, int y, int d, int din, u8 mask) { DrawCan16RingInv_Fast(pDrawCan16, x, y, d, din, mask); }
void Draw15RingInv_Fast(int x, int y, int d, int din, u8 mask) { DrawCan15RingInv_Fast(pDrawCan15, x, y, d, din, mask); }

// Draw ring inverted (checks clipping limits)
void DrawCan16RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan16RoundInv(can, x, y, d, mask);
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
		DrawCan16CircleInv(can, x, y, d, mask2);
		return;
	}

	int r = (d+1)/2;
	int rin = (din+1)/2;
	int x1 = -r;
	int x2 = +r;
	int y1 = -r;
	int y2 = +r;
	int wb = can->wb/2;
	u16 *s0, *s;
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
		s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16RingInv(int x, int y, int d, int din, u8 mask) { DrawCan16RingInv(pDrawCan16, x, y, d, din, mask); }
void Draw15RingInv(int x, int y, int d, int din, u8 mask) { DrawCan15RingInv(pDrawCan15, x, y, d, din, mask); }

// ----------------------------------------------------------------------------
//                              Draw triangle
// ----------------------------------------------------------------------------

// Draw triangle (does not check clipping limits)
void DrawCan16Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col)
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
			if (k > 0) DrawCan16HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan16HLine_Fast(can, xmin, y, k, col);
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
			if (k > 0) DrawCan16HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan16HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan16HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan16HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw16Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan16Triangle_Fast(pDrawCan16, x1, y1, x2, y2, x3, y3, col); }
void Draw15Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan15Triangle_Fast(pDrawCan15, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle (checks clipping limits)
void DrawCan16Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col)
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
		DrawCan16Triangle_Fast(can, x1, y1, x2, y2, x3, y3, col);
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
			if (k > 0) DrawCan16HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan16HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan16HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan16HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan16HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan16HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw16Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan16Triangle(pDrawCan16, x1, y1, x2, y2, x3, y3, col); }
void Draw15Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan15Triangle(pDrawCan15, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle inverted (does not check clipping limits)
void DrawCan16TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
			if (k > 0) DrawCan16HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan16HLineInv_Fast(can, xmin, y, k);
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
			if (k > 0) DrawCan16HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan16HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan16HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan16HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw16TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan16TriangleInv_Fast(pDrawCan16, x1, y1, x2, y2, x3, y3); }
void Draw15TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan15TriangleInv_Fast(pDrawCan15, x1, y1, x2, y2, x3, y3); }

// Draw triangle inverted (checks clipping limits)
void DrawCan16TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
		DrawCan16TriangleInv_Fast(can, x1, y1, x2, y2, x3, y3);
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
			if (k > 0) DrawCan16HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan16HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan16HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan16HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan16HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan16HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw16TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan16TriangleInv(pDrawCan16, x1, y1, x2, y2, x3, y3); }
void Draw15TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan15TriangleInv(pDrawCan15, x1, y1, x2, y2, x3, y3); }

// ----------------------------------------------------------------------------
//                              Draw fill area
// ----------------------------------------------------------------------------

// Draw fill sub-area (internal function) ... 16-bit
void _DrawCan16SubFill(sDrawCan* can, int x, int y, u16 fnd, u16 col)
{
	int wb = can->wb/2;

	// prepare buffer address
	u16* s = &((u16*)can->buf)[x + (y - can->basey)*wb];

	// save start position -> x2, s2
	int x2 = x;
	u16* s2 = s;

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
	u16* s1 = s;

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
		s = s1 - wb;
		y--;
		while (x <= x2)
		{
			if (*s == fnd) _DrawCan16SubFill(can, x, y, fnd, col);
			x++;
			s++;
		}
		y++;
	}

	// search segments in DOWN-Y direction
	if (y < can->clipy2-1)
	{
		x = x1;
		s = s1 + wb;
		y++;
		while (x <= x2)
		{
			if (*s == fnd) _DrawCan16SubFill(can, x, y, fnd, col);
			x++;
			s++;
		}
	}

	// update dirty area in X direction
	if (x1 < can->dirtyx1) can->dirtyx1 = x1;
	if (x2 >= can->dirtyx2) can->dirtyx2 = x2+1;
}

// Draw fill area ... 16-bit
void DrawCan16Fill(sDrawCan* can, int x, int y, u16 col)
{
	// check clipping
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// get color to search
	u16 fnd = DrawCan16GetPoint_Fast(can, x, y);

	// fill if color is different
	if (fnd != col) _DrawCan16SubFill(can, x, y, fnd, col);
}
void Draw16Fill(int x, int y, u16 col) { DrawCan16Fill(pDrawCan16, x, y, col); }

// Draw fill sub-area (internal function) ... 15-bit
void _DrawCan15SubFill(sDrawCan* can, int x, int y, u16 fnd, u16 col)
{
	int wb = can->wb/2;

	// prepare buffer address
	u16* s = &((u16*)can->buf)[x + (y - can->basey)*wb];

	// save start position -> x2, s2
	int x2 = x;
	u16* s2 = s;

	// fill start point
	*s = col;

	// update dirty area in Y direction
	if (y < can->dirtyy1) can->dirtyy1 = y;
	if (y >= can->dirtyy2) can->dirtyy2 = y+1;

	// search start of segment in LEFT-X direction -> x1, s1
	int clipx1 = can->clipx1;
	while ((x > clipx1) && ((s[-1] & 0x7fff) == fnd))
	{
		x--;
		s--;
		*s = col;
	}
	int x1 = x;
	u16* s1 = s;

	// search end of segment in RIGHT-X direction -> x2, s2
	int clipx2 = can->clipx2-1;
	while ((x2 < clipx2) && ((s2[1] & 0x7fff) == fnd))
	{
		x2++;
		s2++;
		*s2 = col;
	}

	// search segments in UP-Y direction
	if (y > can->clipy1)
	{
		x = x1;
		s = s1 - wb;
		y--;
		while (x <= x2)
		{
			if ((*s & 0x7fff) == fnd) _DrawCan15SubFill(can, x, y, fnd, col);
			x++;
			s++;
		}
		y++;
	}

	// search segments in DOWN-Y direction
	if (y < can->clipy2-1)
	{
		x = x1;
		s = s1 + wb;
		y++;
		while (x <= x2)
		{
			if ((*s & 0x7fff) == fnd) _DrawCan15SubFill(can, x, y, fnd, col);
			x++;
			s++;
		}
	}

	// update dirty area in X direction
	if (x1 < can->dirtyx1) can->dirtyx1 = x1;
	if (x2 >= can->dirtyx2) can->dirtyx2 = x2+1;
}

// Draw fill area ... 15-bit
void DrawCan15Fill(sDrawCan* can, int x, int y, u16 col)
{
	// check clipping
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// get color to search
	u16 fnd = DrawCan15GetPoint_Fast(can, x, y);

	// fill if color is different
	if (fnd != col) _DrawCan15SubFill(can, x, y, fnd, col);
}
void Draw15Fill(int x, int y, u16 col) { DrawCan15Fill(pDrawCan15, x, y, col); }

// ----------------------------------------------------------------------------
//                             Draw charactter
// ----------------------------------------------------------------------------
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw16SelFont(const u8* font, u8 fontw, u8 fonth) { DrawCanSelFont(pDrawCan16, font, fontw, fonth); }

void Draw16SelFont8x8()		{ DrawCanSelFont8x8(pDrawCan16); }		// sans-serif bold, height 8 lines
void Draw16SelFont8x14()	{ DrawCanSelFont8x14(pDrawCan16); }		// sans-serif bold, height 14 lines
void Draw16SelFont8x16()	{ DrawCanSelFont8x16(pDrawCan16); }		// sans-serif bold, height 16 lines
void Draw16SelFont8x14Serif()	{ DrawCanSelFont8x14Serif(pDrawCan16); }	// serif bold, height 14 lines
void Draw16SelFont8x16Serif()	{ DrawCanSelFont8x16Serif(pDrawCan16); }	// serif bold, height 16 lines
void Draw16SelFont6x8()		{ DrawCanSelFont6x8(pDrawCan16); }		// condensed font, width 6 pixels, height 8 lines
void Draw16SelFont8x8Game()	{ DrawCanSelFont8x8Game(pDrawCan16); }		// game font, height 8 lines
void Draw16SelFont8x8Ibm()	{ DrawCanSelFont8x8Ibm(pDrawCan16); }		// IBM charset font, height 8 lines
void Draw16SelFont8x14Ibm()	{ DrawCanSelFont8x14Ibm(pDrawCan16); }		// IBM charset font, height 14 lines
void Draw16SelFont8x16Ibm()	{ DrawCanSelFont8x16Ibm(pDrawCan16); }		// IBM charset font, height 16 lines
void Draw16SelFont8x8IbmThin()	{ DrawCanSelFont8x8IbmThin(pDrawCan16); }	// IBM charset thin font, height 8 lines
void Draw16SelFont8x8Italic()	{ DrawCanSelFont8x8Italic(pDrawCan16); }	// italic, height 8 lines
void Draw16SelFont8x8Thin()	{ DrawCanSelFont8x8Thin(pDrawCan16); }		// thin font, height 8 lines
void Draw16SelFont5x8()		{ DrawCanSelFont5x8(pDrawCan16); }		// tiny font, width 5 pixels, height 8 lines

void Draw15SelFont(const u8* font, u8 fontw, u8 fonth) { DrawCanSelFont(pDrawCan15, font, fontw, fonth); }

void Draw15SelFont8x8()		{ DrawCanSelFont8x8(pDrawCan15); }		// sans-serif bold, height 8 lines
void Draw15SelFont8x14()	{ DrawCanSelFont8x14(pDrawCan15); }		// sans-serif bold, height 14 lines
void Draw15SelFont8x16()	{ DrawCanSelFont8x16(pDrawCan15); }		// sans-serif bold, height 16 lines
void Draw15SelFont8x14Serif()	{ DrawCanSelFont8x14Serif(pDrawCan15); }	// serif bold, height 14 lines
void Draw15SelFont8x16Serif()	{ DrawCanSelFont8x16Serif(pDrawCan15); }	// serif bold, height 16 lines
void Draw15SelFont6x8()		{ DrawCanSelFont6x8(pDrawCan15); }		// condensed font, width 6 pixels, height 8 lines
void Draw15SelFont8x8Game()	{ DrawCanSelFont8x8Game(pDrawCan15); }		// game font, height 8 lines
void Draw15SelFont8x8Ibm()	{ DrawCanSelFont8x8Ibm(pDrawCan15); }		// IBM charset font, height 8 lines
void Draw15SelFont8x14Ibm()	{ DrawCanSelFont8x14Ibm(pDrawCan15); }		// IBM charset font, height 14 lines
void Draw15SelFont8x16Ibm()	{ DrawCanSelFont8x16Ibm(pDrawCan15); }		// IBM charset font, height 16 lines
void Draw15SelFont8x8IbmThin()	{ DrawCanSelFont8x8IbmThin(pDrawCan15); }	// IBM charset thin font, height 8 lines
void Draw15SelFont8x8Italic()	{ DrawCanSelFont8x8Italic(pDrawCan15); }	// italic, height 8 lines
void Draw15SelFont8x8Thin()	{ DrawCanSelFont8x8Thin(pDrawCan15); }		// thin font, height 8 lines
void Draw15SelFont5x8()		{ DrawCanSelFont5x8(pDrawCan15); }		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan16Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley)
{
	int i, j, wb, scalex2, scaley2;
	u8 b;

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// destination address
	wb = can->wb/2;
	u16* d0 = &((u16*)can->buf)[x + (y - can->basey)*wb];
	u16* d;

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
void Draw16Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan16Char_Fast(pDrawCan16, ch, x, y, col, scalex, scaley); }
void Draw15Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan15Char_Fast(pDrawCan15, ch, x, y, col, scalex, scaley); }

// Draw character with transparent background (checks clipping limits)
void DrawCan16Char(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan16Char_Fast(can, ch, x, y, col, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan16Point(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan16Point(can, x, y, col);
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
void Draw16Char(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan16Char(pDrawCan16, ch, x, y, col, scalex, scaley); }
void Draw15Char(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan15Char(pDrawCan15, ch, x, y, col, scalex, scaley); }

// Draw character with background (does not check clipping limits)
void DrawCan16CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
{
	int i, j, wb, scaley2, scalex2;
	u8 b;

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// destination address
	wb = can->wb/2;
	u16* d0 = &((u16*)can->buf)[x + (y - can->basey)*wb];
	u16* d;

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
void Draw16CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan16CharBg_Fast(pDrawCan16, ch, x, y, col, bgcol, scalex, scaley); }
void Draw15CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan15CharBg_Fast(pDrawCan15, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character with background (checks clipping limits)
void DrawCan16CharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan16CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);
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
				DrawCan16Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				DrawCan16Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw16CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan16CharBg(pDrawCan16, ch, x, y, col, bgcol, scalex, scaley); }
void Draw15CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan15CharBg(pDrawCan15, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character inverted (does not check clipping limits)
void DrawCan16CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
{
	int i, j, wb, scaley2, scalex2;
	u8 b;

	// prepare pointer to font sample
	const u8* s = &can->font[(u8)ch];

	// destination address
	wb = can->wb/2;
	u16* d0 = &((u16*)can->buf)[x + (y - can->basey)*wb];
	u16* d;

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
void Draw16CharInv_Fast(char ch, int x, int y, int scalex, int scaley) { DrawCan16CharInv_Fast(pDrawCan16, ch, x, y, scalex, scaley); }
void Draw15CharInv_Fast(char ch, int x, int y, int scalex, int scaley) { DrawCan15CharInv_Fast(pDrawCan15, ch, x, y, scalex, scaley); }

// Draw character inverted (checks clipping limits)
void DrawCan16CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
		DrawCan16CharInv_Fast(can, ch, x, y, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan16PointInv(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan16PointInv(can, x, y);
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
void Draw16CharInv(char ch, int x, int y, int scalex, int scaley) { DrawCan16CharInv(pDrawCan16, ch, x, y, scalex, scaley); }
void Draw15CharInv(char ch, int x, int y, int scalex, int scaley) { DrawCan15CharInv(pDrawCan15, ch, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                             Draw text
// ----------------------------------------------------------------------------
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan16Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan16Char_Fast(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw16Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan16Text_Fast(pDrawCan16, text, len, x, y, col, scalex, scaley); }
void Draw15Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan15Text_Fast(pDrawCan15, text, len, x, y, col, scalex, scaley); }

// Draw text with transparent background (checks clipping limits)
void DrawCan16Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan16Char(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw16Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan16Text(pDrawCan16, text, len, x, y, col, scalex, scaley); }
void Draw15Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan15Text(pDrawCan15, text, len, x, y, col, scalex, scaley); }

// Draw text with background (does not check clipping limits)
void DrawCan16TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan16CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw16TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan16TextBg_Fast(pDrawCan16, text, len, x, y, col, bgcol, scalex, scaley); }
void Draw15TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan15TextBg_Fast(pDrawCan15, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text with background (checks clipping limits)
void DrawCan16TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan16CharBg(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw16TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan16TextBg(pDrawCan16, text, len, x, y, col, bgcol, scalex, scaley); }
void Draw15TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan15TextBg(pDrawCan15, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text inverted (does not check clipping limits)
void DrawCan16TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan16CharInv_Fast(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw16TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan16TextInv_Fast(pDrawCan16, text, len, x, y, scalex, scaley); }
void Draw15TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan15TextInv_Fast(pDrawCan15, text, len, x, y, scalex, scaley); }

// Draw text inverted (checks clipping limits)
void DrawCan16TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan16CharInv(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw16TextInv(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan16TextInv(pDrawCan16, text, len, x, y, scalex, scaley); }
void Draw15TextInv(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan15TextInv(pDrawCan15, text, len, x, y, scalex, scaley); }

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
void DrawCan16Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask)
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
	int wb = can->wb/2;
	u16* s0 = &((u16*)can->buf)[x + (y - can->basey)*wb];
	u16* s;

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
void Draw16Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan16Ellipse_Fast(pDrawCan16, x, y, dx, dy, col, mask); }
void Draw15Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan15Ellipse_Fast(pDrawCan15, x, y, dx, dy, col, mask); }

// Draw ellipse (checks clipping limits)
void DrawCan16Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask)
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
		DrawCan16Ellipse_Fast(can, x, y, dx, dy, col, mask);
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
	int wb = can->wb/2;
	int basey = can->basey;
	u16* s0 = (u16*)can->buf;
	u16* s;

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
void Draw16Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan16Ellipse(pDrawCan16, x, y, dx, dy, col, mask); }
void Draw15Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan15Ellipse(pDrawCan15, x, y, dx, dy, col, mask); }

// Draw ellipse inverted (does not check clipping limits)
void DrawCan16EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	int wb = can->wb/2;
	u16* s0 = &((u16*)can->buf)[x + (y - can->basey)*wb];
	u16* s;

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
void Draw16EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan16EllipseInv_Fast(pDrawCan16, x, y, dx, dy, mask); }
void Draw15EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan15EllipseInv_Fast(pDrawCan15, x, y, dx, dy, mask); }

// Draw ellipse inverted (checks clipping limits)
void DrawCan16EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
		DrawCan16EllipseInv_Fast(can, x, y, dx, dy, mask);
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
	int wb = can->wb/2;
	int basey = can->basey;
	u16* s0 = (u16*)can->buf;
	u16* s;

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
void Draw16EllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan16EllipseInv(pDrawCan16, x, y, dx, dy, mask); }
void Draw15EllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan15EllipseInv(pDrawCan15, x, y, dx, dy, mask); }

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
void DrawCan16FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask)
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
	int wb = can->wb/2;
	u16 *s0, *s;
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
	s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan16FillEllipse_Fast(pDrawCan16, x, y, dx, dy, col, mask); }
void Draw15FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan15FillEllipse_Fast(pDrawCan15, x, y, dx, dy, col, mask); }

// Draw filled ellipse (checks clipping limits)
void DrawCan16FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask)
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
	int wb = can->wb/2;
	u16 *s0, *s;
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
	s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan16FillEllipse(pDrawCan16, x, y, dx, dy, col, mask); }
void Draw15FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan15FillEllipse(pDrawCan15, x, y, dx, dy, col, mask); }

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan16FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	int wb = can->wb/2;
	u16 *s0, *s;
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
	s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan16FillEllipseInv_Fast(pDrawCan16, x, y, dx, dy, mask); }
void Draw15FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan15FillEllipseInv_Fast(pDrawCan15, x, y, dx, dy, mask); }

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan16FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	int wb = can->wb/2;
	u16 *s0, *s;
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
	s0 = &((u16*)can->buf)[(x+x1) + ((y+y1) - can->basey)*wb];
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
void Draw16FillEllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan16FillEllipseInv(pDrawCan16, x, y, dx, dy, mask); }
void Draw15FillEllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan15FillEllipseInv(pDrawCan15, x, y, dx, dy, mask); }

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
void DrawCan16Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs/2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	wbs = ws;
	const u16* s = &((const u16*)src)[xs + ys*wbs];

	// copy image
	w *= 2;
	for (; h > 0; h--)
	{
		memcpy(d, s, w);
		d += wbd;
		s += wbs;
	}
}
void Draw16Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan16Img(pDrawCan16, xd, yd, src, xs, ys, w, h, wbs); }
void Draw15Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan15Img(pDrawCan15, xd, yd, src, xs, ys, w, h, wbs); }

// Draw image inverted with the same format as destination
void DrawCan16ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs/2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	wbs = ws;
	const u16* s = &((const u16*)src)[xs + ys*wbs];

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
void Draw16ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan16ImgInv(pDrawCan16, xd, yd, src, xs, ys, w, h, wbs); }
void Draw15ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan15ImgInv(pDrawCan15, xd, yd, src, xs, ys, w, h, wbs); }

// Draw transparent image with the same format as destination ... 16-bit
void DrawCan16Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col)
{
	// width of source image
	int ws = wbs/2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	wbs = ws;
	const u16* s = &((const u16*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u16 c;
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
void Draw16Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan16Blit(pDrawCan16, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image with the same format as destination ... 15-bit
void DrawCan15Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col)
{
	// width of source image
	int ws = wbs/2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	wbs = ws;
	const u16* s = &((const u16*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u16 c;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			c = *s & 0x7fff;
			if (c != col) *d = c;
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw15Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan15Blit(pDrawCan15, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image with the same format as destination, with substitute color ... 16-bit
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan16BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs)
{
	// width of source image
	int ws = wbs/2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	wbs = ws;
	const u16* s = &((const u16*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u16 c;
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
void Draw16BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { DrawCan16BlitSubs(pDrawCan16, xd, yd, src, xs, ys, w, h, wbs, col, fnd, subs); }

// Draw transparent image with the same format as destination, with substitute color ... 15-bit
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan15BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs)
{
	// width of source image
	int ws = wbs/2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	wbs = ws;
	const u16* s = &((const u16*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u16 c;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			c = *s & 0x7fff;
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
void Draw15BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { DrawCan15BlitSubs(pDrawCan15, xd, yd, src, xs, ys, w, h, wbs, col, fnd, subs); }

// Draw transparent image inverted with the same format as destination ... 16-bit
void DrawCan16BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col)
{
	// width of source image
	int ws = wbs/2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	wbs = ws;
	const u16* s = &((const u16*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u16 c;
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
void Draw16BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan16BlitInv(pDrawCan16, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image inverted with the same format as destination ... 15-bit
void DrawCan15BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col)
{
	// width of source image
	int ws = wbs/2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	wbs = ws;
	const u16* s = &((const u16*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u16 c;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			c = *s & 0x7fff;
			if (c != col) *d ^= c;
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw15BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan15BlitInv(pDrawCan15, xd, yd, src, xs, ys, w, h, wbs, col); }

// Get image from canvas to buffer ... 16-bit
//  can ... source canvas
//  xs ... source X coordinate
//  ys ... source Y coordinate
//  w ... width
//  h ... height
//  dst ... destination buffer
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  wbd ... pitch of destination buffer (length of line in bytes)
void DrawCan16GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd)
{
	wbd /= 2;

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
	int wbs = can->wb/2;
	const u16* s = &((const u16*)can->buf)[xs + (ys - can->basey)*wbs];

	// destination address
	u16* d = &((u16*)dst)[xd + yd*wbd];

	// copy image
	w *= 2;
	for (; h > 0; h--)
	{
		memcpy(d, s, w);
		d += wbd;
		s += wbs;
	}
}
void Draw16GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { DrawCan16GetImg(pDrawCan16, xs, ys, w, h, dst, xd, yd, wbd); }

// Get image from canvas to buffer ... 15-bit
//  can ... source canvas
//  xs ... source X coordinate
//  ys ... source Y coordinate
//  w ... width
//  h ... height
//  dst ... destination buffer
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  wbd ... pitch of destination buffer (length of line in bytes)
void DrawCan15GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd)
{
	wbd /= 2;

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
	int wbs = can->wb/2;
	const u16* s = &((const u16*)can->buf)[xs + (ys - can->basey)*wbs];

	// destination address
	u16* d = &((u16*)dst)[xd + yd*wbd];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			*d = *s & 0x7fff;
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw15GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { DrawCan15GetImg(pDrawCan15, xs, ys, w, h, dst, xd, yd, wbd); }

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
void DrawCan16ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src_img, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
{
	const u16* src = (const u16*)src_img;

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
	u16* d = &((u16*)can->buf)[xd + can->wb*(yd - can->basey)/2]; // destination image
	int wbd = can->wb/2 - wd;
	int i, x2, y2;
	wbs >>= 1;

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

void DrawCan15ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan16ImgMat(can, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

void Draw16ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan16ImgMat(pDrawCan16, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

void Draw15ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan16ImgMat(pDrawCan16, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

// ----------------------------------------------------------------------------
//                   Draw image specific to 16-bit canvas
// ----------------------------------------------------------------------------
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

// Draw 12-bit image to 16-bit destination canvas
void DrawCan16Img12(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2/3;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2*3 + ys*wbs];
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		s = s0;
		xxs = xs;
		for (i = w; i > 0; i--)
		{
			if ((xxs & 1) == 0)
			{
				c = GETPIXEL12_EVEN(s);
			}
			else
			{
				c = GETPIXEL12_ODD(s);
				s += 3;
			}
			xxs++;
			*d = COL12TO16(c);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Img12(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan16Img12(pDrawCan16, xd, yd, src, xs, ys, w, h, wbs); }

// Draw 8-bit palleted image to 16-bit destination canvas
void DrawCan16Img8(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

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
			*d = pal[c];
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw16Img8(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan16Img8(pDrawCan16, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 6-bit palleted image to 16-bit destination canvas
void DrawCan16Img6(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*5/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/5;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*5)*6;
	const u32* s;

	// copy image
	int i;
	wbd -= w;
	wbs /= 4;
	u8 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x3f);
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}
			*d = pal[c];
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Img6(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan16Img6(pDrawCan16, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 4-bit palleted image to 16-bit destination canvas
void DrawCan16Img4(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2 + ys*wbs];
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u8 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xs;
		s = s0;
		for (i = w; i > 0; i--)
		{
			if ((xxs & 1) == 0)
			{
				c = GETPIXEL4_EVEN(s);
			}
			else
			{
				c = GETPIXEL4_ODD(s);
				s++;
			}
			xxs++;
			*d = pal[c];
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Img4(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan16Img4(pDrawCan16, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 3-bit palleted image to 16-bit destination canvas
void DrawCan16Img3(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*10/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/10;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*10)*3;
	const u32* s;

	// copy image
	int i;
	wbd -= w;
	wbs /= 4;
	u8 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x07);
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}
			*d = pal[c];
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Img3(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan16Img3(pDrawCan16, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 2-bit palleted image to 16-bit destination canvas
void DrawCan16Img2(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/4;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = (3 - (xs & 3))*2;
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u8 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x03);
			xxs -= 2;
			if (xxs < 0)
			{
				xxs = 6;
				s++;
			}
			*d = pal[c];
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Img2(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan16Img2(pDrawCan16, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 1-bit image with specific colors to 16-bit destination canvas
//  col ... color of pixel with value '1'
//  bgcol ... color of pixel with value '0'
void DrawCan16Img1(sDrawCan* can, int xd, int yd, const void* src, u16 col, u16 bgcol, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*8;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/8;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = 7 - (xs & 1);
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u8 c;
	int xxs;
	u16 pal[2];
	pal[0] = bgcol;
	pal[1] = col;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x01);
			xxs--;
			if (xxs < 0)
			{
				xxs = 7;
				s++;
			}
			*d = pal[c];
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Img1(int xd, int yd, const void* src, u16 bgcol, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan16Img1(pDrawCan16, xd, yd, src, col, bgcol, xs, ys, w, h, wbs); }

// Draw 12-bit image transparent to 16-bit destination canvas
void DrawCan16Blit12(sDrawCan* can, int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2/3;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2*3 + ys*wbs];
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		s = s0;
		xxs = xs;
		for (i = w; i > 0; i--)
		{
			if ((xxs & 1) == 0)
			{
				c = GETPIXEL12_EVEN(s);
			}
			else
			{
				c = GETPIXEL12_ODD(s);
				s += 3;
			}
			xxs++;
			if (c != col) *d = COL12TO16(c);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Blit12(int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan16Blit12(pDrawCan16, xd, yd, src, col, xs, ys, w, h, wbs); }

// Draw 8-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit8(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s = &((const u8*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u16 c;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			c = pal[*s];
			if (c != col) *d = c;
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw16Blit8(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan16Blit8(pDrawCan16, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 6-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit6(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*5/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/5;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*5)*6;
	const u32* s;

	// copy image
	int i;
	wbd -= w;
	wbs /= 4;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = pal[(*s >> xxs) & 0x3f];
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}
			if (c != col) *d = c;
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Blit6(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan16Blit6(pDrawCan16, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 4-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit4(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2 + ys*wbs];
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xs;
		s = s0;
		for (i = w; i > 0; i--)
		{
			if ((xxs & 1) == 0)
			{
				c = pal[GETPIXEL4_EVEN(s)];
			}
			else
			{
				c = pal[GETPIXEL4_ODD(s)];
				s++;
			}
			xxs++;
			if (c != col) *d = c;
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Blit4(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan16Blit4(pDrawCan16, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 3-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit3(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*10/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/10;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*10)*3;
	const u32* s;

	// copy image
	int i;
	wbd -= w;
	wbs /= 4;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = pal[(*s >> xxs) & 0x07];
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}
			if (c != col) *d = c;
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Blit3(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan16Blit3(pDrawCan16, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 2-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit2(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/4;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = (3 - (xs & 3))*2;
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = pal[(*s >> xxs) & 0x03];
			xxs -= 2;
			if (xxs < 0)
			{
				xxs = 6;
				s++;
			}
			if (c != col) *d = c;
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Blit2(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan16Blit2(pDrawCan16, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 1-bit image transparent with specific colors to 16-bit destination canvas
//  col ... color of non-transparent pixel with value '1'
void DrawCan16Blit1(sDrawCan* can, int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*8;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/8;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = 7 - (xs & 1);
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u8 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x01);
			xxs--;
			if (xxs < 0)
			{
				xxs = 7;
				s++;
			}
			if (c != 0) *d = col;
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw16Blit1(int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan16Blit1(pDrawCan16, xd, yd, src, col, xs, ys, w, h, wbs); }

/* @TODO
// Draw 4-bit palleted image with 2D transformation matrix
//  can ... destination canvas
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  wd ... destination width
//  hd ... destination height
//  src ... source image
//  pal ... pallets
//  ws ... source image width
//  hs ... source image height
//  wbs ... pitch of source image (length of line in bytes)
//  m ... transformation matrix (should be prepared using PrepDrawImg function)
//  mode ... draw mode DRAWIMG_*
//  color ... key or border color (DRAWIMG_PERSP mode: horizon offset)
// Note to wrap and perspective mode: Width and height of source image should be power of 2, or it will render slower.
void DrawCan16Img4Mat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src_img, const u16* pal, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
{
	const u8* src = (const u16*)src_img;

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
	u16* d = &((u16*)can->buf)[xd + can->wb*(yd - can->basey)/2]; // destination image
	int wbd = can->wb/2 - wd;
	int i, x2, y2;
	const u8* s;
	u8 pix;

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
					s = &src[x2/2 + y2*wbs];
					*d++ = pal[GETPIXEL4(s, x2)];
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
					s = &src[x2/2 + y2*wbs];
					*d++ = pal[GETPIXEL4(s, x2)];
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
					if (((u32)x2 < ww) && ((u32)y2 < hh))
					{
						s = &src[x2/2 + y2*wbs];
						*d = pal[GETPIXEL4(s, x2)];
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

void Draw16Img4Mat(int xd, int yd, int wd, int hd, const void* src, const u16* pal, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan16Img4Mat(pDrawCan16, xd, yd, wd, hd, src, pal, ws, hs, wbs, m, mode, color); }
*/

// ----------------------------------------------------------------------------
//                   Draw image specific to 15-bit canvas
// ----------------------------------------------------------------------------
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

// Draw 12-bit image to 15-bit destination canvas
void DrawCan15Img12(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2/3;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2*3 + ys*wbs];
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		s = s0;
		xxs = xs;
		for (i = w; i > 0; i--)
		{
			if ((xxs & 1) == 0)
			{
				c = GETPIXEL12_EVEN(s);
			}
			else
			{
				c = GETPIXEL12_ODD(s);
				s += 3;
			}
			xxs++;
			*d = COL12TO15(c);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Img12(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan15Img12(pDrawCan15, xd, yd, src, xs, ys, w, h, wbs); }

// Draw 8-bit palleted image to 15-bit destination canvas
void DrawCan15Img8(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

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
			*d = COL16TO15(pal[c]);
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw15Img8(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan15Img8(pDrawCan15, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 6-bit palleted image to 15-bit destination canvas
void DrawCan15Img6(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*5/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/5;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*5)*6;
	const u32* s;

	// copy image
	int i;
	wbd -= w;
	wbs /= 4;
	u8 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x3f);
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}
			*d = COL16TO15(pal[c]);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Img6(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan15Img6(pDrawCan15, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 4-bit palleted image to 15-bit destination canvas
void DrawCan15Img4(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2 + ys*wbs];
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u8 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xs;
		s = s0;
		for (i = w; i > 0; i--)
		{
			if ((xxs & 1) == 0)
			{
				c = GETPIXEL4_EVEN(s);
			}
			else
			{
				c = GETPIXEL4_ODD(s);
				s++;
			}
			xxs++;
			*d = COL16TO15(pal[c]);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Img4(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan15Img4(pDrawCan15, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 3-bit palleted image to 15-bit destination canvas
void DrawCan15Img3(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*10/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/10;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*10)*3;
	const u32* s;

	// copy image
	int i;
	wbd -= w;
	wbs /= 4;
	u8 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x07);
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}
			*d = COL16TO15(pal[c]);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Img3(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan15Img3(pDrawCan15, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 2-bit palleted image to 15-bit destination canvas
void DrawCan15Img2(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/4;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = (3 - (xs & 3))*2;
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u8 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = (u8)((*s >> xxs) & 0x03);
			xxs -= 2;
			if (xxs < 0)
			{
				xxs = 6;
				s++;
			}
			*d = COL16TO15(pal[c]);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Img2(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs) { DrawCan15Img2(pDrawCan15, xd, yd, src, pal, xs, ys, w, h, wbs); }

// Draw 1-bit image with specific colors to 15-bit destination canvas
//  col ... color of pixel with value '1'
//  bgcol ... color of pixel with value '0'
void Draw15Img1(int xd, int yd, const void* src, u16 bgcol, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan15Img1(pDrawCan15, xd, yd, src, col, bgcol, xs, ys, w, h, wbs); }

// Draw 12-bit image transparent to 15-bit destination canvas
void DrawCan15Blit12(sDrawCan* can, int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2/3;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2*3 + ys*wbs];
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		s = s0;
		xxs = xs;
		for (i = w; i > 0; i--)
		{
			if ((xxs & 1) == 0)
			{
				c = GETPIXEL12_EVEN(s);
			}
			else
			{
				c = GETPIXEL12_ODD(s);
				s += 3;
			}
			xxs++;
			c = COL12TO16(c);
			if (c != col) *d = COL16TO15(c);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Blit12(int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan15Blit12(pDrawCan15, xd, yd, src, col, xs, ys, w, h, wbs); }

// Draw 8-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit8(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s = &((const u8*)src)[xs + ys*wbs];

	// copy image
	int i;
	wbd -= w;
	wbs -= w;
	u16 c;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			c = pal[*s];
			if (c != col) *d = COL16TO15(c);
			d++;
			s++;
		}
		d += wbd;
		s += wbs;
	}
}
void Draw15Blit8(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan15Blit8(pDrawCan15, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 6-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit6(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*5/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/5;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*5)*6;
	const u32* s;

	// copy image
	int i;
	wbd -= w;
	wbs /= 4;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = pal[(*s >> xxs) & 0x3f];
			xxs += 6;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}
			if (c != col) *d = COL16TO15(c);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Blit6(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan15Blit6(pDrawCan15, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 4-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit4(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2 + ys*wbs];
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xs;
		s = s0;
		for (i = w; i > 0; i--)
		{
			if ((xxs & 1) == 0)
			{
				c = GETPIXEL4_EVEN(s);
			}
			else
			{
				c = GETPIXEL4_ODD(s);
				s++;
			}
			xxs++;
			c = pal[c];
			if (c != col) *d = COL16TO15(c);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Blit4(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan15Blit4(pDrawCan15, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 3-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit3(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*10/4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/10;
	const u32* s0 = (u32*)&((const u8*)src)[xsint*4 + ys*wbs];
	int xsfrac = (xs - xsint*10)*3;
	const u32* s;

	// copy image
	int i;
	wbd -= w;
	wbs /= 4;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = pal[(*s >> xxs) & 0x07];
			xxs += 3;
			if (xxs == 30)
			{
				xxs = 0;
				s++;
			}
			if (c != col) *d = COL16TO15(c);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Blit3(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan15Blit3(pDrawCan15, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 2-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit2(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*4;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb/2;
	u16* d = &((u16*)can->buf)[xd + (yd - can->basey)*wbd];

	// source address
	int xsint = xs/4;
	const u8* s0 = &((const u8*)src)[xsint + ys*wbs];
	int xsfrac = (3 - (xs & 3))*2;
	const u8* s;

	// copy image
	int i;
	wbd -= w;
	u16 c;
	int xxs;
	for (; h > 0; h--)
	{
		xxs = xsfrac;
		s = s0;
		for (i = w; i > 0; i--)
		{
			c = pal[(*s >> xxs) & 0x03];
			xxs -= 2;
			if (xxs < 0)
			{
				xxs = 6;
				s++;
			}
			if (c != col) *d = COL16TO15(c);
			d++;
		}
		d += wbd;
		s0 += wbs;
	}
}
void Draw15Blit2(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan15Blit2(pDrawCan15, xd, yd, src, pal, col, xs, ys, w, h, wbs); }

// Draw 1-bit image transparent with specific colors to 15-bit destination canvas
//  col ... color of non-transparent pixel with value '1'
void Draw15Blit1(int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs) { DrawCan15Blit1(pDrawCan15, xd, yd, src, col, xs, ys, w, h, wbs); }

#undef DRAWCAN_IMGLIMIT

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// drawing function interface 16-bit
const sDrawCanFnc DrawCan16Fnc = {
	.pDrawPitch		= Draw16Pitch,			// calculate pitch of graphics line from image width
	.pDrawMaxWidth		= Draw16MaxWidth,		// calculate max. width of image from the pitch
	// Clear canvas
	.pDrawCanClearCol	= DrawCan16ClearCol,		// Clear canvas with color
	.pDrawCanClear		= DrawCan16Clear,		// Clear canvas with black color
	// Draw point
	.pDrawCanPoint_Fast	= DrawCan16Point_Fast,		// Draw point (does not check clipping limits)
	.pDrawCanPoint		= DrawCan16Point,		// Draw point (checks clipping limits)
	.pDrawCanPointInv_Fast	= DrawCan16PointInv_Fast,	// Draw point inverted (does not check clipping limits)
	.pDrawCanPointInv	= DrawCan16PointInv,		// Draw point inverted (checks clipping limits)
	// Get point
	.pDrawCanGetPoint_Fast	= DrawCan16GetPoint_Fast,	// Get point (does not check clipping limits)
	.pDrawCanGetPoint	= DrawCan16GetPoint,		// Get point (checks clipping limits)
	// Draw rectangle
	.pDrawCanRect_Fast	= DrawCan16Rect_Fast,		// Draw rectangle (does not check clipping limits)
	.pDrawCanRect		= DrawCan16Rect,		// Draw rectangle (checks clipping limits)
	.pDrawCanRectInv_Fast	= DrawCan16RectInv_Fast,	// Draw rectangle inverted (does not check clipping limits)
	.pDrawCanRectInv	= DrawCan16RectInv,		// Draw rectangle inverted (checks clipping limits
	// Draw horizontal line
	.pDrawCanHLine_Fast	= DrawCan16HLine_Fast,		// Draw horizontal line (does not check clipping limits)
	.pDrawCanHLine		= DrawCan16HLine,		// Draw horizontal line (checks clipping limits; negative width flip line)
	.pDrawCanHLineInv_Fast	= DrawCan16HLineInv_Fast,	// Draw horizontal line inverted (does not check clipping limits)
	.pDrawCanHLineInv	= DrawCan16HLineInv,		// Draw horizontal line inverted (checks clipping limits; negative width flip line)
	// Draw vertical line
	.pDrawCanVLine_Fast	= DrawCan16VLine_Fast,		// Draw vertical line (does not check clipping limits)
	.pDrawCanVLine		= DrawCan16VLine,		// Draw vertical line (checks clipping limits; negative height flip line)
	.pDrawCanVLineInv_Fast	= DrawCan16VLineInv_Fast,	// Draw vertical line inverted (does not check clipping limits)
	.pDrawCanVLineInv	= DrawCan16VLineInv,		// Draw vertical line inverted (checks clipping limits; negative height flip line)
	// Draw frame
	.pDrawCanFrame_Fast	= DrawCan16Frame_Fast,		// Draw 1-pixel frame (does not check clipping limits)
	.pDrawCanFrame		= DrawCan16Frame,		// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameInv_Fast	= DrawCan16FrameInv_Fast,	// Draw 1-pixel frame inverted (does not check clipping limits)
	.pDrawCanFrameInv	= DrawCan16FrameInv,		// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameW_Fast	= DrawCan16FrameW_Fast,		// Draw thick frame (does not check clipping limits)
	.pDrawCanFrameW		= DrawCan16FrameW,		// Draw thick frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameWInv_Fast	= DrawCan16FrameWInv_Fast,	// Draw thick frame inverted (does not check clipping limits)
	.pDrawCanFrameWInv	= DrawCan16FrameWInv,		// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
	// Draw line
	.pDrawCanLineOver_Fast	= DrawCan16LineOver_Fast,	// Draw line with overlapped pixels (does not check clipping limits)
	.pDrawCanLine_Fast	= DrawCan16Line_Fast,		// Draw line (does not check clipping limits)
	.pDrawCanLineOver	= DrawCan16LineOver,		// Draw line with overlapped pixels (checks clipping limits)
	.pDrawCanLine		= DrawCan16Line,		// Draw line (checks clipping limits)
	.pDrawCanLineOverInv_Fast = DrawCan16LineOverInv_Fast,	// Draw line inverted with overlapped pixels (does not check clipping limits)
	.pDrawCanLineInv_Fast	= DrawCan16LineInv_Fast,	// Draw line inverted (does not check clipping limits)
	.pDrawCanLineOverInv	= DrawCan16LineOverInv,		// Draw line inverted with overlapped pixels (checks clipping limits)
	.pDrawCanLineInv	= DrawCan16LineInv,		// Draw line inverted (checks clipping limits)
	.pDrawCanLineW_Fast	= DrawCan16LineW_Fast,		// Draw thick line (does not check clipping limits)
	.pDrawCanLineW		= DrawCan16LineW,		// Draw thick line (checks clipping limits)
	.pDrawCanLineWInv_Fast	= DrawCan16LineWInv_Fast,	// Draw thick line inverted (does not check clipping limits)
	.pDrawCanLineWInv	= DrawCan16LineWInv,		// Draw thick line inverted (checks clipping limits)
	// Draw round (filled circle)
	.pDrawCanRound_Fast	= DrawCan16Round_Fast,		// Draw round (does not check clipping limits)
	.pDrawCanRound		= DrawCan16Round,		// Draw round (checks clipping limits)
	.pDrawCanRoundInv_Fast	= DrawCan16RoundInv_Fast,	// Draw round inverted (does not check clipping limits)
	.pDrawCanRoundInv	= DrawCan16RoundInv,		// Draw round inverted (checks clipping limits)
	// Draw circle
	.pDrawCanCircle_Fast	= DrawCan16Circle_Fast,		// Draw circle or arc (does not check clipping limits)
	.pDrawCanCircle		= DrawCan16Circle,		// Draw circle or arc (checks clipping limits)
	.pDrawCanCircleInv_Fast	= DrawCan16CircleInv_Fast,	// Draw circle or arc inverted (does not check clipping limits)
	.pDrawCanCircleInv	= DrawCan16CircleInv,		// Draw circle or arc inverted (checks clipping limits)
	// Draw ring
	.pDrawCanRing_Fast	= DrawCan16Ring_Fast,		// Draw ring (does not check clipping limits)
	.pDrawCanRing		= DrawCan16Ring,		// Draw ring (checks clipping limits)
	.pDrawCanRingInv_Fast	= DrawCan16RingInv_Fast,	// Draw ring inverted (does not check clipping limits)
	.pDrawCanRingInv	= DrawCan16RingInv,		// Draw ring inverted (checks clipping limits)
	// Draw triangle
	.pDrawCanTriangle_Fast	= DrawCan16Triangle_Fast,	// Draw triangle (does not check clipping limits)
	.pDrawCanTriangle	= DrawCan16Triangle,		// Draw triangle (checks clipping limits)
	.pDrawCanTriangleInv_Fast = DrawCan16TriangleInv_Fast,	// Draw triangle inverted (does not check clipping limits)
	.pDrawCanTriangleInv	= DrawCan16TriangleInv,		// Draw triangle inverted (checks clipping limits)
	// Draw fill area
	.pDrawCanFill		= DrawCan16Fill,		// Draw fill area
	// Draw character
	.pDrawCanChar_Fast	= DrawCan16Char_Fast,		// Draw character with transparent background (does not check clipping limits)
	.pDrawCanChar		= DrawCan16Char,		// Draw character with transparent background (checks clipping limits)
	.pDrawCanCharBg_Fast	= DrawCan16CharBg_Fast,		// Draw character with background (does not check clipping limits)
	.pDrawCanCharBg		= DrawCan16CharBg,		// Draw character with background (checks clipping limits)
	.pDrawCanCharInv_Fast	= DrawCan16CharInv_Fast,	// Draw character inverted (does not check clipping limits)
	.pDrawCanCharInv	= DrawCan16CharInv,		// Draw character inverted (checks clipping limits)
	// Draw text
	.pDrawCanText_Fast	= DrawCan16Text_Fast,		// Draw text with transparent background (does not check clipping limits)
	.pDrawCanText		= DrawCan16Text,		// Draw text with transparent background (checks clipping limits)
	.pDrawCanTextBg_Fast	= DrawCan16TextBg_Fast,		// Draw text with background (does not check clipping limits)
	.pDrawCanTextBg		= DrawCan16TextBg,		// Draw text with background (checks clipping limits)
	.pDrawCanTextInv_Fast	= DrawCan16TextInv_Fast,	// Draw text inverted (does not check clipping limits)
	.pDrawCanTextInv	= DrawCan16TextInv,		// Draw text inverted (checks clipping limits)
	// Draw ellipse
	.pDrawCanEllipse_Fast	= DrawCan16Ellipse_Fast,	// Draw ellipse (does not check clipping limits)
	.pDrawCanEllipse	= DrawCan16Ellipse,		// Draw ellipse (checks clipping limits)
	.pDrawCanEllipseInv_Fast = DrawCan16EllipseInv_Fast,	// Draw ellipse inverted (does not check clipping limits)
	.pDrawCanEllipseInv	= DrawCan16EllipseInv,		// Draw ellipse inverted (checks clipping limits)
	// Draw filled ellipse
	.pDrawCanFillEllipse_Fast = DrawCan16FillEllipse_Fast,	// Draw filled ellipse (does not check clipping limits)
	.pDrawCanFillEllipse	= DrawCan16FillEllipse,		// Draw filled ellipse (checks clipping limits)
	.pDrawCanFillEllipseInv_Fast = DrawCan16FillEllipseInv_Fast, // Draw filled ellipse inverted (does not check clipping limits)
	.pDrawCanFillEllipseInv = DrawCan16FillEllipseInv,	// Draw filled ellipse inverte (checks clipping limits)
	// Draw image
	.pDrawCanImg		= DrawCan16Img,			// Draw image with the same format as destination
	.pDrawCanImgInv		= DrawCan16ImgInv,		// Draw image inverted with the same format as destination
	.pDrawCanBlit		= DrawCan16Blit,		// Draw transparent image with the same format as destination
	.pDrawCanBlitInv	= DrawCan16BlitInv,		// Draw transparent image inverted with the same format as destination
	.pDrawCanBlitSubs	= DrawCan16BlitSubs,		// Draw transparent image with the same format as destination, with substitute color
	.pDrawCanGetImg		= DrawCan16GetImg,		// Get image from canvas to buffer
	.pDrawCanImgMat		= DrawCan16ImgMat,		// Draw image with 2D transformation matrix
	// colors
	.pColRgb		= Draw16ColRgb,			// convert RGB888 color to pixel color
#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
	.pColRand		= Draw16ColRand,		// random color
#endif
	.col_black		= COL16_BLACK,			// black color
	.col_blue		= COL16_BLUE,			// blue color
	.col_green		= COL16_GREEN,			// green color
	.col_cyan		= COL16_CYAN,			// cyan color
	.col_red		= COL16_RED,			// red color
	.col_magenta		= COL16_MAGENTA,		// magenta color
	.col_yellow		= COL16_YELLOW,			// yellow color
	.col_white		= COL16_WHITE,			// white color
	.col_gray		= COL16_GRAY,			// gray color
	.col_dkblue		= COL16_DKBLUE,			// dark blue color
	.col_dkgreen		= COL16_DKGREEN,		// dark green color
	.col_dkcyan		= COL16_DKCYAN,			// dark cyan color
	.col_dkred		= COL16_DKRED,			// dark red color
	.col_dkmagenta		= COL16_DKMAGENTA,		// dark magenta color
	.col_dkyellow		= COL16_DKYELLOW,		// dark yellow color
	.col_dkwhite		= COL16_DKWHITE,		// dark white color
	.col_dkgray		= COL16_DKGRAY,			// dark gray color
	.col_ltblue		= COL16_LTBLUE,			// light blue color
	.col_ltgreen		= COL16_LTGREEN,		// light green color
	.col_ltcyan		= COL16_LTCYAN,			// light cyan color
	.col_ltred		= COL16_LTRED,			// light red color
	.col_ltmagenta		= COL16_LTMAGENTA,		// light magenta color
	.col_ltyellow		= COL16_LTYELLOW,		// light yellow color
	.col_ltgray		= COL16_LTGRAY,			// light gray color
	.col_azure		= COL16_AZURE,			// azure blue color
	.col_orange		= COL16_ORANGE,			// orange color
	.col_brown		= COL16_BROWN,			// brown color
};

// drawing function interface 15-bit
const sDrawCanFnc DrawCan15Fnc = {
	.pDrawPitch		= Draw15Pitch,			// calculate pitch of graphics line from image width
	.pDrawMaxWidth		= Draw15MaxWidth,		// calculate max. width of image from the pitch
	// Clear canvas
	.pDrawCanClearCol	= DrawCan15ClearCol,		// Clear canvas with color
	.pDrawCanClear		= DrawCan15Clear,		// Clear canvas with black color
	// Draw point
	.pDrawCanPoint_Fast	= DrawCan15Point_Fast,		// Draw point (does not check clipping limits)
	.pDrawCanPoint		= DrawCan15Point,		// Draw point (checks clipping limits)
	.pDrawCanPointInv_Fast	= DrawCan15PointInv_Fast,	// Draw point inverted (does not check clipping limits)
	.pDrawCanPointInv	= DrawCan15PointInv,		// Draw point inverted (checks clipping limits)
	// Get point
	.pDrawCanGetPoint_Fast	= DrawCan15GetPoint_Fast,	// Get point (does not check clipping limits)
	.pDrawCanGetPoint	= DrawCan15GetPoint,		// Get point (checks clipping limits)
	// Draw rectangle
	.pDrawCanRect_Fast	= DrawCan15Rect_Fast,		// Draw rectangle (does not check clipping limits)
	.pDrawCanRect		= DrawCan15Rect,		// Draw rectangle (checks clipping limits)
	.pDrawCanRectInv_Fast	= DrawCan15RectInv_Fast,	// Draw rectangle inverted (does not check clipping limits)
	.pDrawCanRectInv	= DrawCan15RectInv,		// Draw rectangle inverted (checks clipping limits
	// Draw horizontal line
	.pDrawCanHLine_Fast	= DrawCan15HLine_Fast,		// Draw horizontal line (does not check clipping limits)
	.pDrawCanHLine		= DrawCan15HLine,		// Draw horizontal line (checks clipping limits; negative width flip line)
	.pDrawCanHLineInv_Fast	= DrawCan15HLineInv_Fast,	// Draw horizontal line inverted (does not check clipping limits)
	.pDrawCanHLineInv	= DrawCan15HLineInv,		// Draw horizontal line inverted (checks clipping limits; negative width flip line)
	// Draw vertical line
	.pDrawCanVLine_Fast	= DrawCan15VLine_Fast,		// Draw vertical line (does not check clipping limits)
	.pDrawCanVLine		= DrawCan15VLine,		// Draw vertical line (checks clipping limits; negative height flip line)
	.pDrawCanVLineInv_Fast	= DrawCan15VLineInv_Fast,	// Draw vertical line inverted (does not check clipping limits)
	.pDrawCanVLineInv	= DrawCan15VLineInv,		// Draw vertical line inverted (checks clipping limits; negative height flip line)
	// Draw frame
	.pDrawCanFrame_Fast	= DrawCan15Frame_Fast,		// Draw 1-pixel frame (does not check clipping limits)
	.pDrawCanFrame		= DrawCan15Frame,		// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameInv_Fast	= DrawCan15FrameInv_Fast,	// Draw 1-pixel frame inverted (does not check clipping limits)
	.pDrawCanFrameInv	= DrawCan15FrameInv,		// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameW_Fast	= DrawCan15FrameW_Fast,		// Draw thick frame (does not check clipping limits)
	.pDrawCanFrameW		= DrawCan15FrameW,		// Draw thick frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameWInv_Fast	= DrawCan15FrameWInv_Fast,	// Draw thick frame inverted (does not check clipping limits)
	.pDrawCanFrameWInv	= DrawCan15FrameWInv,		// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
	// Draw line
	.pDrawCanLineOver_Fast	= DrawCan15LineOver_Fast,	// Draw line with overlapped pixels (does not check clipping limits)
	.pDrawCanLine_Fast	= DrawCan15Line_Fast,		// Draw line (does not check clipping limits)
	.pDrawCanLineOver	= DrawCan15LineOver,		// Draw line with overlapped pixels (checks clipping limits)
	.pDrawCanLine		= DrawCan15Line,		// Draw line (checks clipping limits)
	.pDrawCanLineOverInv_Fast = DrawCan15LineOverInv_Fast,	// Draw line inverted with overlapped pixels (does not check clipping limits)
	.pDrawCanLineInv_Fast	= DrawCan15LineInv_Fast,	// Draw line inverted (does not check clipping limits)
	.pDrawCanLineOverInv	= DrawCan15LineOverInv,		// Draw line inverted with overlapped pixels (checks clipping limits)
	.pDrawCanLineInv	= DrawCan15LineInv,		// Draw line inverted (checks clipping limits)
	.pDrawCanLineW_Fast	= DrawCan15LineW_Fast,		// Draw thick line (does not check clipping limits)
	.pDrawCanLineW		= DrawCan15LineW,		// Draw thick line (checks clipping limits)
	.pDrawCanLineWInv_Fast	= DrawCan15LineWInv_Fast,	// Draw thick line inverted (does not check clipping limits)
	.pDrawCanLineWInv	= DrawCan15LineWInv,		// Draw thick line inverted (checks clipping limits)
	// Draw round (filled circle)
	.pDrawCanRound_Fast	= DrawCan15Round_Fast,		// Draw round (does not check clipping limits)
	.pDrawCanRound		= DrawCan15Round,		// Draw round (checks clipping limits)
	.pDrawCanRoundInv_Fast	= DrawCan15RoundInv_Fast,	// Draw round inverted (does not check clipping limits)
	.pDrawCanRoundInv	= DrawCan15RoundInv,		// Draw round inverted (checks clipping limits)
	// Draw circle
	.pDrawCanCircle_Fast	= DrawCan15Circle_Fast,		// Draw circle or arc (does not check clipping limits)
	.pDrawCanCircle		= DrawCan15Circle,		// Draw circle or arc (checks clipping limits)
	.pDrawCanCircleInv_Fast	= DrawCan15CircleInv_Fast,	// Draw circle or arc inverted (does not check clipping limits)
	.pDrawCanCircleInv	= DrawCan15CircleInv,		// Draw circle or arc inverted (checks clipping limits)
	// Draw ring
	.pDrawCanRing_Fast	= DrawCan15Ring_Fast,		// Draw ring (does not check clipping limits)
	.pDrawCanRing		= DrawCan15Ring,		// Draw ring (checks clipping limits)
	.pDrawCanRingInv_Fast	= DrawCan15RingInv_Fast,	// Draw ring inverted (does not check clipping limits)
	.pDrawCanRingInv	= DrawCan15RingInv,		// Draw ring inverted (checks clipping limits)
	// Draw triangle
	.pDrawCanTriangle_Fast	= DrawCan15Triangle_Fast,	// Draw triangle (does not check clipping limits)
	.pDrawCanTriangle	= DrawCan15Triangle,		// Draw triangle (checks clipping limits)
	.pDrawCanTriangleInv_Fast = DrawCan15TriangleInv_Fast,	// Draw triangle inverted (does not check clipping limits)
	.pDrawCanTriangleInv	= DrawCan15TriangleInv,		// Draw triangle inverted (checks clipping limits)
	// Draw fill area
	.pDrawCanFill		= DrawCan15Fill,		// Draw fill area
	// Draw character
	.pDrawCanChar_Fast	= DrawCan15Char_Fast,		// Draw character with transparent background (does not check clipping limits)
	.pDrawCanChar		= DrawCan15Char,		// Draw character with transparent background (checks clipping limits)
	.pDrawCanCharBg_Fast	= DrawCan15CharBg_Fast,		// Draw character with background (does not check clipping limits)
	.pDrawCanCharBg		= DrawCan15CharBg,		// Draw character with background (checks clipping limits)
	.pDrawCanCharInv_Fast	= DrawCan15CharInv_Fast,	// Draw character inverted (does not check clipping limits)
	.pDrawCanCharInv	= DrawCan15CharInv,		// Draw character inverted (checks clipping limits)
	// Draw text
	.pDrawCanText_Fast	= DrawCan15Text_Fast,		// Draw text with transparent background (does not check clipping limits)
	.pDrawCanText		= DrawCan15Text,		// Draw text with transparent background (checks clipping limits)
	.pDrawCanTextBg_Fast	= DrawCan15TextBg_Fast,		// Draw text with background (does not check clipping limits)
	.pDrawCanTextBg		= DrawCan15TextBg,		// Draw text with background (checks clipping limits)
	.pDrawCanTextInv_Fast	= DrawCan15TextInv_Fast,	// Draw text inverted (does not check clipping limits)
	.pDrawCanTextInv	= DrawCan15TextInv,		// Draw text inverted (checks clipping limits)
	// Draw ellipse
	.pDrawCanEllipse_Fast	= DrawCan15Ellipse_Fast,	// Draw ellipse (does not check clipping limits)
	.pDrawCanEllipse	= DrawCan15Ellipse,		// Draw ellipse (checks clipping limits)
	.pDrawCanEllipseInv_Fast = DrawCan15EllipseInv_Fast,	// Draw ellipse inverted (does not check clipping limits)
	.pDrawCanEllipseInv	= DrawCan15EllipseInv,		// Draw ellipse inverted (checks clipping limits)
	// Draw filled ellipse
	.pDrawCanFillEllipse_Fast = DrawCan15FillEllipse_Fast,	// Draw filled ellipse (does not check clipping limits)
	.pDrawCanFillEllipse	= DrawCan15FillEllipse,		// Draw filled ellipse (checks clipping limits)
	.pDrawCanFillEllipseInv_Fast = DrawCan15FillEllipseInv_Fast, // Draw filled ellipse inverted (does not check clipping limits)
	.pDrawCanFillEllipseInv = DrawCan15FillEllipseInv,	// Draw filled ellipse inverte (checks clipping limits)
	// Draw image
	.pDrawCanImg		= DrawCan15Img,			// Draw image with the same format as destination
	.pDrawCanImgInv		= DrawCan15ImgInv,		// Draw image inverted with the same format as destination
	.pDrawCanBlit		= DrawCan15Blit,		// Draw transparent image with the same format as destination
	.pDrawCanBlitInv	= DrawCan15BlitInv,		// Draw transparent image inverted with the same format as destination
	.pDrawCanBlitSubs	= DrawCan15BlitSubs,		// Draw transparent image with the same format as destination, with substitute color
	.pDrawCanGetImg		= DrawCan15GetImg,		// Get image from canvas to buffer
	.pDrawCanImgMat		= DrawCan15ImgMat,		// Draw image with 2D transformation matrix
	// colors
	.pColRgb		= Draw15ColRgb,			// convert RGB888 color to pixel color
#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
	.pColRand		= Draw15ColRand,		// random color
#endif
	.col_black		= COL15_BLACK,			// black color
	.col_blue		= COL15_BLUE,			// blue color
	.col_green		= COL15_GREEN,			// green color
	.col_cyan		= COL15_CYAN,			// cyan color
	.col_red		= COL15_RED,			// red color
	.col_magenta		= COL15_MAGENTA,		// magenta color
	.col_yellow		= COL15_YELLOW,			// yellow color
	.col_white		= COL15_WHITE,			// white color
	.col_gray		= COL15_GRAY,			// gray color
	.col_dkblue		= COL15_DKBLUE,			// dark blue color
	.col_dkgreen		= COL15_DKGREEN,		// dark green color
	.col_dkcyan		= COL15_DKCYAN,			// dark cyan color
	.col_dkred		= COL15_DKRED,			// dark red color
	.col_dkmagenta		= COL15_DKMAGENTA,		// dark magenta color
	.col_dkyellow		= COL15_DKYELLOW,		// dark yellow color
	.col_dkwhite		= COL15_DKWHITE,		// dark white color
	.col_dkgray		= COL15_DKGRAY,			// dark gray color
	.col_ltblue		= COL15_LTBLUE,			// light blue color
	.col_ltgreen		= COL15_LTGREEN,		// light green color
	.col_ltcyan		= COL15_LTCYAN,			// light cyan color
	.col_ltred		= COL15_LTRED,			// light red color
	.col_ltmagenta		= COL15_LTMAGENTA,		// light magenta color
	.col_ltyellow		= COL15_LTYELLOW,		// light yellow color
	.col_ltgray		= COL15_LTGRAY,			// light gray color
	.col_azure		= COL15_AZURE,			// azure blue color
	.col_orange		= COL15_ORANGE,			// orange color
	.col_brown		= COL15_BROWN,			// brown color
};

#endif // USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

#endif // USE_DRAWCAN
