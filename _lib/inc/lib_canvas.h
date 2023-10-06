
// ****************************************************************************
//
//                                   Canvas
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

#if USE_CANVAS			// use Canvas (lib_canvas.c, lib_canvas.h)

#ifndef _LIB_CANVAS_H
#define _LIB_CANVAS_H

#include "../../_sdk/inc/sdk_interp.h"
#include "lib_mat2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// canvas format
//   Note: do not use enum, symbols could not be used by the preprocessor
#define CANVAS_8	0	// 8-bit pixels
#define CANVAS_4	1	// 4-bit pixels
#define CANVAS_2	2	// 2-bit pixels
#define CANVAS_1	3	// 1-bit pixels
#define CANVAS_PLANE2	4	// 4 colors on 2 planes
#define CANVAS_ATTRIB8	5	// 2x4 bit color attributes per 8x8 pixel sample
				//  draw functions:	bit 0..3 = draw color
				//			bit 4 = draw color is background color

// canvas descriptor
typedef struct {
	u8*	img;	// image data
	u8*	img2;	// image data 2 (2nd plane of CANVAS_PLANE2, attributes of CANVAS_ATTRIB8)
	int	w;	// width
	int	h;	// height
	int	wb;	// pitch (bytes between lines)
	u8	format;	// canvas format CANVAS_*
} sCanvas;

// Draw rectangle
void CanvasRect(sCanvas* canvas, int x, int y, int w, int h, u8 col);

// Draw frame
void CanvasFrame(sCanvas* canvas, int x, int y, int w, int h, u8 col);

// clear canvas (fill with black color)
void CanvasClear(sCanvas* canvas);

// Draw point
void CanvasPoint(sCanvas* canvas, int x, int y, u8 col);

// Draw line
void CanvasLine(sCanvas* canvas, int x1, int y1, int x2, int y2, u8 col);

// Draw filled circle
//  x0, y0 ... coordinate of center
//  r ... radius
//  col ... color
//     col with CANVAS_ATTRIB8 format: bit 0..3 = draw color, bit 4 = draw color is background color
//  mask ... mask of used octants (use 0xff = 255 = draw whole circle)
//         . B2|B1 .
//       B3 .  |  . B0
//       ------o------
//       B4 .  |  . B7
//         . B5|B6 .
void CanvasFillCircle(sCanvas* canvas, int x0, int y0, int r, u8 col, u8 mask);

// Draw circle
//  x0, y0 ... coordinate of center
//  r ... radius
//  col ... color
//     col with CANVAS_ATTRIB8 format: bit 0..3 = draw color, bit 4 = draw color is background color
//  mask ... mask of used octants (use 0xff = 255 = draw whole circle)
//         . B2|B1 .
//       B3 .  |  . B0
//       ------o------
//       B4 .  |  . B7
//         . B5|B6 .
void CanvasCircle(sCanvas* canvas, int x0, int y0, int r, u8 col, u8 mask);

// Draw text (transparent background)
//   font = pointer to 1-bit font, font width = 8 pixels
void CanvasText(sCanvas* canvas, const char* text, int x, int y, u8 col,
	const void* font, int fontheight, int scalex, int scaley);

// Draw small text (transparent background)
//   font = pointer to 1-bit font, font width < 8 pixels
void CanvasSmallText(sCanvas* canvas, const char* text, int x, int y, u8 col,
	const void* font, int fontheight, int fontwidth);

// Draw text with background
//   font = pointer to 1-bit font, font width = 8 pixels
void CanvasTextBg(sCanvas* canvas, const char* text, int x, int y, u8 col, u8 bgcol,
	const void* font, int fontheight, int scalex, int scaley);

// Draw small text with background
//   font = pointer to 1-bit font, font width < 8 pixels
void CanvasSmallTextBg(sCanvas* canvas, const char* text, int x, int y, u8 col, u8 bgcol,
	const void* font, int fontheight, int fontwidth);

// Draw image
void CanvasImg(sCanvas* canvas, sCanvas* src, int xd, int yd, int xs, int ys, int w, int h);

// Draw image with transparency (source and destination must have same format, col = transparency key color)
//  CANVAS_ATTRIB8 format replaced by CanvasImg function
void CanvasBlit(sCanvas* canvas, sCanvas* src, int xd, int yd, int xs, int ys, int w, int h, u8 col);

// CanvasImgMat mode
enum {
	CANVASIMG_WRAP,		// wrap image
	CANVASIMG_NOBORDER,	// no border (transparent border)
	CANVASIMG_CLAMP,	// clamp image (use last pixel as border)
	CANVASIMG_COLOR,	// color border
	CANVASIMG_TRANSP,	// transparent image with key color
	CANVASIMG_PERSP,	// perspective floor
};

#if USE_MAT2D			// use 2D transformation matrix (lib_mat2d.c, lib_mat2d.h)

// draw 8-bit image with 2D transformation matrix
//  canvas ... destination canvas
//  src ... source canvas with image
//  x ... destination coordinate X
//  y ... destination coordinate Y
//  w ... destination width
//  h ... destination height
//  m ... transformation matrix (should be prepared using PrepDrawImg function)
//  mode ... draw mode CANVASIMG_*
//  color ... key or border color
// Note to wrap and perspective mode: Width and height of source image must be power of 2!
void CanvasImgMat(sCanvas* canvas, const sCanvas* src, int x, int y, int w, int h,
	const sMat2D* m, u8 mode, u8 color);

// draw tile map using perspective projection
//  canvas ... destination canvas
//  src ... source canvas with column of 8-bit square tiles (width = tile size, must be power of 2)
//  map ... byte map of tile indices
//  mapwbits ... number of bits of map width (number of tiles; width must be power of 2)
//  maphbits ... number of bits of map height (number of tiles; height must be power of 2)
//  tilebits ... number of bits of tile size (e.g. 5 = tile 32x32 pixel)
//  x ... destination coordinate X
//  y ... destination coordinate Y
//  w ... destination width
//  h ... destination height
//  mat ... transformation matrix (should be prepared using PrepDrawImg function)
//  horizon ... horizon offset (0=do not use perspective projection)
void CanvasTileMap(sCanvas* canvas, const sCanvas* src, const u8* map, int mapwbits, int maphbits,
	int tilebits, int x, int y, int w, int h, const sMat2D* mat, u8 horizon);

#endif // USE_MAT2D

// draw image line interpolated
//  canvas = destination canvas (8-bit pixel format)
//  src = source canvas (source image in 8-bit pixel format)
//  xd,yd = destination coordinates
//  xs,ys = source coordinates
//  wd = destination width
//  ws = source width
// Overflow in X direction is not checked!
void CanvasImgLine(sCanvas* canvas, sCanvas* src, int xd, int yd, int xs, int ys, int wd, int ws);

#ifdef __cplusplus
}
#endif

#endif // _LIB_CANVAS_H

#endif // USE_CANVAS			// use Canvas (lib_canvas.c, lib_canvas.h)
