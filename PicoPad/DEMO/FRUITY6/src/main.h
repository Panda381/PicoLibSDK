
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
extern const u8 Loop110Snd[142096];
extern const u8 Loop111Snd[61336];
extern const u8 Loop112Snd[80744];
extern const u8 Loop113Snd[44216];
extern const u8 Loop114Snd[22112];
extern const u8 Loop115Snd[9412];
extern const u8 Loop116Snd[72688];
extern const u8 Loop117Snd[20880];
extern const u8 Loop118Snd[78716];
extern const u8 Loop119Snd[38356];
extern const u8 Loop120Snd[150972];
extern const u8 Loop121Snd[75504];
extern const u8 Loop122Snd[135928];
extern const u8 Loop123Snd[99616];
extern const u8 Loop124Snd[72512];
extern const u8 Loop125Snd[152048];
extern const u8 Loop126Snd[154668];
extern const u8 Loop127Snd[38352];
extern const u8 Loop128Snd[76244];
extern const u8 Loop129Snd[18152];
extern const u8 Loop130Snd[25900];

#define IMA_SAMPBLOCK	1017	// number of samples per block
#define LOOP_NUM	21	// number of music loops
#define LOOP_FIRST	110	// index of first music loop

#endif // _MAIN_H
