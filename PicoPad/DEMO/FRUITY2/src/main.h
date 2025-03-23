
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 16-bit pixel graphics
// image width: 233 pixels
// image height: 240 lines
// image pitch: 468 bytes
#define LOGO_W	233
#define LOGO_W_FULL 234
#define LOGO_H	240
extern const u16 LogoImg[56160] __attribute__ ((aligned(4)));

// sound format: MP3, MPEG-1 layer 3, 80 kbps, 32 kHz
extern const u8 Loop031Snd[19800];
extern const u8 Loop032Snd[59400];
extern const u8 Loop033Snd[51840];
extern const u8 Loop034Snd[80640];
extern const u8 Loop035Snd[65880];
extern const u8 Loop036Snd[140760];
extern const u8 Loop037Snd[46440];
extern const u8 Loop038Snd[99720];
extern const u8 Loop039Snd[46800];
extern const u8 Loop040Snd[74520];
extern const u8 Loop041Snd[110520];
extern const u8 Loop042Snd[137520];
extern const u8 Loop043Snd[69120];
extern const u8 Loop044Snd[69120];
extern const u8 Loop045Snd[52920];
extern const u8 Loop046Snd[69120];
extern const u8 Loop047Snd[82800];
extern const u8 Loop048Snd[80640];
extern const u8 Loop049Snd[113760];
extern const u8 Loop050Snd[96840];

#define LOOP_NUM	20	// number of music loops
#define LOOP_FIRST	31	// index of first music loop

#endif // _MAIN_H
