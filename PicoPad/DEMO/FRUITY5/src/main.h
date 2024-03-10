
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 4-bit paletted pixel graphics
// image width: 233 pixels
// image height: 240 lines
// image pitch: 117 bytes
#define LOGO_W	233
#define LOGO_H	240
extern const u16 LogoImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 LogoImg[28080] __attribute__ ((aligned(4)));

// sound format: Intel IMA ADPCM mono 4-bit 22050Hz
extern const u8 Loop089Snd[150100];
extern const u8 Loop090Snd[101500];
extern const u8 Loop091Snd[134900];
extern const u8 Loop092Snd[173284];
extern const u8 Loop093Snd[76124];
extern const u8 Loop094Snd[76124];
extern const u8 Loop095Snd[76124];
extern const u8 Loop096Snd[22204];
extern const u8 Loop097Snd[88812];
extern const u8 Loop098Snd[114592];
extern const u8 Loop099Snd[108124];
extern const u8 Loop100Snd[107896];
extern const u8 Loop101Snd[142324];
extern const u8 Loop102Snd[73612];
extern const u8 Loop103Snd[71032];
extern const u8 Loop104Snd[71060];
extern const u8 Loop105Snd[145016];
extern const u8 Loop106Snd[142048];
extern const u8 Loop107Snd[9164];
extern const u8 Loop108Snd[77212];
extern const u8 Loop109Snd[20188];

#define IMA_SAMPBLOCK	1017	// number of samples per block
#define LOOP_NUM	21	// number of music loops
#define LOOP_FIRST	89	// index of first music loop

#endif // _MAIN_H
