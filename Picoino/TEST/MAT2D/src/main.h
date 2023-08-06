
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 8-bit pixel graphics
// image width: 96 pixels
// image height: 128 lines
// image pitch: 96 bytes
extern const u8 RaspberryImg[12288] __attribute__ ((aligned(4)));

#define IMGW	96
#define	IMGH	128

#endif // _MAIN_H
