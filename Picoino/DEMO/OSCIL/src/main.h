
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

#define GRIDX	33
#define GRIDY	53
#define GRIDW	131
#define GRIDH	90
#define GRIDWB	((GRIDW+3)&~3)

// format: 8-bit pixel graphics
// image width: 132 pixels
// image height: 90 lines
// image pitch: 132 bytes
extern const u8 GridImg[11880] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 320 pixels
// image height: 240 lines
// image pitch: 320 bytes
extern const u8 OscilImg[76800] __attribute__ ((aligned(4)));

#endif // _MAIN_H
