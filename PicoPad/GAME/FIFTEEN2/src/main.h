
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

#define TILEW	80	// tile width
#define TILEH	60	// tile height
#define TILESX	4	// game tiles in X direction (192 pixels)
#define TILESY	4	// game tiles in Y direction (192 pixels)
#define TILESNUM (TILESX*TILESY) // number of tiles (= 16)
#define TILE_EMPTY_INX	15	// empty tile index
#define TILE_EMPTY	1	// empty tile
#define TILE_WAIT	2	// wait tile
#define TILE_WAIT2	3	// wait tile 2

#define BOARDW	(TILESX*TILEW)	// board width
#define BOARDH	(TILESY*TILEH)	// board height
#define BOARDX	((WIDTH-BOARDW)/2) // board X coordinate
#define BOARDY	((HEIGHT-BOARDH)/2) // board Y coordinate

// format: 8-bit paletted pixel graphics
// image width: 80 pixels
// image height: 240 lines
// image pitch: 80 bytes
extern const u16 TilesImg_Pal[14] __attribute__ ((aligned(4)));
extern const u8 TilesImg[19200] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 240 lines
// image pitch: 320 bytes
#define IMGW	320	// image width
extern const u16 DigitsImg_Pal[21] __attribute__ ((aligned(4)));
extern const u8 DigitsImg[76800] __attribute__ ((aligned(4)));

extern const u16 GirlWithKittenImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 GirlWithKittenImg[76800] __attribute__ ((aligned(4)));

extern const u16 KittensInBoxImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KittensInBoxImg[76800] __attribute__ ((aligned(4)));

extern const u16 KittensInForestImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KittensInForestImg[76800] __attribute__ ((aligned(4)));

extern const u16 KittensOnMeadowImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KittensOnMeadowImg[76800] __attribute__ ((aligned(4)));

extern const u16 KittensWithButterfliesImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KittensWithButterfliesImg[76800] __attribute__ ((aligned(4)));

extern const u16 KittensWithFlowersImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KittensWithFlowersImg[76800] __attribute__ ((aligned(4)));

extern const u16 KittensWithSnowdropsImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KittensWithSnowdropsImg[76800] __attribute__ ((aligned(4)));

extern const u16 PegasusImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 PegasusImg[76800] __attribute__ ((aligned(4)));

extern const u16 PersianKittenImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 PersianKittenImg[76800] __attribute__ ((aligned(4)));

extern const u16 PhoenixImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 PhoenixImg[76800] __attribute__ ((aligned(4)));

extern const u16 SeaDraqonImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 SeaDraqonImg[76800] __attribute__ ((aligned(4)));

extern const u16 TwoPersianKittensImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 TwoPersianKittensImg[76800] __attribute__ ((aligned(4)));

extern const u16 TwoPuppyDogsImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 TwoPuppyDogsImg[76800] __attribute__ ((aligned(4)));

// sounds
extern const u8 MoveSnd[2354];
extern const u8 BumpSnd[1012];
extern const u8 ShufflingSnd[5620];
extern const u8 FanfareSnd[74871];

#endif // _MAIN_H
