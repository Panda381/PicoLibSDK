
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

#define FLAGW	180	// flag width
#define FLAGH	120	// flag height
#define FLAGX	9	// flag X coordinate relative to mast
#define FLAGY	45	// flag Y base coordinate

#define MASTW	12	// mast width
#define MASTH	210	// mast height
#define MASTX	55	// mast X coordinate

// format: 8-bit pixel graphics
// image width: 320 pixels
// image height: 240 lines
// image pitch: 320 bytes
extern ALIGNED const u8 CloudsImg[76800];

// format: 8-bit pixel graphics
// image width: 180 pixels
// image height: 120 lines
// image pitch: 180 bytes
extern ALIGNED const u8 FlagImg[21600];

// format: 8-bit pixel graphics
// image width: 12 pixels
// image height: 210 lines
// image pitch: 12 bytes
extern ALIGNED const u8 MastImg[2520];

#endif // _MAIN_H
