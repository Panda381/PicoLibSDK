
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
extern const u8 Loop110Snd[128880];
extern const u8 Loop111Snd[55800];
extern const u8 Loop112Snd[73440];
extern const u8 Loop113Snd[40680];
extern const u8 Loop114Snd[20520];
extern const u8 Loop115Snd[9360];
extern const u8 Loop116Snd[66240];
extern const u8 Loop117Snd[19440];
extern const u8 Loop118Snd[71640];
extern const u8 Loop119Snd[35280];
extern const u8 Loop120Snd[136800];
extern const u8 Loop121Snd[68760];
extern const u8 Loop122Snd[123120];
extern const u8 Loop123Snd[90360];
extern const u8 Loop124Snd[65880];
extern const u8 Loop125Snd[137520];
extern const u8 Loop126Snd[140040];
extern const u8 Loop127Snd[35280];
extern const u8 Loop128Snd[69480];
extern const u8 Loop129Snd[16920];
extern const u8 Loop130Snd[24120];

#define LOOP_NUM	21	// number of music loops
#define LOOP_FIRST	110	// index of first music loop

#endif // _MAIN_H
