
// ****************************************************************************
//
//                     Drawing to canvas at 8-bit format
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

#if USE_DRAWCAN && USE_DRAWCAN8		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#ifndef _LIB_DRAWCAN8_H
#define _LIB_DRAWCAN8_H

#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// default drawing canvas for 8-bit format
extern sDrawCan DrawCan8;

// current drawing canvas for 8-bit format
extern sDrawCan* pDrawCan8;

// set default drawing canvas for 8-bit format
void SetDrawCan8(sDrawCan* can);

// get current width, height and buffer of the drawing canvas of 8-bit format
INLINE int Draw8Width() { return pDrawCan8->w; }
INLINE int Draw8Height() { return pDrawCan8->h; }
INLINE u8* Draw8Buf() { return pDrawCan8->buf; }

// convert RGB888 color to 8-bit pixel color RGB332
u16 Draw8ColRgb(u8 r, u8 g, u8 b);

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 8-bit pixel color RGB332
u16 Draw8ColRand();
#endif

// 8-bit colors, format RGB332 (components are 0..255)
#define COLOR8(r,g,b)	((u8)( ((r)&0xe0) | (((g)&0xe0)>>3) | (((b)&0xc0)>>6) ))
// - base colors
#define COL8_BLACK	COLOR8(  0,  0,  0)
#define COL8_BLUE	COLOR8(  0,  0,255)
#define COL8_GREEN	COLOR8(  0,255,  0)
#define COL8_CYAN	COLOR8(  0,255,255)
#define COL8_RED	COLOR8(255,  0,  0)
#define COL8_MAGENTA	COLOR8(255,  0,255)
#define COL8_YELLOW	COLOR8(255,255,  0)
#define COL8_WHITE	COLOR8(255,255,255)
#define COL8_GRAY	COLOR8(128,128,128)
// - dark colors
#define COL8_DKBLUE	COLOR8(  0,  0,127)
#define COL8_DKGREEN	COLOR8(  0,127,  0)
#define COL8_DKCYAN	COLOR8(  0,127,127)
#define COL8_DKRED	COLOR8(127,  0,  0)
#define COL8_DKMAGENTA	COLOR8(127,  0,127)
#define COL8_DKYELLOW	COLOR8(127,127,  0)
#define COL8_DKWHITE	COLOR8(127,127,127)
#define COL8_DKGRAY	COLOR8( 64, 64, 64)
// - light colors
#define COL8_LTBLUE	COLOR8(127,127,255)
#define COL8_LTGREEN	COLOR8(127,255,127)
#define COL8_LTCYAN	COLOR8(127,255,255)
#define COL8_LTRED	COLOR8(255,127,127)
#define COL8_LTMAGENTA	COLOR8(255,127,255)
#define COL8_LTYELLOW	COLOR8(255,255,127)
#define COL8_LTGRAY	COLOR8(192,192,192)
// - special colors
#define COL8_AZURE	COLOR8(  0,127,255)
#define COL8_ORANGE	COLOR8(255,127,  0)
#define COL8_BROWN	COLOR8(192, 96,  0)

#define COL8_RANDOM	(RandU8())

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw8Pitch(int w);

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw8MaxWidth(int pitch);

// ----- Clear canvas

// Clear canvas with color
void DrawCan8ClearCol(sDrawCan* can, u16 col);
void Draw8ClearCol(u16 col);

// Clear canvas with black color
void DrawCan8Clear(sDrawCan* can);
void Draw8Clear();

// ----- Draw point

// Draw point (does not check clipping limits)
void DrawCan8Point_Fast(sDrawCan* can, int x, int y, u16 col);
void Draw8Point_Fast(int x, int y, u16 col);

// Draw point (checks clipping limits)
void DrawCan8Point(sDrawCan* can, int x, int y, u16 col);
void Draw8Point(int x, int y, u16 col);

// Draw point inverted (does not check clipping limits)
void DrawCan8PointInv_Fast(sDrawCan* can, int x, int y);
void Draw8PointInv_Fast(int x, int y);

// Draw point inverted (checks clipping limits)
void DrawCan8PointInv(sDrawCan* can, int x, int y);
void Draw8PointInv(int x, int y);

// ----- Get point

// Get point (does not check clipping limits)
u16 DrawCan8GetPoint_Fast(sDrawCan* can, int x, int y);
u16 Draw8GetPoint_Fast(int x, int y);

// Get point (checks clipping limits)
u16 DrawCan8GetPoint(sDrawCan* can, int x, int y);
u16 Draw8GetPoint(int x, int y);

// ----- Draw rectangle

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan8Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw8Rect_Fast(int x, int y, int w, int h, u16 col);

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan8Rect(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw8Rect(int x, int y, int w, int h, u16 col);

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan8RectInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw8RectInv_Fast(int x, int y, int w, int h);

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan8RectInv(sDrawCan* can, int x, int y, int w, int h);
void Draw8RectInv(int x, int y, int w, int h);

// ----- Draw horizontal line

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan8HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col);
void Draw8HLine_Fast(int x, int y, int w, u16 col);

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan8HLine(sDrawCan* can, int x, int y, int w, u16 col);
void Draw8HLine(int x, int y, int w, u16 col);

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan8HLineInv_Fast(sDrawCan* can, int x, int y, int w);
void Draw8HLineInv_Fast(int x, int y, int w);

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan8HLineInv(sDrawCan* can, int x, int y, int w);
void Draw8HLineInv(int x, int y, int w);

// ----- Draw vertical line

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan8VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col);
void Draw8VLine_Fast(int x, int y, int h, u16 col);

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan8VLine(sDrawCan* can, int x, int y, int h, u16 col);
void Draw8VLine(int x, int y, int h, u16 col);

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan8VLineInv_Fast(sDrawCan* can, int x, int y, int h);
void Draw8VLineInv_Fast(int x, int y, int h);

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan8VLineInv(sDrawCan* can, int x, int y, int h);
void Draw8VLineInv(int x, int y, int h);

// ----- Draw frame

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan8Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw8Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan8Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw8Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan8FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw8FrameInv_Fast(int x, int y, int w, int h);

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan8FrameInv(sDrawCan* can, int x, int y, int w, int h);
void Draw8FrameInv(int x, int y, int w, int h);

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan8FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw8FrameW_Fast(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan8FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw8FrameW(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan8FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw8FrameWInv_Fast(int x, int y, int w, int h, int thick);

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan8FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw8FrameWInv(int x, int y, int w, int h, int thick);

// ----- Draw line

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan8LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw8LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (does not check clipping limits)
void DrawCan8Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw8Line_Fast(int x1, int y1, int x2, int y2, u16 col);

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan8LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw8LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (checks clipping limits)
void DrawCan8Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw8Line(int x1, int y1, int x2, int y2, u16 col);

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan8LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw8LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (does not check clipping limits)
void DrawCan8LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw8LineInv_Fast(int x1, int y1, int x2, int y2);

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan8LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw8LineOverInv(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (checks clipping limits)
void DrawCan8LineInv(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw8LineInv(int x1, int y1, int x2, int y2);

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan8LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw8LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan8LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw8LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan8LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw8LineWInv_Fast(int x1, int y1, int x2, int y2, int thick);

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan8LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw8LineWInv(int x1, int y1, int x2, int y2, int thick);

// ----- Draw round (filled circle)

//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCan8Round_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw8Round_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw round (checks clipping limits; diameter must be > 0)
void DrawCan8Round(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw8Round(int x, int y, int d, u16 col, u8 mask);

// Draw round inverted (does not check clipping limits)
void DrawCan8RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw8RoundInv_Fast(int x, int y, int d, u8 mask);

// Draw round inverted (checks clipping limits)
void DrawCan8RoundInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw8RoundInv(int x, int y, int d, u8 mask);

// ----- Draw circle
//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan8Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw8Circle_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc (checks clipping limits)
void DrawCan8Circle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw8Circle(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan8CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw8CircleInv_Fast(int x, int y, int d, u8 mask);

// Draw circle or arc inverted (checks clipping limits)
void DrawCan8CircleInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw8CircleInv(int x, int y, int d, u8 mask);

// ----- Draw ring
//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring

// Draw ring (does not check clipping limits)
void DrawCan8Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw8Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring (checks clipping limits)
void DrawCan8Ring(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw8Ring(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring inverted (does not check clipping limits)
void DrawCan8RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw8RingInv_Fast(int x, int y, int d, int din, u8 mask);

// Draw ring inverted (checks clipping limits)
void DrawCan8RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw8RingInv(int x, int y, int d, int din, u8 mask);

// ----- Draw triangle

// Draw triangle (does not check clipping limits)
void DrawCan8Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw8Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle (checks clipping limits)
void DrawCan8Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw8Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle inverted (does not check clipping limits)
void DrawCan8TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw8TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3);

// Draw triangle inverted (checks clipping limits)
void DrawCan8TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw8TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3);

// ----- Draw fill area

// Draw fill area
void DrawCan8Fill(sDrawCan* can, int x, int y, u16 col);
void Draw8Fill(int x, int y, u16 col);

// ----- Draw character
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw8SelFont(const u8* font, u8 fontw, u8 fonth);

void Draw8SelFont8x8();		// sans-serif bold, height 8 lines
void Draw8SelFont8x14();	// sans-serif bold, height 14 lines
void Draw8SelFont8x16();	// sans-serif bold, height 16 lines
void Draw8SelFont8x14Serif();	// serif bold, height 14 lines
void Draw8SelFont8x16Serif();	// serif bold, height 16 lines
void Draw8SelFont6x8();		// condensed font, width 6 pixels, height 8 lines
void Draw8SelFont8x8Game();	// game font, height 8 lines
void Draw8SelFont8x8Ibm();	// IBM charset font, height 8 lines
void Draw8SelFont8x14Ibm();	// IBM charset font, height 14 lines
void Draw8SelFont8x16Ibm();	// IBM charset font, height 16 lines
void Draw8SelFont8x8IbmThin();	// IBM charset thin font, height 8 lines
void Draw8SelFont8x8Italic();	// italic, height 8 lines
void Draw8SelFont8x8Thin();	// thin font, height 8 lines
void Draw8SelFont5x8();		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan8Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw8Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with transparent background (checks clipping limits)
void DrawCan8Char(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw8Char(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with background (does not check clipping limits)
void DrawCan8CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw8CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character with background (checks clipping limits)
void DrawCan8CharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw8CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character inverted (does not check clipping limits)
void DrawCan8CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw8CharInv_Fast(char ch, int x, int y, int scalex, int scaley);

// Draw character inverted (checks clipping limits)
void DrawCan8CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw8CharInv(char ch, int x, int y, int scalex, int scaley);

// ----- Draw text
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan8Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw8Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with transparent background (checks clipping limits)
void DrawCan8Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw8Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with background (does not check clipping limits)
void DrawCan8TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw8TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text with background (checks clipping limits)
void DrawCan8TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw8TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text inverted (does not check clipping limits)
void DrawCan8TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw8TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley);

// Draw text inverted (checks clipping limits)
void DrawCan8TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw8TextInv(const char* text, int len, int x, int y, int scalex, int scaley);

// ----- Draw ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan8Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw8Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse (checks clipping limits)
void DrawCan8Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw8Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse inverted (does not check clipping limits)
void DrawCan8EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw8EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw ellipse inverted (checks clipping limits)
void DrawCan8EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw8EllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw filled ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse

// Draw filled ellipse (does not check clipping limits)
void DrawCan8FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw8FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse (checks clipping limits)
void DrawCan8FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw8FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan8FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw8FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan8FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw8FillEllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw image
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
void DrawCan8Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw8Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw image inverted with the same format as destination
void DrawCan8ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw8ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw transparent image with the same format as destination
void DrawCan8Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw8Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image inverted with the same format as destination
void DrawCan8BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw8BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan8BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void Draw8BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);

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
void DrawCan8GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void Draw8GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);

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
void DrawCan8ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
void Draw8ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
// drawing function interface
extern const sDrawCanFnc DrawCan8Fnc;
#endif

#ifdef __cplusplus
}
#endif

#endif // _LIB_DRAWCAN8_H

#endif // USE_DRAWCAN
