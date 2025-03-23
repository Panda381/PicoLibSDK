
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
extern const u8 Loop001Snd[88920];
extern const u8 Loop002Snd[54000];
extern const u8 Loop003Snd[34920];
extern const u8 Loop004Snd[69120];
extern const u8 Loop005Snd[27720];
extern const u8 Loop006Snd[70920];
extern const u8 Loop007Snd[39240];
extern const u8 Loop008Snd[40680];
extern const u8 Loop009Snd[72000];
extern const u8 Loop010Snd[34920];
extern const u8 Loop011Snd[44280];
extern const u8 Loop012Snd[126000];
extern const u8 Loop013Snd[66960];
extern const u8 Loop014Snd[24840];
extern const u8 Loop015Snd[69120];
extern const u8 Loop016Snd[160560];
extern const u8 Loop017Snd[38880];
extern const u8 Loop018Snd[40680];
extern const u8 Loop019Snd[37800];
extern const u8 Loop020Snd[36000];
extern const u8 Loop021Snd[72360];
extern const u8 Loop022Snd[51120];
extern const u8 Loop023Snd[75600];
extern const u8 Loop024Snd[28800];
extern const u8 Loop025Snd[77040];
extern const u8 Loop026Snd[49680];
extern const u8 Loop027Snd[69120];
extern const u8 Loop028Snd[34920];
extern const u8 Loop029Snd[69120];
extern const u8 Loop030Snd[36720];

#define LOOP_NUM	30	// number of music loops
#define LOOP_FIRST	1	// index of first music loop

#endif // _MAIN_H
