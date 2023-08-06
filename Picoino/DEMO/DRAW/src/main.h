
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 8-bit pixel graphics
// image width: 32 pixels
// image height: 40 lines
// image pitch: 32 bytes
extern const u8 RPi8Img[1280] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 32 pixels
// image height: 32 lines
// image pitch: 32 bytes
extern const u8 Peter8Img[1024] __attribute__ ((aligned(4)));

#endif // _MAIN_H
