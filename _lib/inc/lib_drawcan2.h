
// ****************************************************************************
//
//                      Drawing to canvas at 2-bit format
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

#if USE_DRAWCAN	&& USE_DRAWCAN2		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#ifndef _LIB_DRAWCAN2_H
#define _LIB_DRAWCAN2_H

#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// default drawing canvas for 2-bit format
extern sDrawCan DrawCan2;

// current drawing canvas for 2-bit format
extern sDrawCan* pDrawCan2;

// set default drawing canvas for 2-bit format
void SetDrawCan2(sDrawCan* can);

// get current width, height and buffer of the drawing canvas of 2-bit format
INLINE int Draw2Width() { return pDrawCan2->w; }
INLINE int Draw2Height() { return pDrawCan2->h; }
INLINE u8* Draw2Buf() { return pDrawCan2->buf; }

// convert RGB888 color to 2-bit pixel color Y2
u16 Draw2ColRgb(u8 r, u8 g, u8 b);

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 2-bit pixel color Y2
u16 Draw2ColRand();
#endif

// 2-bit colors, format Y2 (components are 0..255, levels 0, 85, 170, 255)
#define COLOR2(r,g,b)	( ((r)+(g)+(b)) / 210 )

// 2-bit colors, format Y2 (value 0..3)
#define COL2_BLACK	0
#define COL2_DKGRAY	1
#define COL2_GRAY	2
#define COL2_WHITE	3
#define COL2_RANDOM	(RandU8()&3)

// color mode
#define COL2_BLUE	1
#define COL2_RED	2
#define COL2_YELLOW	3

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw2Pitch(int w);

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw2MaxWidth(int pitch);

// ----- Clear canvas

// Clear canvas with color
void DrawCan2ClearCol(sDrawCan* can, u16 col);
void Draw2ClearCol(u16 col);

// Clear canvas with black color
void DrawCan2Clear(sDrawCan* can);
void Draw2Clear();

// ----- Draw point

// Draw point (does not check clipping limits)
void DrawCan2Point_Fast(sDrawCan* can, int x, int y, u16 col);
void Draw2Point_Fast(int x, int y, u16 col);

// Draw point (checks clipping limits)
void DrawCan2Point(sDrawCan* can, int x, int y, u16 col);
void Draw2Point(int x, int y, u16 col);

// Draw point inverted (does not check clipping limits)
void DrawCan2PointInv_Fast(sDrawCan* can, int x, int y);
void Draw2PointInv_Fast(int x, int y);

// Draw point inverted (checks clipping limits)
void DrawCan2PointInv(sDrawCan* can, int x, int y);
void Draw2PointInv(int x, int y);

// ----- Get point

// Get point (does not check clipping limits)
u16 DrawCan2GetPoint_Fast(sDrawCan* can, int x, int y);
u16 Draw2GetPoint_Fast(int x, int y);

// Get point (checks clipping limits)
u16 DrawCan2GetPoint(sDrawCan* can, int x, int y);
u16 Draw2GetPoint(int x, int y);

// ----- Draw rectangle

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan2Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw2Rect_Fast(int x, int y, int w, int h, u16 col);

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan2Rect(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw2Rect(int x, int y, int w, int h, u16 col);

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan2RectInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw2RectInv_Fast(int x, int y, int w, int h);

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan2RectInv(sDrawCan* can, int x, int y, int w, int h);
void Draw2RectInv(int x, int y, int w, int h);

// ----- Draw horizontal line

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan2HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col);
void Draw2HLine_Fast(int x, int y, int w, u16 col);

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan2HLine(sDrawCan* can, int x, int y, int w, u16 col);
void Draw2HLine(int x, int y, int w, u16 col);

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan2HLineInv_Fast(sDrawCan* can, int x, int y, int w);
void Draw2HLineInv_Fast(int x, int y, int w);

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan2HLineInv(sDrawCan* can, int x, int y, int w);
void Draw2HLineInv(int x, int y, int w);

// ----- Draw vertical line

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan2VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col);
void Draw2VLine_Fast(int x, int y, int h, u16 col);

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan2VLine(sDrawCan* can, int x, int y, int h, u16 col);
void Draw2VLine(int x, int y, int h, u16 col);

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan2VLineInv_Fast(sDrawCan* can, int x, int y, int h);
void Draw2VLineInv_Fast(int x, int y, int h);

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan2VLineInv(sDrawCan* can, int x, int y, int h);
void Draw2VLineInv(int x, int y, int h);

// ----- Draw frame

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan2Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw2Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan2Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw2Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan2FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw2FrameInv_Fast(int x, int y, int w, int h);

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan2FrameInv(sDrawCan* can, int x, int y, int w, int h);
void Draw2FrameInv(int x, int y, int w, int h);

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan2FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw2FrameW_Fast(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan2FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw2FrameW(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan2FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw2FrameWInv_Fast(int x, int y, int w, int h, int thick);

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan2FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw2FrameWInv(int x, int y, int w, int h, int thick);

// ----- Draw line

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan2LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw2LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (does not check clipping limits)
void DrawCan2Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw2Line_Fast(int x1, int y1, int x2, int y2, u16 col);

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan2LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw2LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (checks clipping limits)
void DrawCan2Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw2Line(int x1, int y1, int x2, int y2, u16 col);

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan2LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw2LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (does not check clipping limits)
void DrawCan2LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw2LineInv_Fast(int x1, int y1, int x2, int y2);

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan2LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw2LineOverInv(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (checks clipping limits)
void DrawCan2LineInv(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw2LineInv(int x1, int y1, int x2, int y2);

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan2LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw2LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan2LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw2LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan2LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw2LineWInv_Fast(int x1, int y1, int x2, int y2, int thick);

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan2LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw2LineWInv(int x1, int y1, int x2, int y2, int thick);

// ----- Draw round (filled circle)

//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCan2Round_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw2Round_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw round (checks clipping limits)
void DrawCan2Round(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw2Round(int x, int y, int d, u16 col, u8 mask);

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan2RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw2RoundInv_Fast(int x, int y, int d, u8 mask);

// Draw round inverted (checks clipping limits)
void DrawCan2RoundInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw2RoundInv(int x, int y, int d, u8 mask);

// ----- Draw circle
//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan2Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw2Circle_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc (checks clipping limits)
void DrawCan2Circle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw2Circle(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan2CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw2CircleInv_Fast(int x, int y, int d, u8 mask);

// Draw circle or arc inverted (checks clipping limits)
void DrawCan2CircleInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw2CircleInv(int x, int y, int d, u8 mask);

// ----- Draw ring
//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring

// Draw ring (does not check clipping limits)
void DrawCan2Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw2Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring (checks clipping limits)
void DrawCan2Ring(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw2Ring(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring inverted (does not check clipping limits)
void DrawCan2RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw2RingInv_Fast(int x, int y, int d, int din, u8 mask);

// Draw ring inverted (checks clipping limits)
void DrawCan2RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw2RingInv(int x, int y, int d, int din, u8 mask);

// ----- Draw triangle

// Draw triangle (does not check clipping limits)
void DrawCan2Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw2Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle (checks clipping limits)
void DrawCan2Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw2Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle inverted (does not check clipping limits)
void DrawCan2TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw2TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3);

// Draw triangle inverted (checks clipping limits)
void DrawCan2TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw2TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3);

// ----- Draw fill area

// Draw fill area
void DrawCan2Fill(sDrawCan* can, int x, int y, u16 col);
void Draw2Fill(int x, int y, u16 col);

// ----- Draw character
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw2SelFont(const u8* font, u8 fontw, u8 fonth);

void Draw2SelFont8x8();		// sans-serif bold, height 8 lines
void Draw2SelFont8x14();	// sans-serif bold, height 14 lines
void Draw2SelFont8x16();	// sans-serif bold, height 16 lines
void Draw2SelFont8x14Serif();	// serif bold, height 14 lines
void Draw2SelFont8x16Serif();	// serif bold, height 16 lines
void Draw2SelFont6x8();		// condensed font, width 6 pixels, height 8 lines
void Draw2SelFont8x8Game();	// game font, height 8 lines
void Draw2SelFont8x8Ibm();	// IBM charset font, height 8 lines
void Draw2SelFont8x14Ibm();	// IBM charset font, height 14 lines
void Draw2SelFont8x16Ibm();	// IBM charset font, height 16 lines
void Draw2SelFont8x8IbmThin();	// IBM charset thin font, height 8 lines
void Draw2SelFont8x8Italic();	// italic, height 8 lines
void Draw2SelFont8x8Thin();	// thin font, height 8 lines
void Draw2SelFont5x8();		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan2Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw2Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with transparent background (checks clipping limits)
void DrawCan2Char(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw2Char(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with background (does not check clipping limits)
void DrawCan2CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw2CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character with background (checks clipping limits)
void DrawCan2CharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw2CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character inverted (does not check clipping limits)
void DrawCan2CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw2CharInv_Fast(char ch, int x, int y, int scalex, int scaley);

// Draw character inverted (checks clipping limits)
void DrawCan2CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw2CharInv(char ch, int x, int y, int scalex, int scaley);

// ----- Draw text
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan2Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw2Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with transparent background (checks clipping limits)
void DrawCan2Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw2Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with background (does not check clipping limits)
void DrawCan2TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw2TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text with background (checks clipping limits)
void DrawCan2TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw2TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text inverted (does not check clipping limits)
void DrawCan2TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw2TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley);

// Draw text inverted (checks clipping limits)
void DrawCan2TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw2TextInv(const char* text, int len, int x, int y, int scalex, int scaley);

// ----- Draw ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan2Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw2Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse (checks clipping limits)
void DrawCan2Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw2Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse inverted (does not check clipping limits)
void DrawCan2EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw2EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw ellipse inverted (checks clipping limits)
void DrawCan2EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw2EllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw filled ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse

// Draw filled ellipse (does not check clipping limits)
void DrawCan2FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw2FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse (checks clipping limits)
void DrawCan2FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw2FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan2FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw2FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan2FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw2FillEllipseInv(int x, int y, int dx, int dy, u8 mask);

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
void DrawCan2Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw2Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw image inverted with the same format as destination
void DrawCan2ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw2ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw transparent image with the same format as destination
void DrawCan2Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw2Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image inverted with the same format as destination
void DrawCan2BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw2BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan2BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void Draw2BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);

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
void DrawCan2GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void Draw2GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);

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
void DrawCan2ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
void Draw2ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
// drawing function interface
extern const sDrawCanFnc DrawCan2Fnc;
#endif

#ifdef __cplusplus
}
#endif

#endif // _LIB_DRAWCAN2_H

#endif // USE_DRAWCAN
