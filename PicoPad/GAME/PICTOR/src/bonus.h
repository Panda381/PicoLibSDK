
// ****************************************************************************
//
//                                 Bonus
//
// ****************************************************************************

#define BONUS_SCORE	200		// bonus points
#define BONUS_BIGSCORE	10000		// big bonus points
#define BONUS_BIGSCORE_TEXT	"10000"	// big bonus text
#define BONUS_BIGSCORE_LEN	5	// length of bit bonus text

// bonus index
#define BONUS_HEART	0		// heart
#define BONUS_TOP	1		// Picopad top cover
#define BONUS_CPU	2		// Picopad CPU
#define BONUS_DISP	3		// Picopad display
#define BONUS_BAT	4		// Picopad battery
#define BONUS_ON	5		// Picopad is ON

#define BONUS_NUM	6		// number of bonuses

#define BONUS_COLFIRST	1		// index of first collected bonus
#define BONUS_COLLAST	4		// index of last collected bonus

// bonus mask
#define BONUS_MASK_TOP	BIT(BONUS_TOP)	// Picopad top cover
#define BONUS_MASK_CPU	BIT(BONUS_CPU)	// Picopad CPU
#define BONUS_MASK_DISP	BIT(BONUS_DISP)	// Picopad display
#define BONUS_MASK_BAT	BIT(BONUS_BAT)	// Picopad battery

#define BONUS_MASK_ALL	(B1+B2+B3+B4)	// Picopad all components

// bonus template
typedef struct {
	const u16*	pal;		// palettes
	const u8*	img;		// 4-bit image
	int		w;		// image width
	int		wall;		// total width
	int		h;		// image height
	COLTYPE		trans;		// transparent color
} sBonusTemp;

// bonus templates
extern const sBonusTemp BonusTemp[BONUS_NUM];

// current bonus
extern int BonusMask;			// mask of collected Picopad components (BONUS_MASK_*)
extern int BonusInx;			// index of current bonus (-1 if bonus is not active)
extern const sBonusTemp* Bonus;		// current bonus template
extern int BonusX;			// coordinate X of current bonus
extern int BonusY;			// coordinate Y of current bonus
extern int BonusPhase;			// bonus animation phase (0..2)

// initialize bonus on start of next level (requires initialized EnemyRandSeed)
void InitBonus();

// shift bonus
void BonusShift();

// display bonus
void BonusDisp();

// collect bonus
void BonusCollect();

// 0: Heart
// format: 4-bit paletted pixel graphics
// image width: 16 pixels
// image height: 16 lines
// image pitch: 8 bytes
extern const u16 HeartImg_Pal[3] __attribute__ ((aligned(4)));
extern const u8 HeartImg[128] __attribute__ ((aligned(4)));
#define HEART_W			16		// width
#define HEART_WALL		16		// total width
#define HEART_H			16		// height
#define HEART_TRANS		COL_MAGENTA	// transparent color

// 1: Picopad top cover 
// format: 4-bit paletted pixel graphics
// image width: 39 pixels
// image height: 16 lines
// image pitch: 20 bytes
extern const u16 Picopad1Img_Pal[16] __attribute__ ((aligned(4)));
extern const u8 Picopad1Img[320] __attribute__ ((aligned(4)));
#define PICOPAD1_W		39		// width
#define PICOPAD1_WALL		39		// total width
#define PICOPAD1_H		16		// height
#define PICOPAD1_TRANS		COL_MAGENTA	// transparent color

// 2: Picopad CPU
// format: 4-bit paletted pixel graphics
// image width: 26 pixels
// image height: 10 lines
// image pitch: 13 bytes
extern const u16 Picopad2Img_Pal[16] __attribute__ ((aligned(4)));
extern const u8 Picopad2Img[130] __attribute__ ((aligned(4)));
#define PICOPAD2_W		26		// width
#define PICOPAD2_WALL		26		// total width
#define PICOPAD2_H		10		// height
#define PICOPAD2_TRANS		COL_MAGENTA	// transparent color

// 3: Picopad display
// format: 4-bit paletted pixel graphics
// image width: 22 pixels
// image height: 14 lines
// image pitch: 11 bytes
extern const u16 Picopad3Img_Pal[15] __attribute__ ((aligned(4)));
extern const u8 Picopad3Img[154] __attribute__ ((aligned(4)));
#define PICOPAD3_W		22		// width
#define PICOPAD3_WALL		22		// total width
#define PICOPAD3_H		14		// height
#define PICOPAD3_TRANS		COL_MAGENTA	// transparent color

// 4: Picopad battery
// format: 4-bit paletted pixel graphics
// image width: 9 pixels
// image height: 14 lines
// image pitch: 5 bytes
extern const u16 Picopad4Img_Pal[16] __attribute__ ((aligned(4)));
extern const u8 Picopad4Img[70] __attribute__ ((aligned(4)));
#define PICOPAD4_W		9		// width
#define PICOPAD4_WALL		9		// total width
#define PICOPAD4_H		14		// height
#define PICOPAD4_TRANS		COL_MAGENTA	// transparent color

// 5: Picopad complet on
// format: 4-bit paletted pixel graphics
// image width: 40 pixels
// image height: 16 lines
// image pitch: 20 bytes
extern const u16 PicopadImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 PicopadImg[320] __attribute__ ((aligned(4)));
#define PICOPAD_W		40		// width
#define PICOPAD_WALL		40		// total width
#define PICOPAD_H		16		// height
#define PICOPAD_TRANS		COL_MAGENTA	// transparent color

// Bonus
// format: 4-bit paletted pixel graphics
// image width: 192 pixels
// image height: 32 lines
// image pitch: 96 bytes
extern const u16 BonuscloudImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 BonuscloudImg[3072] __attribute__ ((aligned(4)));
#define BONUS_W			32		// width
#define BONUS_WALL		192		// total width
#define BONUS_H			32		// height
#define BONUS_TRANS		COL_MAGENTA	// transparent color
#define BONUS_PHASES		6		// number of animation phases
