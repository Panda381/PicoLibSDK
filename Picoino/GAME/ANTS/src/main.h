
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// === images

// format: 8-bit pixel graphics
// image width: 320 pixels
// image height: 240 lines
// image pitch: 320 bytes
extern const u8 BackgroundImg[76800] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 128 pixels
// image height: 64 lines
// image pitch: 128 bytes
extern const u8 BlacksRedsImg[8192] __attribute__ ((aligned(4)));

#define BLACKSREDSWALL 128
#define BLACKSREDSW 64
#define BLACKSREDSH 32

// format: 8-bit pixel graphics
// image width: 1320 pixels
// image height: 60 lines
// image pitch: 1320 bytes
extern const u8 CardsImg[79200] __attribute__ ((aligned(4)));
#define CARDSALLW	1320
#define CARDW		40			// card width
#define CARDH		60			// card height

// format: 8-bit pixel graphics
// image width: 128 pixels
// image height: 116 lines
// image pitch: 128 bytes
extern const u8 CastleImg[14848] __attribute__ ((aligned(4)));
#define CASTLEALLW	128
#define CASTLEW		64
#define CASTLEH		116

// format: 8-bit pixel graphics
// image width: 32 pixels
// image height: 21 lines
// image pitch: 32 bytes
extern const u8 CloudImg[672] __attribute__ ((aligned(4)));
#define CLOUDW	32
#define CLOUDH	21

// format: 8-bit pixel graphics
// image width: 16 pixels
// image height: 30 lines
// image pitch: 16 bytes
extern const u8 CursorImg[480] __attribute__ ((aligned(4)));
#define CURSORW 16
#define CURSORH 30

// format: 8-bit pixel graphics
// image width: 8 pixels
// image height: 108 lines
// image pitch: 8 bytes
extern const u8 FenceImg[864] __attribute__ ((aligned(4)));
#define FENCEW	8
#define FENCEH	108

// format: 8-bit pixel graphics
// image width: 192 pixels
// image height: 20 lines
// image pitch: 192 bytes
extern const u8 FlagsImg[3840] __attribute__ ((aligned(4)));
#define FLAGSALLW	192
#define FLAGSW		24
#define FLAGSH		20

// format: 8-bit pixel graphics
// image width: 80 pixels
// image height: 16 lines
// image pitch: 80 bytes
extern const u8 GrassImg[1280] __attribute__ ((aligned(4)));
#define GRASSW 80
#define GRASSH 16
#define GRASSY (HEIGHT-CARDH-GRASSH)

// format: 8-bit pixel graphics
// image width: 240 pixels
// image height: 120 lines
// image pitch: 240 bytes
extern const u8 OpenImg[28800] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 320 pixels
// image height: 40 lines
// image pitch: 320 bytes
extern const u8 OpenSelImg[12800] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 60 pixels
// image height: 40 lines
// image pitch: 60 bytes
extern const u8 OpenSel2Img[2400] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 40 pixels
// image height: 40 lines
// image pitch: 40 bytes
extern const u8 OpenSel3Img[1600] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 40 pixels
// image height: 60 lines
// image pitch: 40 bytes
//extern const u8 ShadowImg[2400] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 44 pixels
// image height: 144 lines
// image pitch: 44 bytes
extern const u8 StateImg[6336] __attribute__ ((aligned(4)));
#define STATEW	41
#define STATEH	144
#define STATEY (36-2)
#define STATEDY (STATEH/8)	// 18

// format: 8-bit pixel graphics
// image width: 140 pixels
// image height: 53 lines
// image pitch: 140 bytes
extern const u8 TitleImg[7420] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 60 pixels
// image height: 50 lines
// image pitch: 60 bytes
extern const u8 Title2Img[3000] __attribute__ ((aligned(4)));

// format: 8-bit pixel graphics
// image width: 40 pixels
// image height: 32 lines
// image pitch: 40 bytes
extern const u8 TrumpetImg[1280] __attribute__ ((aligned(4)));
#define TRUMPETALLW	38
#define TRUMPETW	19
#define TRUMPETH	32

// format: 8-bit pixel graphics
// image width: 1024 pixels
// image height: 32 lines
// image pitch: 1024 bytes
extern const u8 WinImg[32768] __attribute__ ((aligned(4)));
#define WINALLW	1024
#define WINW	128
#define WINH	32

// === sounds
extern const u8 ApplauseSnd[29168];
extern const u8 BirdsSnd[192184];
extern const u8 BuildCastleSnd[19444];
extern const u8 BuildFenceSnd[10618];
extern const u8 CardSnd[2388];
extern const u8 CurseSnd[35080];
extern const u8 DecreaseStocksSnd[12210];
extern const u8 FanfaresSnd[59678];
extern const u8 IncreasePowerSnd[24338];
extern const u8 IncreaseStocksSnd[16588];
extern const u8 RuinCastleSnd[23724];
extern const u8 RuinFenceSnd[9134];

#endif // _MAIN_H
