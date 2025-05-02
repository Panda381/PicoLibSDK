
// ****************************************************************************
//
//                      Drawing to canvas at 12-bit format
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

#if USE_DRAWCAN	&& USE_DRAWCAN12	// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#include "../../_font/_include.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_float.h"
#include "../inc/lib_text.h"
#include "../inc/lib_rand.h"
#include "../inc/lib_drawcan.h"
#include "../inc/lib_drawcan12.h"

#ifndef WIDTH
#define WIDTH		320		// display width
#endif

#ifndef HEIGHT
#define HEIGHT		240		// display height
#endif

// default drawing canvas for 12-bit format
sDrawCan DrawCan12 = {
	// format
	.format = DRAWCAN_FORMAT_12,	// u8	format;		// canvas format CANVAS_*
	.colbit = 12,			// u8	colbit;		// number of bits per pixel
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
	.wb = (WIDTH+7)/8*12,		// s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	.striph = HEIGHT,		// s16	striph;		// strip height (number of lines)

	// print
	.printposnum = WIDTH/FONTW,	// u16	printposnum;	// number of text positions per row (= w / fontw)
	.printrownum = HEIGHT/FONTH,	// u16	printrownum;	// number of text rows (= h / fonth)
	.printpos = 0,			// u16	printpos;	// console print character position
	.printrow = 0,			// u16	printrow;	// console print character row
	.printcol = COL12_WHITE,	// u16	printcol;	// console print color

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
	.drawfnc = NULL, // &DrawCan12Fnc,	// const struct sDrawCanFnc_* drawfnc; // drawing functions
};

// current drawing canvas for 12-bit format
sDrawCan* pDrawCan12 = &DrawCan12;

// set default drawing canvas for 12-bit format
void SetDrawCan12(sDrawCan* can) { pDrawCan12 = can; }

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw12Pitch(int w) { return (w + 7)/8*12; }

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw12MaxWidth(int pitch) { return pitch*2/3; }

// convert RGB888 color to 12-bit pixel color RGB444
u16 Draw12ColRgb(u8 r, u8 g, u8 b) { return COLOR12(r, g, b); }

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 12-bit pixel color RGB444
u16 Draw12ColRand() { return COL12_RANDOM; }
#endif

// ----------------------------------------------------------------------------
//                            Clear canvas
// ----------------------------------------------------------------------------

// Clear canvas with color
void DrawCan12ClearCol(sDrawCan* can, u16 col)
{
	int x = can->clipx1;
	int y = can->clipy1;
	int w = can->clipx2 - x;
	int h = can->clipy2 - y;
	DrawCan12Rect(can, x, y, w, h, col);
}
void Draw12ClearCol(u16 col) { DrawCan12ClearCol(pDrawCan12, col); }

// Clear canvas with black color
void DrawCan12Clear(sDrawCan* can) { DrawCan12ClearCol(can, COL12_BLACK); }
void Draw12Clear() { DrawCan12Clear(pDrawCan12); }

// ----------------------------------------------------------------------------
//                               Draw point
// ----------------------------------------------------------------------------

// set even 12-bit pixel
#define SETPIXEL12_EVEN(ddd,ccc) { (ddd)[0] = (u8)ccc; (ddd)[1] = ((ddd)[1] & 0xf0) | (u8)((ccc)>>8); }
#define INVPIXEL12_EVEN(ddd) { (ddd)[0] = ~(ddd)[0]; (ddd)[1] ^= 0x0f; }
#define INVCOLPIXEL12_EVEN(ddd,ccc) { (ddd)[0] ^= (u8)(ccc); (ddd)[1] ^= (u8)((ccc)>>8); }

// set odd 12-bit pixel
#define SETPIXEL12_ODD(ddd,ccc) { (ddd)[1] = ((ddd)[1] & 0x0f) | (u8)((ccc)<<4); (ddd)[2] = (u8)((ccc)>>4); }
#define INVPIXEL12_ODD(ddd) { (ddd)[1] ^= 0xf0; (ddd)[2] = ~(ddd)[2]; }
#define INVCOLPIXEL12_ODD(ddd,ccc) { (ddd)[1] ^= (u8)((ccc)<<4); (ddd)[2] ^= (u8)((ccc)>>4); }

// set 12-bit pixel by x coordinate
#define SETPIXEL12(ddd,xxx,ccc) {				\
	if (((xxx) & 1) == 0)					\
		SETPIXEL12_EVEN(ddd,ccc)			\
	else							\
		SETPIXEL12_ODD(ddd,ccc); }

#define INVPIXEL12(ddd,xxx) {					\
	if (((xxx) & 1) == 0)					\
		INVPIXEL12_EVEN(ddd)				\
	else							\
		INVPIXEL12_ODD(ddd); }

// Draw point (does not check clipping limits)
void DrawCan12Point_Fast(sDrawCan* can, int x, int y, u16 col)
{
	// draw pixel
	u8* d = &can->buf[x/2*3 + (y - can->basey)*can->wb];
	SETPIXEL12(d, x, col);
}
void Draw12Point_Fast(int x, int y, u16 col) { DrawCan12Point_Fast(pDrawCan12, x, y, col); }

// Draw point (checks clipping limits)
void DrawCan12Point(sDrawCan* can, int x, int y, u16 col)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// draw pixel
	u8* d = &can->buf[x/2*3 + (y - can->basey)*can->wb];
	SETPIXEL12(d, x, col);
}
void Draw12Point(int x, int y, u16 col) { DrawCan12Point(pDrawCan12, x, y, col); }

// Draw point inverted (does not check clipping limits)
void DrawCan12PointInv_Fast(sDrawCan* can, int x, int y)
{
	// invert pixel
	u8* d = &can->buf[x/2*3 + (y - can->basey)*can->wb];
	INVPIXEL12(d, x);
}
void Draw12PointInv_Fast(int x, int y) { DrawCan12PointInv_Fast(pDrawCan12, x, y); }

// Draw point inverted (checks clipping limits)
void DrawCan12PointInv(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// invert pixel
	u8* d = &can->buf[x/2*3 + (y - can->basey)*can->wb];
	INVPIXEL12(d, x);
}
void Draw12PointInv(int x, int y) { DrawCan12PointInv(pDrawCan12, x, y); }

// ----------------------------------------------------------------------------
//                               Get point
// ----------------------------------------------------------------------------

// get even 12-bit pixel
#define GETPIXEL12_EVEN(ddd) ( (ddd)[0] | (((ddd)[1] & 0x0f) << 8) )

// get odd 12-bit pixel
#define GETPIXEL12_ODD(ddd) ( ((ddd)[1] >> 4) | ((ddd)[2] << 4) )

// get 12-bit pixel by x coordinate
#define GETPIXEL12(ddd,xxx) ( (((xxx) & 1) == 0) ? GETPIXEL12_EVEN(ddd) : GETPIXEL12_ODD(ddd) )

// Get point (does not check clipping limits)
u16 DrawCan12GetPoint_Fast(sDrawCan* can, int x, int y)
{
	u8* d = &can->buf[x/2*3 + (y - can->basey)*can->wb];
	return GETPIXEL12(d, x);
}
u16 Draw12GetPoint_Fast(int x, int y) { return DrawCan12GetPoint_Fast(pDrawCan12, x, y); }

// Get point (checks clipping limits)
u16 DrawCan12GetPoint(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return COL12_BLACK;

	// get pixel
	u8* d = &can->buf[x/2*3 + (y - can->basey)*can->wb];
	return GETPIXEL12(d, x);
}
u16 Draw12GetPoint(int x, int y) { return DrawCan12GetPoint(pDrawCan12, x, y); }

// ----------------------------------------------------------------------------
//                            Draw rectangle
// ----------------------------------------------------------------------------

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan12Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col)
{
	// draw
	int wb = can->wb;
	u8* d0 = &can->buf[x/2*3 + (y - can->basey)*wb];
	u8* d;
	int i = col | (col << 12);
	u8 col1 = (u8)i;
	u8 col2 = (u8)(i >> 8);
	u8 col3 = (u8)(i >> 16);
	for (; h > 0; h--)
	{
		d = d0;
		i = w;

		// store first odd pixel
		if ((x & 1) != 0)
		{
			SETPIXEL12_ODD(d, col);
			d += 3;
			i--;
		}

		// store inner pixels
		for (; i > 1; i -= 2)
		{
			d[0] = col1;
			d[1] = col2;
			d[2] = col3;
			d += 3;
		}

		// store last even pixel
		if (i > 0) SETPIXEL12_EVEN(d, col);

		// shift buffer address
		d0 += wb;
	}
}
void Draw12Rect_Fast(int x, int y, int w, int h, u16 col) { DrawCan12Rect_Fast(pDrawCan12, x, y, w, h, col); }

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan12Rect(sDrawCan* can, int x, int y, int w, int h, u16 col)
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
	DrawCan12Rect_Fast(can, x, y, w, h, col);
}
void Draw12Rect(int x, int y, int w, int h, u16 col) { DrawCan12Rect(pDrawCan12, x, y, w, h, col); }

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan12RectInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// invert
	int wb = can->wb;
	u8* d0 = &can->buf[x/2*3 + (y - can->basey)*wb];
	u8* d;
	int i;
	for (; h > 0; h--)
	{
		d = d0;
		i = w;

		// invert first odd pixel
		if ((x & 1) != 0)
		{
			INVPIXEL12_ODD(d);
			d += 3;
			i--;
		}

		// invert inner pixels
		for (; i > 1; i -= 2)
		{
			d[0] = ~d[0];
			d[1] = ~d[1];
			d[2] = ~d[2];
			d += 3;
		}

		// invert last even pixel
		if (i > 0) INVPIXEL12_EVEN(d);

		// shift buffer address
		d0 += wb;
	}
}
void Draw12RectInv_Fast(int x, int y, int w, int h) { DrawCan12RectInv_Fast(pDrawCan12, x, y, w, h); }

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan12RectInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan12RectInv_Fast(can, x, y, w, h);
}
void Draw12RectInv(int x, int y, int w, int h) { DrawCan12RectInv(pDrawCan12, x, y, w, h); }

// ----------------------------------------------------------------------------
//                          Draw horizontal line
// ----------------------------------------------------------------------------

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan12HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col)
{
	// draw
	u8* d = &can->buf[x/2*3 + (y - can->basey)*can->wb];
	int i = col | (col << 12);
	u8 col1 = (u8)i;
	u8 col2 = (u8)(i >> 8);
	u8 col3 = (u8)(i >> 16);

	// store first odd pixel
	if ((x & 1) != 0)
	{
		SETPIXEL12_ODD(d, col);
		d += 3;
		w--;
	}

	// store inner pixels
	for (; w > 1; w -= 2)
	{
		d[0] = col1;
		d[1] = col2;
		d[2] = col3;
		d += 3;
	}

	// store last even pixel
	if (w > 0) SETPIXEL12_EVEN(d, col);
}
void Draw12HLine_Fast(int x, int y, int w, u16 col) { DrawCan12HLine_Fast(pDrawCan12, x, y, w, col); }

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan12HLine(sDrawCan* can, int x, int y, int w, u16 col)
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
	DrawCan12HLine_Fast(can, x, y, w, col);
}
void Draw12HLine(int x, int y, int w, u16 col) { DrawCan12HLine(pDrawCan12, x, y, w, col); }

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan12HLineInv_Fast(sDrawCan* can, int x, int y, int w)
{
	// invert
	u8* d = &can->buf[x/2*3 + (y - can->basey)*can->wb];

	// invert first odd pixel
	if ((x & 1) != 0)
	{
		INVPIXEL12_ODD(d);
		d += 3;
		w--;
	}

	// invert inner pixels
	for (; w > 1; w -= 2)
	{
		d[0] = ~d[0];
		d[1] = ~d[1];
		d[2] = ~d[2];
		d += 3;
	}

	// invert last even pixel
	if (w > 0) INVPIXEL12_EVEN(d);
}
void Draw12HLineInv_Fast(int x, int y, int w) { DrawCan12HLineInv_Fast(pDrawCan12, x, y, w); }

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan12HLineInv(sDrawCan* can, int x, int y, int w)
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
	DrawCan12HLineInv_Fast(can, x, y, w);
}
void Draw12HLineInv(int x, int y, int w) { DrawCan12HLineInv(pDrawCan12, x, y, w); }

// ----------------------------------------------------------------------------
//                           Draw vertical line
// ----------------------------------------------------------------------------

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan12VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col)
{
	// draw
	int wb = can->wb;
	u8* d = &can->buf[x/2*3 + (y - can->basey)*wb];

	// even pixel
	if ((x & 1) == 0)
	{	
		for (; h > 0; h--)
		{
			SETPIXEL12_EVEN(d, col);
			d += wb;
		}
	}

	// odd pixel
	else
	{	
		for (; h > 0; h--)
		{
			SETPIXEL12_ODD(d, col);
			d += wb;
		}
	}
}
void Draw12VLine_Fast(int x, int y, int h, u16 col) { DrawCan12VLine_Fast(pDrawCan12, x, y, h, col); }

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan12VLine(sDrawCan* can, int x, int y, int h, u16 col)
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
	DrawCan12VLine_Fast(can, x, y, h, col);
}
void Draw12VLine(int x, int y, int h, u16 col) { DrawCan12VLine(pDrawCan12, x, y, h, col); }

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan12VLineInv_Fast(sDrawCan* can, int x, int y, int h)
{
	// invert
	int wb = can->wb;
	u8* d = &can->buf[x/2*3 + (y - can->basey)*wb];

	// even pixel
	if ((x & 1) == 0)
	{	
		for (; h > 0; h--)
		{
			INVPIXEL12_EVEN(d);
			d += wb;
		}
	}

	// odd pixel
	else
	{	
		for (; h > 0; h--)
		{
			INVPIXEL12_ODD(d);
			d += wb;
		}
	}
}
void Draw12VLineInv_Fast(int x, int y, int h) { DrawCan12VLineInv_Fast(pDrawCan12, x, y, h); }

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan12VLineInv(sDrawCan* can, int x, int y, int h)
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
	DrawCan12VLineInv_Fast(can, x, y, h);
}
void Draw12VLineInv(int x, int y, int h) { DrawCan12VLineInv(pDrawCan12, x, y, h); }

// ----------------------------------------------------------------------------
//                           Draw frame
// ----------------------------------------------------------------------------

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan12Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
{
	// bottom line (dark)
	DrawCan12HLine_Fast(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan12VLine_Fast(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan12HLine_Fast(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan12VLine_Fast(can, x, y+1, h-2, col_light);
	}
}
void Draw12Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan12Frame_Fast(pDrawCan12, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan12Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
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
	DrawCan12HLine(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan12VLine(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan12HLine(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan12VLine(can, x, y+1, h-2, col_light);
	}
}
void Draw12Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan12Frame(pDrawCan12, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan12FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// bottom line
	DrawCan12HLineInv_Fast(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan12VLineInv_Fast(can, x+w-1, y, h-1);

		// top line
		DrawCan12HLineInv_Fast(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan12VLineInv_Fast(can, x, y+1, h-2);
	}
}
void Draw12FrameInv_Fast(int x, int y, int w, int h) { DrawCan12FrameInv_Fast(pDrawCan12, x, y, w, h); }

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan12FrameInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan12HLineInv(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan12VLineInv(can, x+w-1, y, h-1);

		// top line
		DrawCan12HLineInv(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan12VLineInv(can, x, y+1, h-2);
	}
}
void Draw12FrameInv(int x, int y, int w, int h) { DrawCan12FrameInv(pDrawCan12, x, y, w, h); }

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan12FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan12Rect_Fast(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan12Rect_Fast(can, x, y, w, thick, col);

		// bottom line
		DrawCan12Rect_Fast(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan12Rect_Fast(can, x, y, thick, h, col);

		// right line
		DrawCan12Rect_Fast(can, x+w-thick, y, thick, h, col);
	}
}
void Draw12FrameW_Fast(int x, int y, int w, int h, u16 col, int thick) { DrawCan12FrameW_Fast(pDrawCan12, x, y, w, h, col, thick); }

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan12FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
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
		DrawCan12Rect(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan12Rect(can, x, y, w, thick, col);

		// bottom line
		DrawCan12Rect(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan12Rect(can, x, y, thick, h, col);

		// right line
		DrawCan12Rect(can, x+w-thick, y, thick, h, col);
	}
}
void Draw12FrameW(int x, int y, int w, int h, u16 col, int thick) { DrawCan12Frame(pDrawCan12, x, y, w, h, col, thick); }

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan12FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan12RectInv_Fast(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan12RectInv_Fast(can, x, y, w, thick);

		// bottom line
		DrawCan12RectInv_Fast(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan12RectInv_Fast(can, x, y, thick, h);

		// right line
		DrawCan12RectInv_Fast(can, x+w-thick, y, thick, h);
	}
}
void Draw12FrameWInv_Fast(int x, int y, int w, int h, int thick) { DrawCan12FrameWInv_Fast(pDrawCan12, x, y, w, h, thick); }

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan12FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick)
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
		DrawCan12RectInv(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan12RectInv(can, x, y, w, thick);

		// bottom line
		DrawCan12RectInv(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan12RectInv(can, x, y, thick, h);

		// right line
		DrawCan12RectInv(can, x+w-thick, y, thick, h);
	}
}
void Draw12FrameWInv(int x, int y, int w, int h, int thick) { DrawCan12FrameWInv(pDrawCan12, x, y, w, h, thick); }

// ----------------------------------------------------------------------------
//                           Draw line
// ----------------------------------------------------------------------------

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan12LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over)
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
	int sxdir = (sx >> 1) & 1;

	// destination address
	u8* d = &can->buf[x1/2*3 + (y1 - can->basey)*wb];

	// draw first pixel
	SETPIXEL12(d,x1,col);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			if ((x1 & 1) == sxdir) d += sx*3;
			if (p > 0)
			{
				if (over) SETPIXEL12(d,x1,col); // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			SETPIXEL12(d,x1,col);
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
				if (over) SETPIXEL12(d,x1,col); // draw overlapped pixel
				x1 += sx;
				if ((x1 & 1) == sxdir) d += sx*3;
				p -= dy;
			}
			p += m;
			SETPIXEL12(d,x1,col);
		}
	}
}
void Draw12LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan12LineOver_Fast(pDrawCan12, x1, y1, x2, y2, col, over); }

// Draw line (does not check clipping limits)
void DrawCan12Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan12LineOver_Fast(pDrawCan12, x1, y1, x2, y2, col, False); }
void Draw12Line_Fast(int x1, int y1, int x2, int y2, u16 col) { Draw12LineOver_Fast(x1, y1, x2, y2, col, False); }

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan12LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over)
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
	int sxdir = (sx >> 1) & 1;

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u8* d = &can->buf[x1/2*3 + (y1 - can->basey)*wb];

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax))	SETPIXEL12(d,x1,col);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			if ((x1 & 1) == sxdir) d += sx*3;
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) SETPIXEL12(d,x1,col);
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) SETPIXEL12(d,x1,col);
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) SETPIXEL12(d,x1,col);
				x1 += sx;
				if ((x1 & 1) == sxdir) d += sx*3;
				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) SETPIXEL12(d,x1,col);
		}
	}
}
void Draw12LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan12LineOver(pDrawCan12, x1, y1, x2, y2, col, over); }

// Draw line (checks clipping limits)
void DrawCan12Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan12LineOver(can, x1, y1, x2, y2, col, False); }
void Draw12Line(int x1, int y1, int x2, int y2, u16 col) { DrawCan12Line(pDrawCan12, x1, y1, x2, y2, col); }

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan12LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	int sxdir = (sx >> 1) & 1;

	// destination address
	u8* d = &can->buf[x1/2*3 + (y1 - can->basey)*wb];

	// draw first pixel
	INVPIXEL12(d,x1);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			if ((x1 & 1) == sxdir) d += sx*3;
			if (p > 0)
			{
				if (over) INVPIXEL12(d,x1); // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			INVPIXEL12(d,x1);
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
				if (over) INVPIXEL12(d,x1); // draw overlapped pixel
				x1 += sx;
				if ((x1 & 1) == sxdir) d += sx*3;
				p -= dy;
			}
			p += m;
			INVPIXEL12(d,x1);
		}
	}
}
void Draw12LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over) { DrawCan12LineOverInv_Fast(pDrawCan12, x1, y1, x2, y2, over); }

// Draw line inverted (does not check clipping limits)
void DrawCan12LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan12LineOverInv_Fast(can, x1, y1, x2, y2, False); }
void Draw12LineInv_Fast(int x1, int y1, int x2, int y2) { DrawCan12LineOverInv_Fast(pDrawCan12, x1, y1, x2, y2, False); }

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan12LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	int sxdir = (sx >> 1) & 1;

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u8* d = &can->buf[x1/2*3 + (y1 - can->basey)*wb];

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax))	INVPIXEL12(d,x1);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			if ((x1 & 1) == sxdir) d += sx*3;
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) INVPIXEL12(d,x1);
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) INVPIXEL12(d,x1);
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) INVPIXEL12(d,x1);
				x1 += sx;
				if ((x1 & 1) == sxdir) d += sx*3;
				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) INVPIXEL12(d,x1);
		}
	}
}
void Draw12LineOverInv(int x1, int y1, int x2, int y2, Bool over) { DrawCan12LineOverInv(pDrawCan12, x1, y1, x2, y2, over); }

// Draw line inverted (checks clipping limits)
void DrawCan12LineInv(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan12LineOverInv(can, x1, y1, x2, y2, False); }
void Draw12LineInv(int x1, int y1, int x2, int y2) { DrawCan12LineOverInv(pDrawCan12, x1, y1, x2, y2, False); }

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan12LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan12LineOver_Fast(can, x1, y1, x2, y2, col, False);
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
		DrawCan12Round_Fast(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan12Round_Fast(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan12LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan12LineOver_Fast(can, x1, y1, x2, y2, col, over);
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
		DrawCan12LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan12LineOver_Fast(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw12LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan12LineW_Fast(pDrawCan12, x1, y1, x2, y2, col, thick, round); }

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan12LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan12LineOver(can, x1, y1, x2, y2, col, False);
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
		DrawCan12Round(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan12Round(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan12LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan12LineOver(can, x1, y1, x2, y2, col, over);
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
		DrawCan12LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan12LineOver(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw12LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan12LineW(pDrawCan12, x1, y1, x2, y2, col, thick, round); }

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan12LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan12LineOverInv_Fast(can, x1, y1, x2, y2, False);
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
		DrawCan12LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan12LineOverInv_Fast(can, x1, y1, x2, y2, over);
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
		DrawCan12LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan12LineOverInv_Fast(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw12LineWInv_Fast(int x1, int y1, int x2, int y2, int thick) { DrawCan12LineWInv_Fast(pDrawCan12, x1, y1, x2, y2, thick); }

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan12LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan12LineOverInv(can, x1, y1, x2, y2, False);
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
		DrawCan12LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan12LineOverInv(can, x1, y1, x2, y2, over);
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
		DrawCan12LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan12LineOverInv(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw12LineWInv(int x1, int y1, int x2, int y2, int thick) { DrawCan12LineWInv(pDrawCan12, x1, y1, x2, y2, thick); }

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
void DrawCan12Round_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask)
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
		// prepare buffer address
		s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) SETPIXEL12(s,xx,col);
				if ((xx & 1) != 0) s += 3;
				xx++;
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
		s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) SETPIXEL12(s,xx,col);
				if ((xx & 1) != 0) s += 3;
				xx++;
			}
			s0 += wb;
		}
	}
}
void Draw12Round_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan12Round_Fast(pDrawCan12, x, y, d, col, mask); }

// Draw round (checks clipping limits)
void DrawCan12Round(sDrawCan* can, int x, int y, int d, u16 col, u8 mask)
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
		// clipping
		DRAWCANROUND_CLIP();

		// update dirty area
		DrawCanDirtyRect_Fast(can, x+x1, y+y1, x2-x1+1, y2-y1+1);

		// prepare buffer address
		s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) SETPIXEL12(s,xx,col);
				if ((xx & 1) != 0) s += 3;
				xx++;
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
		s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) SETPIXEL12(s,xx,col);
				if ((xx & 1) != 0) s += 3;
				xx++;
			}
			s0 += wb;
		}
	}
}
void Draw12Round(int x, int y, int d, u16 col, u8 mask) { DrawCan12Round(pDrawCan12, x, y, d, col, mask); }

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan12RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
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
		s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) INVPIXEL12(s,xx);
				if ((xx & 1) != 0) s += 3;
				xx++;
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
		s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) INVPIXEL12(s,xx);
				if ((xx & 1) != 0) s += 3;
				xx++;
			}
			s0 += wb;
		}
	}
}
void Draw12RoundInv_Fast(int x, int y, int d, u8 mask) { DrawCan12RoundInv_Fast(pDrawCan12, x, y, d, mask); }

// Draw round inverted (checks clipping limits)
void DrawCan12RoundInv(sDrawCan* can, int x, int y, int d, u8 mask)
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
		s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
		r2 = r*(r-1);
		if (d == 3) r2--;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*x + y*y) <= r2) INVPIXEL12(s,xx);
				if ((xx & 1) != 0) s += 3;
				xx++;
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
		s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) INVPIXEL12(s,xx);
				if ((xx & 1) != 0) s += 3;
				xx++;
			}
			s0 += wb;
		}
	}
}
void Draw12RoundInv(int x, int y, int d, u8 mask) { DrawCan12RoundInv(pDrawCan12, x, y, d, mask); }

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
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		s = &s0[x2/2*3 + (y2-basey)*wb]; \
		SETPIXEL12(s, x2, col); }

#define DRAWCANCIRCLE_PIX(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2/2*3 + (y2-basey)*wb]; \
		SETPIXEL12(s, x2, col); } }

#define DRAWCANCIRCLE_PIXINVFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		s = &s0[x2/2*3 + (y2-basey)*wb]; \
		INVPIXEL12(s, x2); }

#define DRAWCANCIRCLE_PIXINV(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2/2*3 + (y2-basey)*wb]; \
		INVPIXEL12(s, x2); } }

//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan12Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan12Point_Fast(can, x, y, col);
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
void Draw12Circle_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan12Circle_Fast(pDrawCan12, x, y, d, col, mask); }

// Draw circle or arc (checks clipping limits)
void DrawCan12Circle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan12Point(can, x, y, col);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan12Circle_Fast(can, x, y, d, col, mask);
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
void Draw12Circle(int x, int y, int d, u16 col, u8 mask) { DrawCan12Circle(pDrawCan12, x, y, d, col, mask); }

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan12CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan12PointInv_Fast(can, x, y);
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
void Draw12CircleInv_Fast(int x, int y, int d, u8 mask) { DrawCan12CircleInv_Fast(pDrawCan12, x, y, d, mask); }

// Draw circle or arc inverted (checks clipping limits)
void DrawCan12CircleInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan12PointInv(can, x, y);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan12CircleInv_Fast(can, x, y, d, mask);
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
void Draw12CircleInv(int x, int y, int d, u8 mask) { DrawCan12CircleInv(pDrawCan12, x, y, d, mask); }

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
void DrawCan12Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask)
{
	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan12Round_Fast(can, x, y, d, col, mask);
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
		DrawCan12Circle_Fast(can, x, y, d, col, mask2);
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
	int r2, rin2, xx, xxx;

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
					s = &s0[xxx/2*3];
					SETPIXEL12(s, xxx, col);
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
					s = &s0[xxx/2*3];
					SETPIXEL12(s, xxx, col);
				}
			}
			s0 += wb;
		}
	}
}
void Draw12Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan12Ring_Fast(pDrawCan12, x, y, d, din, col, mask); }

// Draw ring (checks clipping limits)
void DrawCan12Ring(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask)
{
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan12Round(can, x, y, d, col, mask);
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
		DrawCan12Circle(can, x, y, d, col, mask2);
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
	int r2, rin2, xx, xxx;

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
					s = &s0[xxx/2*3];
					SETPIXEL12(s, xxx, col);
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
					s = &s0[xxx/2*3];
					SETPIXEL12(s, xxx, col);
				}
			}
			s0 += wb;
		}
	}
}
void Draw12Ring(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan12Ring(pDrawCan12, x, y, d, din, col, mask); }

// Draw ring inverted (does not check clipping limits)
void DrawCan12RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan12RoundInv_Fast(can, x, y, d, mask);
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
		DrawCan12CircleInv_Fast(can, x, y, d, mask2);
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
	int r2, rin2, xx, xxx;

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
					s = &s0[xxx/2*3];
					INVPIXEL12(s, xxx);
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
					s = &s0[xxx/2*3];
					INVPIXEL12(s, xxx);
				}
			}
			s0 += wb;
		}
	}
}
void Draw12RingInv_Fast(int x, int y, int d, int din, u8 mask) { DrawCan12RingInv_Fast(pDrawCan12, x, y, d, din, mask); }

// Draw ring inverted (checks clipping limits)
void DrawCan12RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan12RoundInv(can, x, y, d, mask);
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
		DrawCan12CircleInv(can, x, y, d, mask2);
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
	int r2, rin2, xx, xxx;

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
					s = &s0[xxx/2*3];
					INVPIXEL12(s, xxx);
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
					s = &s0[xxx/2*3];
					INVPIXEL12(s, xxx);
				}
			}
			s0 += wb;
		}
	}
}
void Draw12RingInv(int x, int y, int d, int din, u8 mask) { DrawCan12RingInv(pDrawCan12, x, y, d, din, mask); }

// ----------------------------------------------------------------------------
//                              Draw triangle
// ----------------------------------------------------------------------------

// Draw triangle (does not check clipping limits)
void DrawCan12Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col)
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
			if (k > 0) DrawCan12HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan12HLine_Fast(can, xmin, y, k, col);
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
			if (k > 0) DrawCan12HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan12HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan12HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan12HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw12Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan12Triangle_Fast(pDrawCan12, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle (checks clipping limits)
void DrawCan12Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col)
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
		DrawCan12Triangle_Fast(can, x1, y1, x2, y2, x3, y3, col);
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
			if (k > 0) DrawCan12HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan12HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan12HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan12HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan12HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan12HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw12Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan12Triangle(pDrawCan12, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle inverted (does not check clipping limits)
void DrawCan12TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
			if (k > 0) DrawCan12HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan12HLineInv_Fast(can, xmin, y, k);
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
			if (k > 0) DrawCan12HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan12HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan12HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan12HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw12TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan12TriangleInv_Fast(pDrawCan12, x1, y1, x2, y2, x3, y3); }

// Draw triangle inverted (checks clipping limits)
void DrawCan12TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
		DrawCan12TriangleInv_Fast(can, x1, y1, x2, y2, x3, y3);
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
			if (k > 0) DrawCan12HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan12HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan12HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan12HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan12HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan12HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw12TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan12TriangleInv(pDrawCan12, x1, y1, x2, y2, x3, y3); }

// ----------------------------------------------------------------------------
//                              Draw fill area
// ----------------------------------------------------------------------------

// Draw fill sub-area (internal function)
void _DrawCan12SubFill(sDrawCan* can, int x, int y, u16 fnd, u16 col)
{
	// save start position -> x2
	int x2 = x;

	// fill start point
	DrawCan12Point_Fast(can, x, y, col);

	// update dirty area in Y direction
	if (y < can->dirtyy1) can->dirtyy1 = y;
	if (y >= can->dirtyy2) can->dirtyy2 = y+1;

	// search start of segment in LEFT-X direction -> x1
	int clipx1 = can->clipx1;
	while ((x > clipx1) && (DrawCan12GetPoint_Fast(can, x-1, y) == fnd))
	{
		x--;
		DrawCan12Point_Fast(can, x, y, col);
	}
	int x1 = x;

	// search end of segment in RIGHT-X direction -> x2
	int clipx2 = can->clipx2-1;
	while ((x2 < clipx2) && (DrawCan12GetPoint_Fast(can, x2+1, y) == fnd))
	{
		x2++;
		DrawCan12Point_Fast(can, x2, y, col);
	}

	// search segments in UP-Y direction
	if (y > can->clipy1)
	{
		x = x1;
		y--;
		while (x <= x2)
		{
			if (DrawCan12GetPoint_Fast(can, x, y) == fnd) _DrawCan12SubFill(can, x, y, fnd, col);
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
			if (DrawCan12GetPoint_Fast(can, x, y) == fnd) _DrawCan12SubFill(can, x, y, fnd, col);
			x++;
		}
	}

	// update dirty area in X direction
	if (x1 < can->dirtyx1) can->dirtyx1 = x1;
	if (x2 >= can->dirtyx2) can->dirtyx2 = x2+1;
}

// Draw fill area
void DrawCan12Fill(sDrawCan* can, int x, int y, u16 col)
{
	// check clipping
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// get color to search
	u16 fnd = DrawCan12GetPoint_Fast(can, x, y);

	// fill if color is different
	if (fnd != col) _DrawCan12SubFill(can, x, y, fnd, col);
}
void Draw12Fill(int x, int y, u16 col) { DrawCan12Fill(pDrawCan12, x, y, col); }

// ----------------------------------------------------------------------------
//                             Draw charactter
// ----------------------------------------------------------------------------
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw12SelFont(const u8* font, u8 fontw, u8 fonth) { DrawCanSelFont(pDrawCan12, font, fontw, fonth); }

void Draw12SelFont8x8()		{ DrawCanSelFont8x8(pDrawCan12); }		// sans-serif bold, height 8 lines
void Draw12SelFont8x14()	{ DrawCanSelFont8x14(pDrawCan12); }		// sans-serif bold, height 14 lines
void Draw12SelFont8x16()	{ DrawCanSelFont8x16(pDrawCan12); }		// sans-serif bold, height 16 lines
void Draw12SelFont8x14Serif()	{ DrawCanSelFont8x14Serif(pDrawCan12); }	// serif bold, height 14 lines
void Draw12SelFont8x16Serif()	{ DrawCanSelFont8x16Serif(pDrawCan12); }	// serif bold, height 16 lines
void Draw12SelFont6x8()		{ DrawCanSelFont6x8(pDrawCan12); }		// condensed font, width 6 pixels, height 8 lines
void Draw12SelFont8x8Game()	{ DrawCanSelFont8x8Game(pDrawCan12); }		// game font, height 8 lines
void Draw12SelFont8x8Ibm()	{ DrawCanSelFont8x8Ibm(pDrawCan12); }		// IBM charset font, height 8 lines
void Draw12SelFont8x14Ibm()	{ DrawCanSelFont8x14Ibm(pDrawCan12); }		// IBM charset font, height 14 lines
void Draw12SelFont8x16Ibm()	{ DrawCanSelFont8x16Ibm(pDrawCan12); }		// IBM charset font, height 16 lines
void Draw12SelFont8x8IbmThin()	{ DrawCanSelFont8x8IbmThin(pDrawCan12); }	// IBM charset thin font, height 8 lines
void Draw12SelFont8x8Italic()	{ DrawCanSelFont8x8Italic(pDrawCan12); }	// italic, height 8 lines
void Draw12SelFont8x8Thin()	{ DrawCanSelFont8x8Thin(pDrawCan12); }		// thin font, height 8 lines
void Draw12SelFont5x8()		{ DrawCanSelFont5x8(pDrawCan12); }		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan12Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley)
{
	int i, j, wb, scalex2, scaley2;
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
				if ((b & 0x80) != 0) DrawCan12Point_Fast(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan12Point_Fast(can, x, y, col);
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
void Draw12Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan12Char_Fast(pDrawCan12, ch, x, y, col, scalex, scaley); }

// Draw character with transparent background (checks clipping limits)
void DrawCan12Char(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan12Char_Fast(can, ch, x, y, col, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan12Point(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan12Point(can, x, y, col);
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
void Draw12Char(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan12Char(pDrawCan12, ch, x, y, col, scalex, scaley); }

// Draw character with background (does not check clipping limits)
void DrawCan12CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
				DrawCan12Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				DrawCan12Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw12CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan12CharBg_Fast(pDrawCan12, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character with background (checks clipping limits)
void DrawCan12CharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan12CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);
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
				DrawCan12Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				DrawCan12Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw12CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan12CharBg(pDrawCan12, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character inverted (does not check clipping limits)
void DrawCan12CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
				if ((b & 0x80) != 0) DrawCan12PointInv_Fast(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan12PointInv_Fast(can, x, y);
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
void Draw12CharInv_Fast(char ch, int x, int y, int scalex, int scaley) { DrawCan12CharInv_Fast(pDrawCan12, ch, x, y, scalex, scaley); }

// Draw character inverted (checks clipping limits)
void DrawCan12CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
		DrawCan12CharInv_Fast(can, ch, x, y, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan12PointInv(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan12PointInv(can, x, y);
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
void Draw12CharInv(char ch, int x, int y, int scalex, int scaley) { DrawCan12CharInv(pDrawCan12, ch, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                             Draw text
// ----------------------------------------------------------------------------
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan12Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan12Char_Fast(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw12Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan12Text_Fast(pDrawCan12, text, len, x, y, col, scalex, scaley); }

// Draw text with transparent background (checks clipping limits)
void DrawCan12Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan12Char(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw12Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan12Text(pDrawCan12, text, len, x, y, col, scalex, scaley); }

// Draw text with background (does not check clipping limits)
void DrawCan12TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan12CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw12TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan12TextBg_Fast(pDrawCan12, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text with background (checks clipping limits)
void DrawCan12TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan12CharBg(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw12TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan12TextBg(pDrawCan12, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text inverted (does not check clipping limits)
void DrawCan12TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan12CharInv_Fast(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw12TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan12TextInv_Fast(pDrawCan12, text, len, x, y, scalex, scaley); }

// Draw text inverted (checks clipping limits)
void DrawCan12TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan12CharInv(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw12TextInv(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan12TextInv(pDrawCan12, text, len, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                               Draw ellipse
// ----------------------------------------------------------------------------
// Using mid-point ellipse drawing algorithm

#define DRAWCANELLIPSE_PIXFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		s = &s0[x2/2*3 + (y2-basey)*wb]; \
		SETPIXEL12(s, x2, col); }

#define DRAWCANELLIPSE_PIX(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2/2*3 + (y2-basey)*wb]; \
		SETPIXEL12(s, x2, col); } }

#define DRAWCANELLIPSE_PIXINVFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		s = &s0[x2/2*3 + (y2-basey)*wb]; \
		INVPIXEL12(s, x2); }

#define DRAWCANELLIPSE_PIXINV(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2/2*3 + (y2-basey)*wb]; \
		INVPIXEL12(s, x2); } }

//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse
//		DRAWCAN_ELLIPSE_ARC0	= draw arc 0..90 deg
//		DRAWCAN_ELLIPSE_ARC1	= draw arc 90..180 deg
//		DRAWCAN_ELLIPSE_ARC2	= draw arc 180..270 deg
//		DRAWCAN_ELLIPSE_ARC3	= draw arc 270..360 deg
//		DRAWCAN_ELLIPSE_ALL	= draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan12Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask)
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
void Draw12Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan12Ellipse_Fast(pDrawCan12, x, y, dx, dy, col, mask); }

// Draw ellipse (checks clipping limits)
void DrawCan12Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask)
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
		DrawCan12Ellipse_Fast(can, x, y, dx, dy, col, mask);
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
void Draw12Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan12Ellipse(pDrawCan12, x, y, dx, dy, col, mask); }

// Draw ellipse inverted (does not check clipping limits)
void DrawCan12EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
void Draw12EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan12EllipseInv_Fast(pDrawCan12, x, y, dx, dy, mask); }

// Draw ellipse inverted (checks clipping limits)
void DrawCan12EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
		DrawCan12EllipseInv_Fast(can, x, y, dx, dy, mask);
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
void Draw12EllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan12EllipseInv(pDrawCan12, x, y, dx, dy, mask); }

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
void DrawCan12FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask)
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
	s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	int xx0 = x+x1;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		int xxx = xx0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) SETPIXEL12(s,xxx,col);
			if ((xxx & 1) != 0) s += 3;
			xxx++;
		}
		s0 += wb;
	}
}
void Draw12FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan12FillEllipse_Fast(pDrawCan12, x, y, dx, dy, col, mask); }

// Draw filled ellipse (checks clipping limits)
void DrawCan12FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask)
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
	s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	int xx0 = x+x1;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		int xxx = xx0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) SETPIXEL12(s,xxx,col);
			if ((xxx & 1) != 0) s += 3;
			xxx++;
		}
		s0 += wb;
	}
}
void Draw12FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan12FillEllipse(pDrawCan12, x, y, dx, dy, col, mask); }

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan12FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	int xx0 = x+x1;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		int xxx = xx0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) INVPIXEL12(s,xxx);
			if ((xxx & 1) != 0) s += 3;
			xxx++;
		}
		s0 += wb;
	}
}
void Draw12FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan12FillEllipseInv_Fast(pDrawCan12, x, y, dx, dy, mask); }

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan12FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	s0 = &can->buf[(x+x1)/2*3 + ((y+y1) - can->basey)*wb];
	r2 = (rx-1+cx)*(ry-1+cy);

	// draw ellipse
	r2 = rx2*ry2;
	int xx0 = x+x1;
	for (yy = y1; yy <= y2; yy++)
	{
		s = s0;
		int xxx = xx0;
		for (xx = x1; xx <= x2; xx++)
		{
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) INVPIXEL12(s,xxx);
			if ((xxx & 1) != 0) s += 3;
			xxx++;
		}
		s0 += wb;
	}
}
void Draw12FillEllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan12FillEllipseInv(pDrawCan12, x, y, dx, dy, mask); }

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
void DrawCan12Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2/3;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2*3 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2*3 + ys*wbs];

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u16 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				SETPIXEL12_EVEN(d,c);
			}
			else
			{
				SETPIXEL12_ODD(d,c);
				d += 3;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw12Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan12Img(pDrawCan12, xd, yd, src, xs, ys, w, h, wbs); }

// Draw image inverted with the same format as destination
void DrawCan12ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2/3;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2*3 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2*3 + ys*wbs];

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u16 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				INVCOLPIXEL12_EVEN(d,c);
			}
			else
			{
				INVCOLPIXEL12_ODD(d,c);
				d += 3;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw12ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan12ImgInv(pDrawCan12, xd, yd, src, xs, ys, w, h, wbs); }

// Draw transparent image with the same format as destination
void DrawCan12Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col)
{
	// width of source image
	int ws = wbs*2/3;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2*3 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2*3 + ys*wbs];

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u16 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				if (c != col) SETPIXEL12_EVEN(d,c);
			}
			else
			{
				if (c != col) SETPIXEL12_ODD(d,c);
				d += 3;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw12Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan12Blit(pDrawCan12, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan12BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs)
{
	// width of source image
	int ws = wbs*2/3;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2*3 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2*3 + ys*wbs];

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u16 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				if (c != col)
				{
					if (c == fnd) c = subs;
					SETPIXEL12_EVEN(d,c);
				}
			}
			else
			{
				if (c != col)
				{
					if (c == fnd) c = subs;
					SETPIXEL12_ODD(d,c);
				}
				d += 3;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw12BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { DrawCan12BlitSubs(pDrawCan12, xd, yd, src, xs, ys, w, h, wbs, col, fnd, subs); }

// Draw transparent image inverted with the same format as destination
void DrawCan12BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col)
{
	// width of source image
	int ws = wbs*2/3;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2*3 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2*3 + ys*wbs];

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u16 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				if (c != col) INVCOLPIXEL12_EVEN(d,c);
			}
			else
			{
				if (c != col) INVCOLPIXEL12_ODD(d,c);
				d += 3;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw12BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan12BlitInv(pDrawCan12, xd, yd, src, xs, ys, w, h, wbs, col); }

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
void DrawCan12GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd)
{
	// width of destination image
	int wd = wbd*2/3;

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
	const u8* s0 = &can->buf[xs/2*3 + (ys - can->basey)*wbs];

	// destination address
	u8* d0 = &((u8*)dst)[xd/2*3 + yd*wbd];

	// copy image
	int i;
	u8* d;
	const u8* s;
	int xxs, xxd;
	u16 c;
	for (; h > 0; h--)
	{
		d = d0;
		s = s0;
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				SETPIXEL12_EVEN(d,c);
			}
			else
			{
				SETPIXEL12_ODD(d,c);
				d += 3;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw12GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { DrawCan12GetImg(pDrawCan12, xs, ys, w, h, dst, xd, yd, wbd); }

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
void DrawCan12ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src_img, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
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
	int i, x2, y2;
	const u8* s;
	u8* d;
	u16 pix;
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

					s = &src[x2/2*3 + y2*wbs];
					pix = GETPIXEL12(s, x2);
					d = &d0[xd/2*3];
					SETPIXEL12(d, xd, pix);
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

					s = &src[x2/2*3 + y2*wbs];
					pix = GETPIXEL12(s, x2);
					d = &d0[xd/2*3];
					SETPIXEL12(d, xd, pix);
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
						s = &src[x2/2*3 + y2*wbs];
						pix = GETPIXEL12(s, x2);
						d = &d0[xd/2*3];
						SETPIXEL12(d, xd, pix);
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
					d = &d0[xd/2*3];
					SETPIXEL12(d, xd, col);
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

					s = &src[x2/2*3 + y2*wbs];
					pix = GETPIXEL12(s, x2);
					d = &d0[xd/2*3];
					SETPIXEL12(d, xd, pix);
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
					d = &d0[xd/2*3];
					SETPIXEL12(d, xd, color);
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
						s = &src[x2/2*3 + y2*wbs];
						pix = GETPIXEL12(s, x2);
					}
					else
						pix = color;

					d = &d0[xd/2*3];
					SETPIXEL12(d, xd, pix);
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
						s = &src[x2/2*3 + y2*wbs];
						pix = GETPIXEL12(s, x2);
						if (pix != color)
						{
							d = &d0[xd/2*3];
							SETPIXEL12(d, xd, pix);
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

					s = &src[x2/2*3 + y2*wbs];
					pix = GETPIXEL12(s, x2);
					d = &d0[xd/2*3];
					SETPIXEL12(d, xd, pix);
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

					s = &src[x2/2*3 + y2*wbs];
					pix = GETPIXEL12(s, x2);
					d = &d0[xd/2*3];
					SETPIXEL12(d, xd, pix);
					xd++;
				}
				y0++;
				d0 += wbd;
			}
		}
	}
}

void Draw12ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan12ImgMat(pDrawCan12, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

#undef DRAWCAN_IMGLIMIT

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// drawing function interface
const sDrawCanFnc DrawCan12Fnc = {
	.pDrawPitch		= Draw12Pitch,			// calculate pitch of graphics line from image width
	.pDrawMaxWidth		= Draw12MaxWidth,		// calculate max. width of image from the pitch
	// Clear canvas
	.pDrawCanClearCol	= DrawCan12ClearCol,		// Clear canvas with color
	.pDrawCanClear		= DrawCan12Clear,		// Clear canvas with black color
	// Draw point
	.pDrawCanPoint_Fast	= DrawCan12Point_Fast,		// Draw point (does not check clipping limits)
	.pDrawCanPoint		= DrawCan12Point,		// Draw point (checks clipping limits)
	.pDrawCanPointInv_Fast	= DrawCan12PointInv_Fast,	// Draw point inverted (does not check clipping limits)
	.pDrawCanPointInv	= DrawCan12PointInv,		// Draw point inverted (checks clipping limits)
	// Get point
	.pDrawCanGetPoint_Fast	= DrawCan12GetPoint_Fast,	// Get point (does not check clipping limits)
	.pDrawCanGetPoint	= DrawCan12GetPoint,		// Get point (checks clipping limits)
	// Draw rectangle
	.pDrawCanRect_Fast	= DrawCan12Rect_Fast,		// Draw rectangle (does not check clipping limits)
	.pDrawCanRect		= DrawCan12Rect,		// Draw rectangle (checks clipping limits)
	.pDrawCanRectInv_Fast	= DrawCan12RectInv_Fast,	// Draw rectangle inverted (does not check clipping limits)
	.pDrawCanRectInv	= DrawCan12RectInv,		// Draw rectangle inverted (checks clipping limits
	// Draw horizontal line
	.pDrawCanHLine_Fast	= DrawCan12HLine_Fast,		// Draw horizontal line (does not check clipping limits)
	.pDrawCanHLine		= DrawCan12HLine,		// Draw horizontal line (checks clipping limits; negative width flip line)
	.pDrawCanHLineInv_Fast	= DrawCan12HLineInv_Fast,	// Draw horizontal line inverted (does not check clipping limits)
	.pDrawCanHLineInv	= DrawCan12HLineInv,		// Draw horizontal line inverted (checks clipping limits; negative width flip line)
	// Draw vertical line
	.pDrawCanVLine_Fast	= DrawCan12VLine_Fast,		// Draw vertical line (does not check clipping limits)
	.pDrawCanVLine		= DrawCan12VLine,		// Draw vertical line (checks clipping limits; negative height flip line)
	.pDrawCanVLineInv_Fast	= DrawCan12VLineInv_Fast,	// Draw vertical line inverted (does not check clipping limits)
	.pDrawCanVLineInv	= DrawCan12VLineInv,		// Draw vertical line inverted (checks clipping limits; negative height flip line)
	// Draw frame
	.pDrawCanFrame_Fast	= DrawCan12Frame_Fast,		// Draw 1-pixel frame (does not check clipping limits)
	.pDrawCanFrame		= DrawCan12Frame,		// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameInv_Fast	= DrawCan12FrameInv_Fast,	// Draw 1-pixel frame inverted (does not check clipping limits)
	.pDrawCanFrameInv	= DrawCan12FrameInv,		// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameW_Fast	= DrawCan12FrameW_Fast,		// Draw thick frame (does not check clipping limits)
	.pDrawCanFrameW		= DrawCan12FrameW,		// Draw thick frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameWInv_Fast	= DrawCan12FrameWInv_Fast,	// Draw thick frame inverted (does not check clipping limits)
	.pDrawCanFrameWInv	= DrawCan12FrameWInv,		// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
	// Draw line
	.pDrawCanLineOver_Fast	= DrawCan12LineOver_Fast,	// Draw line with overlapped pixels (does not check clipping limits)
	.pDrawCanLine_Fast	= DrawCan12Line_Fast,		// Draw line (does not check clipping limits)
	.pDrawCanLineOver	= DrawCan12LineOver,		// Draw line with overlapped pixels (checks clipping limits)
	.pDrawCanLine		= DrawCan12Line,		// Draw line (checks clipping limits)
	.pDrawCanLineOverInv_Fast = DrawCan12LineOverInv_Fast,	// Draw line inverted with overlapped pixels (does not check clipping limits)
	.pDrawCanLineInv_Fast	= DrawCan12LineInv_Fast,	// Draw line inverted (does not check clipping limits)
	.pDrawCanLineOverInv	= DrawCan12LineOverInv,		// Draw line inverted with overlapped pixels (checks clipping limits)
	.pDrawCanLineInv	= DrawCan12LineInv,		// Draw line inverted (checks clipping limits)
	.pDrawCanLineW_Fast	= DrawCan12LineW_Fast,		// Draw thick line (does not check clipping limits)
	.pDrawCanLineW		= DrawCan12LineW,		// Draw thick line (checks clipping limits)
	.pDrawCanLineWInv_Fast	= DrawCan12LineWInv_Fast,	// Draw thick line inverted (does not check clipping limits)
	.pDrawCanLineWInv	= DrawCan12LineWInv,		// Draw thick line inverted (checks clipping limits)
	// Draw round (filled circle)
	.pDrawCanRound_Fast	= DrawCan12Round_Fast,		// Draw round (does not check clipping limits)
	.pDrawCanRound		= DrawCan12Round,		// Draw round (checks clipping limits)
	.pDrawCanRoundInv_Fast	= DrawCan12RoundInv_Fast,	// Draw round inverted (does not check clipping limits)
	.pDrawCanRoundInv	= DrawCan12RoundInv,		// Draw round inverted (checks clipping limits)
	// Draw circle
	.pDrawCanCircle_Fast	= DrawCan12Circle_Fast,		// Draw circle or arc (does not check clipping limits)
	.pDrawCanCircle		= DrawCan12Circle,		// Draw circle or arc (checks clipping limits)
	.pDrawCanCircleInv_Fast	= DrawCan12CircleInv_Fast,	// Draw circle or arc inverted (does not check clipping limits)
	.pDrawCanCircleInv	= DrawCan12CircleInv,		// Draw circle or arc inverted (checks clipping limits)
	// Draw ring
	.pDrawCanRing_Fast	= DrawCan12Ring_Fast,		// Draw ring (does not check clipping limits)
	.pDrawCanRing		= DrawCan12Ring,		// Draw ring (checks clipping limits)
	.pDrawCanRingInv_Fast	= DrawCan12RingInv_Fast,	// Draw ring inverted (does not check clipping limits)
	.pDrawCanRingInv	= DrawCan12RingInv,		// Draw ring inverted (checks clipping limits)
	// Draw triangle
	.pDrawCanTriangle_Fast	= DrawCan12Triangle_Fast,	// Draw triangle (does not check clipping limits)
	.pDrawCanTriangle	= DrawCan12Triangle,		// Draw triangle (checks clipping limits)
	.pDrawCanTriangleInv_Fast = DrawCan12TriangleInv_Fast,	// Draw triangle inverted (does not check clipping limits)
	.pDrawCanTriangleInv	= DrawCan12TriangleInv,		// Draw triangle inverted (checks clipping limits)
	// Draw fill area
	.pDrawCanFill		= DrawCan12Fill,		// Draw fill area
	// Draw character
	.pDrawCanChar_Fast	= DrawCan12Char_Fast,		// Draw character with transparent background (does not check clipping limits)
	.pDrawCanChar		= DrawCan12Char,		// Draw character with transparent background (checks clipping limits)
	.pDrawCanCharBg_Fast	= DrawCan12CharBg_Fast,		// Draw character with background (does not check clipping limits)
	.pDrawCanCharBg		= DrawCan12CharBg,		// Draw character with background (checks clipping limits)
	.pDrawCanCharInv_Fast	= DrawCan12CharInv_Fast,	// Draw character inverted (does not check clipping limits)
	.pDrawCanCharInv	= DrawCan12CharInv,		// Draw character inverted (checks clipping limits)
	// Draw text
	.pDrawCanText_Fast	= DrawCan12Text_Fast,		// Draw text with transparent background (does not check clipping limits)
	.pDrawCanText		= DrawCan12Text,		// Draw text with transparent background (checks clipping limits)
	.pDrawCanTextBg_Fast	= DrawCan12TextBg_Fast,		// Draw text with background (does not check clipping limits)
	.pDrawCanTextBg		= DrawCan12TextBg,		// Draw text with background (checks clipping limits)
	.pDrawCanTextInv_Fast	= DrawCan12TextInv_Fast,	// Draw text inverted (does not check clipping limits)
	.pDrawCanTextInv	= DrawCan12TextInv,		// Draw text inverted (checks clipping limits)
	// Draw ellipse
	.pDrawCanEllipse_Fast	= DrawCan12Ellipse_Fast,	// Draw ellipse (does not check clipping limits)
	.pDrawCanEllipse	= DrawCan12Ellipse,		// Draw ellipse (checks clipping limits)
	.pDrawCanEllipseInv_Fast = DrawCan12EllipseInv_Fast,	// Draw ellipse inverted (does not check clipping limits)
	.pDrawCanEllipseInv	= DrawCan12EllipseInv,		// Draw ellipse inverted (checks clipping limits)
	// Draw filled ellipse
	.pDrawCanFillEllipse_Fast = DrawCan12FillEllipse_Fast,	// Draw filled ellipse (does not check clipping limits)
	.pDrawCanFillEllipse	= DrawCan12FillEllipse,		// Draw filled ellipse (checks clipping limits)
	.pDrawCanFillEllipseInv_Fast = DrawCan12FillEllipseInv_Fast, // Draw filled ellipse inverted (does not check clipping limits)
	.pDrawCanFillEllipseInv = DrawCan12FillEllipseInv,	// Draw filled ellipse inverte (checks clipping limits)
	// Draw image
	.pDrawCanImg		= DrawCan12Img,			// Draw image with the same format as destination
	.pDrawCanImgInv		= DrawCan12ImgInv,		// Draw image inverted with the same format as destination
	.pDrawCanBlit		= DrawCan12Blit,		// Draw transparent image with the same format as destination
	.pDrawCanBlitInv	= DrawCan12BlitInv,		// Draw transparent image inverted with the same format as destination
	.pDrawCanBlitSubs	= DrawCan12BlitSubs,		// Draw transparent image with the same format as destination, with substitute color
	.pDrawCanGetImg		= DrawCan12GetImg,		// Get image from canvas to buffer
	.pDrawCanImgMat		= DrawCan12ImgMat,		// Draw image with 2D transformation matrix
	// colors
	.pColRgb		= Draw12ColRgb,			// convert RGB888 color to pixel color
#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
	.pColRand		= Draw12ColRand,		// random color
#endif
	.col_black		= COL12_BLACK,			// black color
	.col_blue		= COL12_BLUE,			// blue color
	.col_green		= COL12_GREEN,			// green color
	.col_cyan		= COL12_CYAN,			// cyan color
	.col_red		= COL12_RED,			// red color
	.col_magenta		= COL12_MAGENTA,		// magenta color
	.col_yellow		= COL12_YELLOW,			// yellow color
	.col_white		= COL12_WHITE,			// white color
	.col_gray		= COL12_GRAY,			// gray color
	.col_dkblue		= COL12_DKBLUE,			// dark blue color
	.col_dkgreen		= COL12_DKGREEN,		// dark green color
	.col_dkcyan		= COL12_DKCYAN,			// dark cyan color
	.col_dkred		= COL12_DKRED,			// dark red color
	.col_dkmagenta		= COL12_DKMAGENTA,		// dark magenta color
	.col_dkyellow		= COL12_DKYELLOW,		// dark yellow color
	.col_dkwhite		= COL12_DKWHITE,		// dark white color
	.col_dkgray		= COL12_DKGRAY,			// dark gray color
	.col_ltblue		= COL12_LTBLUE,			// light blue color
	.col_ltgreen		= COL12_LTGREEN,		// light green color
	.col_ltcyan		= COL12_LTCYAN,			// light cyan color
	.col_ltred		= COL12_LTRED,			// light red color
	.col_ltmagenta		= COL12_LTMAGENTA,		// light magenta color
	.col_ltyellow		= COL12_LTYELLOW,		// light yellow color
	.col_ltgray		= COL12_LTGRAY,			// light gray color
	.col_azure		= COL12_AZURE,			// azure blue color
	.col_orange		= COL12_ORANGE,			// orange color
	.col_brown		= COL12_BROWN,			// brown color
};

#endif // USE_DRAWCAN0

#endif // USE_DRAWCAN
