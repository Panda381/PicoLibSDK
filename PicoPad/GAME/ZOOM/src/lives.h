
// ****************************************************************************
//
//                                Lives
//
// ****************************************************************************

#ifndef _LIVES_H
#define _LIVES_H

extern int	Lives;		// number of lives

// format: 4-bit paletted pixel graphics
// image width: 22 pixels
// image height: 6 lines
// image pitch: 11 bytes
extern const u16 LiveImg_Pal[5] __attribute__ ((aligned(4)));
extern const u8 LiveImg[66] __attribute__ ((aligned(4)));
#define LIVE_IMG_W	22
#define LIVE_IMG_H	6
#define LIVE_IMG_KEY	COL_MAGENTA

extern const u8 LivesSnd[51242];

// display number of lives
void DispLives();

#endif // _LIVES_H
