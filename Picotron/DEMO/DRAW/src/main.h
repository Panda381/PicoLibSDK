
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 4-bit pixel graphics
// image width: 64 pixels
// image height: 64 lines
// image pitch: 32 bytes
extern const u8 PeterImg[2048] __attribute__ ((aligned(4)));

// format: 4-bit pixel graphics
// image width: 64 pixels
// image height: 80 lines
// image pitch: 32 bytes
extern const u8 RPiImg[2560] __attribute__ ((aligned(4)));

#endif // _MAIN_H
