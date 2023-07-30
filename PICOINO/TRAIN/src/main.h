
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 8-bit pixel graphics
// image width: 320 pixels
// image height: 176 lines
// image pitch: 320 bytes
extern const u8 TilesImg[56320] __attribute__ ((aligned(4)));
#define TILESIMGW	320
#define TILESNUMX	20	// number of tiles in X direction

// sounds
extern const u8 CollectSnd[3642];
extern const u8 CrashSnd[50146];
extern const u8 StepSnd[2243];
extern const u8 SuccessSnd[32784];

#endif // _MAIN_H
