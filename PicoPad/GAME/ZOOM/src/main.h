
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// debug switchwes (comment all switches on release version!)
//#define DEBUG_SKIP_OPEN		// skip open screen
//#define DEBUG_SKIP_ENTER		// skip entering screen
//#define DEBUG_NOCRASH			// no crash spaceship
//#define DEBUG_LIVES		3	// number of start lives 1.. (default 3)
//#define DEBUG_LEVEL		0	// start level 0.. (default 0)
//#define DEBUG_SUBLEVEL	0	// start sublevel 0..3 (default 0)
//#define DEBUG_GOALS		10	// number of goals

// sound channels
#define SNDCHAN_SHOT	2	// shooting
#define SNDCHAN_FUEL	3	// fuel indicator

// game variables
extern int	Level;		// current game level (0..)
extern int	SubLevel;	// current game sub-level (0..)

#define FRAC	6	// fraction bits of coordinates and fuel

#define HORIZON	56	// horizon Y coordinate

// format: 4-bit paletted pixel graphics
// image width: 48 pixels
// image height: 7 lines
// image pitch: 24 bytes
extern const u16 City1Img_Pal[6] __attribute__ ((aligned(4)));
extern const u8 City1Img[168] __attribute__ ((aligned(4)));
#define CITY1_IMG_W	48
#define CITY1_IMG_H	7
#define CITY1_IMG_KEY	COL_MAGENTA

// format: 4-bit paletted pixel graphics
// image width: 82 pixels
// image height: 8 lines
// image pitch: 41 bytes
extern const u16 City2Img_Pal[6] __attribute__ ((aligned(4)));
extern const u8 City2Img[328] __attribute__ ((aligned(4)));
#define CITY2_IMG_W	82
#define CITY2_IMG_H	8
#define CITY2_IMG_KEY	COL_MAGENTA

// format: 4-bit paletted pixel graphics
// image width: 104 pixels
// image height: 384 lines
// image pitch: 52 bytes
extern const u16 ExplosionImg_Pal[8] __attribute__ ((aligned(4)));
extern const u8 ExplosionImg[19968] __attribute__ ((aligned(4)));
#define EXPLOSION_IMG_W		104
#define EXPLOSION_IMG_H		48
#define EXPLOSION_IMG_KEY	COL_BLACK

// format: compressed 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 56 lines
// image pitch: 320 bytes
extern const u16 SkylineImg_Pal[60] __attribute__ ((aligned(4)));
extern const u8 SkylineImg[1507] __attribute__ ((aligned(4)));
#define SKYLINE_IMG_W	320
#define SKYLINE_IMG_H	56

// sound format: PCM mono 8-bit 22050Hz
extern const u8 EnteringSnd[78946];
extern const u8 ExplosionSnd[55067];
extern const u8 HitSnd[12308];

// delay with corrections
void Delay(int ms);

// initialize new game
void NewGame();

// entering sector
void Enter();

// display all game screen
void DispAll();

// initialize new level
void InitLevel();

// one game
void Game();

#endif // _MAIN_H
