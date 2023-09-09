
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 16-bit pixel graphics
// image width: 32 pixels
// image height: 240 lines
// image pitch: 64 bytes
extern const u16 IconImg[7680] __attribute__ ((aligned(4)));

// sound format: PCM mono 8-bit 22050Hz
extern const u8 TestSnd[154512];

#endif // _MAIN_H
