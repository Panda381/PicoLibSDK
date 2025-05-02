
// ****************************************************************************
//
//                             Drawing to canvas
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

#if USE_DRAWCAN 		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#ifndef _LIB_DRAWCAN_H
#define _LIB_DRAWCAN_H

#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// drawing canvas format
enum {
	DRAWCAN_FORMAT_NONE = 0,	// invalid format

#if USE_DRAWCAN1		// 1=use DrawCan1 1-bit functions, if use drawing canvas
	DRAWCAN_FORMAT_1,		// 1-bit per pixel Y1 (8 pixels per byte, MSB first pixel, LSB last pixel)
					//	[7654 3210]
#endif

#if USE_DRAWCAN2		// 1=use DrawCan2 2-bit functions, if use drawing canvas
	DRAWCAN_FORMAT_2,		// 2-bits per pixel Y2 (4 pixels per byte, MSB first pixel, LSB last pixel)
					//	[3322 1100]
#endif

#if USE_DRAWCAN3		// 1=use DrawCan3 3-bit functions, if use drawing canvas
	DRAWCAN_FORMAT_3,		// 3-bits per pixel at format RGB111 (10 pixels per 32-bit word, LSB fist pixel, MSB last pixel)
					//	[xx99 9888 7776 6655 5444 3332 2211 1000]
#endif

#if USE_DRAWCAN4		// 1=use DrawCan4 4-bit functions, if use drawing canvas
	DRAWCAN_FORMAT_4,		// 4-bits per pixel at format YRGB1111 (2 pixels per byte, MSB first pixel, LSB last pixel)
					//	[1111 0000]
#endif

#if USE_DRAWCAN6		// 1=use DrawCan6 6-bit functions, if use drawing canvas
	DRAWCAN_FORMAT_6,		// 6-bits per pixel at format RGB222 (5 pixels per 32-bit word, LSB fist pixel, MSB last pixel)
					//	[xx44 4444 3333 3322 2222 1111 1100 0000]
#endif

#if USE_DRAWCAN8		// 1=use DrawCan8 8-bit functions, if use drawing canvas
	DRAWCAN_FORMAT_8,		// 8-bits per pixel at format RGB332 (1 pixel per byte)
					//	[RRRGGGBB]
#endif

#if USE_DRAWCAN12		// 1=use DrawCan12 12-bit functions, if use drawing canvas
	DRAWCAN_FORMAT_12,		// 12-bits per pixel at format RGB222 (8 pixels per three 32-bit words, or 2 pixels per 3 bytes, LSB fist pixel, MSB last pixel)
					//	[2222 2222 1111 1111 1111 0000 0000 0000]
					//	[5555 4444 4444 4444 3333 3333 3333 2222]
					//	[7777 7777 7777 6666 6666 6666 5555 5555]
#endif

#if USE_DRAWCAN16		// 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
	DRAWCAN_FORMAT_15,		// 15-bits per pixel at format RGB555 (1 pixel per 2 bytes, bit 15 is ignored)
					//	[.RRRRRGGGGGBBBBB]
	DRAWCAN_FORMAT_16,		// 16-bits per pixel at format RGB565 (1 pixel per 2 bytes)
					//	[RRRRRGGGGGGBBBBB]
#endif

	DRAWCAN_FORMAT_NUM
};

struct sDrawCanFnc_;

// drawing canvas descriptor
typedef struct {
	// format
	u8	format;		// drawing canvas format DRAWCAN_FORMAT_*
	u8	colbit;		// number of bits per pixel
	u8	strip;		// current strip index
	u8	stripnum;	// number of strips

	// font
	u8	fontw;		// font width (number of pixels, max. 8)
	u8	fonth;		// font height (number of lines)
	u8	printinv;	// offset added to printed character (print characters 128 = inverted, 0 = normal)
	u8	printsize;	// font size 0=normal, 1=double-height, 2=double-width, 3=double-size

	// dimension
	s16	w;		// width
	s16	h;		// height
	s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	s16	striph;		// strip height (number of lines)

	// print
	u16	printposnum;	// number of text positions per row (= w / fontw)
	u16	printrownum;	// number of text rows (= h / fonth)
	u16	printpos;	// console print character position
	u16	printrow;	// console print character row
	u16	printcol;	// console print color

// @TODO: The stripping back buffer is currently not supported

	// clipping
	s16	basey;		// base Y of buffer strip
	s16	clipx1;		// clip X min
	s16	clipx2;		// clip X+1 max
	s16	clipy1;		// clip Y min (current, limited by buffer strip)
	s16	clipy2;		// clip Y+1 max (current, limited by buffer strip)
	s16	clipy1_orig;	// clip Y min (original, without strip clipping)
	s16	clipy2_orig;	// clip Y+1 max (original, without strip clipping)

	// dirty window
	s16	dirtyx1;	// dirty window start X
	s16	dirtyx2;	// dirty window end X
	s16	dirtyy1;	// dirty window start Y
	s16	dirtyy2;	// dirty window end Y

	// data buffer
	u8*	buf;		// image data buffer (drawing buffer - back buffer or front buffer)
	u8*	frontbuf;	// front buffer (display buffer), or NULL = use only one buffer

	// last system time of auto update
	u32	autoupdatelast;	// last system time of auto update

	// font
	const u8* font;		// pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)

	// drawing functions interfaces
	const struct sDrawCanFnc_* drawfnc; // drawing functions
} sDrawCan;

// default drawing canvas
extern sDrawCan DrawCan;

// current global drawing canvas
extern sDrawCan* pDrawCan;

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// set default drawing canvas
void SetDrawCan(sDrawCan* can);

// get current width, height and buffer of the drawing canvas
INLINE int DrawWidth() { return pDrawCan->w; }
INLINE int DrawHeight() { return pDrawCan->h; }
INLINE u8* DrawBuf() { return pDrawCan->buf; }

#endif // USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// draw round (filled circle), ring or filled ellipse quarters
#define	DRAWCAN_ROUND_NOTOP	B0		// hide top part of the round
#define DRAWCAN_ROUND_NOBOTTOM	B1		// hide bottom part of the round
#define DRAWCAN_ROUND_NOLEFT	B2		// hide left part of the round
#define DRAWCAN_ROUND_NORIGHT	B3		// hide right part of the round

#define DRAWCAN_ROUND_ALL	0			// draw whole round
#define DRAWCAN_ROUND_TOP	DRAWCAN_ROUND_NOBOTTOM	// draw top half-round
#define DRAWCAN_ROUND_BOTTOM	DRAWCAN_ROUND_NOTOP	// draw bottom half-round
#define DRAWCAN_ROUND_LEFT	DRAWCAN_ROUND_NORIGHT	// draw left half-round
#define DRAWCAN_ROUND_RIGHT	DRAWCAN_ROUND_NOLEFT	// draw right half-round

// draw circle arcs
#define DRAWCAN_CIRCLE_ARC0	B0		// draw arc 0..45 deg
#define DRAWCAN_CIRCLE_ARC1	B1		// draw arc 45..90 deg
#define DRAWCAN_CIRCLE_ARC2	B2		// draw arc 90..135 deg
#define DRAWCAN_CIRCLE_ARC3	B3		// draw arc 135..180 deg
#define DRAWCAN_CIRCLE_ARC4	B4		// draw arc 180..225 deg
#define DRAWCAN_CIRCLE_ARC5	B5		// draw arc 225..270 deg
#define DRAWCAN_CIRCLE_ARC6	B6		// draw arc 270..315 deg
#define DRAWCAN_CIRCLE_ARC7	B7		// draw arc 315..360 deg

#define DRAWCAN_CIRCLE_ALL	0xff		// draw whole circle
#define DRAWCAN_CIRCLE_TOP	(DRAWCAN_CIRCLE_ARC0 | DRAWCAN_CIRCLE_ARC1 | DRAWCAN_CIRCLE_ARC2 | DRAWCAN_CIRCLE_ARC3) // draw top half-circle
#define DRAWCAN_CIRCLE_BOTTOM	(DRAWCAN_CIRCLE_ARC4 | DRAWCAN_CIRCLE_ARC5 | DRAWCAN_CIRCLE_ARC6 | DRAWCAN_CIRCLE_ARC7) // draw bottom half-circle
#define DRAWCAN_CIRCLE_LEFT	(DRAWCAN_CIRCLE_ARC2 | DRAWCAN_CIRCLE_ARC3 | DRAWCAN_CIRCLE_ARC4 | DRAWCAN_CIRCLE_ARC5) // draw left half-circle
#define DRAWCAN_CIRCLE_RIGHT	(DRAWCAN_CIRCLE_ARC6 | DRAWCAN_CIRCLE_ARC7 | DRAWCAN_CIRCLE_ARC0 | DRAWCAN_CIRCLE_ARC1) // draw right half-circle

// draw ellipse arcs
#define DRAWCAN_ELLIPSE_ARC0	B0		// draw arc 0..90 deg
#define DRAWCAN_ELLIPSE_ARC1	B1		// draw arc 90..180 deg
#define DRAWCAN_ELLIPSE_ARC2	B2		// draw arc 180..270 deg
#define DRAWCAN_ELLIPSE_ARC3	B3		// draw arc 270..360 deg

#define DRAWCAN_ELLIPSE_ALL	0x0f		// draw whole ellipse
#define DRAWCAN_ELLIPSE_TOP	(DRAWCAN_ELLIPSE_ARC0 | DRAWCAN_ELLIPSE_ARC1)	// draw top half-ellipse
#define DRAWCAN_ELLIPSE_BOTTOM	(DRAWCAN_ELLIPSE_ARC2 | DRAWCAN_ELLIPSE_ARC3)	// draw bottom half-ellipse
#define DRAWCAN_ELLIPSE_LEFT	(DRAWCAN_ELLIPSE_ARC1 | DRAWCAN_ELLIPSE_ARC2)	// draw left half-ellipse
#define DRAWCAN_ELLIPSE_RIGHT	(DRAWCAN_ELLIPSE_ARC0 | DRAWCAN_ELLIPSE_ARC3)	// draw right half-ellipse

#define DRAWCAN_ELLIPSE_MAXD	430		// max. diameter (X or Y) of the ellipse

// DrawImgMat mode
enum {
	DRAWIMG_WRAP,		// wrap image
	DRAWIMG_NOBORDER,	// no border (transparent border)
	DRAWIMG_CLAMP,		// clamp image (use last pixel as border)
	DRAWIMG_COLOR,		// color border
	DRAWIMG_TRANSP,		// transparent image with key color
	DRAWIMG_PERSP,		// perspective floor
};

// set dirty all frame buffer
void DrawCanDirtyAll(sDrawCan* can);
void DispDirtyAll();

// set dirty none
void DrawCanDirtyNone(sDrawCan* can);

// update dirty area by point (does not check clipping limits)
void DrawCanDirtyPoint_Fast(sDrawCan* can, int x, int y);

// update dirty area by point (does check clipping limits)
void DrawCanDirtyPoint(sDrawCan* can, int x, int y);

// update dirty area by rectangle (does not check clipping limits)
void DrawCanDirtyRect_Fast(sDrawCan* can, int x, int y, int w, int h);

// update dirty area by rectangle (does check clipping limits)
void DrawCanDirtyRect(sDrawCan* can, int x, int y, int w, int h);

// check if rectangle is clipped and if it is safe to use fast variant of the function
Bool DrawCanRectClipped(sDrawCan* can, int x, int y, int w, int h);

// Display update - transfer image from back buffer to front buffer
// @TODO: So far there is only a simplified update where the whole buffer (without clipping) is transferred using DMA.
void DrawCanUpdateAll(sDrawCan* can);
void DispUpdateAll();

// update screen, if dirty
void DrawCanUpdate(sDrawCan* can);
void DispUpdate();

// auto update after delta time in [ms] of running program
void DrawCanAutoUpdate(sDrawCan* can, u32 ms);
void DispAutoUpdate(u32 ms);

// scroll screen one row up
void DrawCanScroll(sDrawCan* can);
void DrawScroll();

// console print character (without display update)
//   Control characters:
//     0x01 '\1' ^A ... set not-inverted text
//     0x02 '\2' ^B ... set inverted text (shift character code by 0x80)
//     0x03 '\3' ^C ... use normal-sized font (default)
//     0x04 '\4' ^D ... use double-height font
//     0x05 '\5' ^E ... use double-width font
//     0x06 '\6' ^F ... use double-sized font
//     0x07 '\a' ^G ... (bell) move cursor 1 position right (no print; uses width of normal-sized font)
//     0x08 '\b' ^H ... (back space) move cursor 1 position left (no print; uses width of normal-sized font)
//     0x09 '\t' ^I ... (tabulator) move cursor to next 8-character position, print spaces (uses width of normal-sized font)
//     0x0A '\n' ^J ... (new line) move cursor to start of next row
//     0x0B '\v' ^K ... (vertical tabulator) move cursor to start of previous row
//     0x0C '\f' ^L ... (form feed) clear screen, reset cursor position and set default color
//     0x0D '\r' ^M ... (carriage return) move cursor to start of current row
//     0x10 '\20' ^P ... set gray text color (COL_GRAY, default)
//     0x11 '\21' ^Q ... set blue text color (COL_AZURE)
//     0x12 '\22' ^R ... set green text color (COL_GREEN)
//     0x13 '\23' ^S ... set cyan text color (COL_CYAN)
//     0x14 '\24' ^T ... set red text color (COL_RED)
//     0x15 '\25' ^U ... set magenta text color (COL_MAGENTA)
//     0x16 '\26' ^V ... set yellow text color (COL_YELLOW)
//     0x17 '\27' ^W ... set white text color (COL_WHITE)
void DrawCanPrintCharRaw(sDrawCan* can, char ch);
void DrawPrintCharRaw(char ch);

// console print character (with display update; Control characters - see DrawPrintCharRaw)
void DrawCanPrintChar(sDrawCan* can, char ch);
void DrawPrintChar(char ch);

// console print text (Control characters - see DrawPrintCharRaw)
//  If text contains digit after hex numeric code of control character,
//  split text to more parts: use "\4" "1" instead of "\41".
void DrawCanPrintText(sDrawCan* can, const char* txt);
void DrawPrintText(const char* txt);

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// formatted print string to drawing console, with argument list (returns number of characters, without terminating 0)
u32 DrawPrintArg(const char* fmt, va_list args);

// formatted print string to drawing console, with variadic arguments (returns number of characters, without terminating 0)
NOINLINE u32 DrawPrint(const char* fmt, ...);

#endif // USE_STREAM

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void DrawCanSelFont(sDrawCan* can, const u8* font, u8 fontw, u8 fonth);

void DrawCanSelFont8x8(sDrawCan* can);		// sans-serif bold, height 8 lines
void DrawCanSelFont8x14(sDrawCan* can);		// sans-serif bold, height 14 lines
void DrawCanSelFont8x16(sDrawCan* can);		// sans-serif bold, height 16 lines
void DrawCanSelFont8x14Serif(sDrawCan* can);	// serif bold, height 14 lines
void DrawCanSelFont8x16Serif(sDrawCan* can);	// serif bold, height 16 lines
void DrawCanSelFont6x8(sDrawCan* can);		// condensed font, width 6 pixels, height 8 lines
void DrawCanSelFont8x8Game(sDrawCan* can);	// game font, height 8 lines
void DrawCanSelFont8x8Ibm(sDrawCan* can);	// IBM charset font, height 8 lines
void DrawCanSelFont8x14Ibm(sDrawCan* can);	// IBM charset font, height 14 lines
void DrawCanSelFont8x16Ibm(sDrawCan* can);	// IBM charset font, height 16 lines
void DrawCanSelFont8x8IbmThin(sDrawCan* can);	// IBM charset thin font, height 8 lines
void DrawCanSelFont8x8Italic(sDrawCan* can);	// italic, height 8 lines
void DrawCanSelFont8x8Thin(sDrawCan* can);	// thin font, height 8 lines
void DrawCanSelFont5x8(sDrawCan* can);		// tiny font, width 5 pixels, height 8 lines

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

void DrawSelFont(const u8* font, u8 fontw, u8 fonth);

void DrawSelFont8x8();			// sans-serif bold, height 8 lines
void DrawSelFont8x14();			// sans-serif bold, height 14 lines
void DrawSelFont8x16();			// sans-serif bold, height 16 lines
void DrawSelFont8x14Serif();		// serif bold, height 14 lines
void DrawSelFont8x16Serif();		// serif bold, height 16 lines
void DrawSelFont6x8();			// condensed font, width 6 pixels, height 8 lines
void DrawSelFont8x8Game();		// game font, height 8 lines
void DrawSelFont8x8Ibm();		// IBM charset font, height 8 lines
void DrawSelFont8x14Ibm();		// IBM charset font, height 14 lines
void DrawSelFont8x16Ibm();		// IBM charset font, height 16 lines
void DrawSelFont8x8IbmThin();		// IBM charset thin font, height 8 lines
void DrawSelFont8x8Italic();		// italic, height 8 lines
void DrawSelFont8x8Thin();		// thin font, height 8 lines
void DrawSelFont5x8();			// tiny font, width 5 pixels, height 8 lines

// For backward compatibility with Draw version 1
INLINE void SelFont5x8() { DrawSelFont5x8(); }
INLINE void SelFont6x8() { DrawSelFont6x8(); }
INLINE void SelFont8x8() { DrawSelFont8x8(); }
INLINE void SelFont8x14() { DrawSelFont8x14(); }
INLINE void SelFont8x16() { DrawSelFont8x16(); }

#endif // USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// drawing function interface
typedef int (*fDrawPitch)(int w);
typedef int (*fDrawMaxWidth)(int pitch);
typedef void (*fDrawCanClearCol)(sDrawCan* can, u16 col);
typedef void (*fDrawCanClear)(sDrawCan* can);
typedef void (*fDrawCanPoint_Fast)(sDrawCan* can, int x, int y, u16 col);
typedef void (*fDrawCanPoint)(sDrawCan* can, int x, int y, u16 col);
typedef void (*fDrawCanPointInv_Fast)(sDrawCan* can, int x, int y);
typedef void (*fDrawCanPointInv)(sDrawCan* can, int x, int y);
typedef u16 (*fDrawCanGetPoint_Fast)(sDrawCan* can, int x, int y);
typedef u16 (*fDrawCanGetPoint)(sDrawCan* can, int x, int y);
typedef void (*fDrawCanRect_Fast)(sDrawCan* can, int x, int y, int w, int h, u16 col);
typedef void (*fDrawCanRect)(sDrawCan* can, int x, int y, int w, int h, u16 col);
typedef void (*fDrawCanRectInv_Fast)(sDrawCan* can, int x, int y, int w, int h);
typedef void (*fDrawCanRectInv)(sDrawCan* can, int x, int y, int w, int h);
typedef void (*fDrawCanHLine_Fast)(sDrawCan* can, int x, int y, int w, u16 col);
typedef void (*fDrawCanHLine)(sDrawCan* can, int x, int y, int w, u16 col);
typedef void (*fDrawCanHLineInv_Fast)(sDrawCan* can, int x, int y, int w);
typedef void (*fDrawCanHLineInv)(sDrawCan* can, int x, int y, int w);
typedef void (*fDrawCanVLine_Fast)(sDrawCan* can, int x, int y, int h, u16 col);
typedef void (*fDrawCanVLine)(sDrawCan* can, int x, int y, int h, u16 col);
typedef void (*fDrawCanVLineInv_Fast)(sDrawCan* can, int x, int y, int h);
typedef void (*fDrawCanVLineInv)(sDrawCan* can, int x, int y, int h);
typedef void (*fDrawCanFrame_Fast)(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
typedef void (*fDrawCanFrame)(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
typedef void (*fDrawCanFrameInv_Fast)(sDrawCan* can, int x, int y, int w, int h);
typedef void (*fDrawCanFrameInv)(sDrawCan* can, int x, int y, int w, int h);
typedef void (*fDrawCanFrameW_Fast)(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
typedef void (*fDrawCanFrameW)(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
typedef void (*fDrawCanFrameWInv_Fast)(sDrawCan* can, int x, int y, int w, int h, int thick);
typedef void (*fDrawCanFrameWInv)(sDrawCan* can, int x, int y, int w, int h, int thick);
typedef void (*fDrawCanLineOver_Fast)(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
typedef void (*fDrawCanLine_Fast)(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
typedef void (*fDrawCanLineOver)(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
typedef void (*fDrawCanLine)(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
typedef void (*fDrawCanLineOverInv_Fast)(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
typedef void (*fDrawCanLineInv_Fast)(sDrawCan* can, int x1, int y1, int x2, int y2);
typedef void (*fDrawCanLineOverInv)(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
typedef void (*fDrawCanLineInv)(sDrawCan* can, int x1, int y1, int x2, int y2);
typedef void (*fDrawCanLineW_Fast)(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
typedef void (*fDrawCanLineW)(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
typedef void (*fDrawCanLineWInv_Fast)(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
typedef void (*fDrawCanLineWInv)(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
typedef void (*fDrawCanRound_Fast)(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
typedef void (*fDrawCanRound)(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
typedef void (*fDrawCanRoundInv_Fast)(sDrawCan* can, int x, int y, int d, u8 mask);
typedef void (*fDrawCanRoundInv)(sDrawCan* can, int x, int y, int d, u8 mask);
typedef void (*fDrawCanCircle_Fast)(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
typedef void (*fDrawCanCircle)(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
typedef void (*fDrawCanCircleInv_Fast)(sDrawCan* can, int x, int y, int d, u8 mask);
typedef void (*fDrawCanCircleInv)(sDrawCan* can, int x, int y, int d, u8 mask);
typedef void (*fDrawCanRing_Fast)(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
typedef void (*fDrawCanRing)(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
typedef void (*fDrawCanRingInv_Fast)(sDrawCan* can, int x, int y, int d, int din, u8 mask);
typedef void (*fDrawCanRingInv)(sDrawCan* can, int x, int y, int d, int din, u8 mask);
typedef void (*fDrawCanTriangle_Fast)(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
typedef void (*fDrawCanTriangle)(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
typedef void (*fDrawCanTriangleInv_Fast)(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
typedef void (*fDrawCanTriangleInv)(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
typedef void (*fDrawCanFill)(sDrawCan* can, int x, int y, u16 col);
typedef void (*fDrawCanChar_Fast)(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
typedef void (*fDrawCanChar)(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
typedef void (*fDrawCanCharBg_Fast)(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
typedef void (*fDrawCanCharBg)(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
typedef void (*fDrawCanCharInv_Fast)(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
typedef void (*fDrawCanCharInv)(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
typedef void (*fDrawCanText_Fast)(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
typedef void (*fDrawCanText)(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
typedef void (*fDrawCanTextBg_Fast)(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
typedef void (*fDrawCanTextBg)(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
typedef void (*fDrawCanTextInv_Fast)(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
typedef void (*fDrawCanTextInv)(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
typedef void (*fDrawCanEllipse_Fast)(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
typedef void (*fDrawCanEllipse)(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
typedef void (*fDrawCanEllipseInv_Fast)(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
typedef void (*fDrawCanEllipseInv)(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
typedef void (*fDrawCanFillEllipse_Fast)(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
typedef void (*fDrawCanFillEllipse)(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
typedef void (*fDrawCanFillEllipseInv_Fast)(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
typedef void (*fDrawFillEllipseInv_Fast)(int x, int y, int dx, int dy, u8 mask);
typedef void (*fDrawCanFillEllipseInv)(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
typedef void (*fDrawCanImg)(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
typedef void (*fDrawCanImgInv)(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
typedef void (*fDrawCanBlit)(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
typedef void (*fDrawCanBlitInv)(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
typedef void (*fDrawCanBlitSubs)(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
typedef void (*fDrawCanGetImg)(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
typedef void (*fDrawCanImgMat)(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
typedef u16 (*fDrawColRgb)(u8 r, u8 g, u8 b);
typedef u16 (*fDrawColRand)();

typedef struct sDrawCanFnc_ {
	fDrawPitch		pDrawPitch;		// calculate pitch of graphics line from image width
	fDrawMaxWidth		pDrawMaxWidth;		// calculate max. width of image from the pitch
	// Clear canvas
	fDrawCanClearCol	pDrawCanClearCol;	// Clear canvas with color
	fDrawCanClear		pDrawCanClear;		// Clear canvas with black color
	// Draw point
	fDrawCanPoint_Fast	pDrawCanPoint_Fast;	// Draw point (does not check clipping limits)
	fDrawCanPoint		pDrawCanPoint;		// Draw point (checks clipping limits)
	fDrawCanPointInv_Fast	pDrawCanPointInv_Fast;	// Draw point inverted (does not check clipping limits)
	fDrawCanPointInv	pDrawCanPointInv;	// Draw point inverted (checks clipping limits)
	// Get point
	fDrawCanGetPoint_Fast	pDrawCanGetPoint_Fast;	// Get point (does not check clipping limits)
	fDrawCanGetPoint	pDrawCanGetPoint;	// Get point (checks clipping limits)
	// Draw rectangle
	fDrawCanRect_Fast	pDrawCanRect_Fast;	// Draw rectangle (does not check clipping limits)
	fDrawCanRect		pDrawCanRect;		// Draw rectangle (checks clipping limits)
	fDrawCanRectInv_Fast	pDrawCanRectInv_Fast;	// Draw rectangle inverted (does not check clipping limits)
	fDrawCanRectInv		pDrawCanRectInv;	// Draw rectangle inverted (checks clipping limits
	// Draw horizontal line
	fDrawCanHLine_Fast	pDrawCanHLine_Fast;	// Draw horizontal line (does not check clipping limits)
	fDrawCanHLine		pDrawCanHLine;		// Draw horizontal line (checks clipping limits; negative width flip line)
	fDrawCanHLineInv_Fast	pDrawCanHLineInv_Fast;	// Draw horizontal line inverted (does not check clipping limits)
	fDrawCanHLineInv	pDrawCanHLineInv;	// Draw horizontal line inverted (checks clipping limits; negative width flip line)
	// Draw vertical line
	fDrawCanVLine_Fast	pDrawCanVLine_Fast;	// Draw vertical line (does not check clipping limits)
	fDrawCanVLine		pDrawCanVLine;		// Draw vertical line (checks clipping limits; negative height flip line)
	fDrawCanVLineInv_Fast	pDrawCanVLineInv_Fast;	// Draw vertical line inverted (does not check clipping limits)
	fDrawCanVLineInv	pDrawCanVLineInv;	// Draw vertical line inverted (checks clipping limits; negative height flip line)
	// Draw frame
	fDrawCanFrame_Fast	pDrawCanFrame_Fast;	// Draw 1-pixel frame (does not check clipping limits)
	fDrawCanFrame		pDrawCanFrame;		// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
	fDrawCanFrameInv_Fast	pDrawCanFrameInv_Fast;	// Draw 1-pixel frame inverted (does not check clipping limits)
	fDrawCanFrameInv	pDrawCanFrameInv;	// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
	fDrawCanFrameW_Fast	pDrawCanFrameW_Fast;	// Draw thick frame (does not check clipping limits)
	fDrawCanFrameW		pDrawCanFrameW;		// Draw thick frame (checks clipping limits; negative dimensions flip frame)
	fDrawCanFrameWInv_Fast	pDrawCanFrameWInv_Fast;	// Draw thick frame inverted (does not check clipping limits)
	fDrawCanFrameWInv	pDrawCanFrameWInv;	// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
	// Draw line
	fDrawCanLineOver_Fast	pDrawCanLineOver_Fast;	// Draw line with overlapped pixels (does not check clipping limits)
	fDrawCanLine_Fast	pDrawCanLine_Fast;	// Draw line (does not check clipping limits)
	fDrawCanLineOver	pDrawCanLineOver;	// Draw line with overlapped pixels (checks clipping limits)
	fDrawCanLine		pDrawCanLine;		// Draw line (checks clipping limits)
	fDrawCanLineOverInv_Fast pDrawCanLineOverInv_Fast; // Draw line inverted with overlapped pixels (does not check clipping limits)
	fDrawCanLineInv_Fast	pDrawCanLineInv_Fast;	// Draw line inverted (does not check clipping limits)
	fDrawCanLineOverInv	pDrawCanLineOverInv;	// Draw line inverted with overlapped pixels (checks clipping limits)
	fDrawCanLineInv		pDrawCanLineInv;	// Draw line inverted (checks clipping limits)
	fDrawCanLineW_Fast	pDrawCanLineW_Fast;	// Draw thick line (does not check clipping limits)
	fDrawCanLineW		pDrawCanLineW;		// Draw thick line (checks clipping limits)
	fDrawCanLineWInv_Fast	pDrawCanLineWInv_Fast;	// Draw thick line inverted (does not check clipping limits)
	fDrawCanLineWInv	pDrawCanLineWInv;	// Draw thick line inverted (checks clipping limits)
	// Draw round (filled circle)
	fDrawCanRound_Fast	pDrawCanRound_Fast;	// Draw round (does not check clipping limits)
	fDrawCanRound		pDrawCanRound;		// Draw round (checks clipping limits)
	fDrawCanRoundInv_Fast	pDrawCanRoundInv_Fast;	// Draw round inverted (does not check clipping limits)
	fDrawCanRoundInv	pDrawCanRoundInv;	// Draw round inverted (checks clipping limits)
	// Draw circle
	fDrawCanCircle_Fast	pDrawCanCircle_Fast;	// Draw circle or arc (does not check clipping limits)
	fDrawCanCircle		pDrawCanCircle;		// Draw circle or arc (checks clipping limits)
	fDrawCanCircleInv_Fast	pDrawCanCircleInv_Fast;	// Draw circle or arc inverted (does not check clipping limits)
	fDrawCanCircleInv	pDrawCanCircleInv;	// Draw circle or arc inverted (checks clipping limits)
	// Draw ring
	fDrawCanRing_Fast	pDrawCanRing_Fast;	// Draw ring (does not check clipping limits)
	fDrawCanRing		pDrawCanRing;		// Draw ring (checks clipping limits)
	fDrawCanRingInv_Fast	pDrawCanRingInv_Fast;	// Draw ring inverted (does not check clipping limits)
	fDrawCanRingInv		pDrawCanRingInv;	// Draw ring inverted (checks clipping limits)
	// Draw triangle
	fDrawCanTriangle_Fast	pDrawCanTriangle_Fast;	// Draw triangle (does not check clipping limits)
	fDrawCanTriangle	pDrawCanTriangle;	// Draw triangle (checks clipping limits)
	fDrawCanTriangleInv_Fast pDrawCanTriangleInv_Fast; // Draw triangle inverted (does not check clipping limits)
	fDrawCanTriangleInv	pDrawCanTriangleInv;	// Draw triangle inverted (checks clipping limits)
	// Draw fill area
	fDrawCanFill		pDrawCanFill;		// Draw fill area
	// Draw character
	fDrawCanChar_Fast	pDrawCanChar_Fast;	// Draw character with transparent background (does not check clipping limits)
	fDrawCanChar		pDrawCanChar;		// Draw character with transparent background (checks clipping limits)
	fDrawCanCharBg_Fast	pDrawCanCharBg_Fast;	// Draw character with background (does not check clipping limits)
	fDrawCanCharBg		pDrawCanCharBg;		// Draw character with background (checks clipping limits)
	fDrawCanCharInv_Fast	pDrawCanCharInv_Fast;	// Draw character inverted (does not check clipping limits)
	fDrawCanCharInv		pDrawCanCharInv;	// Draw character inverted (checks clipping limits)
	// Draw text
	fDrawCanText_Fast	pDrawCanText_Fast;	// Draw text with transparent background (does not check clipping limits)
	fDrawCanText		pDrawCanText;		// Draw text with transparent background (checks clipping limits)
	fDrawCanTextBg_Fast	pDrawCanTextBg_Fast;	// Draw text with background (does not check clipping limits)
	fDrawCanTextBg		pDrawCanTextBg;		// Draw text with background (checks clipping limits)
	fDrawCanTextInv_Fast	pDrawCanTextInv_Fast;	// Draw text inverted (does not check clipping limits)
	fDrawCanTextInv		pDrawCanTextInv;	// Draw text inverted (checks clipping limits)
	// Draw ellipse
	fDrawCanEllipse_Fast	pDrawCanEllipse_Fast;	// Draw ellipse (does not check clipping limits)
	fDrawCanEllipse		pDrawCanEllipse;	// Draw ellipse (checks clipping limits)
	fDrawCanEllipseInv_Fast	pDrawCanEllipseInv_Fast; // Draw ellipse inverted (does not check clipping limits)
	fDrawCanEllipseInv	pDrawCanEllipseInv;	// Draw ellipse inverted (checks clipping limits)
	// Draw filled ellipse
	fDrawCanFillEllipse_Fast pDrawCanFillEllipse_Fast; // Draw filled ellipse (does not check clipping limits)
	fDrawCanFillEllipse	pDrawCanFillEllipse;	// Draw filled ellipse (checks clipping limits)
	fDrawCanFillEllipseInv_Fast pDrawCanFillEllipseInv_Fast; // Draw filled ellipse inverted (does not check clipping limits)
	fDrawCanFillEllipseInv	pDrawCanFillEllipseInv;	// Draw filled ellipse inverte (checks clipping limits)
	// Draw image
	fDrawCanImg		pDrawCanImg;		// Draw image with the same format as destination
	fDrawCanImgInv		pDrawCanImgInv;		// Draw image inverted with the same format as destination
	fDrawCanBlit		pDrawCanBlit;		// Draw transparent image with the same format as destination
	fDrawCanBlitInv		pDrawCanBlitInv;	// Draw transparent image inverted with the same format as destination
	fDrawCanBlitSubs	pDrawCanBlitSubs;	// Draw transparent image with the same format as destination, with substitute color
	fDrawCanGetImg		pDrawCanGetImg;		// Get image from canvas to buffer
	fDrawCanImgMat		pDrawCanImgMat;		// Draw image with 2D transformation matrix
	// colors
	fDrawColRgb		pColRgb;		// convert RGB888 color to pixel color
	fDrawColRand		pColRand;		// random color
	const u16		col_black;		// black color
	const u16		col_blue;		// blue color
	const u16		col_green;		// green color
	const u16		col_cyan;		// cyan color
	const u16		col_red;		// red color
	const u16		col_magenta;		// magenta color
	const u16		col_yellow;		// yellow color
	const u16		col_white;		// white color
	const u16		col_gray;		// gray color
	const u16		col_dkblue;		// dark blue color
	const u16		col_dkgreen;		// dark green color
	const u16		col_dkcyan;		// dark cyan color
	const u16		col_dkred;		// dark red color
	const u16		col_dkmagenta;		// dark magenta color
	const u16		col_dkyellow;		// dark yellow color
	const u16		col_dkwhite;		// dark white color
	const u16		col_dkgray;		// dark gray color
	const u16		col_ltblue;		// light blue color
	const u16		col_ltgreen;		// light green color
	const u16		col_ltcyan;		// light cyan color
	const u16		col_ltred;		// light red color
	const u16		col_ltmagenta;		// light magenta color
	const u16		col_ltyellow;		// light yellow color
	const u16		col_ltgray;		// light gray color
	const u16		col_azure;		// azure blue color
	const u16		col_orange;		// orange color
	const u16		col_brown;		// brown color
} sDrawCanFnc;

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// list of drawing function interfaces
extern const sDrawCanFnc* DrawCanFncList[DRAWCAN_FORMAT_NUM];

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int DrawPitch(int w);

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int DrawMaxWidth(int pitch);

// ----- Clear canvas

// Clear canvas with color
void DrawCanClearCol(sDrawCan* can, u16 col);
void DrawClearCol(u16 col);

// Clear canvas with black color
void DrawCanClear(sDrawCan* can);
void DrawClear();

// Fast clear canvas with DMA
void DrawCanClearFast(sDrawCan* can);
void DrawClearFast();

// ----- Draw point

// Draw point (does not check clipping limits)
void DrawCanPoint_Fast(sDrawCan* can, int x, int y, u16 col);
void DrawPoint_Fast(int x, int y, u16 col);

// Draw point (checks clipping limits)
void DrawCanPoint(sDrawCan* can, int x, int y, u16 col);
void DrawPoint(int x, int y, u16 col);

// Draw point inverted (does not check clipping limits)
void DrawCanPointInv_Fast(sDrawCan* can, int x, int y);
void DrawPointInv_Fast(int x, int y);

// Draw point inverted (checks clipping limits)
void DrawCanPointInv(sDrawCan* can, int x, int y);
void DrawPointInv(int x, int y);

// ----- Get point

// Get point (does not check clipping limits)
u16 DrawCanGetPoint_Fast(sDrawCan* can, int x, int y);
u16 DrawGetPoint_Fast(int x, int y);

// Get point (checks clipping limits)
u16 DrawCanGetPoint(sDrawCan* can, int x, int y);
u16 DrawGetPoint(int x, int y);

// ----- Draw rectangle

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCanRect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col);
void DrawRect_Fast(int x, int y, int w, int h, u16 col);

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCanRect(sDrawCan* can, int x, int y, int w, int h, u16 col);
void DrawRect(int x, int y, int w, int h, u16 col);

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCanRectInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void DrawRectInv_Fast(int x, int y, int w, int h);

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCanRectInv(sDrawCan* can, int x, int y, int w, int h);
void DrawRectInv(int x, int y, int w, int h);

// ----- Draw horizontal line

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCanHLine_Fast(sDrawCan* can, int x, int y, int w, u16 col);
void DrawHLine_Fast(int x, int y, int w, u16 col);

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCanHLine(sDrawCan* can, int x, int y, int w, u16 col);
void DrawHLine(int x, int y, int w, u16 col);

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCanHLineInv_Fast(sDrawCan* can, int x, int y, int w);
void DrawHLineInv_Fast(int x, int y, int w);

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCanHLineInv(sDrawCan* can, int x, int y, int w);
void DrawHLineInv(int x, int y, int w);

// ----- Draw vertical line

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCanVLine_Fast(sDrawCan* can, int x, int y, int h, u16 col);
void DrawVLine_Fast(int x, int y, int h, u16 col);

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCanVLine(sDrawCan* can, int x, int y, int h, u16 col);
void DrawVLine(int x, int y, int h, u16 col);

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCanVLineInv_Fast(sDrawCan* can, int x, int y, int h);
void DrawVLineInv_Fast(int x, int y, int h);

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCanVLineInv(sDrawCan* can, int x, int y, int h);
void DrawVLineInv(int x, int y, int h);

// ----- Draw frame

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCanFrame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void DrawFrame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCanFrame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);

// For backward compatibility with Draw version 1
//void DrawFrame(int x, int y, int w, int h, u16 col_light, u16 col_dark);
void DrawFrame3D(int x, int y, int w, int h, u16 col_light, u16 col_dark);
INLINE void DrawFrame(int x, int y, int w, int h, u16 col) { DrawFrame3D(x, y, w, h, col, col); }

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCanFrameInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void DrawFrameInv_Fast(int x, int y, int w, int h);

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCanFrameInv(sDrawCan* can, int x, int y, int w, int h);
void DrawFrameInv(int x, int y, int w, int h);

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCanFrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void DrawFrameW_Fast(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCanFrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void DrawFrameW(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCanFrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick);
void DrawFrameWInv_Fast(int x, int y, int w, int h, int thick);

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCanFrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick);
void DrawFrameWInv(int x, int y, int w, int h, int thick);

// For backward compatibility with Draw version 1
INLINE void DrawFrameInvW(int x, int y, int w, int h, int thick) { DrawFrameWInv(x, y, w, h, thick); }

// ----- Draw line

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCanLineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void DrawLineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (does not check clipping limits)
void DrawCanLine_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void DrawLine_Fast(int x1, int y1, int x2, int y2, u16 col);

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCanLineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void DrawLineOver(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (checks clipping limits)
void DrawCanLine(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void DrawLine(int x1, int y1, int x2, int y2, u16 col);

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCanLineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void DrawLineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (does not check clipping limits)
void DrawCanLineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2);
void DrawLineInv_Fast(int x1, int y1, int x2, int y2);

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCanLineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void DrawLineOverInv(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (checks clipping limits)
void DrawCanLineInv(sDrawCan* can, int x1, int y1, int x2, int y2);
void DrawLineInv(int x1, int y1, int x2, int y2);

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCanLineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void DrawLineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCanLineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void DrawLineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCanLineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void DrawLineWInv_Fast(int x1, int y1, int x2, int y2, int thick);

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCanLineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void DrawLineWInv(int x1, int y1, int x2, int y2, int thick);

// For backward compatibility with Draw version 1
INLINE void DrawLineInvW(int x1, int y1, int x2, int y2, int w, Bool round) { DrawLineWInv(x1, y1, x2, y2, w); }

// ----- Draw round (filled circle)

//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCanRound_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void DrawRound_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw round (checks clipping limits)
void DrawCanRound(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void DrawRound(int x, int y, int d, u16 col, u8 mask);

// For backward compatibility with Draw version 1
// Draw filled circle
INLINE void DrawFillCircle(int x0, int y0, int r, u16 col) { DrawRound(x0, y0, r*2, col, DRAWCAN_ROUND_ALL); }

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCanRoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void DrawRoundInv_Fast(int x, int y, int d, u8 mask);

// Draw round inverted (checks clipping limits)
void DrawCanRoundInv(sDrawCan* can, int x, int y, int d, u8 mask);
void DrawRoundInv(int x, int y, int d, u8 mask);

// For backward compatibility with Draw version 1
INLINE void DrawFillCircleInv(int x, int y, int r) { DrawRoundInv(x, y, r*2, DRAWCAN_ROUND_ALL); }

// ----- Draw circle
//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCanCircle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void DrawCircle_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc (checks clipping limits)
void DrawCanCircle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);

// For backward compatibility with Draw version 1
//void DrawCircle(int x, int y, int d, u16 col, u8 mask);
void DrawCircleArc(int x, int y, int d, u16 col, u8 mask);
INLINE void DrawCircle(int x, int y, int r, u16 col) { DrawCircleArc(x, y, r*2, col, DRAWCAN_CIRCLE_ALL); }

// Draw circle or arc inverted (does not check clipping limits)
void DrawCanCircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void DrawCircleInv_Fast(int x, int y, int d, u8 mask);

// Draw circle or arc inverted (checks clipping limits)
void DrawCanCircleInv(sDrawCan* can, int x, int y, int d, u8 mask);

// For backward compatibility with Draw version 1
//void DrawCircleInv(int x, int y, int d, u8 mask);
void DrawCircleArcInv(int x, int y, int d, u8 mask);
INLINE void DrawCircleInv(int x, int y, int r) { DrawCircleArcInv(x, y, r*2, DRAWCAN_CIRCLE_ALL); }

// ----- Draw ring
//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring

// Draw ring (does not check clipping limits)
void DrawCanRing_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void DrawRing_Fast(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring (checks clipping limits)
void DrawCanRing(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);

// For backward compatibility with Draw version 1
//void DrawRing(int x, int y, int d, int din, u16 col, u8 mask);
void DrawRingMask(int x, int y, int d, int din, u16 col, u8 mask);
INLINE void DrawRing(int x, int y, int rin, int rout, u16 col) { DrawRingMask(x, y, rout*2, rin*2, col, DRAWCAN_ROUND_ALL); }

// Draw ring inverted (does not check clipping limits)
void DrawCanRingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void DrawRingInv_Fast(int x, int y, int d, int din, u8 mask);

// Draw ring inverted (checks clipping limits)
void DrawCanRingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask);

// For backward compatibility with Draw version 1
//void DrawRingInv(int x, int y, int d, int din, u8 mask);
void DrawRingInvMask(int x, int y, int d, int din, u8 mask);
INLINE void DrawRingInv(int x, int y, int rin, int rout) { DrawRingInvMask(x, y, rout*2, rin*2, DRAWCAN_ROUND_ALL); }

// ----- Draw triangle

// Draw triangle (does not check clipping limits)
void DrawCanTriangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void DrawTriangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle (checks clipping limits)
void DrawCanTriangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle inverted (does not check clipping limits)
void DrawCanTriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void DrawTriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3);

// Draw triangle inverted (checks clipping limits)
void DrawCanTriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void DrawTriangleInv(int x1, int y1, int x2, int y2, int x3, int y3);

// ----- Draw fill area

// Draw fill area
void DrawCanFill(sDrawCan* can, int x, int y, u16 col);
void DrawFill(int x, int y, u16 col);

// ----- Draw character
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw character with transparent background (does not check clipping limits)
void DrawCanChar_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void DrawChar_Fast(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with transparent background (checks clipping limits)
void DrawCanChar(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);

// For backward compatibility with Draw version 1
//void DrawChar(char ch, int x, int y, u16 col, int scalex, int scaley);
void DrawCharSize(char ch, int x, int y, u16 col, int scalex, int scaley);
INLINE void DrawChar(char ch, int x, int y, u16 col) { DrawCharSize(ch, x, y, col, 1, 1); }
INLINE void DrawCharH(char ch, int x, int y, u16 col) { DrawCharSize(ch, x, y, col, 1, 2); }
INLINE void DrawCharW(char ch, int x, int y, u16 col) { DrawCharSize(ch, x, y, col, 2, 1); }
INLINE void DrawChar2(char ch, int x, int y, u16 col) { DrawCharSize(ch, x, y, col, 2, 2); }

// Draw character with background (does not check clipping limits)
void DrawCanCharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void DrawCharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character with background (checks clipping limits)
void DrawCanCharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// For backward compatibility with Draw version 1
//void DrawCharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void DrawCharBgSize(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
INLINE void DrawCharBg(char ch, int x, int y, u16 col, u16 bgcol) { DrawCharBgSize(ch, x, y, col, bgcol, 1, 1); }
INLINE void DrawCharBgH(char ch, int x, int y, u16 col, u16 bgcol) { DrawCharBgSize(ch, x, y, col, bgcol, 1, 2); }
INLINE void DrawCharBgW(char ch, int x, int y, u16 col, u16 bgcol) { DrawCharBgSize(ch, x, y, col, bgcol, 2, 1); }
INLINE void DrawCharBg2(char ch, int x, int y, u16 col, u16 bgcol) { DrawCharBgSize(ch, x, y, col, bgcol, 2, 2); }
INLINE void DrawCharBg4(char ch, int x, int y, u16 col, u16 bgcol) { DrawCharBgSize(ch, x, y, col, bgcol, 4, 4); }

// Draw character inverted (does not check clipping limits)
void DrawCanCharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void DrawCharInv_Fast(char ch, int x, int y, int scalex, int scaley);

// Draw character inverted (checks clipping limits)
void DrawCanCharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void DrawCharInv(char ch, int x, int y, int scalex, int scaley);

// ----- Draw text
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCanText_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void DrawText_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with transparent background (checks clipping limits)
void DrawCanText(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// For backward compatibility with Draw version 1
//void DrawText(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void DrawTextSize(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
INLINE void DrawText(const char* text, int x, int y, u16 col) { DrawTextSize(text, -1, x, y, col, 1, 1); }
INLINE void DrawTextH(const char* text, int x, int y, u16 col) { DrawTextSize(text, -1, x, y, col, 1, 2); }
INLINE void DrawTextW(const char* text, int x, int y, u16 col) { DrawTextSize(text, -1, x, y, col, 2, 1); }
INLINE void DrawText2(const char* text, int x, int y, u16 col) { DrawTextSize(text, -1, x, y, col, 2, 2); }

// Draw text with background (does not check clipping limits)
void DrawCanTextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void DrawTextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text with background (checks clipping limits)
void DrawCanTextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// For backward compatibility with Draw version 1
//void DrawTextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void DrawTextBgSize(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
INLINE void DrawTextBg(const char* text, int x, int y, u16 col, u16 bgcol) { DrawTextBgSize(text, -1, x, y, col, bgcol, 1, 1); }
INLINE void DrawTextBgH(const char* text, int x, int y, u16 col, u16 bgcol) { DrawTextBgSize(text, -1, x, y, col, bgcol, 1, 2); }
INLINE void DrawTextBgW(const char* text, int x, int y, u16 col, u16 bgcol) { DrawTextBgSize(text, -1, x, y, col, bgcol, 2, 1); }
INLINE void DrawTextBg2(const char* text, int x, int y, u16 col, u16 bgcol) { DrawTextBgSize(text, -1, x, y, col, bgcol, 2, 2); }
INLINE void DrawTextBg4(const char* text, int x, int y, u16 col, u16 bgcol) { DrawTextBgSize(text, -1, x, y, col, bgcol, 4, 4); }

// Draw text inverted (does not check clipping limits)
void DrawCanTextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void DrawTextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley);

// Draw text inverted (checks clipping limits)
void DrawCanTextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void DrawTextInv(const char* text, int len, int x, int y, int scalex, int scaley);

// ----- Draw ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCanEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void DrawEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse (checks clipping limits)
void DrawCanEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void DrawEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse inverted (does not check clipping limits)
void DrawCanEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void DrawEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw ellipse inverted (checks clipping limits)
void DrawCanEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void DrawEllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw filled ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse

// Draw filled ellipse (does not check clipping limits)
void DrawCanFillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void DrawFillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse (checks clipping limits)
void DrawCanFillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void DrawFillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCanFillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void DrawFillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw filled ellipse inverte (checks clipping limits)
void DrawCanFillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void DrawFillEllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw image
//  can ... destination canvas
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  src ... source image
//  xs ... source X coordinate
//  ys ... source Y coordinate
//  w ... source width to draw
//  h ... source height to draw
//  wbs ... pitch of source image (length of line in bytes)
//  col ... key transparent color

// Draw image with the same format as destination
void DrawCanImg(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// For backward compatibility with Draw version 1
//void DrawImg(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void DrawImg2(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
INLINE void DrawImg(const void* src, int xs, int ys, int xd, int yd, int w, int h, int ws) { DrawImg2(xd, yd, src, xs, ys, w, h, DrawPitch(ws)); }

// Draw image inverted with the same format as destination
void DrawCanImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void DrawImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw transparent image with the same format as destination
void DrawCanBlit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// For backward compatibility with Draw version 1
//void DrawBlit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void DrawBlit2(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
INLINE void DrawBlit(const void* src, int xs, int ys, int xd, int yd, int w, int h, int ws, u16 col) { DrawBlit2(xd, yd, src, xs, ys, w, h, DrawPitch(ws), col); }

// Draw transparent image inverted with the same format as destination
void DrawCanBlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void DrawBlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image with the same format as destination, with substitute color
void DrawCanBlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);

// For backward compatibility with Draw version 1
//void DrawBlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void DrawBlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
INLINE void DrawBlitSubst(const void* src, int xs, int ys, int xd, int yd, int w, int h, int ws, u16 col, u16 fnd, u16 subst) { DrawBlitSubs(xd, yd, src, xs, ys, w, h, DrawPitch(ws), col, fnd, subst); }

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
void DrawCanGetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);

// For backward compatibility with Draw version 1
//void DrawGetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void DrawGetImg2(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
INLINE void DrawGetImg(void* dst, int x, int y, int w, int h) { DrawGetImg2(x, y, w, h, dst, 0, 0, DrawPitch(w)); }

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
void DrawCanImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);

// For backward compatibility with Draw version 1
void DrawImgMat2D(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
INLINE void DrawImgMat(const void* src, int ws, int hs, int xd, int yd, int wd, int hd, const sMat2D* m, u8 mode, u16 color)
	{ DrawImgMat2D(xd, yd, wd, hd, src, ws, hs, DrawPitch(ws), m, mode, color); }

// ----- Colors

// convert RGB888 color to pixel color
u16 DrawCanColRgb(sDrawCan* can, u8 r, u8 g, u8 b);
u16 DrawColRgb(u8 r, u8 g, u8 b);
#define COL_RGB(r,g,b) DrawColRgb((r),(g),(b))

// random pixel color
u16 DrawCanColRand(sDrawCan* can);
u16 DrawColRand();
#define COL_RANDOM DrawColRand()

// base colors
u16 DrawCanColBlack(sDrawCan* can);	// black color
u16 DrawColBlack();
#define COL_BLACK DrawColBlack()

u16 DrawCanColBlue(sDrawCan* can);	// blue color
u16 DrawColBlue();
#define COL_BLUE DrawColBlue()

u16 DrawCanColGreen(sDrawCan* can);	// green color
u16 DrawColGreen();
#define COL_GREEN DrawColGreen()

u16 DrawCanColCyan(sDrawCan* can);	// cyan color
u16 DrawColCyan();
#define COL_CYAN DrawColCyan()

u16 DrawCanColRed(sDrawCan* can);	// red color
u16 DrawColRed();
#define COL_RED DrawColRed()

u16 DrawCanColMagenta(sDrawCan* can);	// magenta color
u16 DrawColMagenta();
#define COL_MAGENTA DrawColMagenta()

u16 DrawCanColYellow(sDrawCan* can);	// yellow color
u16 DrawColYellow();
#define COL_YELLOW DrawColYellow()

u16 DrawCanColWhite(sDrawCan* can);	// white color
u16 DrawColWhite();
#define COL_WHITE DrawColWhite()

u16 DrawCanColGray(sDrawCan* can);	// gray color
u16 DrawColGray();
#define COL_GRAY DrawColGray()

u16 DrawCanColDkBlue(sDrawCan* can);	// dark blue color
u16 DrawColDkBlue();
#define COL_DKBLUE DrawColDkBlue()

u16 DrawCanColDkGreen(sDrawCan* can);	// dark green color
u16 DrawColDkGreen();
#define COL_DKGREEN DrawColDkGreen()

u16 DrawCanColDkCyan(sDrawCan* can);	// dark cyan color
u16 DrawColDkCyan();
#define COL_DKCYAN DrawColDkCyan()

u16 DrawCanColDkRed(sDrawCan* can);	// dark red color
u16 DrawColDkRed();
#define COL_DKRED DrawColDkRed()

u16 DrawCanColDkMagenta(sDrawCan* can);	// dark magenta color
u16 DrawColDkMagenta();
#define COL_DKMAGENTA DrawColDkMagenta()

u16 DrawCanColDkYellow(sDrawCan* can);	// dark yellow color
u16 DrawColDkYellow();
#define COL_DKYELLOW DrawColDkYellow()

u16 DrawCanColDkWhite(sDrawCan* can);	// dark white color
u16 DrawColDkWhite();
#define COL_DKWHITE DrawColDkWhite()

u16 DrawCanColDkGray(sDrawCan* can);	// dark gray color
u16 DrawColDkGray();
#define COL_DKGRAY DrawColDkGray()

u16 DrawCanColLtBlue(sDrawCan* can);	// light blue color
u16 DrawColLtBlue();
#define COL_LTBLUE DrawColLtBlue()

u16 DrawCanColLtGreen(sDrawCan* can);	// light green color
u16 DrawColLtGreen();
#define COL_LTGREEN DrawColLtGreen()

u16 DrawCanColLtCyan(sDrawCan* can);	// light cyan color
u16 DrawColLtCyan();
#define COL_LTCYAN DrawColLtCyan()

u16 DrawCanColLtRed(sDrawCan* can);	// light red color
u16 DrawColLtRed();
#define COL_LTRED DrawColLtRed()

u16 DrawCanColLtMagenta(sDrawCan* can);	// light magenta color
u16 DrawColLtMagenta();
#define COL_LTMAGENTA DrawColLtMagenta()

u16 DrawCanColLtYellow(sDrawCan* can);	// light yellow color
u16 DrawColLtYellow();
#define COL_LTYELLOW DrawColLtYellow()

u16 DrawCanColLtGray(sDrawCan* can);	// light gray color
u16 DrawColLtGray();
#define COL_LTGRAY DrawColLtGray()

u16 DrawCanColAzure(sDrawCan* can);	// azure blue color
u16 DrawColAzure();
#define COL_AZURE DrawColAzure()

u16 DrawCanColOrange(sDrawCan* can);	// orange color
u16 DrawColOrange();
#define COL_ORANGE DrawColOrange()

u16 DrawCanColBrown(sDrawCan* can);	// brown color
u16 DrawColBrown();
#define COL_BROWN DrawColBrown()

#endif // USE_DRAWCAN0

#ifdef __cplusplus
}
#endif

#endif // _LIB_DRAWCAN_H

#endif // USE_DRAWCAN
