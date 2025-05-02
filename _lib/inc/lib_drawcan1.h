
// ****************************************************************************
//
//                      Drawing to canvas at 1-bit format
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

#if USE_DRAWCAN && USE_DRAWCAN1		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#ifndef _LIB_DRAWCAN1_H
#define _LIB_DRAWCAN1_H

#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// default drawing canvas for 1-bit format
extern sDrawCan DrawCan1;

// current drawing canvas for 1-bit format
extern sDrawCan* pDrawCan1;

// set default drawing canvas for 1-bit format
void SetDrawCan1(sDrawCan* can);

// get current width, height and buffer of the drawing canvas of 1-bit format
INLINE int Draw1Width() { return pDrawCan1->w; }
INLINE int Draw1Height() { return pDrawCan1->h; }
INLINE u8* Draw1Buf() { return pDrawCan1->buf; }

// convert RGB888 color to 1-bit pixel color Y1
u16 Draw1ColRgb(u8 r, u8 g, u8 b);

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 1-bit pixel color Y1
u16 Draw1ColRand();
#endif

// 1-bit colors, format Y1 (components are 0..255, levels 0, 255)
#define COLOR1(r,g,b)	( ((r)+(g)+(b)) / 384 )

// 1-bit colors, format Y1 (value 0..1)
#define COL1_BLACK	0
#define COL1_WHITE	1
#define COL1_RANDOM	(RandU8()&1)

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw1Pitch(int w);

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw1MaxWidth(int pitch);

// ----- Clear canvas

// Clear canvas with color
void DrawCan1ClearCol(sDrawCan* can, u16 col);
void Draw1ClearCol(u16 col);

// Clear canvas with black color
void DrawCan1Clear(sDrawCan* can);
void Draw1Clear();

// ----- Draw point

// Draw point (does not check clipping limits)
void DrawCan1Point_Fast(sDrawCan* can, int x, int y, u16 col);
void Draw1Point_Fast(int x, int y, u16 col);

// Draw point (checks clipping limits)
void DrawCan1Point(sDrawCan* can, int x, int y, u16 col);
void Draw1Point(int x, int y, u16 col);

// Draw point inverted (does not check clipping limits)
void DrawCan1PointInv_Fast(sDrawCan* can, int x, int y);
void Draw1PointInv_Fast(int x, int y);

// Draw point inverted (checks clipping limits)
void DrawCan1PointInv(sDrawCan* can, int x, int y);
void Draw1PointInv(int x, int y);

// ----- Get point

// Get point (does not check clipping limits)
u16 DrawCan1GetPoint_Fast(sDrawCan* can, int x, int y);
u16 Draw1GetPoint_Fast(int x, int y);

// Get point (checks clipping limits)
u16 DrawCan1GetPoint(sDrawCan* can, int x, int y);
u16 Draw1GetPoint(int x, int y);

// ----- Draw rectangle

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan1Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw1Rect_Fast(int x, int y, int w, int h, u16 col);

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan1Rect(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw1Rect(int x, int y, int w, int h, u16 col);

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan1RectInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw1RectInv_Fast(int x, int y, int w, int h);

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan1RectInv(sDrawCan* can, int x, int y, int w, int h);
void Draw1RectInv(int x, int y, int w, int h);

// ----- Draw horizontal line

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan1HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col);
void Draw1HLine_Fast(int x, int y, int w, u16 col);

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan1HLine(sDrawCan* can, int x, int y, int w, u16 col);
void Draw1HLine(int x, int y, int w, u16 col);

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan1HLineInv_Fast(sDrawCan* can, int x, int y, int w);
void Draw1HLineInv_Fast(int x, int y, int w);

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan1HLineInv(sDrawCan* can, int x, int y, int w);
void Draw1HLineInv(int x, int y, int w);

// ----- Draw vertical line

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan1VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col);
void Draw1VLine_Fast(int x, int y, int h, u16 col);

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan1VLine(sDrawCan* can, int x, int y, int h, u16 col);
void Draw1VLine(int x, int y, int h, u16 col);

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan1VLineInv_Fast(sDrawCan* can, int x, int y, int h);
void Draw1VLineInv_Fast(int x, int y, int h);

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan1VLineInv(sDrawCan* can, int x, int y, int h);
void Draw1VLineInv(int x, int y, int h);

// ----- Draw frame

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan1Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw1Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan1Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw1Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan1FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw1FrameInv_Fast(int x, int y, int w, int h);

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan1FrameInv(sDrawCan* can, int x, int y, int w, int h);
void Draw1FrameInv(int x, int y, int w, int h);

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan1FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw1FrameW_Fast(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan1FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw1FrameW(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan1FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw1FrameWInv_Fast(int x, int y, int w, int h, int thick);

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan1FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw1FrameWInv(int x, int y, int w, int h, int thick);

// ----- Draw line

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan1LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw1LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (does not check clipping limits)
void DrawCan1Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw1Line_Fast(int x1, int y1, int x2, int y2, u16 col);

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan1LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw1LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (checks clipping limits)
void DrawCan1Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw1Line(int x1, int y1, int x2, int y2, u16 col);

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan1LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw1LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (does not check clipping limits)
void DrawCan1LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw1LineInv_Fast(int x1, int y1, int x2, int y2);

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan1LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw1LineOverInv(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (checks clipping limits)
void DrawCan1LineInv(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw1LineInv(int x1, int y1, int x2, int y2);

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan1LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw1LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan1LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw1LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan1LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw1LineWInv_Fast(int x1, int y1, int x2, int y2, int thick);

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan1LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw1LineWInv(int x1, int y1, int x2, int y2, int thick);

// ----- Draw round (filled circle)

//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCan1Round_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw1Round_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw round (checks clipping limits)
void DrawCan1Round(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw1Round(int x, int y, int d, u16 col, u8 mask);

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan1RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw1RoundInv_Fast(int x, int y, int d, u8 mask);

// Draw round inverted (checks clipping limits)
void DrawCan1RoundInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw1RoundInv(int x, int y, int d, u8 mask);

// ----- Draw circle
//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan1Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw1Circle_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc (checks clipping limits)
void DrawCan1Circle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw1Circle(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan1CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw1CircleInv_Fast(int x, int y, int d, u8 mask);

// Draw circle or arc inverted (checks clipping limits)
void DrawCan1CircleInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw1CircleInv(int x, int y, int d, u8 mask);

// ----- Draw ring
//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring

// Draw ring (does not check clipping limits)
void DrawCan1Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw1Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring (checks clipping limits)
void DrawCan1Ring(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw1Ring(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring inverted (does not check clipping limits)
void DrawCan1RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw1RingInv_Fast(int x, int y, int d, int din, u8 mask);

// Draw ring inverted (checks clipping limits)
void DrawCan1RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw1RingInv(int x, int y, int d, int din, u8 mask);

// ----- Draw triangle

// Draw triangle (does not check clipping limits)
void DrawCan1Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw1Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle (checks clipping limits)
void DrawCan1Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw1Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle inverted (does not check clipping limits)
void DrawCan1TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw1TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3);

// Draw triangle inverted (checks clipping limits)
void DrawCan1TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw1TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3);

// ----- Draw fill area

// Draw fill area
void DrawCan1Fill(sDrawCan* can, int x, int y, u16 col);
void Draw1Fill(int x, int y, u16 col);

// ----- Draw character
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw1SelFont(const u8* font, u8 fontw, u8 fonth);

void Draw1SelFont8x8();		// sans-serif bold, height 8 lines
void Draw1SelFont8x14();	// sans-serif bold, height 14 lines
void Draw1SelFont8x16();	// sans-serif bold, height 16 lines
void Draw1SelFont8x14Serif();	// serif bold, height 14 lines
void Draw1SelFont8x16Serif();	// serif bold, height 16 lines
void Draw1SelFont6x8();		// condensed font, width 6 pixels, height 8 lines
void Draw1SelFont8x8Game();	// game font, height 8 lines
void Draw1SelFont8x8Ibm();	// IBM charset font, height 8 lines
void Draw1SelFont8x14Ibm();	// IBM charset font, height 14 lines
void Draw1SelFont8x16Ibm();	// IBM charset font, height 16 lines
void Draw1SelFont8x8IbmThin();	// IBM charset thin font, height 8 lines
void Draw1SelFont8x8Italic();	// italic, height 8 lines
void Draw1SelFont8x8Thin();	// thin font, height 8 lines
void Draw1SelFont5x8();		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan1Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw1Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with transparent background (checks clipping limits)
void DrawCan1Char(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw1Char(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with background (does not check clipping limits)
void DrawCan1CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw1CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character with background (checks clipping limits)
void DrawCan1CharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw1CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character inverted (does not check clipping limits)
void DrawCan1CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw1CharInv_Fast(char ch, int x, int y, int scalex, int scaley);

// Draw character inverted (checks clipping limits)
void DrawCan1CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw1CharInv(char ch, int x, int y, int scalex, int scaley);

// ----- Draw text
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan1Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw1Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with transparent background (checks clipping limits)
void DrawCan1Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw1Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with background (does not check clipping limits)
void DrawCan1TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw1TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text with background (checks clipping limits)
void DrawCan1TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw1TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text inverted (does not check clipping limits)
void DrawCan1TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw1TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley);

// Draw text inverted (checks clipping limits)
void DrawCan1TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw1TextInv(const char* text, int len, int x, int y, int scalex, int scaley);

// ----- Draw ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan1Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw1Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse (checks clipping limits)
void DrawCan1Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw1Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse inverted (does not check clipping limits)
void DrawCan1EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw1EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw ellipse inverted (checks clipping limits)
void DrawCan1EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw1EllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw filled ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse

// Draw filled ellipse (does not check clipping limits)
void DrawCan1FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw1FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse (checks clipping limits)
void DrawCan1FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw1FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan1FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw1FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan1FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw1FillEllipseInv(int x, int y, int dx, int dy, u8 mask);

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
void DrawCan1Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw1Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw image inverted with the same format as destination
void DrawCan1ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw1ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw transparent image with the same format as destination
void DrawCan1Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw1Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image inverted with the same format as destination
void DrawCan1BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw1BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image with the same format as destination, with substitute color
// - This function is here only for compatibility, and is replaced by the DrawCan1Blit/Draw1Blit function
void DrawCan1BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void Draw1BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);

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
void DrawCan1GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void Draw1GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);

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
void DrawCan1ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
void Draw1ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
// drawing function interface
extern const sDrawCanFnc DrawCan1Fnc;
#endif

#ifdef __cplusplus
}
#endif

#endif // _LIB_DRAWCAN1_H

#endif // USE_DRAWCAN
