
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 8-bit pixel graphics
// image width: 320 pixels
// image height: 240 lines
// image pitch: 320 bytes
extern const u8 CloudsImg[76800] __attribute__ ((aligned(4)));
#define CLOUDSW	320	// clouds width
#define CLOUDSH	240	// clouds height

// format: 8-bit pixel graphics
// image width: 80 pixels
// image height: 104 lines
// image pitch: 80 bytes
extern const u8 HotairImg[8320] __attribute__ ((aligned(4)));
#define HOTAIRW 80	// hot-air balloon width
#define HOTAIRH	104	// hot-air balloon height
#define HOTAIR_NUM 3	// number of hot-air balloons
#define HOTAIR_MINSPEED 1 // min. move speed
#define HOTAIR_MAXSPEED 2 // max. move speed

// format: 8-bit pixel graphics
// image width: 40 pixels
// image height: 91 lines
// image pitch: 40 bytes
extern const u8 BlueImg[3640] __attribute__ ((aligned(4)));
extern const u8 GreenImg[3640] __attribute__ ((aligned(4)));
extern const u8 RedImg[3640] __attribute__ ((aligned(4)));
extern const u8 YellowImg[3640] __attribute__ ((aligned(4)));
#define BALLOONW 40	// balloon width
#define BALLOONH 91	// balloon height
#define BALLOON_NUM 20	// number of party balloons
#define BALLOON_SPEED 2	// balloon speed

#endif // _MAIN_H
