
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 16-bit pixel graphics
// image width: 96 pixels
// image height: 128 lines
// image pitch: 192 bytes
extern const u16 RaspberryImg[12288] __attribute__ ((aligned(4)));

#define IMGW	96
#define	IMGH	128

#endif // _MAIN_H
