
// ****************************************************************************
//
//                     Drawing to canvas at 4-bit format
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

#if USE_DRAWCAN	&& USE_DRAWCAN4		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#include "../../_font/_include.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_float.h"
#include "../inc/lib_text.h"
#include "../inc/lib_rand.h"
#include "../inc/lib_drawcan.h"
#include "../inc/lib_drawcan4.h"

#ifndef WIDTH
#define WIDTH		320		// display width
#endif

#ifndef HEIGHT
#define HEIGHT		240		// display height
#endif

// default drawing canvas for 4-bit format
sDrawCan DrawCan4 = {
	// format
	.format = DRAWCAN_FORMAT_4,	// u8	format;		// canvas format CANVAS_*
	.colbit = 4,			// u8	colbit;		// number of bits per pixel
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
	.wb = ((WIDTH+1)/2+3)&~3,	// s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	.striph = HEIGHT,		// s16	striph;		// strip height (number of lines)

	// print
	.printposnum = WIDTH/FONTW,	// u16	printposnum;	// number of text positions per row (= w / fontw)
	.printrownum = HEIGHT/FONTH,	// u16	printrownum;	// number of text rows (= h / fonth)
	.printpos = 0,			// u16	printpos;	// console print character position
	.printrow = 0,			// u16	printrow;	// console print character row
	.printcol = COL4_WHITE,		// u16	printcol;	// console print color

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
	.drawfnc = NULL, //&DrawCan4Fnc,	// const struct sDrawCanFnc_* drawfnc; // drawing functions
};

// current drawing canvas for 4-bit format
sDrawCan* pDrawCan4 = &DrawCan4;

// set default drawing canvas for 4-bit format
void SetDrawCan4(sDrawCan* can) { pDrawCan4 = can; }

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw4Pitch(int w) { return ((w + 1)/2 + 3) & ~3; }

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw4MaxWidth(int pitch) { return pitch*2; }

// convert RGB888 color to 4-bit pixel color YRGB1111
u16 Draw4ColRgb(u8 r, u8 g, u8 b) { return COLOR4(r,g,b); }

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 4-bit pixel color YRGB1111
u16 Draw4ColRand() { return COL4_RANDOM; }
#endif

// ----------------------------------------------------------------------------
//                            Clear canvas
// ----------------------------------------------------------------------------

// Clear canvas with color
void DrawCan4ClearCol(sDrawCan* can, u16 col8)
{
	u8 col = (u8)col8;
	int x = can->clipx1;
	int y = can->clipy1;
	int w = can->clipx2 - x;
	int h = can->clipy2 - y;
	DrawCan4Rect(can, x, y, w, h, col);
}
void Draw4ClearCol(u16 col) { DrawCan4ClearCol(pDrawCan4, col); }

// Clear canvas with black color
void DrawCan4Clear(sDrawCan* can) { DrawCan4ClearCol(can, COL4_BLACK); }
void Draw4Clear() { DrawCan4Clear(pDrawCan4); }

// ----------------------------------------------------------------------------
//                               Draw point
// ----------------------------------------------------------------------------

// set even 4-bit pixel
#define SETPIXEL4_EVEN(ddd,ccc) { *(ddd) = (*(ddd) & 0x0f) | ((ccc) << 4); }
#define INVPIXEL4_EVEN(ddd) { *(ddd) ^= 0xf0; }
#define INVCOLPIXEL4_EVEN(ddd,ccc) { *(ddd) ^= (u8)((ccc)<<4); }

// set odd 4-bit pixel
#define SETPIXEL4_ODD(ddd,ccc) { *(ddd) = (*(ddd) & 0xf0) | (ccc); }
#define INVPIXEL4_ODD(ddd) { *(ddd) ^= 0x0f; }
#define INVCOLPIXEL4_ODD(ddd,ccc) { *(ddd) ^= (ccc); }

// set 4-bit pixel by x coordinate
#define SETPIXEL4(ddd,xxx,ccc) {				\
	if (((xxx) & 1) == 0)					\
		SETPIXEL4_EVEN(ddd,ccc)				\
	else							\
		SETPIXEL4_ODD(ddd,ccc); }

#define INVPIXEL4(ddd,xxx) {					\
	if (((xxx) & 1) == 0)					\
		INVPIXEL4_EVEN(ddd)				\
	else							\
		INVPIXEL4_ODD(ddd); }

// Draw point (does not check clipping limits)
void DrawCan4Point_Fast(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// draw pixel
	u8* d = &can->buf[x/2 + (y - can->basey)*can->wb];
	SETPIXEL4(d, x, col);
}
void Draw4Point_Fast(int x, int y, u16 col) { DrawCan4Point_Fast(pDrawCan4, x, y, col); }

// Draw point (checks clipping limits)
void DrawCan4Point(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// draw pixel
	u8* d = &can->buf[x/2 + (y - can->basey)*can->wb];
	SETPIXEL4(d, x, col);
}
void Draw4Point(int x, int y, u16 col) { DrawCan4Point(pDrawCan4, x, y, col); }

// Draw point inverted (does not check clipping limits)
void DrawCan4PointInv_Fast(sDrawCan* can, int x, int y)
{
	// invert pixel
	u8* d = &can->buf[x/2 + (y - can->basey)*can->wb];
	INVPIXEL4(d, x);
}
void Draw4PointInv_Fast(int x, int y) { DrawCan4PointInv_Fast(pDrawCan4, x, y); }

// Draw point inverted (checks clipping limits)
void DrawCan4PointInv(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// update dirty area
	DrawCanDirtyPoint_Fast(can, x, y);

	// invert pixel
	u8* d = &can->buf[x/2 + (y - can->basey)*can->wb];
	INVPIXEL4(d, x);
}
void Draw4PointInv(int x, int y) { DrawCan4PointInv(pDrawCan4, x, y); }

// ----------------------------------------------------------------------------
//                               Get point
// ----------------------------------------------------------------------------

// get even 4-bit pixel
#define GETPIXEL4_EVEN(ddd) ( *(ddd) >> 4 )

// get odd 4-bit pixel
#define GETPIXEL4_ODD(ddd) ( *(ddd) & 0x0f )

// get 4-bit pixel by x coordinate
#define GETPIXEL4(ddd,xxx) ( (((xxx) & 1) == 0) ? GETPIXEL4_EVEN(ddd) : GETPIXEL4_ODD(ddd) )

// Get point (does not check clipping limits)
u16 DrawCan4GetPoint_Fast(sDrawCan* can, int x, int y)
{
	u8* d = &can->buf[x/2 + (y - can->basey)*can->wb];
	return GETPIXEL4(d, x);
}
u16 Draw4GetPoint_Fast(int x, int y) { return DrawCan4GetPoint_Fast(pDrawCan4, x, y); }

// Get point (checks clipping limits)
u16 DrawCan4GetPoint(sDrawCan* can, int x, int y)
{
	// check coordinates
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return COL4_BLACK;

	// get pixel
	u8* d = &can->buf[x/2 + (y - can->basey)*can->wb];
	return GETPIXEL4(d, x);
}
u16 Draw4GetPoint(int x, int y) { return DrawCan4GetPoint(pDrawCan4, x, y); }

// ----------------------------------------------------------------------------
//                            Draw rectangle
// ----------------------------------------------------------------------------

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan4Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col8)
{
	u8 col = (u8)col8;

	// draw
	u8* d0 = &can->buf[x/2 + (y - can->basey)*can->wb];
	u8* d;
	int wb = can->wb;
	int i;
	u8 col2 = col | (col << 4);
	for (; h > 0; h--)
	{
		d = d0;
		i = w;

		// store first odd pixel
		if ((x & 1) != 0)
		{
			SETPIXEL4_ODD(d, col);
			d++;
			i--;
		}

		// store inner pixels
		for (; i > 1; i -= 2) *d++ = col2;

		// store last even pixel
		if (i > 0) SETPIXEL4_EVEN(d, col);

		// shift buffer address
		d0 += wb;
	}
}
void Draw4Rect_Fast(int x, int y, int w, int h, u16 col) { DrawCan4Rect_Fast(pDrawCan4, x, y, w, h, col); }

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan4Rect(sDrawCan* can, int x, int y, int w, int h, u16 col8)
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
	DrawCan4Rect_Fast(can, x, y, w, h, col);
}
void Draw4Rect(int x, int y, int w, int h, u16 col) { DrawCan4Rect(pDrawCan4, x, y, w, h, col); }

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan4RectInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// invert
	u8* d0 = &can->buf[x/2 + (y - can->basey)*can->wb];
	u8* d;
	int wb = can->wb;
	int i;
	for (; h > 0; h--)
	{
		d = d0;
		i = w;

		// invert first odd pixel
		if ((x & 1) != 0)
		{
			INVPIXEL4_ODD(d);
			d++;
			i--;
		}

		// invert inner pixels
		for (; i > 1; i -= 2)
		{
			*d = ~*d;
			d++;
		}

		// invert last even pixel
		if (i > 0) INVPIXEL4_EVEN(d);

		// shift buffer address
		d0 += wb;
	}
}
void Draw4RectInv_Fast(int x, int y, int w, int h) { DrawCan4RectInv_Fast(pDrawCan4, x, y, w, h); }

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan4RectInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan4RectInv_Fast(can, x, y, w, h);
}
void Draw4RectInv(int x, int y, int w, int h) { DrawCan4RectInv(pDrawCan4, x, y, w, h); }

// ----------------------------------------------------------------------------
//                          Draw horizontal line
// ----------------------------------------------------------------------------

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan4HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col8)
{
	u8 col = (u8)col8;

	// draw
	u8* d = &can->buf[x/2 + (y - can->basey)*can->wb];
	u8 col2 = col | (col << 4);

	// store first odd pixel
	if ((x & 1) != 0)
	{
		SETPIXEL4_ODD(d, col);
		d++;
		w--;
	}

	// store inner pixels
	for (; w > 1; w -= 2) *d++ = col2;

	// store last even pixel
	if (w > 0) SETPIXEL4_EVEN(d, col);
}
void Draw4HLine_Fast(int x, int y, int w, u16 col) { DrawCan4HLine_Fast(pDrawCan4, x, y, w, col); }

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan4HLine(sDrawCan* can, int x, int y, int w, u16 col)
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
	DrawCan4HLine_Fast(can, x, y, w, col);
}
void Draw4HLine(int x, int y, int w, u16 col) { DrawCan4HLine(pDrawCan4, x, y, w, col); }

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan4HLineInv_Fast(sDrawCan* can, int x, int y, int w)
{
	// invert
	u8* d = &can->buf[x/2 + (y - can->basey)*can->wb];

	// invert first odd pixel
	if ((x & 1) != 0)
	{
		INVPIXEL4_ODD(d);
		d++;
		w--;
	}

	// invert inner pixels
	for (; w > 1; w -= 2)
	{
		*d = ~*d;
		d++;
	}

	// invert last even pixel
	if (w > 0) INVPIXEL4_EVEN(d);
}
void Draw4HLineInv_Fast(int x, int y, int w) { DrawCan4HLineInv_Fast(pDrawCan4, x, y, w); }

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan4HLineInv(sDrawCan* can, int x, int y, int w)
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
	DrawCan4HLineInv_Fast(can, x, y, w);
}
void Draw4HLineInv(int x, int y, int w) { DrawCan4HLineInv(pDrawCan4, x, y, w); }

// ----------------------------------------------------------------------------
//                           Draw vertical line
// ----------------------------------------------------------------------------

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan4VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col8)
{
	u8 col = (u8)col8;

	// draw
	int wb = can->wb;
	u8* d = &can->buf[x/2 + (y - can->basey)*wb];

	// even pixel
	if ((x & 1) == 0)
	{
		for (; h > 0; h--)
		{
			SETPIXEL4_EVEN(d, col);
			d += wb;
		}
	}

	// odd pixel
	else
	{
		for (; h > 0; h--)
		{
			SETPIXEL4_ODD(d, col);
			d += wb;
		}
	}
}
void Draw4VLine_Fast(int x, int y, int h, u16 col) { DrawCan4VLine_Fast(pDrawCan4, x, y, h, col); }

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan4VLine(sDrawCan* can, int x, int y, int h, u16 col)
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
	DrawCan4VLine_Fast(can, x, y, h, col);
}
void Draw4VLine(int x, int y, int h, u16 col) { DrawCan4VLine(pDrawCan4, x, y, h, col); }

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan4VLineInv_Fast(sDrawCan* can, int x, int y, int h)
{
	// draw
	int wb = can->wb;
	u8* d = &can->buf[x/2 + (y - can->basey)*wb];

	// even pixel
	if ((x & 1) == 0)
	{
		for (; h > 0; h--)
		{
			INVPIXEL4_EVEN(d);
			d += wb;
		}
	}

	// odd pixel
	else
	{
		for (; h > 0; h--)
		{
			INVPIXEL4_ODD(d);
			d += wb;
		}
	}
}
void Draw4VLineInv_Fast(int x, int y, int h) { DrawCan4VLineInv_Fast(pDrawCan4, x, y, h); }

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan4VLineInv(sDrawCan* can, int x, int y, int h)
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
	DrawCan4VLineInv_Fast(can, x, y, h);
}
void Draw4VLineInv(int x, int y, int h) { DrawCan4VLineInv(pDrawCan4, x, y, h); }

// ----------------------------------------------------------------------------
//                           Draw frame
// ----------------------------------------------------------------------------

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan4Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
{
	// bottom line (dark)
	DrawCan4HLine_Fast(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan4VLine_Fast(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan4HLine_Fast(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan4VLine_Fast(can, x, y+1, h-2, col_light);
	}
}
void Draw4Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan4Frame_Fast(pDrawCan4, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan4Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark)
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
	DrawCan4HLine(can, x, y+h-1, w, col_dark);
	if (h > 1)
	{
		// right line (dark)
		DrawCan4VLine(can, x+w-1, y, h-1, col_dark);

		// top line (light)
		DrawCan4HLine(can, x, y, w-1, col_light);

		// left line (light)
		if (h > 2) DrawCan4VLine(can, x, y+1, h-2, col_light);
	}
}
void Draw4Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark) { DrawCan4Frame(pDrawCan4, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan4FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// bottom line
	DrawCan4HLineInv_Fast(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan4VLineInv_Fast(can, x+w-1, y, h-1);

		// top line
		DrawCan4HLineInv_Fast(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan4VLineInv_Fast(can, x, y+1, h-2);
	}
}
void Draw4FrameInv_Fast(int x, int y, int w, int h) { DrawCan4FrameInv_Fast(pDrawCan4, x, y, w, h); }

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan4FrameInv(sDrawCan* can, int x, int y, int w, int h)
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
	DrawCan4HLineInv(can, x, y+h-1, w);
	if (h > 1)
	{
		// right line
		DrawCan4VLineInv(can, x+w-1, y, h-1);

		// top line
		DrawCan4HLineInv(can, x, y, w-1);

		// left line
		if (h > 2) DrawCan4VLineInv(can, x, y+1, h-2);
	}
}
void Draw4FrameInv(int x, int y, int w, int h) { DrawCan4FrameInv(pDrawCan4, x, y, w, h); }

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan4FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan4Rect_Fast(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan4Rect_Fast(can, x, y, w, thick, col);

		// bottom line
		DrawCan4Rect_Fast(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan4Rect_Fast(can, x, y, thick, h, col);

		// right line
		DrawCan4Rect_Fast(can, x+w-thick, y, thick, h, col);
	}
}
void Draw4FrameW_Fast(int x, int y, int w, int h, u16 col, int thick) { DrawCan4FrameW_Fast(pDrawCan4, x, y, w, h, col, thick); }

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan4FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick)
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
		DrawCan4Rect(can, x, y, w, h, col);
	}
	else
	{
		// top line
		DrawCan4Rect(can, x, y, w, thick, col);

		// bottom line
		DrawCan4Rect(can, x, y+h-thick, w, thick, col);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan4Rect(can, x, y, thick, h, col);

		// right line
		DrawCan4Rect(can, x+w-thick, y, thick, h, col);
	}
}
void Draw4FrameW(int x, int y, int w, int h, u16 col, int thick) { DrawCan4Frame(pDrawCan4, x, y, w, h, col, thick); }

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan4FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick)
{
	// full frame
	if ((w <= thick) || (h <= thick))
	{
		DrawCan4RectInv_Fast(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan4RectInv_Fast(can, x, y, w, thick);

		// bottom line
		DrawCan4RectInv_Fast(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan4RectInv_Fast(can, x, y, thick, h);

		// right line
		DrawCan4RectInv_Fast(can, x+w-thick, y, thick, h);
	}
}
void Draw4FrameWInv_Fast(int x, int y, int w, int h, int thick) { DrawCan4FrameWInv_Fast(pDrawCan4, x, y, w, h, thick); }

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan4FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick)
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
		DrawCan4RectInv(can, x, y, w, h);
	}
	else
	{
		// top line
		DrawCan4RectInv(can, x, y, w, thick);

		// bottom line
		DrawCan4RectInv(can, x, y+h-thick, w, thick);

		// left line
		h -= 2*thick;
		y += thick;
		DrawCan4RectInv(can, x, y, thick, h);

		// right line
		DrawCan4RectInv(can, x+w-thick, y, thick, h);
	}
}
void Draw4FrameWInv(int x, int y, int w, int h, int thick) { DrawCan4FrameWInv(pDrawCan4, x, y, w, h, thick); }

// ----------------------------------------------------------------------------
//                           Draw line
// ----------------------------------------------------------------------------

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan4LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
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
	int sxdir = (sx >> 1) & 1;

	// destination address
	u8* d = &can->buf[x1/2 + (y1 - can->basey)*wb];

	// draw first pixel
	SETPIXEL4(d,x1,col);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			if ((x1 & 1) == sxdir) d += sx;
			if (p > 0)
			{
				if (over) SETPIXEL4(d,x1,col); // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			SETPIXEL4(d,x1,col);
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
				if (over) SETPIXEL4(d,x1,col); // draw overlapped pixel
				x1 += sx;
				if ((x1 & 1) == sxdir) d += sx;
				p -= dy;
			}
			p += m;
			SETPIXEL4(d,x1,col);
		}
	}
}
void Draw4LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan4LineOver_Fast(pDrawCan4, x1, y1, x2, y2, col, over); }

// Draw line (does not check clipping limits)
void DrawCan4Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan4LineOver_Fast(pDrawCan4, x1, y1, x2, y2, col, False); }
void Draw4Line_Fast(int x1, int y1, int x2, int y2, u16 col) { Draw4LineOver_Fast(x1, y1, x2, y2, col, False); }

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan4LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, Bool over)
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
	int sxdir = (sx >> 1) & 1;

	// destination address (pointer can be out of clipping range, but it is OK for now)
	u8* d = &can->buf[x1/2 + (y1 - can->basey)*wb];

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax))	SETPIXEL4(d,x1,col);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			if ((x1 & 1) == sxdir) d += sx;
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) SETPIXEL4(d,x1,col);
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) SETPIXEL4(d,x1,col);
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) SETPIXEL4(d,x1,col);
				x1 += sx;
				if ((x1 & 1) == sxdir) d += sx;
				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) SETPIXEL4(d,x1,col);
		}
	}
}
void Draw4LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over) { DrawCan4LineOver(pDrawCan4, x1, y1, x2, y2, col, over); }

// Draw line (checks clipping limits)
void DrawCan4Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { DrawCan4LineOver(can, x1, y1, x2, y2, col, False); }
void Draw4Line(int x1, int y1, int x2, int y2, u16 col) { DrawCan4Line(pDrawCan4, x1, y1, x2, y2, col); }

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan4LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	u8* d = &can->buf[x1/2 + (y1 - can->basey)*wb];

	// draw first pixel
	INVPIXEL4(d,x1);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			if ((x1 & 1) == sxdir) d += sx;
			if (p > 0)
			{
				if (over) INVPIXEL4(d,x1); // draw overlapped pixel
				d += ddy;
				p -= dx;
			}
			p += m;
			INVPIXEL4(d,x1);
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
				if (over) INVPIXEL4(d,x1); // draw overlapped pixel
				x1 += sx;
				if ((x1 & 1) == sxdir) d += sx;
				p -= dy;
			}
			p += m;
			INVPIXEL4(d,x1);
		}
	}
}
void Draw4LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over) { DrawCan4LineOverInv_Fast(pDrawCan4, x1, y1, x2, y2, over); }

// Draw line inverted (does not check clipping limits)
void DrawCan4LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan4LineOverInv_Fast(can, x1, y1, x2, y2, False); }
void Draw4LineInv_Fast(int x1, int y1, int x2, int y2) { DrawCan4LineOverInv_Fast(pDrawCan4, x1, y1, x2, y2, False); }

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan4LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over)
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
	u8* d = &can->buf[x1/2 + (y1 - can->basey)*wb];

	// get clipping
	int xmin = can->clipx1;
	int xmax = can->clipx2;
	int ymin = can->clipy1;
	int ymax = can->clipy2;

	// draw first pixel
	if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax))	INVPIXEL4(d,x1);

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		while (x1 != x2)
		{
			x1 += sx;
			if ((x1 & 1) == sxdir) d += sx;
			if (p > 0)
			{
				// draw overlapped pixel
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) INVPIXEL4(d,x1);
				y1 += sy;
				d += ddy;
				p -= dx;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) INVPIXEL4(d,x1);
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
				if (over && (x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) INVPIXEL4(d,x1);
				x1 += sx;
				if ((x1 & 1) == sxdir) d += sx;
				p -= dy;
			}
			p += m;
			if ((x1 >= xmin) && (x1 < xmax) && (y1 >= ymin) && (y1 < ymax)) INVPIXEL4(d,x1);
		}
	}
}
void Draw4LineOverInv(int x1, int y1, int x2, int y2, Bool over) { DrawCan4LineOverInv(pDrawCan4, x1, y1, x2, y2, over); }

// Draw line inverted (checks clipping limits)
void DrawCan4LineInv(sDrawCan* can, int x1, int y1, int x2, int y2) { DrawCan4LineOverInv(can, x1, y1, x2, y2, False); }
void Draw4LineInv(int x1, int y1, int x2, int y2) { DrawCan4LineOverInv(pDrawCan4, x1, y1, x2, y2, False); }

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan4LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan4LineOver_Fast(can, x1, y1, x2, y2, col, False);
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
		DrawCan4Round_Fast(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan4Round_Fast(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan4LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan4LineOver_Fast(can, x1, y1, x2, y2, col, over);
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
		DrawCan4LineOver_Fast(can, x1, y1, x2, y2, col, False);

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
			DrawCan4LineOver_Fast(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw4LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan4LineW_Fast(pDrawCan4, x1, y1, x2, y2, col, thick, round); }

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan4LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col8, int thick, Bool round)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;
	u8 col = (u8)col8;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan4LineOver(can, x1, y1, x2, y2, col, False);
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
		DrawCan4Round(can, x1, y1, thick2, col, DRAWCAN_ROUND_ALL);
		DrawCan4Round(can, x2, y2, thick2, col, DRAWCAN_ROUND_ALL);
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
		DrawCan4LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan4LineOver(can, x1, y1, x2, y2, col, over);
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
		DrawCan4LineOver(can, x1, y1, x2, y2, col, False);

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
			DrawCan4LineOver(can, x1, y1, x2, y2, col, over);
			err += dx2;
		}
	}
}
void Draw4LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { DrawCan4LineW(pDrawCan4, x1, y1, x2, y2, col, thick, round); }

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan4LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan4LineOverInv_Fast(can, x1, y1, x2, y2, False);
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
		DrawCan4LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan4LineOverInv_Fast(can, x1, y1, x2, y2, over);
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
		DrawCan4LineOverInv_Fast(can, x1, y1, x2, y2, False);

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
			DrawCan4LineOverInv_Fast(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw4LineWInv_Fast(int x1, int y1, int x2, int y2, int thick) { DrawCan4LineWInv_Fast(pDrawCan4, x1, y1, x2, y2, thick); }

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan4LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick)
{
	int i, k, dx, dy, dxabs, dyabs;
	Bool over;

	// thick is 1
	if (thick <= 1)
	{
		if (thick == 1) DrawCan4LineOverInv(can, x1, y1, x2, y2, False);
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
		DrawCan4LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan4LineOverInv(can, x1, y1, x2, y2, over);
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
		DrawCan4LineOverInv(can, x1, y1, x2, y2, False);

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
			DrawCan4LineOverInv(can, x1, y1, x2, y2, over);
			err += dx2;
		}
	}
}
void Draw4LineWInv(int x1, int y1, int x2, int y2, int thick) { DrawCan4LineWInv(pDrawCan4, x1, y1, x2, y2, thick); }

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
void DrawCan4Round_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
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
		s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
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
				if ((x*x + y*y) <= r2) SETPIXEL4(s,xx,col);
				if ((xx & 1) != 0) s++;
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
		s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) SETPIXEL4(s,xx,col);
				if ((xx & 1) != 0) s++;
				xx++;
			}
			s0 += wb;
		}
	}
}
void Draw4Round_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan4Round_Fast(pDrawCan4, x, y, d, col, mask); }

// Draw round (checks clipping limits)
void DrawCan4Round(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
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
		s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
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
				if ((x*x + y*y) <= r2) SETPIXEL4(s,xx,col);
				if ((xx & 1) != 0) s++;
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
		s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) SETPIXEL4(s,xx,col);
				if ((xx & 1) != 0) s++;
				xx++;
			}
			s0 += wb;
		}
	}
}
void Draw4Round(int x, int y, int d, u16 col, u8 mask) { DrawCan4Round(pDrawCan4, x, y, d, col, mask); }

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan4RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
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
		s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
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
				if ((x*x + y*y) <= r2) INVPIXEL4(s,xx);
				if ((xx & 1) != 0) s++;
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
		s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) INVPIXEL4(s,xx);
				if ((xx & 1) != 0) s++;
				xx++;
			}
			s0 += wb;
		}
	}
}
void Draw4RoundInv_Fast(int x, int y, int d, u8 mask) { DrawCan4RoundInv_Fast(pDrawCan4, x, y, d, mask); }

// Draw round inverted (checks clipping limits)
void DrawCan4RoundInv(sDrawCan* can, int x, int y, int d, u8 mask)
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
		s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
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
				if ((x*x + y*y) <= r2) INVPIXEL4(s,xx);
				if ((xx & 1) != 0) s++;
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
		s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
		r2 = r*r;

		// draw round
		int xx0 = x+x1;
		for (y = y1; y <= y2; y++)
		{
			s = s0;
			int xx = xx0;
			for (x = x1; x <= x2; x++)
			{
				if ((x*(x-1) + y*(y-1)) < r2) INVPIXEL4(s,xx);
				if ((xx & 1) != 0) s++;
				xx++;
			}
			s0 += wb;
		}
	}
}
void Draw4RoundInv(int x, int y, int d, u8 mask) { DrawCan4RoundInv(pDrawCan4, x, y, d, mask); }

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
		s = &s0[x2/2 + (y2-basey)*wb]; \
		SETPIXEL4(s, x2, col); }

#define DRAWCANCIRCLE_PIX(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2/2 + (y2-basey)*wb]; \
		SETPIXEL4(s, x2, col); } }

#define DRAWCANCIRCLE_PIXINVFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		s = &s0[x2/2 + (y2-basey)*wb]; \
		INVPIXEL4(s, x2); }

#define DRAWCANCIRCLE_PIXINV(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2/2 + (y2-basey)*wb]; \
		INVPIXEL4(s, x2); } }

//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan4Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan4Point_Fast(can, x, y, col);
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
void Draw4Circle_Fast(int x, int y, int d, u16 col, u8 mask) { DrawCan4Circle_Fast(pDrawCan4, x, y, d, col, mask); }

// Draw circle or arc (checks clipping limits)
void DrawCan4Circle(sDrawCan* can, int x, int y, int d, u16 col8, u8 mask)
{
	u8 col = (u8)col8;

	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan4Point(can, x, y, col);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan4Circle_Fast(can, x, y, d, col, mask);
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
void Draw4Circle(int x, int y, int d, u16 col, u8 mask) { DrawCan4Circle(pDrawCan4, x, y, d, col, mask); }

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan4CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan4PointInv_Fast(can, x, y);
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
void Draw4CircleInv_Fast(int x, int y, int d, u8 mask) { DrawCan4CircleInv_Fast(pDrawCan4, x, y, d, mask); }

// Draw circle or arc inverted (checks clipping limits)
void DrawCan4CircleInv(sDrawCan* can, int x, int y, int d, u8 mask)
{
	// check diameter
	if (d <= 1)
	{
		if (d == 1) DrawCan4PointInv(can, x, y);
		return;
	}

	// update dirty area (we ignore undrawn arcs)
	int r = d/2;
	DrawCanDirtyRect(can, x - r, y - r, d, d);

	// use fast variant of the function
	if (DrawCanRectClipped(can, x - r, y - r, d, d))
	{
		DrawCan4CircleInv_Fast(can, x, y, d, mask);
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
void Draw4CircleInv(int x, int y, int d, u8 mask) { DrawCan4CircleInv(pDrawCan4, x, y, d, mask); }

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
void DrawCan4Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	// nothing to draw
	if (din >= d) return;
	u8 col = (u8)col8;

	// draw round
	if (din <= 0)
	{
		DrawCan4Round_Fast(can, x, y, d, col, mask);
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
		DrawCan4Circle_Fast(can, x, y, d, col, mask2);
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
					s = &s0[xxx/2];
					SETPIXEL4(s, xxx, col);
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
					s = &s0[xxx/2];
					SETPIXEL4(s, xxx, col);
				}
			}
			s0 += wb;
		}
	}
}
void Draw4Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan4Ring_Fast(pDrawCan4, x, y, d, din, col, mask); }

// Draw ring (checks clipping limits)
void DrawCan4Ring(sDrawCan* can, int x, int y, int d, int din, u16 col8, u8 mask)
{
	u8 col = (u8)col8;
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan4Round(can, x, y, d, col, mask);
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
		DrawCan4Circle(can, x, y, d, col, mask2);
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
					s = &s0[xxx/2];
					SETPIXEL4(s, xxx, col);
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
					s = &s0[xxx/2];
					SETPIXEL4(s, xxx, col);
				}
			}
			s0 += wb;
		}
	}
}
void Draw4Ring(int x, int y, int d, int din, u16 col, u8 mask) { DrawCan4Ring(pDrawCan4, x, y, d, din, col, mask); }

// Draw ring inverted (does not check clipping limits)
void DrawCan4RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan4RoundInv_Fast(can, x, y, d, mask);
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
		DrawCan4CircleInv_Fast(can, x, y, d, mask2);
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
					s = &s0[xxx/2];
					INVPIXEL4(s, xxx);
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
					s = &s0[xxx/2];
					INVPIXEL4(s, xxx);
				}
			}
			s0 += wb;
		}
	}
}
void Draw4RingInv_Fast(int x, int y, int d, int din, u8 mask) { DrawCan4RingInv_Fast(pDrawCan4, x, y, d, din, mask); }

// Draw ring inverted (checks clipping limits)
void DrawCan4RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask)
{
	if (d < 1) d = 1;

	// nothing to draw
	if (din >= d) return;

	// draw round
	if (din <= 0)
	{
		DrawCan4RoundInv(can, x, y, d, mask);
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
		DrawCan4CircleInv(can, x, y, d, mask2);
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
					s = &s0[xxx/2];
					INVPIXEL4(s, xxx);
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
					s = &s0[xxx/2];
					INVPIXEL4(s, xxx);
				}
			}
			s0 += wb;
		}
	}
}
void Draw4RingInv(int x, int y, int d, int din, u8 mask) { DrawCan4RingInv(pDrawCan4, x, y, d, din, mask); }

// ----------------------------------------------------------------------------
//                              Draw triangle
// ----------------------------------------------------------------------------

// Draw triangle (does not check clipping limits)
void DrawCan4Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
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
			if (k > 0) DrawCan4HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan4HLine_Fast(can, xmin, y, k, col);
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
			if (k > 0) DrawCan4HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan4HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan4HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan4HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw4Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan4Triangle_Fast(pDrawCan4, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle (checks clipping limits)
void DrawCan4Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col8)
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
		DrawCan4Triangle_Fast(can, x1, y1, x2, y2, x3, y3, col);
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
			if (k > 0) DrawCan4HLine_Fast(can, xmax, y, k, col);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan4HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan4HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan4HLine_Fast(can, xmin, y, k, col);
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
				if (k > 0) DrawCan4HLine_Fast(can, xmax, y, k, col);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan4HLine_Fast(can, xmin, y, k, col);
			}
		}
	}
}
void Draw4Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { DrawCan4Triangle(pDrawCan4, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle inverted (does not check clipping limits)
void DrawCan4TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
			if (k > 0) DrawCan4HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan4HLineInv_Fast(can, xmin, y, k);
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
			if (k > 0) DrawCan4HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			k = xmax - xmin + 1;
			if (k > 0) DrawCan4HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan4HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				k = xmax - xmin + 1;
				if (k > 0) DrawCan4HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw4TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan4TriangleInv_Fast(pDrawCan4, x1, y1, x2, y2, x3, y3); }

// Draw triangle inverted (checks clipping limits)
void DrawCan4TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3)
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
		DrawCan4TriangleInv_Fast(can, x1, y1, x2, y2, x3, y3);
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
			if (k > 0) DrawCan4HLineInv_Fast(can, xmax, y, k);
		}
		else
		{
			if (xmin < clipx1) xmin = clipx1;
			if (xmax >= clipx2) xmax = clipx2-1;
			k = xmax - xmin + 1;
			if (k > 0) DrawCan4HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan4HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan4HLineInv_Fast(can, xmin, y, k);
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
				if (k > 0) DrawCan4HLineInv_Fast(can, xmax, y, k);
			}
			else
			{
				if (xmin < clipx1) xmin = clipx1;
				if (xmax >= clipx2) xmax = clipx2-1;
				k = xmax - xmin + 1;
				if (k > 0) DrawCan4HLineInv_Fast(can, xmin, y, k);
			}
		}
	}
}
void Draw4TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3) { DrawCan4TriangleInv(pDrawCan4, x1, y1, x2, y2, x3, y3); }

// ----------------------------------------------------------------------------
//                              Draw fill area
// ----------------------------------------------------------------------------

// Draw fill sub-area (internal function)
void _DrawCan4SubFill(sDrawCan* can, int x, int y, u8 fnd, u8 col)
{
	// save start position -> x2
	int x2 = x;

	// fill start point
	DrawCan4Point_Fast(can, x, y, col);

	// update dirty area in Y direction
	if (y < can->dirtyy1) can->dirtyy1 = y;
	if (y >= can->dirtyy2) can->dirtyy2 = y+1;

	// search start of segment in LEFT-X direction -> x1
	int clipx1 = can->clipx1;
	while ((x > clipx1) && ((u8)DrawCan4GetPoint_Fast(can, x-1, y) == fnd))
	{
		x--;
		DrawCan4Point_Fast(can, x, y, col);
	}
	int x1 = x;

	// search end of segment in RIGHT-X direction -> x2
	int clipx2 = can->clipx2-1;
	while ((x2 < clipx2) && ((u8)DrawCan4GetPoint_Fast(can, x2+1, y) == fnd))
	{
		x2++;
		DrawCan4Point_Fast(can, x2, y, col);
	}

	// search segments in UP-Y direction
	if (y > can->clipy1)
	{
		x = x1;
		y--;
		while (x <= x2)
		{
			if ((u8)DrawCan4GetPoint_Fast(can, x, y) == fnd) _DrawCan4SubFill(can, x, y, fnd, col);
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
			if ((u8)DrawCan4GetPoint_Fast(can, x, y) == fnd) _DrawCan4SubFill(can, x, y, fnd, col);
			x++;
		}
	}

	// update dirty area in X direction
	if (x1 < can->dirtyx1) can->dirtyx1 = x1;
	if (x2 >= can->dirtyx2) can->dirtyx2 = x2+1;
}

// Draw fill area
void DrawCan4Fill(sDrawCan* can, int x, int y, u16 col8)
{
	u8 col = (u8)col8;

	// check clipping
	if ((x < can->clipx1) || (x >= can->clipx2) || (y < can->clipy1) || (y >= can->clipy2)) return;

	// get color to search
	u8 fnd = (u8)DrawCan4GetPoint_Fast(can, x, y);

	// fill if color is different
	if (fnd != col) _DrawCan4SubFill(can, x, y, fnd, col);
}
void Draw4Fill(int x, int y, u16 col) { DrawCan4Fill(pDrawCan4, x, y, col); }

// ----------------------------------------------------------------------------
//                             Draw charactter
// ----------------------------------------------------------------------------
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw4SelFont(const u8* font, u8 fontw, u8 fonth) { DrawCanSelFont(pDrawCan4, font, fontw, fonth); }

void Draw4SelFont8x8()		{ DrawCanSelFont8x8(pDrawCan4); }		// sans-serif bold, height 8 lines
void Draw4SelFont8x14()		{ DrawCanSelFont8x14(pDrawCan4); }		// sans-serif bold, height 14 lines
void Draw4SelFont8x16()		{ DrawCanSelFont8x16(pDrawCan4); }		// sans-serif bold, height 16 lines
void Draw4SelFont8x14Serif()	{ DrawCanSelFont8x14Serif(pDrawCan4); }		// serif bold, height 14 lines
void Draw4SelFont8x16Serif()	{ DrawCanSelFont8x16Serif(pDrawCan4); }		// serif bold, height 16 lines
void Draw4SelFont6x8()		{ DrawCanSelFont6x8(pDrawCan4); }		// condensed font, width 6 pixels, height 8 lines
void Draw4SelFont8x8Game()	{ DrawCanSelFont8x8Game(pDrawCan4); }		// game font, height 8 lines
void Draw4SelFont8x8Ibm()	{ DrawCanSelFont8x8Ibm(pDrawCan4); }		// IBM charset font, height 8 lines
void Draw4SelFont8x14Ibm()	{ DrawCanSelFont8x14Ibm(pDrawCan4); }		// IBM charset font, height 14 lines
void Draw4SelFont8x16Ibm()	{ DrawCanSelFont8x16Ibm(pDrawCan4); }		// IBM charset font, height 16 lines
void Draw4SelFont8x8IbmThin()	{ DrawCanSelFont8x8IbmThin(pDrawCan4); }	// IBM charset thin font, height 8 lines
void Draw4SelFont8x8Italic()	{ DrawCanSelFont8x8Italic(pDrawCan4); }		// italic, height 8 lines
void Draw4SelFont8x8Thin()	{ DrawCanSelFont8x8Thin(pDrawCan4); }		// thin font, height 8 lines
void Draw4SelFont5x8()		{ DrawCanSelFont5x8(pDrawCan4); }		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan4Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
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
				if ((b & 0x80) != 0) DrawCan4Point_Fast(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan4Point_Fast(can, x, y, col);
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
void Draw4Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan4Char_Fast(pDrawCan4, ch, x, y, col, scalex, scaley); }

// Draw character with transparent background (checks clipping limits)
void DrawCan4Char(sDrawCan* can, char ch, int x, int y, u16 col8, int scalex, int scaley)
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
		DrawCan4Char_Fast(can, ch, x, y, col, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan4Point(can, x, y, col);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan4Point(can, x, y, col);
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
void Draw4Char(char ch, int x, int y, u16 col, int scalex, int scaley) { DrawCan4Char(pDrawCan4, ch, x, y, col, scalex, scaley); }

// Draw character with background (does not check clipping limits)
void DrawCan4CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
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
				DrawCan4Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				DrawCan4Point_Fast(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw4CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan4CharBg_Fast(pDrawCan4, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character with background (checks clipping limits)
void DrawCan4CharBg(sDrawCan* can, char ch, int x, int y, u16 col8, u16 bgcol8, int scalex, int scaley)
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
		DrawCan4CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);
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
				DrawCan4Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				DrawCan4Point(can, x, y, ((b & 0x80) != 0) ? col : bgcol);
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
void Draw4CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan4CharBg(pDrawCan4, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character inverted (does not check clipping limits)
void DrawCan4CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
				if ((b & 0x80) != 0) DrawCan4PointInv_Fast(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = can->fontw*scalex; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan4PointInv_Fast(can, x, y);
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
void Draw4CharInv_Fast(char ch, int x, int y, int scalex, int scaley) { DrawCan4CharInv_Fast(pDrawCan4, ch, x, y, scalex, scaley); }

// Draw character inverted (checks clipping limits)
void DrawCan4CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley)
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
		DrawCan4CharInv_Fast(can, ch, x, y, scalex, scaley);
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
				if ((b & 0x80) != 0) DrawCan4PointInv(can, x, y);
				x++;
				b <<= 1;
			}
		}
		else
		{
			scalex2 = scalex;
			for (j = w; j > 0; j--)
			{
				if ((b & 0x80) != 0) DrawCan4PointInv(can, x, y);
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
void Draw4CharInv(char ch, int x, int y, int scalex, int scaley) { DrawCan4CharInv(pDrawCan4, ch, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                             Draw text
// ----------------------------------------------------------------------------
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan4Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan4Char_Fast(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw4Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan4Text_Fast(pDrawCan4, text, len, x, y, col, scalex, scaley); }

// Draw text with transparent background (checks clipping limits)
void DrawCan4Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley)
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
		DrawCan4Char(can, ch, x, y, col, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw4Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { DrawCan4Text(pDrawCan4, text, len, x, y, col, scalex, scaley); }

// Draw text with background (does not check clipping limits)
void DrawCan4TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan4CharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw4TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan4TextBg_Fast(pDrawCan4, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text with background (checks clipping limits)
void DrawCan4TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley)
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
		DrawCan4CharBg(can, ch, x, y, col, bgcol, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw4TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { DrawCan4TextBg(pDrawCan4, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text inverted (does not check clipping limits)
void DrawCan4TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan4CharInv_Fast(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw4TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan4TextInv_Fast(pDrawCan4, text, len, x, y, scalex, scaley); }

// Draw text inverted (checks clipping limits)
void DrawCan4TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley)
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
		DrawCan4CharInv(can, ch, x, y, scalex, scaley);

		// shift to next character position
		x += dx;
	}
}
void Draw4TextInv(const char* text, int len, int x, int y, int scalex, int scaley) { DrawCan4TextInv(pDrawCan4, text, len, x, y, scalex, scaley); }

// ----------------------------------------------------------------------------
//                               Draw ellipse
// ----------------------------------------------------------------------------
// Using mid-point ellipse drawing algorithm

#define DRAWCANELLIPSE_PIXFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		s = &s0[x2/2 + (y2-basey)*wb]; \
		SETPIXEL4(s, x2, col); }

#define DRAWCANELLIPSE_PIX(xxx,yyy,mmm)		\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2/2 + (y2-basey)*wb]; \
		SETPIXEL4(s, x2, col); } }

#define DRAWCANELLIPSE_PIXINVFAST(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		s = &s0[x2/2 + (y2-basey)*wb]; \
		INVPIXEL4(s, x2); }

#define DRAWCANELLIPSE_PIXINV(xxx,yyy,mmm)	\
	if ((mask & mmm) != 0)			\
	{	int x2 = x+(xxx);		\
		int y2 = y+(yyy);		\
		if ((x2 >= clipx1) &&		\
			(x2 < clipx2) &&	\
			(y2 >= clipy1) &&	\
			(y2 < clipy2)) {	\
		s = &s0[x2/2 + (y2-basey)*wb]; \
		INVPIXEL4(s, x2); } }

//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse
//		DRAWCAN_ELLIPSE_ARC0	= draw arc 0..90 deg
//		DRAWCAN_ELLIPSE_ARC1	= draw arc 90..180 deg
//		DRAWCAN_ELLIPSE_ARC2	= draw arc 180..270 deg
//		DRAWCAN_ELLIPSE_ARC3	= draw arc 270..360 deg
//		DRAWCAN_ELLIPSE_ALL	= draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan4Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
void Draw4Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan4Ellipse_Fast(pDrawCan4, x, y, dx, dy, col, mask); }

// Draw ellipse (checks clipping limits)
void DrawCan4Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
		DrawCan4Ellipse_Fast(can, x, y, dx, dy, col, mask);
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
void Draw4Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan4Ellipse(pDrawCan4, x, y, dx, dy, col, mask); }

// Draw ellipse inverted (does not check clipping limits)
void DrawCan4EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
void Draw4EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan4EllipseInv_Fast(pDrawCan4, x, y, dx, dy, mask); }

// Draw ellipse inverted (checks clipping limits)
void DrawCan4EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
		DrawCan4EllipseInv_Fast(can, x, y, dx, dy, mask);
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
void Draw4EllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan4EllipseInv(pDrawCan4, x, y, dx, dy, mask); }

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
void DrawCan4FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
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
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) SETPIXEL4(s,xxx,col);
			if ((xxx & 1) != 0) s++;
			xxx++;
		}
		s0 += wb;
	}
}
void Draw4FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan4FillEllipse_Fast(pDrawCan4, x, y, dx, dy, col, mask); }

// Draw filled ellipse (checks clipping limits)
void DrawCan4FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col8, u8 mask)
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
	s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
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
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) SETPIXEL4(s,xxx,col);
			if ((xxx & 1) != 0) s++;
			xxx++;
		}
		s0 += wb;
	}
}
void Draw4FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { DrawCan4FillEllipse(pDrawCan4, x, y, dx, dy, col, mask); }

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan4FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
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
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) INVPIXEL4(s,xxx);
			if ((xxx & 1) != 0) s++;
			xxx++;
		}
		s0 += wb;
	}
}
void Draw4FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { DrawCan4FillEllipseInv_Fast(pDrawCan4, x, y, dx, dy, mask); }

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan4FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask)
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
	s0 = &can->buf[(x+x1)/2 + ((y+y1) - can->basey)*wb];
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
			if (xx*(xx-cx)*ry2 + yy*(yy-cy)*rx2 < r2) INVPIXEL4(s,xxx);
			if ((xxx & 1) != 0) s++;
			xxx++;
		}
		s0 += wb;
	}
}
void Draw4FillEllipseInv(int x, int y, int dx, int dy, u8 mask) { DrawCan4FillEllipseInv(pDrawCan4, x, y, dx, dy, mask); }

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
void DrawCan4Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2 + ys*wbs];

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
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				SETPIXEL4_EVEN(d,c);
			}
			else
			{
				SETPIXEL4_ODD(d,c);
				d++;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw4Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan4Img(pDrawCan4, xd, yd, src, xs, ys, w, h, wbs); }

// Draw image inverted with the same format as destination
void DrawCan4ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs)
{
	// width of source image
	int ws = wbs*2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2 + ys*wbs];

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
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				INVCOLPIXEL4_EVEN(d,c);
			}
			else
			{
				INVCOLPIXEL4_ODD(d,c);
				d++;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw4ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { DrawCan4ImgInv(pDrawCan4, xd, yd, src, xs, ys, w, h, wbs); }

// Draw transparent image with the same format as destination
void DrawCan4Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs*2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2 + ys*wbs];

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
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				if (c != col) SETPIXEL4_EVEN(d,c);
			}
			else
			{
				if (c != col) SETPIXEL4_ODD(d,c);
				d++;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw4Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan4Blit(pDrawCan4, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan4BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8, u16 fnd8, u16 subs8)
{
	u8 col = (u8)col8;
	u8 fnd = (u8)fnd8;
	u8 subs = (u8)subs8;

	// width of source image
	int ws = wbs*2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2 + ys*wbs];

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
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				if (c != col)
				{
					if (c == fnd) c = subs;
					SETPIXEL4_EVEN(d,c);
				}
			}
			else
			{
				if (c != col)
				{
					if (c == fnd) c = subs;
					SETPIXEL4_ODD(d,c);
				}
				d++;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw4BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { DrawCan4BlitSubs(pDrawCan4, xd, yd, src, xs, ys, w, h, wbs, col, fnd, subs); }

// Draw transparent image inverted with the same format as destination
void DrawCan4BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col8)
{
	u8 col = (u8)col8;

	// width of source image
	int ws = wbs*2;

	// limit image coordinates
	DRAWCAN_IMGLIMIT();

	// update dirty rectangle
	DrawCanDirtyRect_Fast(can, xd, yd, w, h);

	// destination address
	int wbd = can->wb;
	u8* d0 = &can->buf[xd/2 + (yd - can->basey)*wbd];

	// source address
	const u8* s0 = &((const u8*)src)[xs/2 + ys*wbs];

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
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				if (c != col) INVCOLPIXEL4_EVEN(d,c);
			}
			else
			{
				if (c != col) INVCOLPIXEL4_ODD(d,c);
				d++;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw4BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { DrawCan4BlitInv(pDrawCan4, xd, yd, src, xs, ys, w, h, wbs, col); }

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
void DrawCan4GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd)
{
	// width of destination image
	int wd = wbd*2;

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
	const u8* s0 = &can->buf[xs/2 + (ys - can->basey)*wbs];

	// destination address
	u8* d0 = &((u8*)dst)[xd/2 + yd*wbd];

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
		xxs = xs;
		xxd = xd;
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

			if ((xxd & 1) == 0)
			{
				SETPIXEL4_EVEN(d,c);
			}
			else
			{
				SETPIXEL4_ODD(d,c);
				d++;
			}
			xxd++;
		}
		d0 += wbd;
		s0 += wbs;
	}
}
void Draw4GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { DrawCan4GetImg(pDrawCan4, xs, ys, w, h, dst, xd, yd, wbd); }

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
void DrawCan4ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src_img, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
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

					s = &src[x2/2 + y2*wbs];
					pix = GETPIXEL4(s, x2);
					d = &d0[xd/2];
					SETPIXEL4(d, xd, pix);
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

					s = &src[x2/2 + y2*wbs];
					pix = GETPIXEL4(s, x2);
					d = &d0[xd/2];
					SETPIXEL4(d, xd, pix);
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
						s = &src[x2/2 + y2*wbs];
						pix = GETPIXEL4(s, x2);
						d = &d0[xd/2];
						SETPIXEL4(d, xd, pix);
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
					d = &d0[xd/2];
					SETPIXEL4(d, xd, col);
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

					s = &src[x2/2 + y2*wbs];
					pix = GETPIXEL4(s, x2);
					d = &d0[xd/2];
					SETPIXEL4(d, xd, pix);
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
					d = &d0[xd/2];
					SETPIXEL4(d, xd, color);
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
						s = &src[x2/2 + y2*wbs];
						pix = GETPIXEL4(s, x2);
					}
					else
						pix = color;

					d = &d0[xd/2];
					SETPIXEL4(d, xd, pix);
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
						s = &src[x2/2 + y2*wbs];
						pix = GETPIXEL4(s, x2);
						if (pix != color)
						{
							d = &d0[xd/2];
							SETPIXEL4(d, xd, pix);
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

					s = &src[x2/2 + y2*wbs];
					pix = GETPIXEL4(s, x2);
					d = &d0[xd/2];
					SETPIXEL4(d, xd, pix);
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

					s = &src[x2/2 + y2*wbs];
					pix = GETPIXEL4(s, x2);
					d = &d0[xd/2];
					SETPIXEL4(d, xd, pix);
					xd++;
				}
				y0++;
				d0 += wbd;
			}
		}
	}
}
void Draw4ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ DrawCan4ImgMat(pDrawCan4, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

#undef DRAWCAN_IMGLIMIT

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// drawing function interface
const sDrawCanFnc DrawCan4Fnc = {
	.pDrawPitch		= Draw4Pitch,			// calculate pitch of graphics line from image width
	.pDrawMaxWidth		= Draw4MaxWidth,		// calculate max. width of image from the pitch
	// Clear canvas
	.pDrawCanClearCol	= DrawCan4ClearCol,		// Clear canvas with color
	.pDrawCanClear		= DrawCan4Clear,		// Clear canvas with black color
	// Draw point
	.pDrawCanPoint_Fast	= DrawCan4Point_Fast,		// Draw point (does not check clipping limits)
	.pDrawCanPoint		= DrawCan4Point,		// Draw point (checks clipping limits)
	.pDrawCanPointInv_Fast	= DrawCan4PointInv_Fast,	// Draw point inverted (does not check clipping limits)
	.pDrawCanPointInv	= DrawCan4PointInv,		// Draw point inverted (checks clipping limits)
	// Get point
	.pDrawCanGetPoint_Fast	= DrawCan4GetPoint_Fast,	// Get point (does not check clipping limits)
	.pDrawCanGetPoint	= DrawCan4GetPoint,		// Get point (checks clipping limits)
	// Draw rectangle
	.pDrawCanRect_Fast	= DrawCan4Rect_Fast,		// Draw rectangle (does not check clipping limits)
	.pDrawCanRect		= DrawCan4Rect,			// Draw rectangle (checks clipping limits)
	.pDrawCanRectInv_Fast	= DrawCan4RectInv_Fast,		// Draw rectangle inverted (does not check clipping limits)
	.pDrawCanRectInv	= DrawCan4RectInv,		// Draw rectangle inverted (checks clipping limits
	// Draw horizontal line
	.pDrawCanHLine_Fast	= DrawCan4HLine_Fast,		// Draw horizontal line (does not check clipping limits)
	.pDrawCanHLine		= DrawCan4HLine,		// Draw horizontal line (checks clipping limits; negative width flip line)
	.pDrawCanHLineInv_Fast	= DrawCan4HLineInv_Fast,	// Draw horizontal line inverted (does not check clipping limits)
	.pDrawCanHLineInv	= DrawCan4HLineInv,		// Draw horizontal line inverted (checks clipping limits; negative width flip line)
	// Draw vertical line
	.pDrawCanVLine_Fast	= DrawCan4VLine_Fast,		// Draw vertical line (does not check clipping limits)
	.pDrawCanVLine		= DrawCan4VLine,		// Draw vertical line (checks clipping limits; negative height flip line)
	.pDrawCanVLineInv_Fast	= DrawCan4VLineInv_Fast,	// Draw vertical line inverted (does not check clipping limits)
	.pDrawCanVLineInv	= DrawCan4VLineInv,		// Draw vertical line inverted (checks clipping limits; negative height flip line)
	// Draw frame
	.pDrawCanFrame_Fast	= DrawCan4Frame_Fast,		// Draw 1-pixel frame (does not check clipping limits)
	.pDrawCanFrame		= DrawCan4Frame,		// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameInv_Fast	= DrawCan4FrameInv_Fast,	// Draw 1-pixel frame inverted (does not check clipping limits)
	.pDrawCanFrameInv	= DrawCan4FrameInv,		// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameW_Fast	= DrawCan4FrameW_Fast,		// Draw thick frame (does not check clipping limits)
	.pDrawCanFrameW		= DrawCan4FrameW,		// Draw thick frame (checks clipping limits; negative dimensions flip frame)
	.pDrawCanFrameWInv_Fast	= DrawCan4FrameWInv_Fast,	// Draw thick frame inverted (does not check clipping limits)
	.pDrawCanFrameWInv	= DrawCan4FrameWInv,		// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
	// Draw line
	.pDrawCanLineOver_Fast	= DrawCan4LineOver_Fast,	// Draw line with overlapped pixels (does not check clipping limits)
	.pDrawCanLine_Fast	= DrawCan4Line_Fast,		// Draw line (does not check clipping limits)
	.pDrawCanLineOver	= DrawCan4LineOver,		// Draw line with overlapped pixels (checks clipping limits)
	.pDrawCanLine		= DrawCan4Line,			// Draw line (checks clipping limits)
	.pDrawCanLineOverInv_Fast = DrawCan4LineOverInv_Fast,	// Draw line inverted with overlapped pixels (does not check clipping limits)
	.pDrawCanLineInv_Fast	= DrawCan4LineInv_Fast,		// Draw line inverted (does not check clipping limits)
	.pDrawCanLineOverInv	= DrawCan4LineOverInv,		// Draw line inverted with overlapped pixels (checks clipping limits)
	.pDrawCanLineInv	= DrawCan4LineInv,		// Draw line inverted (checks clipping limits)
	.pDrawCanLineW_Fast	= DrawCan4LineW_Fast,		// Draw thick line (does not check clipping limits)
	.pDrawCanLineW		= DrawCan4LineW,		// Draw thick line (checks clipping limits)
	.pDrawCanLineWInv_Fast	= DrawCan4LineWInv_Fast,	// Draw thick line inverted (does not check clipping limits)
	.pDrawCanLineWInv	= DrawCan4LineWInv,		// Draw thick line inverted (checks clipping limits)
	// Draw round (filled circle)
	.pDrawCanRound_Fast	= DrawCan4Round_Fast,		// Draw round (does not check clipping limits)
	.pDrawCanRound		= DrawCan4Round,		// Draw round (checks clipping limits)
	.pDrawCanRoundInv_Fast	= DrawCan4RoundInv_Fast,	// Draw round inverted (does not check clipping limits)
	.pDrawCanRoundInv	= DrawCan4RoundInv,		// Draw round inverted (checks clipping limits)
	// Draw circle
	.pDrawCanCircle_Fast	= DrawCan4Circle_Fast,		// Draw circle or arc (does not check clipping limits)
	.pDrawCanCircle		= DrawCan4Circle,		// Draw circle or arc (checks clipping limits)
	.pDrawCanCircleInv_Fast	= DrawCan4CircleInv_Fast,	// Draw circle or arc inverted (does not check clipping limits)
	.pDrawCanCircleInv	= DrawCan4CircleInv,		// Draw circle or arc inverted (checks clipping limits)
	// Draw ring
	.pDrawCanRing_Fast	= DrawCan4Ring_Fast,		// Draw ring (does not check clipping limits)
	.pDrawCanRing		= DrawCan4Ring,			// Draw ring (checks clipping limits)
	.pDrawCanRingInv_Fast	= DrawCan4RingInv_Fast,		// Draw ring inverted (does not check clipping limits)
	.pDrawCanRingInv	= DrawCan4RingInv,		// Draw ring inverted (checks clipping limits)
	// Draw triangle
	.pDrawCanTriangle_Fast	= DrawCan4Triangle_Fast,	// Draw triangle (does not check clipping limits)
	.pDrawCanTriangle	= DrawCan4Triangle,		// Draw triangle (checks clipping limits)
	.pDrawCanTriangleInv_Fast = DrawCan4TriangleInv_Fast,	// Draw triangle inverted (does not check clipping limits)
	.pDrawCanTriangleInv	= DrawCan4TriangleInv,		// Draw triangle inverted (checks clipping limits)
	// Draw fill area
	.pDrawCanFill		= DrawCan4Fill,			// Draw fill area
	// Draw character
	.pDrawCanChar_Fast	= DrawCan4Char_Fast,		// Draw character with transparent background (does not check clipping limits)
	.pDrawCanChar		= DrawCan4Char,			// Draw character with transparent background (checks clipping limits)
	.pDrawCanCharBg_Fast	= DrawCan4CharBg_Fast,		// Draw character with background (does not check clipping limits)
	.pDrawCanCharBg		= DrawCan4CharBg,		// Draw character with background (checks clipping limits)
	.pDrawCanCharInv_Fast	= DrawCan4CharInv_Fast,		// Draw character inverted (does not check clipping limits)
	.pDrawCanCharInv	= DrawCan4CharInv,		// Draw character inverted (checks clipping limits)
	// Draw text
	.pDrawCanText_Fast	= DrawCan4Text_Fast,		// Draw text with transparent background (does not check clipping limits)
	.pDrawCanText		= DrawCan4Text,			// Draw text with transparent background (checks clipping limits)
	.pDrawCanTextBg_Fast	= DrawCan4TextBg_Fast,		// Draw text with background (does not check clipping limits)
	.pDrawCanTextBg		= DrawCan4TextBg,		// Draw text with background (checks clipping limits)
	.pDrawCanTextInv_Fast	= DrawCan4TextInv_Fast,		// Draw text inverted (does not check clipping limits)
	.pDrawCanTextInv	= DrawCan4TextInv,		// Draw text inverted (checks clipping limits)
	// Draw ellipse
	.pDrawCanEllipse_Fast	= DrawCan4Ellipse_Fast,		// Draw ellipse (does not check clipping limits)
	.pDrawCanEllipse	= DrawCan4Ellipse,		// Draw ellipse (checks clipping limits)
	.pDrawCanEllipseInv_Fast = DrawCan4EllipseInv_Fast,	// Draw ellipse inverted (does not check clipping limits)
	.pDrawCanEllipseInv	= DrawCan4EllipseInv,		// Draw ellipse inverted (checks clipping limits)
	// Draw filled ellipse
	.pDrawCanFillEllipse_Fast = DrawCan4FillEllipse_Fast,	// Draw filled ellipse (does not check clipping limits)
	.pDrawCanFillEllipse	= DrawCan4FillEllipse,		// Draw filled ellipse (checks clipping limits)
	.pDrawCanFillEllipseInv_Fast = DrawCan4FillEllipseInv_Fast, // Draw filled ellipse inverted (does not check clipping limits)
	.pDrawCanFillEllipseInv = DrawCan4FillEllipseInv,	// Draw filled ellipse inverte (checks clipping limits)
	// Draw image
	.pDrawCanImg		= DrawCan4Img,			// Draw image with the same format as destination
	.pDrawCanImgInv		= DrawCan4ImgInv,		// Draw image inverted with the same format as destination
	.pDrawCanBlit		= DrawCan4Blit,			// Draw transparent image with the same format as destination
	.pDrawCanBlitInv	= DrawCan4BlitInv,		// Draw transparent image inverted with the same format as destination
	.pDrawCanBlitSubs	= DrawCan4BlitSubs,		// Draw transparent image with the same format as destination, with substitute color
	.pDrawCanGetImg		= DrawCan4GetImg,		// Get image from canvas to buffer
	.pDrawCanImgMat		= DrawCan4ImgMat,		// Draw image with 2D transformation matrix
	// colors
	.pColRgb		= Draw4ColRgb,			// convert RGB888 color to pixel color
#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
	.pColRand		= Draw4ColRand,			// random color
#endif
	.col_black		= COL4_BLACK,			// black color
	.col_blue		= COL4_BLUE,			// blue color
	.col_green		= COL4_GREEN,			// green color
	.col_cyan		= COL4_CYAN,			// cyan color
	.col_red		= COL4_RED,			// red color
	.col_magenta		= COL4_MAGENTA,			// magenta color
	.col_yellow		= COL4_YELLOW,			// yellow color
	.col_white		= COL4_WHITE,			// white color
	.col_gray		= COL4_GRAY,			// gray color
	.col_dkblue		= COL4_BLUE,			// dark blue color
	.col_dkgreen		= COL4_GREEN,			// dark green color
	.col_dkcyan		= COL4_CYAN,			// dark cyan color
	.col_dkred		= COL4_RED,			// dark red color
	.col_dkmagenta		= COL4_MAGENTA,			// dark magenta color
	.col_dkyellow		= COL4_YELLOW,			// dark yellow color
	.col_dkwhite		= COL4_WHITE,			// dark white color
	.col_dkgray		= COL4_GRAY,			// dark gray color
	.col_ltblue		= COL4_LTBLUE,			// light blue color
	.col_ltgreen		= COL4_LTGREEN,			// light green color
	.col_ltcyan		= COL4_LTCYAN,			// light cyan color
	.col_ltred		= COL4_LTRED,			// light red color
	.col_ltmagenta		= COL4_LTMAGENTA,		// light magenta color
	.col_ltyellow		= COL4_LTYELLOW,		// light yellow color
	.col_ltgray		= COL4_WHITE,			// light gray color
	.col_azure		= COL4_CYAN,			// azure blue color
	.col_orange		= COL4_YELLOW,			// orange color
	.col_brown		= COL4_YELLOW,			// brown color
};

#endif // USE_DRAWCAN0

#endif // USE_DRAWCAN
