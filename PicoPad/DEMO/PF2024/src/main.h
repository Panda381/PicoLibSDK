
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

#define TRANS_COL	0x9111		// transparent color (0x95228c, R=149, G=34, B=140)

// format: compressed 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 240 lines
// image pitch: 320 bytes
extern const u16 BackImg_Pal[249] __attribute__ ((aligned(4)));
extern const u8 BackImg[16048] __attribute__ ((aligned(4)));
#define BACKIMG_W	320
#define BACKIMG_H	240
#define BACKIMG_TW	320

// format: 4-bit paletted pixel graphics
// image width: 46 pixels
// image height: 16 lines
// image pitch: 23 bytes
extern const u16 BirdImg_Pal[6] __attribute__ ((aligned(4)));
extern const u8 BirdImg[368] __attribute__ ((aligned(4)));
#define BIRDIMG_W	23
#define BIRDIMG_H	16
#define BIRDIMG_TW	46

// format: 4-bit paletted pixel graphics
// image width: 82 pixels
// image height: 55 lines
// image pitch: 41 bytes
extern const u16 CatImg_Pal[9] __attribute__ ((aligned(4)));
extern const u8 CatImg[2255] __attribute__ ((aligned(4)));
#define CATIMG_W	41
#define CATIMG_H	55
#define CATIMG_TW	82

// format: 4-bit paletted pixel graphics
// image width: 30 pixels
// image height: 9 lines
// image pitch: 15 bytes
extern const u16 CometImg_Pal[3] __attribute__ ((aligned(4)));
extern const u8 CometImg[135] __attribute__ ((aligned(4)));
#define COMETIMG_W	15
#define COMETIMG_H	9
#define COMETIMG_TW	30

// format: 4-bit paletted pixel graphics
// image width: 200 pixels
// image height: 24 lines
// image pitch: 100 bytes
extern const u16 DigitImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 DigitImg[2400] __attribute__ ((aligned(4)));
#define DIGITIMG_W	20
#define DIGITIMG_H	24
#define DIGITIMG_TW	200

// format: 4-bit paletted pixel graphics
// image width: 156 pixels
// image height: 57 lines
// image pitch: 78 bytes
extern const u16 DogImg_Pal[6] __attribute__ ((aligned(4)));
extern const u8 DogImg[4446] __attribute__ ((aligned(4)));
#define DOGIMG_W	52
#define DOGIMG_H	57
#define DOGIMG_TW	156

// format: 4-bit paletted pixel graphics
// image width: 96 pixels
// image height: 48 lines
// image pitch: 48 bytes
extern const u16 FrameImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 FrameImg[2304] __attribute__ ((aligned(4)));
#define FRAMEIMG_W	96
#define FRAMEIMG_H	48
#define FRAMEIMG_TW	96

#define FRAMEIMG_INX	10
#define FRAMEIMG_INY	10
#define FRAMEIMG_INW	76
#define FRAMEIMG_INH	28

// format: 4-bit paletted pixel graphics
// image width: 111 pixels
// image height: 61 lines
// image pitch: 56 bytes
extern const u16 HareImg_Pal[7] __attribute__ ((aligned(4)));
extern const u8 HareImg[3416] __attribute__ ((aligned(4)));
#define HAREIMG_W	37
#define HAREIMG_H	61
#define HAREIMG_TW	112

// format: 4-bit paletted pixel graphics
// image width: 120 pixels
// image height: 66 lines
// image pitch: 60 bytes
extern const u16 MoleImg_Pal[7] __attribute__ ((aligned(4)));
extern const u8 MoleImg[3960] __attribute__ ((aligned(4)));
#define MOLEIMG_W	40
#define MOLEIMG_H	66
#define MOLEIMG_TW	120

// format: 4-bit paletted pixel graphics
// image width: 96 pixels
// image height: 38 lines
// image pitch: 48 bytes
extern const u16 MoonImg_Pal[13] __attribute__ ((aligned(4)));
extern const u8 MoonImg[1824] __attribute__ ((aligned(4)));
#define MOONIMG_W	48
#define MOONIMG_H	38
#define MOONIMG_TW	96

// format: 4-bit paletted pixel graphics
// image width: 117 pixels
// image height: 49 lines
// image pitch: 59 bytes
extern const u16 MouseImg_Pal[9] __attribute__ ((aligned(4)));
extern const u8 MouseImg[2891] __attribute__ ((aligned(4)));
#define MOUSEIMG_W	39
#define MOUSEIMG_H	49
#define MOUSEIMG_TW	118

// format: 4-bit paletted pixel graphics
// image width: 120 pixels
// image height: 24 lines
// image pitch: 60 bytes
extern const u16 PfAllImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 PfAllImg[1440] __attribute__ ((aligned(4)));
#define PFALLIMG_W	120
#define PFALLIMG_H	24
#define PFALLIMG_TW	120

// format: 4-bit paletted pixel graphics
// image width: 36 pixels
// image height: 24 lines
// image pitch: 18 bytes
extern const u16 PfImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 PfImg[432] __attribute__ ((aligned(4)));
#define PFIMG_W		36
#define PFIMG_H		24
#define PFIMG_TW	36

// format: 4-bit paletted pixel graphics
// image width: 33 pixels
// image height: 11 lines
// image pitch: 17 bytes
extern const u16 SparkImg_Pal[3] __attribute__ ((aligned(4)));
extern const u8 SparkImg[187] __attribute__ ((aligned(4)));
#define SPARKIMG_W	11
#define SPARKIMG_H	11
#define SPARKIMG_TW	34

// format: 4-bit paletted pixel graphics
// image width: 33 pixels
// image height: 11 lines
// image pitch: 17 bytes
extern const u16 StarImg_Pal[3] __attribute__ ((aligned(4)));
extern const u8 StarImg[187] __attribute__ ((aligned(4)));
#define STARIMG_W	11
#define STARIMG_H	11
#define STARIMG_TW	34

// sound format: PCM mono 8-bit 22050Hz
//extern const u8 MusicSnd[1762609];

// sound format: Intel IMA ADPCM mono 4-bit 22050Hz
extern const u16 MusicSnd_SampBlock;
extern const u8 MusicSnd[887808]; // audacity

#endif // _MAIN_H
