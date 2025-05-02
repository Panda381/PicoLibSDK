
// ****************************************************************************
//
//                     Drawing to canvas at 12-bit format
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

#if USE_DRAWCAN	&& USE_DRAWCAN12	// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#ifndef _LIB_DRAWCAN12_H
#define _LIB_DRAWCAN12_H

#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// default drawing canvas for 12-bit format
extern sDrawCan DrawCan12;

// current drawing canvas for 12-bit format
extern sDrawCan* pDrawCan12;

// set default drawing canvas for 12-bit format
void SetDrawCan12(sDrawCan* can);

// get current width, height and buffer of the drawing canvas of 12-bit format
INLINE int Draw12Width() { return pDrawCan12->w; }
INLINE int Draw12Height() { return pDrawCan12->h; }
INLINE u8* Draw12Buf() { return pDrawCan12->buf; }

// convert RGB888 color to 12-bit pixel color RGB444
u16 Draw12ColRgb(u8 r, u8 g, u8 b);

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 12-bit pixel color RGB444
u16 Draw12ColRand();
#endif

// 12-bit colors, format RGB444 (components are 0..255)
#define COLOR12(r,g,b)	((u16)( (((r)&0xf0)<<4) | ((g)&0xf0) | (((b)&0xf0)>>4) ))
// - base colors
#define COL12_BLACK	COLOR12(  0,  0,  0)
#define COL12_BLUE	COLOR12(  0,  0,255)
#define COL12_GREEN	COLOR12(  0,255,  0)
#define COL12_CYAN	COLOR12(  0,255,255)
#define COL12_RED	COLOR12(255,  0,  0)
#define COL12_MAGENTA	COLOR12(255,  0,255)
#define COL12_YELLOW	COLOR12(255,255,  0)
#define COL12_WHITE	COLOR12(255,255,255)
#define COL12_GRAY	COLOR12(128,128,128)
// - dark colors
#define COL12_DKBLUE	COLOR12(  0,  0,127)
#define COL12_DKGREEN	COLOR12(  0,127,  0)
#define COL12_DKCYAN	COLOR12(  0,127,127)
#define COL12_DKRED	COLOR12(127,  0,  0)
#define COL12_DKMAGENTA	COLOR12(127,  0,127)
#define COL12_DKYELLOW	COLOR12(127,127,  0)
#define COL12_DKWHITE	COLOR12(127,127,127)
#define COL12_DKGRAY	COLOR12( 64, 64, 64)
// - light colors
#define COL12_LTBLUE	COLOR12(127,127,255)
#define COL12_LTGREEN	COLOR12(127,255,127)
#define COL12_LTCYAN	COLOR12(127,255,255)
#define COL12_LTRED	COLOR12(255,127,127)
#define COL12_LTMAGENTA	COLOR12(255,127,255)
#define COL12_LTYELLOW	COLOR12(255,255,127)
#define COL12_LTGRAY	COLOR12(192,192,192)
// - special colors
#define COL12_AZURE	COLOR12(  0,127,255)
#define COL12_ORANGE	COLOR12(255,127,  0)
#define COL12_BROWN	COLOR12(192, 96,  0)

#define COL12_RANDOM	(RandU16()&0xfff)

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw12Pitch(int w);

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw12MaxWidth(int pitch);

// ----- Clear canvas

// Clear canvas with color
void DrawCan12ClearCol(sDrawCan* can, u16 col);
void Draw12ClearCol(u16 col);

// Clear canvas with black color
void DrawCan12Clear(sDrawCan* can);
void Draw12Clear();

// ----- Draw point

// Draw point (does not check clipping limits)
void DrawCan12Point_Fast(sDrawCan* can, int x, int y, u16 col);
void Draw12Point_Fast(int x, int y, u16 col);

// Draw point (checks clipping limits)
void DrawCan12Point(sDrawCan* can, int x, int y, u16 col);
void Draw12Point(int x, int y, u16 col);

// Draw point inverted (does not check clipping limits)
void DrawCan12PointInv_Fast(sDrawCan* can, int x, int y);
void Draw12PointInv_Fast(int x, int y);

// Draw point inverted (checks clipping limits)
void DrawCan12PointInv(sDrawCan* can, int x, int y);
void Draw12PointInv(int x, int y);

// ----- Get point

// Get point (does not check clipping limits)
u16 DrawCan12GetPoint_Fast(sDrawCan* can, int x, int y);
u16 Draw12GetPoint_Fast(int x, int y);

// Get point (checks clipping limits)
u16 DrawCan12GetPoint(sDrawCan* can, int x, int y);
u16 Draw12GetPoint(int x, int y);

// ----- Draw rectangle

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan12Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw12Rect_Fast(int x, int y, int w, int h, u16 col);

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan12Rect(sDrawCan* can, int x, int y, int w, int h, u16 col);
void Draw12Rect(int x, int y, int w, int h, u16 col);

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan12RectInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw12RectInv_Fast(int x, int y, int w, int h);

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan12RectInv(sDrawCan* can, int x, int y, int w, int h);
void Draw12RectInv(int x, int y, int w, int h);

// ----- Draw horizontal line

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan12HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col);
void Draw12HLine_Fast(int x, int y, int w, u16 col);

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan12HLine(sDrawCan* can, int x, int y, int w, u16 col);
void Draw12HLine(int x, int y, int w, u16 col);

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan12HLineInv_Fast(sDrawCan* can, int x, int y, int w);
void Draw12HLineInv_Fast(int x, int y, int w);

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan12HLineInv(sDrawCan* can, int x, int y, int w);
void Draw12HLineInv(int x, int y, int w);

// ----- Draw vertical line

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan12VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col);
void Draw12VLine_Fast(int x, int y, int h, u16 col);

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan12VLine(sDrawCan* can, int x, int y, int h, u16 col);
void Draw12VLine(int x, int y, int h, u16 col);

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan12VLineInv_Fast(sDrawCan* can, int x, int y, int h);
void Draw12VLineInv_Fast(int x, int y, int h);

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan12VLineInv(sDrawCan* can, int x, int y, int h);
void Draw12VLineInv(int x, int y, int h);

// ----- Draw frame

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan12Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw12Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan12Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw12Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan12FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h);
void Draw12FrameInv_Fast(int x, int y, int w, int h);

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan12FrameInv(sDrawCan* can, int x, int y, int w, int h);
void Draw12FrameInv(int x, int y, int w, int h);

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan12FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw12FrameW_Fast(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan12FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
void Draw12FrameW(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan12FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw12FrameWInv_Fast(int x, int y, int w, int h, int thick);

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan12FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick);
void Draw12FrameWInv(int x, int y, int w, int h, int thick);

// ----- Draw line

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan12LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw12LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (does not check clipping limits)
void DrawCan12Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw12Line_Fast(int x1, int y1, int x2, int y2, u16 col);

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan12LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw12LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (checks clipping limits)
void DrawCan12Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw12Line(int x1, int y1, int x2, int y2, u16 col);

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan12LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw12LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (does not check clipping limits)
void DrawCan12LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw12LineInv_Fast(int x1, int y1, int x2, int y2);

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan12LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
void Draw12LineOverInv(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (checks clipping limits)
void DrawCan12LineInv(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw12LineInv(int x1, int y1, int x2, int y2);

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan12LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw12LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan12LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw12LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan12LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw12LineWInv_Fast(int x1, int y1, int x2, int y2, int thick);

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan12LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
void Draw12LineWInv(int x1, int y1, int x2, int y2, int thick);

// ----- Draw round (filled circle)

//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCan12Round_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw12Round_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw round (checks clipping limits)
void DrawCan12Round(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw12Round(int x, int y, int d, u16 col, u8 mask);

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan12RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw12RoundInv_Fast(int x, int y, int d, u8 mask);

// Draw round inverted (checks clipping limits)
void DrawCan12RoundInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw12RoundInv(int x, int y, int d, u8 mask);

// ----- Draw circle
//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan12Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw12Circle_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc (checks clipping limits)
void DrawCan12Circle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
void Draw12Circle(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan12CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw12CircleInv_Fast(int x, int y, int d, u8 mask);

// Draw circle or arc inverted (checks clipping limits)
void DrawCan12CircleInv(sDrawCan* can, int x, int y, int d, u8 mask);
void Draw12CircleInv(int x, int y, int d, u8 mask);

// ----- Draw ring
//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring

// Draw ring (does not check clipping limits)
void DrawCan12Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw12Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring (checks clipping limits)
void DrawCan12Ring(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
void Draw12Ring(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring inverted (does not check clipping limits)
void DrawCan12RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw12RingInv_Fast(int x, int y, int d, int din, u8 mask);

// Draw ring inverted (checks clipping limits)
void DrawCan12RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask);
void Draw12RingInv(int x, int y, int d, int din, u8 mask);

// ----- Draw triangle

// Draw triangle (does not check clipping limits)
void DrawCan12Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw12Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle (checks clipping limits)
void DrawCan12Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw12Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle inverted (does not check clipping limits)
void DrawCan12TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw12TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3);

// Draw triangle inverted (checks clipping limits)
void DrawCan12TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
void Draw12TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3);

// ----- Draw fill area

// Draw fill area
void DrawCan12Fill(sDrawCan* can, int x, int y, u16 col);
void Draw12Fill(int x, int y, u16 col);

// ----- Draw character
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw12SelFont(const u8* font, u8 fontw, u8 fonth);

void Draw12SelFont8x8();	// sans-serif bold, height 8 lines
void Draw12SelFont8x14();	// sans-serif bold, height 14 lines
void Draw12SelFont8x16();	// sans-serif bold, height 16 lines
void Draw12SelFont8x14Serif();	// serif bold, height 14 lines
void Draw12SelFont8x16Serif();	// serif bold, height 16 lines
void Draw12SelFont6x8();	// condensed font, width 6 pixels, height 8 lines
void Draw12SelFont8x8Game();	// game font, height 8 lines
void Draw12SelFont8x8Ibm();	// IBM charset font, height 8 lines
void Draw12SelFont8x14Ibm();	// IBM charset font, height 14 lines
void Draw12SelFont8x16Ibm();	// IBM charset font, height 16 lines
void Draw12SelFont8x8IbmThin();	// IBM charset thin font, height 8 lines
void Draw12SelFont8x8Italic();	// italic, height 8 lines
void Draw12SelFont8x8Thin();	// thin font, height 8 lines
void Draw12SelFont5x8();	// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan12Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw12Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with transparent background (checks clipping limits)
void DrawCan12Char(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw12Char(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with background (does not check clipping limits)
void DrawCan12CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw12CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character with background (checks clipping limits)
void DrawCan12CharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw12CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character inverted (does not check clipping limits)
void DrawCan12CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw12CharInv_Fast(char ch, int x, int y, int scalex, int scaley);

// Draw character inverted (checks clipping limits)
void DrawCan12CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
void Draw12CharInv(char ch, int x, int y, int scalex, int scaley);

// ----- Draw text
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan12Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw12Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with transparent background (checks clipping limits)
void DrawCan12Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw12Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with background (does not check clipping limits)
void DrawCan12TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw12TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text with background (checks clipping limits)
void DrawCan12TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw12TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text inverted (does not check clipping limits)
void DrawCan12TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw12TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley);

// Draw text inverted (checks clipping limits)
void DrawCan12TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
void Draw12TextInv(const char* text, int len, int x, int y, int scalex, int scaley);

// ----- Draw ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan12Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw12Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse (checks clipping limits)
void DrawCan12Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw12Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse inverted (does not check clipping limits)
void DrawCan12EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw12EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw ellipse inverted (checks clipping limits)
void DrawCan12EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw12EllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw filled ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse

// Draw filled ellipse (does not check clipping limits)
void DrawCan12FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw12FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse (checks clipping limits)
void DrawCan12FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw12FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan12FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw12FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan12FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
void Draw12FillEllipseInv(int x, int y, int dx, int dy, u8 mask);

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
void DrawCan12Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw12Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw image inverted with the same format as destination
void DrawCan12ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw12ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw transparent image with the same format as destination
void DrawCan12Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw12Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image inverted with the same format as destination
void DrawCan12BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw12BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan12BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void Draw12BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);

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
void DrawCan12GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void Draw12GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);

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
void DrawCan12ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
void Draw12ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
// drawing function interface
extern const sDrawCanFnc DrawCan12Fnc;
#endif

#ifdef __cplusplus
}
#endif

#endif // _LIB_DRAWCAN12_H

#endif // USE_DRAWCAN
