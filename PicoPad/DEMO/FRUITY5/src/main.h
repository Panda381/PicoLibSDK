
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
extern const u8 Loop089Snd[136080];
extern const u8 Loop090Snd[92160];
extern const u8 Loop091Snd[122400];
extern const u8 Loop092Snd[156960];
extern const u8 Loop093Snd[69120];
extern const u8 Loop094Snd[69120];
extern const u8 Loop095Snd[69120];
extern const u8 Loop096Snd[20880];
extern const u8 Loop097Snd[80640];
extern const u8 Loop098Snd[104040];
extern const u8 Loop099Snd[98280];
extern const u8 Loop100Snd[97920];
extern const u8 Loop101Snd[128880];
extern const u8 Loop102Snd[66960];
extern const u8 Loop103Snd[64800];
extern const u8 Loop104Snd[64800];
extern const u8 Loop105Snd[131400];
extern const u8 Loop106Snd[128520];
extern const u8 Loop107Snd[9000];
extern const u8 Loop108Snd[70200];
extern const u8 Loop109Snd[19080];

#define LOOP_NUM	21	// number of music loops
#define LOOP_FIRST	89	// index of first music loop

#endif // _MAIN_H
