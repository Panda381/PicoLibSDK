
// ****************************************************************************
//
//                                Opening
//
// ****************************************************************************

#ifndef _OPEN_H
#define _OPEN_H

// format: 8-bit paletted pixel graphics
// image width: 230 pixels
// image height: 72 lines
// image pitch: 230 bytes
extern const u16 TitleImg_Pal[251] __attribute__ ((aligned(4)));
extern const u8 TitleImg[16560] __attribute__ ((aligned(4)));
#define TITLE_IMG_W	230
#define TITLE_IMG_H	72
#define TITLE_IMG_KEY	COL_MAGENTA

// open screen (returns False to quit the game)
Bool Open();

#endif // _OPEN_H
