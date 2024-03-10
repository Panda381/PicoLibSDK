
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
extern const u8 Loop031Snd[21336];
extern const u8 Loop032Snd[64984];
extern const u8 Loop033Snd[56688];
extern const u8 Loop034Snd[88812];
extern const u8 Loop035Snd[72500];
extern const u8 Loop036Snd[155576];
extern const u8 Loop037Snd[50752];
extern const u8 Loop038Snd[109868];
extern const u8 Loop039Snd[51240];
extern const u8 Loop040Snd[81980];
extern const u8 Loop041Snd[121796];
extern const u8 Loop042Snd[152244];
extern const u8 Loop043Snd[76124];
extern const u8 Loop044Snd[76124];
extern const u8 Loop045Snd[57924];
extern const u8 Loop046Snd[76124];
extern const u8 Loop047Snd[91088];
extern const u8 Loop048Snd[88812];
extern const u8 Loop049Snd[125376];
extern const u8 Loop050Snd[106572];

#define IMA_SAMPBLOCK	1017	// number of samples per block
#define LOOP_NUM	20	// number of music loops
#define LOOP_FIRST	31	// index of first music loop

#endif // _MAIN_H
