
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

#if USE_DRAWCAN && USE_DRAWCAN4		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#ifndef _LIB_DRAWCAN4_H
#define _LIB_DRAWCAN4_H

#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// default drawing canvas for 4-bit format
extern sDrawCan DrawCan4;

// current drawing canvas for 4-bit format YRGB1111
extern sDrawCan* pDrawCan4;

// set default drawing canvas for 4-bit format YRGB1111
void SetDrawCan4(sDrawCan* can);

// get current width, height and buffer of the drawing canvas of 4-bit format
INLINE int Draw4Width() { return pDrawCan4->w; }
INLINE int Draw4Height() { return pDrawCan4->h; }
INLINE u8* Draw4Buf() { return pDrawCan4->buf; }

// convert RGB888 color to 4-bit pixel color YRGB1111
u16 Draw4ColRgb(u8 r, u8 g, u8 b);

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 4-bit pixel color YRGB1111
u16 Draw4ColRand();
#endif

// 4-bit colors, format YRGB1111 (components are 0..255)
#define COLOR4(r,g,b) ( ( ((r)>127)&&((r)<192)&&((g)>127)&&((g)<192)&&((b)>127)&&((b)<192) ) ? 0x07 : \
	( (((b)>>7)&1) | (((g)>>6)&2) | (((r)>>5)&4) | ((((b)>0x3f)&&((g)>0x3f)&&((r)>0x3f)) ? 8 : 0) ) )

// - base colors
#define COL4_BLACK	COLOR4(  0,  0,  0)	// 0x00
#define COL4_BLUE	COLOR4(  0,  0,255)	// 0x01
#define COL4_GREEN	COLOR4(  0,255,  0)	// 0x02
#define COL4_CYAN	COLOR4(  0,255,255)	// 0x03
#define COL4_RED	COLOR4(255,  0,  0)	// 0x04
#define COL4_MAGENTA	COLOR4(255,  0,255)	// 0x05
#define COL4_YELLOW	COLOR4(255,255,  0)	// 0x06
#define COL4_WHITE	COLOR4(255,255,255)	// 0x0f
#define COL4_GRAY	COLOR4(128,128,128)	// 0x07
// - dark colors
#define COL4_DKGRAY	COLOR4( 64, 64, 64)	// 0x08
// - light colors
#define COL4_LTBLUE	COLOR4(127,127,255)	// 0x09
#define COL4_LTGREEN	COLOR4(127,255,127)	// 0x0A
#define COL4_LTCYAN	COLOR4(127,255,255)	// 0x0B
#define COL4_LTRED	COLOR4(255,127,127)	// 0x0C
#define COL4_LTMAGENTA	COLOR4(255,127,255)	// 0x0D
#define COL4_LTYELLOW	COLOR4(255,255,127)	// 0x0E

#define COL4_RANDOM	(RandU8()&0x0f)

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw4Pitch(int w);

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw4MaxWidth(int pitch);

// ----- Clear canvas

// Clear canvas with color
void DrawCan4ClearCol(sDrawCan* can, u16 col);
void Draw4ClearCol(u16 col);

// Clear canvas with black color
void DrawCan4Clear(sDrawCan* can);
void Draw4Clear();

// ----- Draw point

// Draw point (does not check clipping limits)
void DrawCan4Point_Fast(sDrawCan* can, int x, int y, u16 col);
void Draw4Point_Fast(int x, int y, u16 col);

// Draw point (checks clipping limits)
void DrawCan4Point(sDrawCan* can, int x, int y, u16 col);
void Draw4Point(int x, int y, u16 col);

// Draw point inverted (does not check clipping limits)
void DrawCan4PointInv_Fast(sDrawCan* can, int x, int y);
void Draw4PointInv_Fast(int x, int y);

// Draw point inverted (checks clipping limits)
void DrawCan4PointInv(sDrawCan* can, int x, int y);
void Draw4PointInv(int x, int y);

// ----- Get point

// Get point (does not check clipping limits)
u16 DrawCan4GetPoint_Fast(sDrawCan* can, int x, int y);
u16 Draw4GetPoint_Fast(int x, int y);

// Get point (checks clipping limits)
u16 DrawCan4GetPoint(sDrawCan* can, int x, int y);
u16 Draw4GetPoint(int x, int y);

// ----- Draw rectangle

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan4Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw4Rect_Fast(int x, int y, int w, int h, u16 col);

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan4Rect(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw4Rect(int x, int y, int w, int h, u16 col);

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan4RectInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw4RectInv_Fast(int x, int y, int w, int h);

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan4RectInv(sDrawCan* can, int x, int y, int w, int h);
void Draw4RectInv(int x, int y, int w, int h);

// ----- Draw horizontal line

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan4HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col);
void Draw4HLine_Fast(int x, int y, int w, u16 col);

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan4HLine(sDrawCan* can, int x, int y, int w, u16 col);
void Draw4HLine(int x, int y, int w, u16 col);

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan4HLineInv_Fast(sDrawCan* can, int x, int y, int w);
void Draw4HLineInv_Fast(int x, int y, int w);

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan4HLineInv(sDrawCan* can, int x, int y, int w);
void Draw4HLineInv(int x, int y, int w);

// ----- Draw vertical line

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan4VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col);
void Draw4VLine_Fast(int x, int y, int h, u16 col);

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan4VLine(sDrawCan* can, int x, int y, int h, u16 col);
void Draw4VLine(int x, int y, int h, u16 col);

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan4VLineInv_Fast(sDrawCan* can, int x, int y, int h);
void Draw4VLineInv_Fast(int x, int y, int h);

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan4VLineInv(sDrawCan* can, int x, int y, int h);
void Draw4VLineInv(int x, int y, int h);

// ----- Draw frame

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan4Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw4Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan4Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw4Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan4FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw4FrameInv_Fast(int x, int y, int w, int h);

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan4FrameInv(sDrawCan* can, int x, int y, int w, int h);
void Draw4FrameInv(int x, int y, int w, int h);

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan4FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw4FrameW_Fast(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan4FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw4FrameW(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan4FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw4FrameWInv_Fast(int x, int y, int w, int h, int thick);

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan4FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw4FrameWInv(int x, int y, int w, int h, int thick);

// ----- Draw line

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan4LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw4LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (does not check clipping limits)
void DrawCan4Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw4Line_Fast(int x1, int y1, int x2, int y2, u16 col);

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan4LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw4LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (checks clipping limits)
void DrawCan4Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw4Line(int x1, int y1, int x2, int y2, u16 col);

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan4LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw4LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (does not check clipping limits)
void DrawCan4LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw4LineInv_Fast(int x1, int y1, int x2, int y2);

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan4LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw4LineOverInv(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (checks clipping limits)
void DrawCan4LineInv(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw4LineInv(int x1, int y1, int x2, int y2);

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan4LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw4LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan4LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw4LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan4LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw4LineWInv_Fast(int x1, int y1, int x2, int y2, int thick);

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan4LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw4LineWInv(int x1, int y1, int x2, int y2, int thick);

// ----- Draw round (filled circle)

//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCan4Round_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw4Round_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw round (checks clipping limits)
void DrawCan4Round(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw4Round(int x, int y, int d, u16 col, u8 mask);

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan4RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw4RoundInv_Fast(int x, int y, int d, u8 mask);

// Draw round inverted (checks clipping limits)
void DrawCan4RoundInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw4RoundInv(int x, int y, int d, u8 mask);

// ----- Draw circle
//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan4Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw4Circle_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc (checks clipping limits)
void DrawCan4Circle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw4Circle(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan4CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw4CircleInv_Fast(int x, int y, int d, u8 mask);

// Draw circle or arc inverted (checks clipping limits)
void DrawCan4CircleInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw4CircleInv(int x, int y, int d, u8 mask);

// ----- Draw ring
//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring

// Draw ring (does not check clipping limits)
void DrawCan4Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw4Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring (checks clipping limits)
void DrawCan4Ring(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw4Ring(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring inverted (does not check clipping limits)
void DrawCan4RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw4RingInv_Fast(int x, int y, int d, int din, u8 mask);

// Draw ring inverted (checks clipping limits)
void DrawCan4RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw4RingInv(int x, int y, int d, int din, u8 mask);

// ----- Draw triangle

// Draw triangle (does not check clipping limits)
void DrawCan4Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw4Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle (checks clipping limits)
void DrawCan4Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw4Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle inverted (does not check clipping limits)
void DrawCan4TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw4TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3);

// Draw triangle inverted (checks clipping limits)
void DrawCan4TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw4TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3);

// ----- Draw fill area

// Draw fill area
void DrawCan4Fill(sDrawCan* can, int x, int y, u16 col);
void Draw4Fill(int x, int y, u16 col);

// ----- Draw character
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw4SelFont(const u8* font, u8 fontw, u8 fonth);

void Draw4SelFont8x8();		// sans-serif bold, height 8 lines
void Draw4SelFont8x14();	// sans-serif bold, height 14 lines
void Draw4SelFont8x16();	// sans-serif bold, height 16 lines
void Draw4SelFont8x14Serif();	// serif bold, height 14 lines
void Draw4SelFont8x16Serif();	// serif bold, height 16 lines
void Draw4SelFont6x8();		// condensed font, width 6 pixels, height 8 lines
void Draw4SelFont8x8Game();	// game font, height 8 lines
void Draw4SelFont8x8Ibm();	// IBM charset font, height 8 lines
void Draw4SelFont8x14Ibm();	// IBM charset font, height 14 lines
void Draw4SelFont8x16Ibm();	// IBM charset font, height 16 lines
void Draw4SelFont8x8IbmThin();	// IBM charset thin font, height 8 lines
void Draw4SelFont8x8Italic();	// italic, height 8 lines
void Draw4SelFont8x8Thin();	// thin font, height 8 lines
void Draw4SelFont5x8();		// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan4Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw4Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with transparent background (checks clipping limits)
void DrawCan4Char(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw4Char(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with background (does not check clipping limits)
void DrawCan4CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw4CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character with background (checks clipping limits)
void DrawCan4CharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw4CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character inverted (does not check clipping limits)
void DrawCan4CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw4CharInv_Fast(char ch, int x, int y, int scalex, int scaley);

// Draw character inverted (checks clipping limits)
void DrawCan4CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw4CharInv(char ch, int x, int y, int scalex, int scaley);

// ----- Draw text
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan4Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw4Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with transparent background (checks clipping limits)
void DrawCan4Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw4Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with background (does not check clipping limits)
void DrawCan4TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw4TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text with background (checks clipping limits)
void DrawCan4TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw4TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text inverted (does not check clipping limits)
void DrawCan4TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw4TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley);

// Draw text inverted (checks clipping limits)
void DrawCan4TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw4TextInv(const char* text, int len, int x, int y, int scalex, int scaley);

// ----- Draw ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan4Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw4Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse (checks clipping limits)
void DrawCan4Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw4Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse inverted (does not check clipping limits)
void DrawCan4EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw4EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw ellipse inverted (checks clipping limits)
void DrawCan4EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw4EllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw filled ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse

// Draw filled ellipse (does not check clipping limits)
void DrawCan4FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw4FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse (checks clipping limits)
void DrawCan4FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw4FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan4FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw4FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan4FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw4FillEllipseInv(int x, int y, int dx, int dy, u8 mask);

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
void DrawCan4Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw4Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw image inverted with the same format as destination
void DrawCan4ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw4ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw transparent image with the same format as destination
void DrawCan4Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw4Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image inverted with the same format as destination
void DrawCan4BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw4BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan4BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void Draw4BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);

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
void DrawCan4GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void Draw4GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);

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
void DrawCan4ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
void Draw4ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
// drawing function interface
extern const sDrawCanFnc DrawCan4Fnc;
#endif

#ifdef __cplusplus
}
#endif

#endif // _LIB_DRAWCAN4_H

#endif // USE_DRAWCAN
