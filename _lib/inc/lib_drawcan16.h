
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

#if USE_DRAWCAN		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

// 16-bit colors, format RGB565 (components are 0..255)
#define COLOR16(r,g,b)	((u16)( (((r)&0xf8)<<8) | (((g)&0xfc)<<3) | (((b)&0xf8)>>3) ))
// - base colors
#define COL16_BLACK	COLOR16(  0,  0,  0)
#define COL16_BLUE	COLOR16(  0,  0,255)
#define COL16_GREEN	COLOR16(  0,255,  0)
#define COL16_CYAN	COLOR16(  0,255,255)
#define COL16_RED	COLOR16(255,  0,  0)
#define COL16_MAGENTA	COLOR16(255,  0,255)
#define COL16_YELLOW	COLOR16(255,255,  0)
#define COL16_WHITE	COLOR16(255,255,255)
#define COL16_GRAY	COLOR16(128,128,128)
// - dark colors
#define COL16_DKBLUE	COLOR16(  0,  0,127)
#define COL16_DKGREEN	COLOR16(  0,127,  0)
#define COL16_DKCYAN	COLOR16(  0,127,127)
#define COL16_DKRED	COLOR16(127,  0,  0)
#define COL16_DKMAGENTA	COLOR16(127,  0,127)
#define COL16_DKYELLOW	COLOR16(127,127,  0)
#define COL16_DKWHITE	COLOR16(127,127,127)
#define COL16_DKGRAY	COLOR16( 64, 64, 64)
// - light colors
#define COL16_LTBLUE	COLOR16(127,127,255)
#define COL16_LTGREEN	COLOR16(127,255,127)
#define COL16_LTCYAN	COLOR16(127,255,255)
#define COL16_LTRED	COLOR16(255,127,127)
#define COL16_LTMAGENTA	COLOR16(255,127,255)
#define COL16_LTYELLOW	COLOR16(255,255,127)
#define COL16_LTGRAY	COLOR16(192,192,192)
// - special colors
#define COL16_AZURE	COLOR16(  0,127,255)
#define COL16_ORANGE	COLOR16(255,127,  0)
#define COL16_BROWN	COLOR16(192, 96,  0)

#define COL16_RANDOM	(RandU16())

#endif // USE_DRAWCAN



#if USE_DRAWCAN && USE_DRAWCAN16	// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#ifndef _LIB_DRAWCAN16_H
#define _LIB_DRAWCAN16_H

#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

#if USE_DRAWCAN4
int Draw4Pitch(int w);
#else
INLINE int Draw4Pitch(int w) { return ((w + 1)/2 + 3) & ~3; }
#endif

#if USE_DRAWCAN8
int Draw8Pitch(int w);
#else
INLINE int Draw8Pitch(int w) { return (w + 3) & ~3; }
#endif

// default drawing canvas for 15-bit and 16-bit format
extern sDrawCan DrawCan15;
extern sDrawCan DrawCan16;

// current drawing canvas for 15-bit and 16-bit format
extern sDrawCan* pDrawCan15;
extern sDrawCan* pDrawCan16;

// set default drawing canvas for 15-bit and 16-bit format
void SetDrawCan15(sDrawCan* can);
void SetDrawCan16(sDrawCan* can);

// get current width, height and buffer of the drawing canvas of 15-bit format
INLINE int Draw15Width() { return pDrawCan15->w; }
INLINE int Draw15Height() { return pDrawCan15->h; }
INLINE u8* Draw15Buf() { return pDrawCan15->buf; }

// get current width, height and buffer of the drawing canvas of 16-bit format
INLINE int Draw16Width() { return pDrawCan16->w; }
INLINE int Draw16Height() { return pDrawCan16->h; }
INLINE u8* Draw16Buf() { return pDrawCan16->buf; }

// convert RGB888 color to 15-bit pixel color RGB555
u16 Draw15ColRgb(u8 r, u8 g, u8 b);

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 15-bit pixel color RGB555
u16 Draw15ColRand();
#endif

// convert RGB888 color to 16-bit pixel color RGB565
u16 Draw16ColRgb(u8 r, u8 g, u8 b);

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random 16-bit pixel color RGB565
u16 Draw16ColRand();
#endif

// 15-bit colors, format RGB555 (components are 0..255)
#define COLOR15(r,g,b)	((u16)( (((r)&0xf8)<<7) | (((g)&0xf8)<<2) | (((b)&0xf8)>>3) ))
// - base colors
#define COL15_BLACK	COLOR15(  0,  0,  0)
#define COL15_BLUE	COLOR15(  0,  0,255)
#define COL15_GREEN	COLOR15(  0,255,  0)
#define COL15_CYAN	COLOR15(  0,255,255)
#define COL15_RED	COLOR15(255,  0,  0)
#define COL15_MAGENTA	COLOR15(255,  0,255)
#define COL15_YELLOW	COLOR15(255,255,  0)
#define COL15_WHITE	COLOR15(255,255,255)
#define COL15_GRAY	COLOR15(128,128,128)
// - dark colors
#define COL15_DKBLUE	COLOR15(  0,  0,127)
#define COL15_DKGREEN	COLOR15(  0,127,  0)
#define COL15_DKCYAN	COLOR15(  0,127,127)
#define COL15_DKRED	COLOR15(127,  0,  0)
#define COL15_DKMAGENTA	COLOR15(127,  0,127)
#define COL15_DKYELLOW	COLOR15(127,127,  0)
#define COL15_DKWHITE	COLOR15(127,127,127)
#define COL15_DKGRAY	COLOR15( 64, 64, 64)
// - light colors
#define COL15_LTBLUE	COLOR15(127,127,255)
#define COL15_LTGREEN	COLOR15(127,255,127)
#define COL15_LTCYAN	COLOR15(127,255,255)
#define COL15_LTRED	COLOR15(255,127,127)
#define COL15_LTMAGENTA	COLOR15(255,127,255)
#define COL15_LTYELLOW	COLOR15(255,255,127)
#define COL15_LTGRAY	COLOR15(192,192,192)
// - special colors
#define COL15_AZURE	COLOR15(  0,127,255)
#define COL15_ORANGE	COLOR15(255,127,  0)
#define COL15_BROWN	COLOR15(192, 96,  0)

#define COL15_RANDOM	(RandU16()&0x7fff)

// convert 12-bit color RGB444 to 15-bit color RGB555
#define COL12TO15(ccc) ( (((ccc) & 0x00f) << 1) | (((ccc) & 0x0f0) << 2) | (((ccc) & 0xf00) << 3) )

// convert 15-bit color RGB555 to 12-bit color RGB444
#define COL15TO12(ccc) ( (((ccc) >> 1) & 0x00f) | (((ccc) >> 2) & 0x0f0) | (((ccc) >> 3) & 0xf00) )

// convert 12-bit color RGB444 to 16-bit color RGB565
#define COL12TO16(ccc) ( (((ccc) & 0x00f) << 1) | (((ccc) & 0x0f0) << 3) | (((ccc) & 0xf00) << 4) )

// convert 16-bit color RGB565 to 12-bit color RGB444
#define COL16TO12(ccc) ( (((ccc) >> 1) & 0x00f) | (((ccc) >> 3) & 0x0f0) | (((ccc) >> 4) & 0xf00) )

// convert 16-bit color RGB565 to 15-bit color RGB555
#define COL16TO15(ccc) ( ((ccc) & 0x1f) | (((ccc) >> 1) & 0x7fe0) )

// convert 15-bit color RGB555 to 16-bit color RGB565
#define COL15TO16(ccc) ( ((ccc) & 0x1f) | (((ccc) & 0x7fe0) << 1) )

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int Draw16Pitch(int w);
#define Draw15Pitch Draw16Pitch

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int Draw16MaxWidth(int pitch);
#define Draw15MaxWidth Draw16MaxWidth

// ----- Clear canvas

// Clear canvas with color
void DrawCan16ClearCol(sDrawCan* can, u16 col);
#define DrawCan15ClearCol DrawCan16ClearCol
void Draw16ClearCol(u16 col);
void Draw15ClearCol(u16 col);

// Clear canvas with black color
void DrawCan16Clear(sDrawCan* can);
void DrawCan15Clear(sDrawCan* can);
void Draw16Clear();
void Draw15Clear();

// ----- Draw point

// Draw point (does not check clipping limits)
void DrawCan16Point_Fast(sDrawCan* can, int x, int y, u16 col);
#define DrawCan15Point_Fast DrawCan16Point_Fast
void Draw16Point_Fast(int x, int y, u16 col);
void Draw15Point_Fast(int x, int y, u16 col);

// Draw point (checks clipping limits)
void DrawCan16Point(sDrawCan* can, int x, int y, u16 col);
#define DrawCan15Point DrawCan16Point
void Draw16Point(int x, int y, u16 col);
void Draw15Point(int x, int y, u16 col);

// Draw point inverted (does not check clipping limits)
void DrawCan16PointInv_Fast(sDrawCan* can, int x, int y);
#define DrawCan15PointInv_Fast DrawCan16PointInv_Fast
void Draw16PointInv_Fast(int x, int y);
void Draw15PointInv_Fast(int x, int y);

// Draw point inverted (checks clipping limits)
void DrawCan16PointInv(sDrawCan* can, int x, int y);
#define DrawCan15PointInv DrawCan16PointInv
void Draw16PointInv(int x, int y);
void Draw15PointInv(int x, int y);

// ----- Get point

// Get point (does not check clipping limits)
u16 DrawCan16GetPoint_Fast(sDrawCan* can, int x, int y);
u16 DrawCan15GetPoint_Fast(sDrawCan* can, int x, int y);
u16 Draw16GetPoint_Fast(int x, int y);
u16 Draw15GetPoint_Fast(int x, int y);

// Get point (checks clipping limits)
u16 DrawCan16GetPoint(sDrawCan* can, int x, int y);
u16 DrawCan15GetPoint(sDrawCan* can, int x, int y);
u16 Draw16GetPoint(int x, int y);
u16 Draw15GetPoint(int x, int y);

// ----- Draw rectangle

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCan16Rect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col);
#define DrawCan15Rect_Fast DrawCan16Rect_Fast
void Draw16Rect_Fast(int x, int y, int w, int h, u16 col);
void Draw15Rect_Fast(int x, int y, int w, int h, u16 col);

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCan16Rect(sDrawCan* can, int x, int y, int w, int h, u16 col);
#define DrawCan15Rect DrawCan16Rect
void Draw16Rect(int x, int y, int w, int h, u16 col);
void Draw15Rect(int x, int y, int w, int h, u16 col);

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan16RectInv_Fast(sDrawCan* can, int x, int y, int w, int h);
#define DrawCan15RectInv_Fast DrawCan16RectInv_Fast
void Draw16RectInv_Fast(int x, int y, int w, int h);
void Draw15RectInv_Fast(int x, int y, int w, int h);

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCan16RectInv(sDrawCan* can, int x, int y, int w, int h);
#define DrawCan15RectInv DrawCan16RectInv
void Draw16RectInv(int x, int y, int w, int h);
void Draw15RectInv(int x, int y, int w, int h);

// ----- Draw horizontal line

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCan16HLine_Fast(sDrawCan* can, int x, int y, int w, u16 col);
#define DrawCan15HLine_Fast DrawCan16HLine_Fast
void Draw16HLine_Fast(int x, int y, int w, u16 col);
void Draw15HLine_Fast(int x, int y, int w, u16 col);

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCan16HLine(sDrawCan* can, int x, int y, int w, u16 col);
#define DrawCan15HLine DrawCan16HLine
void Draw16HLine(int x, int y, int w, u16 col);
void Draw15HLine(int x, int y, int w, u16 col);

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCan16HLineInv_Fast(sDrawCan* can, int x, int y, int w);
#define DrawCan15HLineInv_Fast DrawCan16HLineInv_Fast
void Draw16HLineInv_Fast(int x, int y, int w);
void Draw15HLineInv_Fast(int x, int y, int w);

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCan16HLineInv(sDrawCan* can, int x, int y, int w);
#define DrawCan15HLineInv DrawCan16HLineInv
void Draw16HLineInv(int x, int y, int w);
void Draw15HLineInv(int x, int y, int w);

// ----- Draw vertical line

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCan16VLine_Fast(sDrawCan* can, int x, int y, int h, u16 col);
#define DrawCan15VLine_Fast DrawCan16VLine_Fast
void Draw16VLine_Fast(int x, int y, int h, u16 col);
void Draw15VLine_Fast(int x, int y, int h, u16 col);

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCan16VLine(sDrawCan* can, int x, int y, int h, u16 col);
#define DrawCan15VLine DrawCan16VLine
void Draw16VLine(int x, int y, int h, u16 col);
void Draw15VLine(int x, int y, int h, u16 col);

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCan16VLineInv_Fast(sDrawCan* can, int x, int y, int h);
#define DrawCan15VLineInv_Fast DrawCan16VLineInv_Fast
void Draw16VLineInv_Fast(int x, int y, int h);
void Draw15VLineInv_Fast(int x, int y, int h);

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCan16VLineInv(sDrawCan* can, int x, int y, int h);
#define DrawCan15VLineInv DrawCan16VLineInv
void Draw16VLineInv(int x, int y, int h);
void Draw15VLineInv(int x, int y, int h);

// ----- Draw frame

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCan16Frame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
#define DrawCan15Frame_Fast DrawCan16Frame_Fast
void Draw16Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw15Frame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCan16Frame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark);
#define DrawCan15Frame DrawCan16Frame
void Draw16Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark);
void Draw15Frame(int x, int y, int w, int h, u16 col_light, u16 col_dark);

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCan16FrameInv_Fast(sDrawCan* can, int x, int y, int w, int h);
#define DrawCan15FrameInv_Fast DrawCan16FrameInv_Fast
void Draw16FrameInv_Fast(int x, int y, int w, int h);
void Draw15FrameInv_Fast(int x, int y, int w, int h);

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan16FrameInv(sDrawCan* can, int x, int y, int w, int h);
#define DrawCan15FrameInv DrawCan16FrameInv
void Draw16FrameInv(int x, int y, int w, int h);
void Draw15FrameInv(int x, int y, int w, int h);

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan16FrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
#define DrawCan15FrameW_Fast DrawCan16FrameW_Fast
void Draw16FrameW_Fast(int x, int y, int w, int h, u16 col, int thick);
void Draw15FrameW_Fast(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCan16FrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick);
#define DrawCan15FrameW DrawCan16FrameW
void Draw16FrameW(int x, int y, int w, int h, u16 col, int thick);
void Draw15FrameW(int x, int y, int w, int h, u16 col, int thick);

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCan16FrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick);
#define DrawCan15FrameWInv_Fast DrawCan16FrameWInv_Fast
void Draw16FrameWInv_Fast(int x, int y, int w, int h, int thick);
void Draw15FrameWInv_Fast(int x, int y, int w, int h, int thick);

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCan16FrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick);
#define DrawCan15FrameWInv DrawCan16FrameWInv
void Draw16FrameWInv(int x, int y, int w, int h, int thick);
void Draw15FrameWInv(int x, int y, int w, int h, int thick);

// ----- Draw line

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCan16LineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
#define DrawCan15LineOver_Fast DrawCan16LineOver_Fast
void Draw16LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw15LineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (does not check clipping limits)
void DrawCan16Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void DrawCan15Line_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw16Line_Fast(int x1, int y1, int x2, int y2, u16 col);
void Draw15Line_Fast(int x1, int y1, int x2, int y2, u16 col);

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan16LineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over);
#define DrawCan15LineOver DrawCan16LineOver
void Draw16LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over);
void Draw15LineOver(int x1, int y1, int x2, int y2, u16 col, Bool over);

// Draw line (checks clipping limits)
void DrawCan16Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void DrawCan15Line(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col);
void Draw16Line(int x1, int y1, int x2, int y2, u16 col);
void Draw15Line(int x1, int y1, int x2, int y2, u16 col);

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCan16LineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
#define DrawCan15LineOverInv_Fast DrawCan16LineOverInv_Fast
void Draw16LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over);
void Draw15LineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (does not check clipping limits)
void DrawCan16LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2);
void DrawCan15LineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw16LineInv_Fast(int x1, int y1, int x2, int y2);
void Draw15LineInv_Fast(int x1, int y1, int x2, int y2);

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCan16LineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over);
#define DrawCan15LineOverInv DrawCan16LineOverInv
void Draw16LineOverInv(int x1, int y1, int x2, int y2, Bool over);
void Draw15LineOverInv(int x1, int y1, int x2, int y2, Bool over);

// Draw line inverted (checks clipping limits)
void DrawCan16LineInv(sDrawCan* can, int x1, int y1, int x2, int y2);
void DrawCan15LineInv(sDrawCan* can, int x1, int y1, int x2, int y2);
void Draw16LineInv(int x1, int y1, int x2, int y2);
void Draw15LineInv(int x1, int y1, int x2, int y2);

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan16LineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
#define DrawCan15LineW_Fast DrawCan16LineW_Fast
void Draw16LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw15LineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan16LineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
#define DrawCan15LineW DrawCan16LineW
void Draw16LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);
void Draw15LineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round);

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan16LineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
#define DrawCan15LineWInv_Fast DrawCan16LineWInv_Fast
void Draw16LineWInv_Fast(int x1, int y1, int x2, int y2, int thick);
void Draw15LineWInv_Fast(int x1, int y1, int x2, int y2, int thick);

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCan16LineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick);
#define DrawCan15LineWInv DrawCan16LineWInv
void Draw16LineWInv(int x1, int y1, int x2, int y2, int thick);
void Draw15LineWInv(int x1, int y1, int x2, int y2, int thick);

// ----- Draw round (filled circle)

//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCan16Round_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
#define DrawCan15Round_Fast DrawCan16Round_Fast
void Draw16Round_Fast(int x, int y, int d, u16 col, u8 mask);
void Draw15Round_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw round (checks clipping limits)
void DrawCan16Round(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
#define DrawCan15Round DrawCan16Round
void Draw16Round(int x, int y, int d, u16 col, u8 mask);
void Draw15Round(int x, int y, int d, u16 col, u8 mask);

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCan16RoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
#define DrawCan15RoundInv_Fast DrawCan16RoundInv_Fast
void Draw16RoundInv_Fast(int x, int y, int d, u8 mask);
void Draw15RoundInv_Fast(int x, int y, int d, u8 mask);

// Draw round inverted (checks clipping limits)
void DrawCan16RoundInv(sDrawCan* can, int x, int y, int d, u8 mask);
#define DrawCan15RoundInv DrawCan16RoundInv
void Draw16RoundInv(int x, int y, int d, u8 mask);
void Draw15RoundInv(int x, int y, int d, u8 mask);

// ----- Draw circle
//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCan16Circle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
#define DrawCan15Circle_Fast DrawCan16Circle_Fast
void Draw16Circle_Fast(int x, int y, int d, u16 col, u8 mask);
void Draw15Circle_Fast(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc (checks clipping limits)
void DrawCan16Circle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask);
#define DrawCan15Circle DrawCan16Circle
void Draw16Circle(int x, int y, int d, u16 col, u8 mask);
void Draw15Circle(int x, int y, int d, u16 col, u8 mask);

// Draw circle or arc inverted (does not check clipping limits)
void DrawCan16CircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask);
#define DrawCan15CircleInv_Fast DrawCan16CircleInv_Fast
void Draw16CircleInv_Fast(int x, int y, int d, u8 mask);
void Draw15CircleInv_Fast(int x, int y, int d, u8 mask);

// Draw circle or arc inverted (checks clipping limits)
void DrawCan16CircleInv(sDrawCan* can, int x, int y, int d, u8 mask);
#define DrawCan15CircleInv DrawCan16CircleInv
void Draw16CircleInv(int x, int y, int d, u8 mask);
void Draw15CircleInv(int x, int y, int d, u8 mask);

// ----- Draw ring
//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring

// Draw ring (does not check clipping limits)
void DrawCan16Ring_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
#define DrawCan15Ring_Fast DrawCan16Ring_Fast
void Draw16Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask);
void Draw15Ring_Fast(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring (checks clipping limits)
void DrawCan16Ring(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask);
#define DrawCan15Ring DrawCan16Ring
void Draw16Ring(int x, int y, int d, int din, u16 col, u8 mask);
void Draw15Ring(int x, int y, int d, int din, u16 col, u8 mask);

// Draw ring inverted (does not check clipping limits)
void DrawCan16RingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask);
#define DrawCan15RingInv_Fast DrawCan16RingInv_Fast
void Draw16RingInv_Fast(int x, int y, int d, int din, u8 mask);
void Draw15RingInv_Fast(int x, int y, int d, int din, u8 mask);

// Draw ring inverted (checks clipping limits)
void DrawCan16RingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask);
#define DrawCan15RingInv DrawCan16RingInv
void Draw16RingInv(int x, int y, int d, int din, u8 mask);
void Draw15RingInv(int x, int y, int d, int din, u8 mask);

// ----- Draw triangle

// Draw triangle (does not check clipping limits)
void DrawCan16Triangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
#define DrawCan15Triangle_Fast DrawCan16Triangle_Fast
void Draw16Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw15Triangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle (checks clipping limits)
void DrawCan16Triangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
#define DrawCan15Triangle DrawCan16Triangle
void Draw16Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);
void Draw15Triangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col);

// Draw triangle inverted (does not check clipping limits)
void DrawCan16TriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
#define DrawCan15TriangleInv_Fast DrawCan16TriangleInv_Fast
void Draw16TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3);
void Draw15TriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3);

// Draw triangle inverted (checks clipping limits)
void DrawCan16TriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3);
#define DrawCan15TriangleInv DrawCan16TriangleInv
void Draw16TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3);
void Draw15TriangleInv(int x1, int y1, int x2, int y2, int x3, int y3);

// ----- Draw fill area

// Draw fill area
void DrawCan16Fill(sDrawCan* can, int x, int y, u16 col);
void DrawCan15Fill(sDrawCan* can, int x, int y, u16 col);
void Draw16Fill(int x, int y, u16 col);
void Draw15Fill(int x, int y, u16 col);

// ----- Draw character
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void Draw16SelFont(const u8* font, u8 fontw, u8 fonth);

void Draw16SelFont8x8();	// sans-serif bold, height 8 lines
void Draw16SelFont8x14();	// sans-serif bold, height 14 lines
void Draw16SelFont8x16();	// sans-serif bold, height 16 lines
void Draw16SelFont8x14Serif();	// serif bold, height 14 lines
void Draw16SelFont8x16Serif();	// serif bold, height 16 lines
void Draw16SelFont6x8();	// condensed font, width 6 pixels, height 8 lines
void Draw16SelFont8x8Game();	// game font, height 8 lines
void Draw16SelFont8x8Ibm();	// IBM charset font, height 8 lines
void Draw16SelFont8x14Ibm();	// IBM charset font, height 14 lines
void Draw16SelFont8x16Ibm();	// IBM charset font, height 16 lines
void Draw16SelFont8x8IbmThin();	// IBM charset thin font, height 8 lines
void Draw16SelFont8x8Italic();	// italic, height 8 lines
void Draw16SelFont8x8Thin();	// thin font, height 8 lines
void Draw16SelFont5x8();	// tiny font, width 5 pixels, height 8 lines

void Draw15SelFont(const u8* font, u8 fontw, u8 fonth);

void Draw15SelFont8x8();	// sans-serif bold, height 8 lines
void Draw15SelFont8x14();	// sans-serif bold, height 14 lines
void Draw15SelFont8x16();	// sans-serif bold, height 16 lines
void Draw15SelFont8x14Serif();	// serif bold, height 14 lines
void Draw15SelFont8x16Serif();	// serif bold, height 16 lines
void Draw15SelFont6x8();	// condensed font, width 6 pixels, height 8 lines
void Draw15SelFont8x8Game();	// game font, height 8 lines
void Draw15SelFont8x8Ibm();	// IBM charset font, height 8 lines
void Draw15SelFont8x14Ibm();	// IBM charset font, height 14 lines
void Draw15SelFont8x16Ibm();	// IBM charset font, height 16 lines
void Draw15SelFont8x8IbmThin();	// IBM charset thin font, height 8 lines
void Draw15SelFont8x8Italic();	// italic, height 8 lines
void Draw15SelFont8x8Thin();	// thin font, height 8 lines
void Draw15SelFont5x8();	// tiny font, width 5 pixels, height 8 lines

// Draw character with transparent background (does not check clipping limits)
void DrawCan16Char_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
#define DrawCan15Char_Fast DrawCan16Char_Fast
void Draw16Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw15Char_Fast(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with transparent background (checks clipping limits)
void DrawCan16Char(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley);
#define DrawCan15Char DrawCan16Char
void Draw16Char(char ch, int x, int y, u16 col, int scalex, int scaley);
void Draw15Char(char ch, int x, int y, u16 col, int scalex, int scaley);

// Draw character with background (does not check clipping limits)
void DrawCan16CharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
#define DrawCan15CharBg_Fast DrawCan16CharBg_Fast
void Draw16CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw15CharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character with background (checks clipping limits)
void DrawCan16CharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
#define DrawCan15CharBg DrawCan16CharBg
void Draw16CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw15CharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw character inverted (does not check clipping limits)
void DrawCan16CharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
#define DrawCan15CharInv_Fast DrawCan16CharInv_Fast
void Draw16CharInv_Fast(char ch, int x, int y, int scalex, int scaley);
void Draw15CharInv_Fast(char ch, int x, int y, int scalex, int scaley);

// Draw character inverted (checks clipping limits)
void DrawCan16CharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley);
#define DrawCan15CharInv DrawCan16CharInv
void Draw16CharInv(char ch, int x, int y, int scalex, int scaley);
void Draw15CharInv(char ch, int x, int y, int scalex, int scaley);

// ----- Draw text
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCan16Text_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
#define DrawCan15Text_Fast DrawCan16Text_Fast
void Draw16Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw15Text_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with transparent background (checks clipping limits)
void DrawCan16Text(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
#define DrawCan15Text DrawCan16Text
void Draw16Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);
void Draw15Text(const char* text, int len, int x, int y, u16 col, int scalex, int scaley);

// Draw text with background (does not check clipping limits)
void DrawCan16TextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
#define DrawCan15TextBg_Fast DrawCan16TextBg_Fast
void Draw16TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw15TextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text with background (checks clipping limits)
void DrawCan16TextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
#define DrawCan15TextBg DrawCan16TextBg
void Draw16TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);
void Draw15TextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley);

// Draw text inverted (does not check clipping limits)
void DrawCan16TextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
#define DrawCan15TextInv_Fast DrawCan16TextInv_Fast
void Draw16TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley);
void Draw15TextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley);

// Draw text inverted (checks clipping limits)
void DrawCan16TextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley);
#define DrawCan15TextInv DrawCan16TextInv
void Draw16TextInv(const char* text, int len, int x, int y, int scalex, int scaley);
void Draw15TextInv(const char* text, int len, int x, int y, int scalex, int scaley);

// ----- Draw ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCan16Ellipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
#define DrawCan15Ellipse_Fast DrawCan16Ellipse_Fast
void Draw16Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw15Ellipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse (checks clipping limits)
void DrawCan16Ellipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
#define DrawCan15Ellipse DrawCan16Ellipse
void Draw16Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw15Ellipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw ellipse inverted (does not check clipping limits)
void DrawCan16EllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
#define DrawCan15EllipseInv_Fast DrawCan16EllipseInv_Fast
void Draw16EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);
void Draw15EllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw ellipse inverted (checks clipping limits)
void DrawCan16EllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
#define DrawCan15EllipseInv DrawCan16EllipseInv
void Draw16EllipseInv(int x, int y, int dx, int dy, u8 mask);
void Draw15EllipseInv(int x, int y, int dx, int dy, u8 mask);

// ----- Draw filled ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse

// Draw filled ellipse (does not check clipping limits)
void DrawCan16FillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
#define DrawCan15FillEllipse_Fast DrawCan16FillEllipse_Fast
void Draw16FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw15FillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse (checks clipping limits)
void DrawCan16FillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask);
#define DrawCan15FillEllipse DrawCan16FillEllipse
void Draw16FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);
void Draw15FillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask);

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCan16FillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
#define DrawCan15FillEllipseInv_Fast DrawCan16FillEllipseInv_Fast
void Draw16FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);
void Draw15FillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask);

// Draw filled ellipse inverte (checks clipping limits)
void DrawCan16FillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask);
#define DrawCan15FillEllipseInv DrawCan16FillEllipseInv
void Draw16FillEllipseInv(int x, int y, int dx, int dy, u8 mask);
void Draw15FillEllipseInv(int x, int y, int dx, int dy, u8 mask);

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
void DrawCan16Img(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
#define DrawCan15Img DrawCan16Img
void Draw16Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw15Img(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw image inverted with the same format as destination
void DrawCan16ImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
#define DrawCan15ImgInv DrawCan16ImgInv
void Draw16ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw15ImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw transparent image with the same format as destination
void DrawCan16Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void DrawCan15Blit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw16Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw15Blit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image inverted with the same format as destination
void DrawCan16BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void DrawCan15BlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw16BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);
void Draw15BlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col);

// Draw transparent image with the same format as destination, with substitute color
//  fnd ... color to find for
//  subs ... color to substitute with
void DrawCan16BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void DrawCan15BlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void Draw16BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);
void Draw15BlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs);

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
void DrawCan16GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void DrawCan15GetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void Draw16GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);
void Draw15GetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd);

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
void DrawCan16ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
void DrawCan15ImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
void Draw16ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);
void Draw15ImgMat(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color);

// ----- Draw image specific to 16-bit canvas
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
void DrawCan16Img12(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw16Img12(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw 8-bit palleted image to 16-bit destination canvas
void DrawCan16Img8(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw16Img8(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// For backward compatibility with Draw version 1
INLINE void DrawImgPal(const u8* src, const u16* pal, int xs, int ys, int xd, int yd, int w, int h, int ws) { Draw16Img8(xd, yd, src, pal, xs, ys, w, h, Draw8Pitch(ws)); }

// Draw 6-bit palleted image to 16-bit destination canvas
void DrawCan16Img6(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw16Img6(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// Draw 4-bit palleted image to 16-bit destination canvas
void DrawCan16Img4(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw16Img4(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// For backward compatibility with Draw version 1
INLINE void DrawImg4Pal(const u8* src, const u16* pal, int xs, int ys, int xd, int yd, int w, int h, int ws) { Draw16Img4(xd, yd, src, pal, xs, ys, w, h, Draw4Pitch(ws)); }

// Draw 3-bit palleted image to 16-bit destination canvas
void DrawCan16Img3(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw16Img3(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// Draw 2-bit palleted image to 16-bit destination canvas
void DrawCan16Img2(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw16Img2(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// Draw 1-bit image with specific colors to 16-bit destination canvas
//  col ... color of pixel with value '1'
//  bgcol ... color of pixel with value '0'
void DrawCan16Img1(sDrawCan* can, int xd, int yd, const void* src, u16 col, u16 bgcol, int xs, int ys, int w, int h, int wbs);
void Draw16Img1(int xd, int yd, const void* src, u16 bgcol, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 12-bit image transparent to 16-bit destination canvas
void DrawCan16Blit12(sDrawCan* can, int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw16Blit12(int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 8-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit8(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw16Blit8(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// For backward compatibility with Draw version 1
INLINE void DrawBlitPal(const u8* src, const u16* pal, int xs, int ys, int xd, int yd, int w, int h, int ws, u16 col) { Draw16Blit8(xd, yd, src, pal, col, xs, ys, w, h, Draw8Pitch(ws)); }

// Draw 6-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit6(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw16Blit6(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 4-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit4(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw16Blit4(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// For backward compatibility with Draw version 1
INLINE void DrawBlit4Pal(const u8* src, const u16* pal, int xs, int ys, int xd, int yd, int w, int h, int ws, u16 col) { Draw16Blit4(xd, yd, src, pal, col, xs, ys, w, h, Draw4Pitch(ws)); }

// Draw 3-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit3(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw16Blit3(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 2-bit palleted image transparent to 16-bit destination canvas
void DrawCan16Blit2(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw16Blit2(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 1-bit image transparent with specific colors to 16-bit destination canvas
//  col ... color of non-transparent pixel with value '1'
void DrawCan16Blit1(sDrawCan* can, int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw16Blit1(int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs);

// ----- Draw image specific to 15-bit canvas
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
void DrawCan15Img12(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);
void Draw15Img12(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs);

// Draw 8-bit palleted image to 15-bit destination canvas
void DrawCan15Img8(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw15Img8(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// Draw 6-bit palleted image to 15-bit destination canvas
void DrawCan15Img6(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw15Img6(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// Draw 4-bit palleted image to 15-bit destination canvas
void DrawCan15Img4(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw15Img4(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// Draw 3-bit palleted image to 15-bit destination canvas
void DrawCan15Img3(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw15Img3(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// Draw 2-bit palleted image to 15-bit destination canvas
void DrawCan15Img2(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);
void Draw15Img2(int xd, int yd, const void* src, const u16* pal, int xs, int ys, int w, int h, int wbs);

// Draw 1-bit image with specific colors to 15-bit destination canvas
//  col ... color of pixel with value '1'
//  bgcol ... color of pixel with value '0'
#define DrawCan15Img1 DrawCan16Img1
void Draw15Img1(int xd, int yd, const void* src, u16 bgcol, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 12-bit image transparent to 15-bit destination canvas
void DrawCan15Blit12(sDrawCan* can, int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw15Blit12(int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 8-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit8(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw15Blit8(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 6-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit6(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw15Blit6(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 4-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit4(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw15Blit4(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 3-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit3(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw15Blit3(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 2-bit palleted image transparent to 15-bit destination canvas
void DrawCan15Blit2(sDrawCan* can, int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);
void Draw15Blit2(int xd, int yd, const void* src, const u16* pal, u16 col, int xs, int ys, int w, int h, int wbs);

// Draw 1-bit image transparent with specific colors to 15-bit destination canvas
//  col ... color of non-transparent pixel with value '1'
void DrawCan15Blit1(sDrawCan* can, int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs);
#define DrawCan15Blit1 DrawCan16Blit1
void Draw15Blit1(int xd, int yd, const void* src, u16 col, int xs, int ys, int w, int h, int wbs);

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
// drawing function interface
extern const sDrawCanFnc DrawCan16Fnc;
extern const sDrawCanFnc DrawCan15Fnc;
#endif

#ifdef __cplusplus
}
#endif

#endif // _LIB_DRAWCAN16_H

#endif // USE_DRAWCAN
