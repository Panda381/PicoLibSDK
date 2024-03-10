
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
extern const u8 Loop051Snd[96300];
extern const u8 Loop052Snd[32896];
extern const u8 Loop053Snd[169160];
extern const u8 Loop054Snd[24004];
extern const u8 Loop055Snd[19036];
extern const u8 Loop056Snd[24224];
extern const u8 Loop057Snd[114592];
extern const u8 Loop058Snd[114592];
extern const u8 Loop059Snd[20500];
extern const u8 Loop060Snd[12492];
extern const u8 Loop061Snd[76124];
extern const u8 Loop062Snd[76124];
extern const u8 Loop063Snd[156724];
extern const u8 Loop064Snd[67880];
extern const u8 Loop065Snd[38064];
extern const u8 Loop066Snd[76124];
extern const u8 Loop067Snd[152244];
extern const u8 Loop068Snd[42120];
extern const u8 Loop069Snd[177616];
extern const u8 Loop070Snd[142096];

#define IMA_SAMPBLOCK	1017	// number of samples per block
#define LOOP_NUM	20	// number of music loops
#define LOOP_FIRST	51	// index of first music loop

#endif // _MAIN_H
