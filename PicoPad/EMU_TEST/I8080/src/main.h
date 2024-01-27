
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 4-bit paletted pixel graphics
// image width: 1760 pixels
// image height: 96 lines
// image pitch: 880 bytes
extern const u16 DigitsImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 DigitsImg[84480] __attribute__ ((aligned(4)));

#define DIG_WALL	1760	// width of digit image
#define DIG_Y		(FONTH*2+16) // Y coordinate of the digits
#define DIG_W		32	// width of one digit
#define DIG_H		48	// height of one digit
#define DIG_NUM		10	// number of digits on display
#define DIG_MAX		(DIG_WALL/DIG_W-1) // max. character (= 54)
#define DIG_SPC		36	// space character

// format: 16-bit pixel graphics
// image width: 192 pixels
// image height: 48 lines
// image pitch: 384 bytes
extern const u16 LedImg[9216] __attribute__ ((aligned(4)));

#define LEDS_NUM	8	// number of LEDs
#define LEDS_W		24	// width of one LED
#define LEDS_H		24	// height of one LED
#define LEDS_DX		40	// X distance
#define LEDS_X		8	// X coordinate
#define LEDS_Y		(DIG_Y+DIG_H+16) // Y coordinate
#define LEDS_WALL	192	// width of LED image

#endif // _MAIN_H
