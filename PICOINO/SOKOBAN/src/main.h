
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// program version and copyright
#define VERSION	"1.0"
#define COPYRIGHT "(c) 2023 Miroslav Nemecek"

// text screen
#define FONTW	8		// font width
#define FONTH	16		// font height
#define TEXTW	(WIDTH/8)	// width of text screen (=40)
#define TEXTH	(HEIGHT/FONTH)	// height of text screen (=15)

// format: 8-bit pixel graphics
// image width: 92 pixels
// image height: 4 lines
// image pitch: 92 bytes
extern const u8 Tiles4Img[368] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 184 pixels
// image height: 8 lines
// image pitch: 184 bytes
extern const u8 Tiles8Img[1472] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 276 pixels
// image height: 12 lines
// image pitch: 276 bytes
extern const u8 Tiles12Img[3312] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 368 pixels
// image height: 16 lines
// image pitch: 368 bytes
extern const u8 Tiles16Img[5888] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 460 pixels
// image height: 20 lines
// image pitch: 460 bytes
extern const u8 Tiles20Img[9200] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 552 pixels
// image height: 24 lines
// image pitch: 552 bytes
extern const u8 Tiles24Img[13248] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 644 pixels
// image height: 28 lines
// image pitch: 644 bytes
extern const u8 Tiles28Img[18032] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 736 pixels
// image height: 32 lines
// image pitch: 736 bytes
extern const u8 Tiles32Img[23552] __attribute__ ((aligned(4)));

// sound format: PCM mono 8-bit 22050Hz
extern const u8 YippeeSnd[17902];

// wait for a character
char WaitChar();

#endif // _MAIN_H
