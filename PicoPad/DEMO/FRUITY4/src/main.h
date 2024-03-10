
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
extern const u8 Loop071Snd[91872];
extern const u8 Loop072Snd[78364];
extern const u8 Loop073Snd[134900];
extern const u8 Loop074Snd[106572];
extern const u8 Loop075Snd[81980];
extern const u8 Loop076Snd[96884];
extern const u8 Loop077Snd[96884];
extern const u8 Loop078Snd[118412];
extern const u8 Loop079Snd[39184];
extern const u8 Loop080Snd[139312];
extern const u8 Loop081Snd[177616];
extern const u8 Loop082Snd[96884];
extern const u8 Loop083Snd[177616];
extern const u8 Loop084Snd[80736];
extern const u8 Loop085Snd[83260];
extern const u8 Loop086Snd[79532];
extern const u8 Loop087Snd[35764];
extern const u8 Loop088Snd[117112];

#define IMA_SAMPBLOCK	1017	// number of samples per block
#define LOOP_NUM	18	// number of music loops
#define LOOP_FIRST	71	// index of first music loop

#endif // _MAIN_H
