
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
extern const u8 Loop071Snd[83520];
extern const u8 Loop072Snd[71280];
extern const u8 Loop073Snd[122400];
extern const u8 Loop074Snd[96840];
extern const u8 Loop075Snd[74520];
extern const u8 Loop076Snd[87840];
extern const u8 Loop077Snd[87840];
extern const u8 Loop078Snd[107280];
extern const u8 Loop079Snd[36000];
extern const u8 Loop080Snd[126360];
extern const u8 Loop081Snd[160560];
extern const u8 Loop082Snd[87840];
extern const u8 Loop083Snd[160560];
extern const u8 Loop084Snd[73440];
extern const u8 Loop085Snd[75600];
extern const u8 Loop086Snd[72360];
extern const u8 Loop087Snd[32760];
extern const u8 Loop088Snd[106200];

#define LOOP_NUM	18	// number of music loops
#define LOOP_FIRST	71	// index of first music loop

#endif // _MAIN_H
