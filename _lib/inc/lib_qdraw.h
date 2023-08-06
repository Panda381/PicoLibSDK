
// ****************************************************************************
//
//                           Draw to QVGA 8-bit buffer
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

#if USE_QDRAW		// use QVGA drawing (lib_qdraw.c, lib_qdraw.h)

#ifndef _QDRAW_H
#define _QDRAW_H

#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEXTWIDTH	(WIDTH/FONTW)	// width of text buffer (=40)
#define TEXTHEIGHT	(HEIGHT/FONTH)	// height of text buffer (=15)
#define TEXTSIZE	(TEXTWIDTH*TEXTHEIGHT) // size of mono text buffer (=600)
#define FTEXTSIZE	(TEXTWIDTH*2*TEXTHEIGHT) // size of text buffer with u8 foreground color (=1200)

// QVGA Colors
//	B0 ... VGA B0 blue
//	B1 ... VGA B1
//	B2 ... VGA G0 green
//	B3 ... VGA G1
//	B4 ... VGA G2
//	B5 ... VGA R0 red
//	B6 ... VGA R1
//	B7 ... VGA R2

// - base colors
#define COL_BLACK	0
#define COL_BLUE	(B0+B1)
#define COL_GREEN	(B2+B3+B4)
#define COL_CYAN	(B0+B1+B2+B3+B4)
#define COL_RED		(B5+B6+B7)
#define COL_MAGENTA	(B0+B1+B5+B6+B7)
#define COL_YELLOW	(B2+B3+B4+B5+B6+B7)
#define COL_WHITE	(B0+B1+B2+B3+B4+B5+B6+B7)
#define COL_GRAY	(B0+B2+B3+B5+B6)
// - dark colors
#define COL_DKBLUE	B0
#define COL_DKGREEN	B3
#define COL_DKCYAN	(B0+B3)
#define COL_DKRED	B6
#define COL_DKMAGENTA	(B0+B6)
#define COL_DKYELLOW	(B3+B6)
#define COL_DKGRAY	(B0+B2+B5)
// - light colors
#define COL_LTBLUE	(COL_BLUE|COL_GRAY)
#define COL_LTGREEN	(COL_GREEN|COL_GRAY)
#define COL_LTCYAN	(COL_CYAN|COL_GRAY)
#define COL_LTRED	(COL_RED|COL_GRAY)
#define COL_LTMAGENTA	(COL_MAGENTA|COL_GRAY)
#define COL_LTYELLOW	(COL_YELLOW|COL_GRAY)
#define COL_LTGRAY	(B0+B1+B3+B4+B6+B7)

#define COL_AZURE	(B0+B1+B2+B3)
#define COL_ORANGE	(B2+B3+B5+B6+B7)

#define COL_PRINT_DEF	COL_GRAY		// default console print color

// compose color from RGB
#define COLRGB(r,g,b) ((u8)(((r)&0xe0)|(((g)&0xe0)>>3)|((b)>>6)))

// length of line of the BMP image in bytes
#define IMGWB(w) (((w)+3)&~3)

extern u8* pDrawBuf; // current draw buffer
extern const u8* pDrawFont; // font
extern int DrawFontHeight; // font height
extern int DrawFontWidth; // font width (5 to 8)
extern u8 DrawPrintPosNum; // number of text positions per row (= WIDTH/DrawFontWidth)
extern u8 DrawPrintRowNum; // number of text rows (= HEIGHT/DrawFontHeight)
extern u8 DrawPrintPos;  // console print character position
extern u8 DrawPrintRow;  // console print character row
extern u8 DrawPrintInv; // offset added to character (128 = print inverted characters, 0 = normal character)
extern u8 DrawPrintSize; // font size: 0=normal, 1=double-height, 2=double-width, 3=double-size
extern u8 DrawPrintCol; // console print color

// select font
void SelFontUpdate(); // update size of text screen after changing font size
void SelFont5x8();
void SelFont6x8();
void SelFont8x8();
void SelFont8x14();
void SelFont8x16();

// Draw rectangle
void DrawRect(int x, int y, int w, int h, u8 col);

// Draw frame
void DrawFrame(int x, int y, int w, int h, u8 col);

// clear canvas with color
void DrawClearCol(u8 col);

// clear canvas (fill with black color)
void DrawClear();

// Draw point
void DrawPoint(int x, int y, u8 col);

// Draw line
void DrawLine(int x1, int y1, int x2, int y2, u8 col);

// Draw filled circle
void DrawFillCircle(int x0, int y0, int r, u8 col);

// Draw circle
void DrawCircle(int x0, int y0, int r, u8 col);

// Draw character (transparent background)
void DrawChar(char ch, int x, int y, u8 col);

// Draw character double height (transparent background)
void DrawCharH(char ch, int x, int y, u8 col);

// Draw character double width (transparent background)
void DrawCharW(char ch, int x, int y, u8 col);

// Draw character double sized (transparent background)
void DrawChar2(char ch, int x, int y, u8 col);

// Draw character with background
void DrawCharBg(char ch, int x, int y, u8 col, u8 bgcol);

// Draw character double height with background
void DrawCharBgH(char ch, int x, int y, u8 col, u8 bgcol);

// Draw character double width with background
void DrawCharBgW(char ch, int x, int y, u8 col, u8 bgcol);

// Draw character double sized with background
void DrawCharBg2(char ch, int x, int y, u8 col, u8 bgcol);

// Draw text (transparent background)
void DrawText(const char* text, int x, int y, u8 col);

// Draw text double width (transparent background)
void DrawTextW(const char* text, int x, int y, u8 col);

// Draw text double height (transparent background)
void DrawTextH(const char* text, int x, int y, u8 col);

// Draw text double sized (transparent background)
void DrawText2(const char* text, int x, int y, u8 col);

// Draw text with background
void DrawTextBg(const char* text, int x, int y, u8 col, u8 bgcol);

// Draw text double width with background
void DrawTextBgW(const char* text, int x, int y, u8 col, u8 bgcol);

// Draw text double height with background
void DrawTextBgH(const char* text, int x, int y, u8 col, u8 bgcol);

// Draw text double sized with background
void DrawTextBg2(const char* text, int x, int y, u8 col, u8 bgcol);

// Draw text buffer
void DrawTextBuf(const char* textbuf, u8 col, u8 bgcol);

// Draw text buffer with foreground color
void DrawFTextBuf(const char* textbuf, u8 bgcol);

// Draw image
void DrawImg(const u8* src, int xd, int yd, int w, int h, int ws);

// Draw image with transparency (col = transparency key color)
void DrawBlit(const u8* src, int xd, int yd, int w, int h, int ws, u8 col);

// DrawImgMat mode
enum {
	DRAWIMG_WRAP,		// wrap image
	DRAWIMG_NOBORDER,	// no border (transparent border)
	DRAWIMG_CLAMP,		// clamp image (use last pixel as border)
	DRAWIMG_COLOR,		// color border
	DRAWIMG_TRANSP,		// transparent image with key color
	DRAWIMG_PERSP,		// perspective floor
};

// draw 8-bit image with 2D transformation matrix
//  src ... source image
//  ws ... source image width
//  hs ... source image height
//  x ... destination coordinate X
//  y ... destination coordinate Y
//  w ... destination width
//  h ... destination height
//  m ... transformation matrix (should be prepared using PrepDrawImg function)
//  mode ... draw mode QDRAWIMG_*
//  color ... key or border color (QDRAWIMG_PERSP mode: horizon offset)
// Note to wrap and perspective mode: Width and height of source image should be power of 2, or it will render slower.
void DrawImgMat(const u8* src, int ws, int hs, int x, int y, int w, int h, const sMat2D* m, u8 mode, u8 color);

// draw tile map using perspective projection
//  src ... source image with column of 8-bit square tiles (width = tile size, must be power of 2)
//  map ... byte map of tile indices
//  mapwbits ... number of bits of map width (number of tiles; width must be power of 2)
//  maphbits ... number of bits of map height (number of tiles; height must be power of 2)
//  tilebits ... number of bits of tile size in pixels (e.g. 5 = tile 32x32 pixel)
//  x ... destination coordinate X
//  y ... destination coordinate Y
//  w ... destination width
//  h ... destination height
//  mat ... transformation matrix (should be prepared using PrepDrawImg function)
//  horizon ... horizon offset (0=do not use perspective projection)
void DrawTileMap(const u8* src, const u8* map, int mapwbits, int maphbits,
	int tilebits, int x, int y, int w, int h, const sMat2D* mat, u8 horizon);

// draw image line interpolated
//  src = source image
//  xd,yd = destination coordinates
//  wd = destination width
//  ws = source width
//  wbs = source line pitch in bytes
void DrawImgLine(const u8* src, int xd, int yd, int wd, int ws, int wbs);

// generate gradient
void GenGrad(u8* dst, int w);

// scroll screen one row up
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
void DrawPrintCharRaw(char ch);

// console print character (with display update; Control characters - see DrawPrintCharRaw)
void DrawPrintChar(char ch);

// console print text (Control characters - see DrawPrintCharRaw)
//  If text contains digit after hex numeric code of control character,
//  split text to more parts: use "\4" "1" instead of "\41".
void DrawPrintText(const char* txt);

// formatted print string to drawing console, with argument list (returns number of characters, without terminating 0)
u32 DrawPrintArg(const char* fmt, va_list args);

// formatted print string to drawing console, with variadic arguments (returns number of characters, without terminating 0)
NOINLINE u32 DrawPrint(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // _QDRAW_H

#endif // USE_QDRAW		// use QVGA drawing (lib_qdraw.c, lib_qdraw.h)
