
// ****************************************************************************
//
//                               PicoVGA8 screen layout
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

#include "../../global.h"	// globals

#if USE_PICOVGA8		// use PicoVGA 8-bit display

#include "../../_sdk/inc/sdk_cpu.h"

#include "picovga8_screen.h"

// current video screen
sScreen Screen = { .num = 0 };	// default video screen
sScreen* pScreen = &Screen;	// pointer to current video screen

// clear screen (set 0 strips, does not modify sprites)
void ScreenClear(sScreen* s)
{
	dmb();
	s->num = 0;
	dmb();
}

// add empty strip to the screen (returns pointer to the strip)
sStrip* ScreenAddStrip(sScreen* s, int height)
{
	int n = s->num;
	sStrip* t = &s->strip[n];
	t->height = height;
	t->num = 0;
	dmb();
	s->num = n + 1;
	dmb();
	return t;
}

// add empty segment to video strip (returns pointer to the segment and initialises is to defaults)
sSegm* ScreenAddSegm(sStrip* strip, int width)
{
	int n = strip->num;
	sSegm* g = &strip->seg[n];
	g->width = width;
	g->wb = width;
	g->offx = 0;
	g->offy = 0;
	g->wrapx = width;
	g->wrapy = strip->height;
	g->data = NULL;
	g->form = GF_COLOR;
	g->dbly = False;
	g->par = 0;
	g->par2 = 0;
	dmb();
	strip->num = n + 1;
	dmb();
	return g;
}

// set video segment to simple color format GF_COLOR
//  col1 = color pattern 4-pixels even line (use macro MULTICOL)
//  col2 = color pattern 4-pixels odd line (use macro MULTICOL)
void ScreenSegmColor(sSegm* segm, u32 col1, u32 col2)
{
	segm->par = col1;
	segm->par2 = col2;
	dmb();
	segm->form = GF_COLOR;
	dmb();
}

// set video segment to gradient with 1 line
//   data = pointer to data buffer with gradient
//   wb = pitch - length of buffer
// To scroll gradient, set virtual dimension wrapx, then shift offx
void ScreenSegmGrad1(sSegm* segm, const void* data, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->wb = wb;
	dmb();
	segm->form = GF_GRAD1;
	dmb();
}

// set video segment to gradient with 2 lines
//   data = pointer to data buffer with gradient
//   wb = pitch - lenght of buffer
// To scroll gradient, set virtual dimension wrapx, then shift offx
void ScreenSegmGrad2(sSegm* segm, const void* data, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->wb = wb;
	dmb();
	segm->form = GF_GRAD2;
	dmb();
}

// set video segment to native 8-bit graphics (R3G3B2)
//   data = pointer to data buffer
//   wb = pitch - number of bytes between lines
// To scroll image, set virtual dimension wrapx and wrapy, then shift offx and offy.
void ScreenSegmGraph8(sSegm* segm, const void* data, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->wb = wb;
	dmb();
	segm->form = GF_GRAPH8;
	dmb();
}

// generate 16-color palette translation table for functions ScreenSegmGraph4
//  trans = pointer to destination palette translation table (u16 trans[256])
//  pal = pointer to source palette of 16 colors (u8 pal[16])
void GenPal16Trans(u16* trans, const u8* pal)
{
	int i, j;
	u16 k;
	for (i = 0; i < 256; i++)
	{
		j = (i >> 4) & 0x0f;
		k = pal[j];

		j = i & 0x0f;
		k |= (u16)pal[j] << 8;

		trans[i] = k;
	}
}

// set video segment to 4-bit palette graphics
//   data = pointer to data buffer
//   trans = pointer to 16-color palette translation table (generated with GenPal16Trans function)
//   wb = pitch - number of bytes between lines
// To scroll image, set virtual dimension wrapx and wrapy, then shift offx and offy.
void ScreenSegmGraph4(sSegm* segm, const void* data, const void* trans, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)trans;
	segm->wb = wb;
	dmb();
	segm->form = GF_GRAPH4;
	dmb();
}

// generate palette 4 translation table for functions ScreenSegmGraph2
//  trans = pointer to destination palette translation table (u32 trans[256])
//  pal = pointer to source palette of 4 colors (u8 pal[4])
void GenPal4Trans(u32* trans, const u8* pal)
{
	int i, j;
	u32 k;
	for (i = 0; i < 256; i++)
	{
		j = (i >> 6) & 0x03;
		k = pal[j];

		j = (i >> 4) & 0x03;
		k |= (u32)pal[j] << 8;

		j = (i >> 2) & 0x03;
		k |= (u32)pal[j] << 16;

		j = i & 0x03;
		k |= (u32)pal[j] << 24;

		trans[i] = k;
	}
}

// set video segment to 2-bit palette graphics
//   data = pointer to data buffer
//   trans = pointer to 4-color palette translation table (generated with GenPal4Trans function)
//   wb = pitch - number of bytes between lines
// To scroll image, set virtual dimension wrapx and wrapy, then shift offx and offy.
void ScreenSegmGraph2(sSegm* segm, const void* data, const void* trans, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)trans;
	segm->wb = wb;
	dmb();
	segm->form = GF_GRAPH2;
	dmb();
}

// set video segment to 1-bit palette graphics
//   data = pointer to data buffer
//   bg = background color
//   fg = foreground color
//   wb = pitch - number of bytes between lines
// To scroll image, set virtual dimension wrapx and wrapy, then shift offx and offy.
void ScreenSegmGraph1(sSegm* segm, const void* data, u8 bg, u8 fg, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = bg | ((u32)fg << 8);
	segm->wb = wb;
	dmb();
	segm->form = GF_GRAPH1;
	dmb();
}

// set video segment to 8-pixel mono text
//   data = pointer to text buffer
//   font = pointer to 1-bit font of 256 characters of width 8 (total width of image 2048 pixels)
//   fontheight = font height
//   bg = background color
//   fg = foreground color
//   wb = pitch - number of bytes between text lines
void ScreenSegmMText(sSegm* segm, const void* data, const void* font, u16 fontheight, u8 bg, u8 fg, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)font;
	segm->par2 = bg | ((u32)fg << 8);
	segm->par3 = fontheight;
	segm->wb = wb;
	dmb();
	segm->form = GF_MTEXT;
	dmb();
}

// set video segment to 8-pixel attribute text
//   data = pointer to text buffer (character + 2x4 bit attributes)
//   font = pointer to 1-bit font of 256 characters of width 8 (total width of image 2048 pixels)
//   fontheight = font height
//   pal = pointer to palette of 16 colors
//   wb = pitch - number of bytes between text lines
void ScreenSegmAText(sSegm* segm, const void* data, const void* font, u16 fontheight, const void* pal, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)font;
	segm->par2 = (u32)pal;
	segm->par3 = fontheight;
	segm->wb = wb;
	dmb();
	segm->form = GF_ATEXT;
	dmb();
}

// set video segment to 8-pixel foreground color text
//   data = pointer to text buffer (character + foreground color)
//   font = pointer to 1-bit font of 256 characters of width 8 (total width of image 2048 pixels)
//   fontheight = font height
//   bg = background color
//   wb = pitch - number of bytes between text lines
void ScreenSegmFText(sSegm* segm, const void* data, const void* font, u16 fontheight, u8 bg, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)font;
	segm->par2 = bg;
	segm->par3 = fontheight;
	segm->wb = wb;
	dmb();
	segm->form = GF_FTEXT;
	dmb();
}

// set video segment to 8-pixel color text
//   data = pointer to text buffer (character + background color + foreground color)
//   font = pointer to 1-bit font of 256 characters of width 8 (total width of image 2048 pixels)
//   fontheight = font height
//   wb = pitch - number of bytes between text lines
void ScreenSegmCText(sSegm* segm, const void* data, const void* font, u16 fontheight, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)font;
	segm->par3 = fontheight;
	segm->wb = wb;
	dmb();
	segm->form = GF_CTEXT;
	dmb();
}

// set video segment to 8-pixel gradient color text
//   data = pointer to text buffer (character + foreground color)
//   font = pointer to 1-bit font of 256 characters of width 8 (total width of image 2048 pixels)
//   fontheight = font height
//   bg = background color
//   grad = pointer to array of gradient colors
//   wb = pitch - number of bytes between text lines
void ScreenSegmGText(sSegm* segm, const void* data, const void* font, u8 fontheight, u8 bg, const void* grad, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)font;
	segm->par3 = bg | (fontheight << 8);
	segm->par2 = (u32)grad;
	segm->wb = wb;
	dmb();
	segm->form = GF_GTEXT;
	dmb();
}

// set video segment to 8-pixel double gradient color text
//   data = pointer to text buffer (character + foreground color)
//   font = pointer to 1-bit font of 256 characters of width 8 (total width of image 2048 pixels)
//   fontheight = font height
//   bg = background color
//   grad = pointer to array of gradient colors
//   wb = pitch - number of bytes between text lines
void ScreenSegmDText(sSegm* segm, const void* data, const void* font, u8 fontheight, u8 bg, const void* grad, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)font;
	segm->par3 = bg | (fontheight << 8);
	segm->par2 = (u32)grad;
	segm->wb = wb;
	dmb();
	segm->form = GF_DTEXT;
	dmb();
}

// set video segment to tiles
//   data = pointer to tile map buffer (with tile indices)
//   tiles = pointer to 1 column of tiles, 1 pixel = 8 bits
//   w = tile width (must be multiple of 4)
//   h = tile height
//   wb = pitch - number of bytes between tile map rows
void ScreenSegmTile(sSegm* segm, const void* data, const void* tiles, int w, int h, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)tiles;
	segm->par2 = (u32)h;
	segm->par3 = (u16)w;
	segm->wb = wb;
	segm->wrapx = (segm->width+w-1)/w*w;
	segm->wrapy = (segm->wrapy+h-1)/h*h;
	dmb();
	segm->form = GF_TILE;
	dmb();
}

// set video segment to alternate tiles
//   data = pointer to tile map buffer (with tile indices)
//   tiles = pointer to 1 row of tiles, 1 pixel = 8 bits
//   w = tile width (must be multiple of 4)
//   h = tile height
//   tilewb = tile width bytes (usually tile width * number of tiles)
//   wb = pitch - number of bytes between tile map rows
void ScreenSegmTile2(sSegm* segm, const void* data, const void* tiles, int w, int h, int tilewb, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)tiles;
	segm->par2 = (u32)h + ((u32)(u16)tilewb << 16);
	segm->par3 = (u16)w;
	segm->wb = wb;
	segm->wrapx = (segm->width+w-1)/w*w;
	segm->wrapy = (segm->wrapy+h-1)/h*h;
	dmb();
	segm->form = GF_TILE2;
	dmb();
}

// set video segment to level graph GF_LEVEL
//   data = pointer to buffer with line samples 0..255
//   bg = background color
//   fg = foreground color
//   zero = Y zero level
void ScreenSegmLevel(sSegm* segm, const void* data, u8 bg, u8 fg, u8 zero)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = bg | ((u32)fg << 8);
	segm->par2 = zero;
	dmb();
	segm->form = GF_LEVEL;
	dmb();
}

// set video segment to leve gradient graph GF_LEVELGRAD
//   data = pointer to buffer with values 0..255 of 4-pixels in rows
//   sample1 = scanline sample < data
//   sample2 = scanline sample >= data
void ScreenSegmLevelGrad(sSegm* segm, const void* data, const void* sample1, const void* sample2)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)sample1;
	segm->par2 = (u32)sample2;
	dmb();
	segm->form = GF_LEVELGRAD;
	dmb();
}

// set video segment to oscilloscope 1-pixel graph GF_OSCIL
//   data = pointer to buffer with line samples 0..255
//   bg = background color
//   fg = foreground color
//   pixh = height of pixels - 1
void ScreenSegmOscil(sSegm* segm, const void* data, u8 bg, u8 fg, int pixh)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = bg | ((u32)fg << 8);
	segm->par2 = pixh;
	dmb();
	segm->form = GF_OSCIL;
	dmb();
}

// set video segment to oscilloscope line graph GF_OSCLINE
//   data = pointer to buffer with line samples 0..255
//   bg = background color
//   fg = foreground color
void ScreenSegmOscLine(sSegm* segm, const void* data, u8 bg, u8 fg)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = bg | ((u32)fg << 8);
	dmb();
	segm->form = GF_OSCLINE;
	dmb();
}

// generate palette 4-planes translation table for function ScreenSegmPlane2
//  trans = pointer to destination palette translation table (u32 trans[256])
//  pal = pointer to source palette of 4 colors (u8 pal[4])
void GenPal4Plane(u32* trans, const u8* pal)
{
	int i, j;
	u32 k;
	for (i = 0; i < 256; i++)
	{
		j = 0;
		if ((i & B7) != 0) j |= B1;
		if ((i & B3) != 0) j |= B0;
		k = pal[j];

		j = 0;
		if ((i & B6) != 0) j |= B1;
		if ((i & B2) != 0) j |= B0;
		k |= (u32)pal[j] << 8;

		j = 0;
		if ((i & B5) != 0) j |= B1;
		if ((i & B1) != 0) j |= B0;
		k |= (u32)pal[j] << 16;

		j = 0;
		if ((i & B4) != 0) j |= B1;
		if ((i & B0) != 0) j |= B0;
		k |= (u32)pal[j] << 24;

		trans[i] = k;
	}
}

// set video segment to 4-color on 2-planes graphics
//   data = pointer to data buffer
//   plane = offset of 2nd graphics plane (in bytes), size of one graphics plane
//   trans = pointer to 4-color palette translation table (generated with GenPal4Plane function)
//   wb = pitch - number of bytes between lines
// To scroll image, set virtual dimension wrapx and wrapy, then shift offx and offy.
void ScreenSegmPlane2(sSegm* segm, const void* data, int plane, const void* trans, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = plane;
	segm->par2 = (u32)trans;
	segm->wb = wb;
	dmb();
	segm->form = GF_PLANE2;
	dmb();
}

// set video segment to 2x4 bit color attribute per 8x8 pixel sample graphics
//   data = pointer to data buffer with mono pixels
//   attr = pointer to color attributes
//   pal = pointer to 16-color palette table
//   wb = pitch - number of bytes between lines
// To scroll image, set virtual dimension wrapx and wrapy, then shift offx and offy.
void ScreenSegmAttrib8(sSegm* segm, const void* data, const void* attr, const u8* pal, int wb)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)attr;
	segm->par2 = (u32)pal;
	segm->wb = wb;
	dmb();
	segm->form = GF_ATTRIB8;
	dmb();
}

// set video segment to horizontal progress indicator GF_PROGRESS
//   data = pointer to buffer with values 0..255 of 4-pixels in rows
//   sample1 = scanline sample < data
//   sample2 = scanline sample >= data
void ScreenSegmProgress(sSegm* segm, const void* data, const void* sample1, const void* sample2)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->par = (u32)sample1;
	segm->par2 = (u32)sample2;
	dmb();
	segm->form = GF_PROGRESS;
	dmb();
}

// set video segment to 8-bit graphics with 2D matrix transformation
//   data = pointer to image data (width and height of image must be power of 2)
//   mat = pointer to array of 6 matrix integer parameters m11, m12...m23 (exported with ExportInt function)
//   xbits = number of bits of image width (image width must be power of 2 and must be = pitch width bytes)
//   ybits = number of bits of image height (image height must be power of 2)
// Use default settings of parameters: offx = 0, offy = 0, wrapx = segment width, wrapy = segment height
void ScreenSegmGraph8Mat(sSegm* segm, const void* data, const int* mat, u16 xbits, u16 ybits)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->wb = (1<<xbits);
	segm->offx = 0;
	segm->offy = 0;
	segm->wrapx = segm->width;
	segm->par = (u32)mat;
	segm->par2 = xbits | ((u32)ybits << 16);
	dmb();
	segm->form = GF_GRAPH8MAT;
	dmb();
}

// set video segment to 8-bit graphics with perspective projection
//   data = pointer to image data (width and height of image must be power of 2)
//   mat = pointer to array of 6 matrix integer parameters m11, m12...m23 (exported with ExportInt function)
//   xbits = number of bits of image width (image width must be power of 2 and must be = pitch width bytes)
//   ybits = number of bits of image height (image height must be power of 2)
//   horiz = horizon offset
// Use default settings of parameters: offx = 0, offy = 0, wrapx = segment width, wrapy = segment height
void ScreenSegmGraph8Persp(sSegm* segm, const void* data, const int* mat, u16 xbits, u16 ybits, u16 horiz)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = data;
	segm->wb = (1<<xbits);
	segm->offx = 0;
	segm->offy = 0;
	segm->wrapx = segm->width;
	segm->par = (u32)mat;
	segm->par2 = xbits | ((u32)ybits << 16);
	segm->par3 = horiz;
	dmb();
	segm->form = GF_GRAPH8PERSP;
	dmb();
}

// set video segment to tiles with perspective
//   map = pointer to tile map with tile indices (width and height must be power of 2)
//   tiles = pointer to 1 column of square tiles, 1 pixel = 8 bits (width and height must be power of 2)
//   mat = pointer to array of 6 matrix integer parameters m11, m12...m23 (exported with ExportInt function)
//   mapwbits = number of bits of tile map width
//   maphbits = number of bits of tile map height
//   tilebits = number of bits of tile width and height
//   horizon = horizon offset/4 (0=do not use perspective projection, <0=vertical flip to display ceiling)
// Use default settings of parameters: offx = 0, offy = 0, wrapx = segment width, wrapy = segment height
void ScreenSegmTilePersp(sSegm* segm, const u8* map, const u8* tiles, const int* mat, 
	u8 mapwbits, u8 maphbits, u8 tilebits, s8 horizon)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = map;
	segm->wb = mapwbits | ((u16)maphbits<<8);
	segm->offx = 0;
	segm->offy = 0;
	segm->wrapx = segm->width;
	segm->par = (u32)tiles;
	segm->par2 = (u32)mat;
	segm->par3 = tilebits | ((u16)horizon<<8);
	dmb();
	segm->form = GF_TILEPERSP;
	dmb();
}

// set video segment to tiles with perspective, 1.5 pixels
//   map = pointer to tile map with tile indices (width and height must be power of 2)
//   tiles = pointer to 1 column of square tiles, 1 pixel = 8 bits (width and height must be power of 2)
//   mat = pointer to array of 6 matrix integer parameters m11, m12...m23 (exported with ExportInt function)
//   mapwbits = number of bits of tile map width
//   maphbits = number of bits of tile map height
//   tilebits = number of bits of tile width and height
//   horizon = horizon offset/4 (0=do not use perspective projection, <0=vertical flip to display ceiling)
// Use default settings of parameters: offx = 0, offy = 0, wrapx = segment width, wrapy = segment height
void ScreenSegmTilePersp15(sSegm* segm, const u8* map, const u8* tiles, const int* mat, 
	u8 mapwbits, u8 maphbits, u8 tilebits, s8 horizon)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = map;
	segm->wb = mapwbits | ((u16)maphbits<<8);
	segm->offx = 0;
	segm->offy = 0;
	segm->wrapx = segm->width;
	segm->par = (u32)tiles;
	segm->par2 = (u32)mat;
	segm->par3 = tilebits | ((u16)horizon<<8);
	dmb();
	segm->form = GF_TILEPERSP15;
	dmb();
}

// set video segment to tiles with perspective, double pixels
//   map = pointer to tile map with tile indices (width and height must be power of 2)
//   tiles = pointer to 1 column of square tiles, 1 pixel = 8 bits (width and height must be power of 2)
//   mat = pointer to array of 6 matrix integer parameters m11, m12...m23 (exported with ExportInt function)
//   mapwbits = number of bits of tile map width
//   maphbits = number of bits of tile map height
//   tilebits = number of bits of tile width and height
//   horizon = horizon offset/4 (0=do not use perspective projection, <0=vertical flip to display ceiling)
// Use default settings of parameters: offx = 0, offy = 0, wrapx = segment width, wrapy = segment height
void ScreenSegmTilePersp2(sSegm* segm, const u8* map, const u8* tiles, const int* mat, 
	u8 mapwbits, u8 maphbits, u8 tilebits, s8 horizon)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = map;
	segm->wb = mapwbits | ((u16)maphbits<<8);
	segm->offx = 0;
	segm->offy = 0;
	segm->wrapx = segm->width;
	segm->par = (u32)tiles;
	segm->par2 = (u32)mat;
	segm->par3 = tilebits | ((u16)horizon<<8);
	dmb();
	segm->form = GF_TILEPERSP2;
	dmb();
}

// set video segment to tiles with perspective, triple pixels
//   map = pointer to tile map with tile indices (width and height must be power of 2)
//   tiles = pointer to 1 column of square tiles, 1 pixel = 8 bits (width and height must be power of 2)
//   mat = pointer to array of 6 matrix integer parameters m11, m12...m23 (exported with ExportInt function)
//   mapwbits = number of bits of tile map width
//   maphbits = number of bits of tile map height
//   tilebits = number of bits of tile width and height
//   horizon = horizon offset/4 (0=do not use perspective projection, <0=vertical flip to display ceiling)
// Use default settings of parameters: offx = 0, offy = 0, wrapx = segment width, wrapy = segment height
void ScreenSegmTilePersp3(sSegm* segm, const u8* map, const u8* tiles, const int* mat, 
	u8 mapwbits, u8 maphbits, u8 tilebits, s8 horizon)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = map;
	segm->wb = mapwbits | ((u16)maphbits<<8);
	segm->offx = 0;
	segm->offy = 0;
	segm->wrapx = segm->width;
	segm->par = (u32)tiles;
	segm->par2 = (u32)mat;
	segm->par3 = tilebits | ((u16)horizon<<8);
	dmb();
	segm->form = GF_TILEPERSP3;
	dmb();
}

// set video segment to tiles with perspective, quadruple pixels
//   map = pointer to tile map with tile indices (width and height must be power of 2)
//   tiles = pointer to 1 column of square tiles, 1 pixel = 8 bits (width and height must be power of 2)
//   mat = pointer to array of 6 matrix integer parameters m11, m12...m23 (exported with ExportInt function)
//   mapwbits = number of bits of tile map width
//   maphbits = number of bits of tile map height
//   tilebits = number of bits of tile width and height
//   horizon = horizon offset/4 (0=do not use perspective projection, <0=vertical flip to display ceiling)
// Use default settings of parameters: offx = 0, offy = 0, wrapx = segment width, wrapy = segment height
void ScreenSegmTilePersp4(sSegm* segm, const u8* map, const u8* tiles, const int* mat, 
	u8 mapwbits, u8 maphbits, u8 tilebits, s8 horizon)
{
	segm->form = GF_COLOR;
	dmb();
	segm->data = map;
	segm->wb = mapwbits | ((u16)maphbits<<8);
	segm->offx = 0;
	segm->offy = 0;
	segm->wrapx = segm->width;
	segm->par = (u32)tiles;
	segm->par2 = (u32)mat;
	segm->par3 = tilebits | ((u16)horizon<<8);
	dmb();
	segm->form = GF_TILEPERSP4;
	dmb();
}

#endif // USE_PICOVGA8		// use PicoVGA 8-bit display
