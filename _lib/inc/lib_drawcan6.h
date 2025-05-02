
// ****************************************************************************
//
//                     Drawing to canvas at 6-bit format
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

#if USE_DRAWCAN	&& USE_DRAWCAN6		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#ifndef _LIB_DRAWCAN6_H
#define _LIB_DRAWCAN6_H

#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// default drawing canvas for 6-bit format
extern sDrawCan DrawCan6;

// current drawing canvas for 6-bit format
extern sDrawCan* pDrawCan6;

// set default drawing canvas for 6-bit format
void SetDrawCan6(sDrawCan* can);

// get current width, height and buffer of the drawing canvas of 6-bit format
INLINE int Draw6Width() { return pDrawCan6->w; }
INLINE int Draw6Height() { return pDrawCan6->h; }
INLINE u8* Draw6Buf() { return pDrawCan6->buf; }

// convert RGB888 color to 6-bit pixel color RGB222
u16 Draw6ColRgb(u8 r, u8 g, u8 b);

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 6-bit pixel color RGB222
u16 Draw6ColRand();
#endif

// 6-bit colors, format RGB222 (components are 0..255)
#define COLOR6(r,g,b)	((u8)( (((r)&0xc0)>>2) | (((g)&0xc0)>>4) | (((b)&0xc0)>>6) ))
// - base colors
#define COL6_BLACK	COLOR6(  0,  0,  0)
#define COL6_BLUE	COLOR6(  0,  0,255)
#define COL6_GREEN	COLOR6(  0,255,  0)
#define COL6_CYAN	COLOR6(  0,255,255)
#define COL6_RED	COLOR6(255,  0,  0)
#define COL6_MAGENTA	COLOR6(255,  0,255)
#define COL6_YELLOW	COLOR6(255,255,  0)
#define COL6_WHITE	COLOR6(255,255,255)
#define COL6_GRAY	COLOR6(128,128,128)
// - dark colors
#define COL6_DKBLUE	COLOR6(  0,  0,127)
#define COL6_DKGREEN	COLOR6(  0,127,  0)
#define COL6_DKCYAN	COLOR6(  0,127,127)
#define COL6_DKRED	COLOR6(127,  0,  0)
#define COL6_DKMAGENTA	COLOR6(127,  0,127)
#define COL6_DKYELLOW	COLOR6(127,127,  0)
#define COL6_DKWHITE	COLOR6(127,127,127)
#define COL6_DKGRAY	COLOR6( 64, 64, 64)
// - light colors
#define COL6_LTBLUE	COLOR6(127,127,255)
#define COL6_LTGREEN	COLOR6(127,255,127)
#define COL6_LTCYAN	COLOR6(127,255,255)
#define COL6_LTRED	COLOR6(255,127,127)
#define COL6_LTMAGENTA	COLOR6(255,127,255)
#define COL6_LTYELLOW	COLOR6(255,255,127)
#define COL6_LTGRAY	COLOR6(192,192,192)
// - special colors
#define COL6_AZURE	COLOR6(  0,127,255)
#define COL6_ORANGE	COLOR6(255,127,  0)
#define COL6_BROWN	COLOR6(192, 96,  0)

#define COL6_RANDOM	(RandU8()&0x3f)

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw6Pitch(int w);

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw6MaxWidth(int pitch);

// ----- Clear canvas

// Clear canvas with color
void DrawCan6ClearCol(sDrawCan* can, u16 col);
void Draw6ClearCol(u16 col);

// Clear canvas with black color
void DrawCan6Clear(sDrawCan* can);
void Draw6Clear();

// ----- Draw point

// Draw point (does not check clipping limits)
void DrawCan6Point_Fast(sDrawCan* can, int x, int y, u16 col);
void Draw6Point_Fast(int x, int y, u16 col);

// Draw point (checks clipping limits)
void DrawCan6Point(sDrawCan* can, int x, int y, u16 col);
void Draw6Point(int x, int y, u16 col);

// Draw point inverted (does not check clipping limits)
void DrawCan6PointInv_Fast(sDrawCan* can, int x, int y);
void Draw6PointInv_Fast(int x, int y);

// Draw point inverted (checks clipping limits)
void DrawCan6PointInv(sDrawCan* can, int x, int y);
void Draw6PointInv(int x, int y);

// ----- Get point

// Get point (does not check clipping limits)
u16 DrawCan6GetPoint_Fast(sDrawCan* can, int x, int y);
u16 Draw6GetPoint_Fast(int x, int y);

// Get point (checks clipping limits)
u16 DrawCan6GetPoint(sDrawCan* can, int x, int y);
u16 Draw6GetPoint(int x, int y);

// ----- Draw rectangle

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan6Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw6Rect_Fast(int x, int y, int w, int h, u16 col);

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan6Rect(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw6Rect(int x, int y, int w, int h, u16 col);

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan6RectInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw6RectInv_Fast(int x, int y, int w, int h);

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan6RectInv(sDrawCan* can, int x, int y, int w, int h);
void Draw6RectInv(int x, int y, int w, int h);

// ----- Draw horizontal line

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan6HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col);
void Draw6HLine_Fast(int x, int y, int w, u16 col);

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan6HLine(sDrawCan* can, int x, int y, int w, u16 col);
void Draw6HLine(int x, int y, int w, u16 col);

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan6HLineInv_Fast(sDrawCan* can, int x, int y, int w);
void Draw6HLineInv_Fast(int x, int y, int w);

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan6HLineInv(sDrawCan* can, int x, int y, int w);
void Draw6HLineInv(int x, int y, int w);

// ----- Draw vertical line

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan6VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col);
void Draw6VLine_Fast(int x, int y, int h, u16 col);

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan6VLine(sDrawCan* can, int x, int y, int h, u16 col);
void Draw6VLine(int x, int y, int h, u16 col);

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan6VLineInv_Fast(sDrawCan* can, int x, int y, int h);
void Draw6VLineInv_Fast(int x, int y, int h);

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan6VLineInv(sDrawCan* can, int x, int y, int h);
void Draw6VLineInv(int x, int y, int h);

// ----- Draw frame

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan6Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw6Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan6Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw6Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan6FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw6FrameInv_Fast(int x, int y, int w, int h);

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan6FrameInv(sDrawCan* can, int x, int y, int w, int h);
void Draw6FrameInv(int x, int y, int w, int h);

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan6FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw6FrameW_Fast(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan6FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw6FrameW(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan6FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw6FrameWInv_Fast(int x, int y, int w, int h, int thick);

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan6FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw6FrameWInv(int x, int y, int w, int h, int thick);

// ----- Draw line

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan6LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw6LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (does not check clipping limits)
void DrawCan6Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw6Line_Fast(int x1, int y1, int x2, int y2, u16 col);

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan6LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw6LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (checks clipping limits)
void DrawCan6Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw6Line(int x1, int y1, int x2, int y2, u16 col);

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan6LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw6LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (does not check clipping limits)
void DrawCan6LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw6LineInv_Fast(int x1, int y1, int x2, int y2);

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan6LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw6LineOverInv(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (checks clipping limits)
void DrawCan6LineInv(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw6LineInv(int x1, int y1, int x2, int y2);

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan6LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw6LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan6LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw6LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan6LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw6LineWInv_Fast(int x1, int y1, int x2, int y2, int thick);

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan6LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw6LineWInv(int x1, int y1, int x2, int y2, int thick);

// ----- Draw round (filled circle)

//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCan6Round_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw6Round_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw round (checks clipping limits)
void DrawCan6Round(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw6Round(int x, int y, int d, u16 col, u8 mask);

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan6RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw6RoundInv_Fast(int x, int y, int d, u8 mask);

// Draw round inverted (checks clipping limits)
void DrawCan6RoundInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw6RoundInv(int x, int y, int d, u8 mask);

// ----- Draw circle
//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan6Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw6Circle_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc (checks clipping limits)
void DrawCan6Circle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw6Circle(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan6CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw6CircleInv_Fast(int x, int y, int d, u8 mask);

// Draw circle or arc inverted (checks clipping limits)
void DrawCan6CircleInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw6CircleInv(int x, int y, int d, u8 mask);

// ----- Draw ring
//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring

// Draw ring (does not check clipping limits)
void DrawCan6Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw6Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring (checks clipping limits)
void DrawCan6Ring(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw6Ring(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring inverted (does not check clipping limits)
void DrawCan6RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw6RingInv_Fast(int x, int y, int d, int din, u8 mask);

// Draw ring inverted (checks clipping limits)
void DrawCan6RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw6RingInv(int x, int y, int d, int din, u8 mask);

// ----- Draw triangle

// Draw triangle (does not check clipping limits)
void DrawCan6Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw6Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle (checks clipping limits)
void DrawCan6Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw6Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle inverted (does not check clipping limits)
void DrawCan6TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw6TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3);

// Draw triangle inverted (checks clipping limits)
void DrawCan6TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw6TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3);

// ----- Draw fill area

// Draw fill area
void DrawCan6Fill(sDrawCan* can, int x, int y, u16 col);
void Draw6Fill(int x, int y, u16 col);

// ----- Draw character
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw6SelFont(const u8* font, u8 fontw, u8 fonth);

void Draw6SelFont8x8();		// sans-serif bold, height 8 lines
void Draw6SelFont8x14();	// sans-serif bold, height 14 lines
void Draw6SelFont8x16();	// sans-serif bold, height 16 lines
void Draw6SelFont8x14Serif();	// serif bold, height 14 lines
void Draw6SelFont8x16Serif();	// serif bold, height 16 lines
void Draw6SelFont6x8();		// condensed font, width 6 pixels, height 8 lines
void Draw6SelFont8x8Game();	// game font, height 8 lines
void Draw6SelFont8x8Ibm();	// IBM charset font, height 8 lines
void Draw6SelFont8x14Ibm();	// IBM charset font, height 14 lines
void Draw6SelFont8x16Ibm();	// IBM charset font, height 16 lines
void Draw6SelFont8x8IbmThin();	// IBM charset thin font, height 8 lines
void Draw6SelFont8x8Italic();	// italic, height 8 lines
void Draw6SelFont8x8Thin();	// thin font, height 8 lines
void Draw6SelFont5x8();		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan6Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw6Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with transparent background (checks clipping limits)
void DrawCan6Char(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw6Char(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with background (does not check clipping limits)
void DrawCan6CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw6CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character with background (checks clipping limits)
void DrawCan6CharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw6CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character inverted (does not check clipping limits)
void DrawCan6CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw6CharInv_Fast(char ch, int x, int y, int scalex, int scaley);

// Draw character inverted (checks clipping limits)
void DrawCan6CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw6CharInv(char ch, int x, int y, int scalex, int scaley);

// ----- Draw text
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan6Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw6Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with transparent background (checks clipping limits)
void DrawCan6Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw6Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with background (does not check clipping limits)
void DrawCan6TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw6TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text with background (checks clipping limits)
void DrawCan6TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw6TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text inverted (does not check clipping limits)
void DrawCan6TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw6TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley);

// Draw text inverted (checks clipping limits)
void DrawCan6TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw6TextInv(const char* text, int len, int x, int y, int scalex, int scaley);

// ----- Draw ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan6Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw6Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse (checks clipping limits)
void DrawCan6Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw6Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse inverted (does not check clipping limits)
void DrawCan6EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw6EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw ellipse inverted (checks clipping limits)
void DrawCan6EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw6EllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw filled ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse

// Draw filled ellipse (does not check clipping limits)
void DrawCan6FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw6FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse (checks clipping limits)
void DrawCan6FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw6FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan6FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw6FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan6FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw6FillEllipseInv(int x, int y, int dx, int dy, u8 mask);

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
void DrawCan6Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw6Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw image inverted with the same format as destination
void DrawCan6ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw6ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw transparent image with the same format as destination
void DrawCan6Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw6Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image inverted with the same format as destination
void DrawCan6BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw6BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan6BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void Draw6BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);

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
void DrawCan6GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void Draw6GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);

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
void DrawCan6ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
void Draw6ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
// drawing function interface
extern const sDrawCanFnc DrawCan6Fnc;
#endif

#ifdef __cplusplus
}
#endif

#endif // _LIB_DRAWCAN6_H

#endif // USE_DRAWCAN
