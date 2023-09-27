
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 16-bit pixel graphics
// image width: 32 pixels
// image height: 32 lines
// image pitch: 64 bytes
extern const u16 BrushImg[1024] __attribute__ ((aligned(4)));
#define BRUSH_IMG_W	32
#define BRUSH_IMG_H	32
#define BRUSH_IMG_KEY	COL_MAGENTA

// format: 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 54 lines
// image pitch: 320 bytes
extern const u16 PaletteImg_Pal[217] __attribute__ ((aligned(4)));
extern const u8 PaletteImg[17280] __attribute__ ((aligned(4)));
#define PALETTE_IMG_W	320
#define PALETTE_IMG_H	54

// format: 4-bit paletted pixel graphics
// image width: 10 pixels
// image height: 10 lines
// image pitch: 5 bytes
extern const u16 ColSelImg_Pal[3] __attribute__ ((aligned(4)));
extern const u8 ColSelImg[50] __attribute__ ((aligned(4)));
#define COLSEL_IMG_W	10
#define COLSEL_IMG_H	10
#define COLSEL_IMG_KEY	COL_MAGENTA

// format: 4-bit paletted pixel graphics
// image width: 8 pixels
// image height: 10 lines
// image pitch: 4 bytes
extern const u16 ColSel2Img_Pal[3] __attribute__ ((aligned(4)));
extern const u8 ColSel2Img[40] __attribute__ ((aligned(4)));
#define COLSEL2_IMG_W	8
#define COLSEL2_IMG_H	10
#define COLSEL2_IMG_KEY	COL_MAGENTA

// format: 4-bit paletted pixel graphics
// image width: 7 pixels
// image height: 10 lines
// image pitch: 4 bytes
extern const u16 ColSel3Img_Pal[3] __attribute__ ((aligned(4)));
extern const u8 ColSel3Img[40] __attribute__ ((aligned(4)));
#define COLSEL3_IMG_W	7
#define COLSEL3_IMG_H	10
#define COLSEL3_IMG_KEY	COL_MAGENTA

#endif // _MAIN_H
