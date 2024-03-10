
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
extern const u8 Loop001Snd[97772];
extern const u8 Loop002Snd[59208];
extern const u8 Loop003Snd[37792];
extern const u8 Loop004Snd[76124];
extern const u8 Loop005Snd[30108];
extern const u8 Loop006Snd[77788];
extern const u8 Loop007Snd[42972];
extern const u8 Loop008Snd[44408];
extern const u8 Loop009Snd[78944];
extern const u8 Loop010Snd[38064];
extern const u8 Loop011Snd[48444];
extern const u8 Loop012Snd[139008];
extern const u8 Loop013Snd[73500];
extern const u8 Loop014Snd[26916];
extern const u8 Loop015Snd[76124];
extern const u8 Loop016Snd[177616];
extern const u8 Loop017Snd[42292];
extern const u8 Loop018Snd[44408];
extern const u8 Loop019Snd[40992];
extern const u8 Loop020Snd[39184];
extern const u8 Loop021Snd[79532];
extern const u8 Loop022Snd[56092];
extern const u8 Loop023Snd[83260];
extern const u8 Loop024Snd[31348];
extern const u8 Loop025Snd[84584];
extern const u8 Loop026Snd[54376];
extern const u8 Loop027Snd[76124];
extern const u8 Loop028Snd[38064];
extern const u8 Loop029Snd[76124];
extern const u8 Loop030Snd[40068];

#define IMA_SAMPBLOCK	1017	// number of samples per block
#define LOOP_NUM	30	// number of music loops
#define LOOP_FIRST	1	// index of first music loop

#endif // _MAIN_H
