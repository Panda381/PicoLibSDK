
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
extern const u8 Loop051Snd[87480];
extern const u8 Loop052Snd[30240];
extern const u8 Loop053Snd[153000];
extern const u8 Loop054Snd[22320];
extern const u8 Loop055Snd[18000];
extern const u8 Loop056Snd[22680];
extern const u8 Loop057Snd[104040];
extern const u8 Loop058Snd[104040];
extern const u8 Loop059Snd[19080];
extern const u8 Loop060Snd[11880];
extern const u8 Loop061Snd[69120];
extern const u8 Loop062Snd[69120];
extern const u8 Loop063Snd[141840];
extern const u8 Loop064Snd[61920];
extern const u8 Loop065Snd[34920];
extern const u8 Loop066Snd[69120];
extern const u8 Loop067Snd[137880];
extern const u8 Loop068Snd[38520];
extern const u8 Loop069Snd[160560];
extern const u8 Loop070Snd[128880];

#define LOOP_NUM	20	// number of music loops
#define LOOP_FIRST	51	// index of first music loop

#endif // _MAIN_H
