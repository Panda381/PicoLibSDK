
// ****************************************************************************
//
//                               Actor Missiles
//
// ****************************************************************************

#define MISSILE_SPEEDX_MIN	5		// missile speed in X direction - min
#define MISSILE_SPEEDX_MID	10		// missile speed in X direction - middle
#define MISSILE_SPEEDX_MAX	20		// missile speed in X direction - max

#define MISSILE_SPEEDY_MIN	2		// missile speed in Y direction - min
#define MISSILE_SPEEDY_MID	4		// missile speed in Y direction - middle
#define MISSILE_SPEEDY_MAX	10		// missile speed in Y direction - max

#define MISSILE_SPEED_FLASH0	10		// missile speed of flash of Stingray
#define MISSILE_SPEED_FLASH1	9		// *cos(22.5)
#define MISSILE_SPEED_FLASH2	7		// *cos(45)
#define MISSILE_SPEED_FLASH3	4		// *cos(67.5)

#define MISSILE_SPEED_BLACKHOLE	5		// black hole speed

#define MISSILE_LIFETIME_FLASH	6		// flash lifetime
#define MISSILE_LIFETIME_SCREAM	12		// scream lifetime

// actor missile template
typedef struct {
	const u16*	pal;		// palettes
	const u8*	img;		// 4-bit image
	const u8*	sound;		// sound
	int		soundlen;	// sound length
	int		w;		// image width
	int		wall;		// total width
	int		h;		// image height
	COLTYPE		trans;		// transparent color
	int		phasenum;	// number of animation phases
	int		reload;		// number of frames to reload weapon
	int		hits;		// number of hits
	Bool		bounce;		// bounce from edges
} sMissileTemp;

// actor missile templates
extern const sMissileTemp MissileTemp[ACT_NUM];

// actor missile descriptor
typedef struct {
	const sMissileTemp*	temp;	// missile template (NULL = not used)
	int		x, y;		// missile coordinate
	int		dx, dy;		// missile coordinate increment
	int		actinx;		// actor index
	int		phase;		// animation phase
	int		lifetime;	// lifetime counter
	int		hits;		// hit counter
	sEnemy*		target;		// target enemy (NULL = none)
} sMissile;

// actor missile list
#define MISSILE_MAX	30	// max. missiles
extern sMissile Missile[MISSILE_MAX];

// 00 Shuriken
// format: 4-bit paletted pixel graphics
// image width: 128 pixels
// image height: 32 lines
// image pitch: 64 bytes
extern const u16 ShurikenImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 ShurikenImg[2048] __attribute__ ((aligned(4)));
#define SHURIKEN_W	32		// shuriken width
#define SHURIKEN_WALL	128		// shuriken total width
#define SHURIKEN_H	32		// shuriken height
#define SHURIKEN_TRANS	COL_MAGENTA	// shuriken transparent color

// 01 Seed
// format: 4-bit paletted pixel graphics
// image width: 8 pixels
// image height: 8 lines
// image pitch: 4 bytes
extern const u16 SeedImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 SeedImg[32] __attribute__ ((aligned(4)));
#define SEED_W		8		// seed width
#define SEED_WALL	8		// seed total width
#define SEED_H		8		// seed height
#define SEED_TRANS	COL_MAGENTA	// seed transparent color

// 02 Berry
// format: 4-bit paletted pixel graphics
// image width: 8 pixels
// image height: 8 lines
// image pitch: 4 bytes
extern const u16 BerryImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 BerryImg[32] __attribute__ ((aligned(4)));
#define BERRY_W		8		// berry width
#define BERRY_WALL	8		// berry total width
#define BERRY_H		8		// berry height
#define BERRY_TRANS	COL_MAGENTA	// berry transparent color

// 03 Sand Ball
// format: 4-bit paletted pixel graphics
// image width: 32 pixels
// image height: 16 lines
// image pitch: 16 bytes
extern const u16 SandballImg_Pal[6] __attribute__ ((aligned(4)));
extern const u8 SandballImg[256] __attribute__ ((aligned(4)));
#define SANDBALL_W		16		// sand ball width
#define SANDBALL_WALL		32		// sand ball total width
#define SANDBALL_H		16		// sand ball height
#define SANDBALL_TRANS		COL_MAGENTA	// sand ball transparent color

// 04 Flash
// format: 4-bit paletted pixel graphics
// image width: 32 pixels
// image height: 16 lines
// image pitch: 16 bytes
extern const u16 FlashImg_Pal[7] __attribute__ ((aligned(4)));
extern const u8 FlashImg[256] __attribute__ ((aligned(4)));
#define FLASH_W			16		// flash width
#define FLASH_WALL		32		// flash total width
#define FLASH_H			16		// flash height
#define FLASH_TRANS		COL_MAGENTA	// flash transparent color

// 05 Crystal
// format: 4-bit paletted pixel graphics
// image width: 16 pixels
// image height: 16 lines
// image pitch: 8 bytes
extern const u16 CrystalImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 CrystalImg[128] __attribute__ ((aligned(4)));
#define CRYSTAL_W		16		// crystal width
#define CRYSTAL_WALL		16		// crystal total width
#define CRYSTAL_H		16		// crystal height
#define CRYSTAL_TRANS		COL_MAGENTA	// crystal transparent color

// 06 Fire Ball
// format: 4-bit paletted pixel graphics
// image width: 64 pixels
// image height: 16 lines
// image pitch: 32 bytes
extern const u16 FireImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 FireImg[512] __attribute__ ((aligned(4)));
#define FIRE_W			16		// fire ball width
#define FIRE_WALL		64		// fire ball total width
#define FIRE_H			16		// fire ball height
#define FIRE_TRANS		COL_MAGENTA	// fire ball transparent color

// 07 Screaming
// format: 4-bit paletted pixel graphics
// image width: 8 pixels
// image height: 16 lines
// image pitch: 4 bytes
extern const u16 ScreamImg_Pal[4] __attribute__ ((aligned(4)));
extern const u8 ScreamImg[64] __attribute__ ((aligned(4)));
#define SCREAM_W		8		// screaming width
#define SCREAM_WALL		8		// screaming total width
#define SCREAM_H		16		// screaming height
#define SCREAM_TRANS		COL_MAGENTA	// screaming transparent color

// 08 Laser
// format: 4-bit paletted pixel graphics
// image width: 8 pixels
// image height: 8 lines
// image pitch: 4 bytes
extern const u16 LaserImg_Pal[5] __attribute__ ((aligned(4)));
extern const u8 LaserImg[32] __attribute__ ((aligned(4)));
#define LASER_W			8		// laser width
#define LASER_WALL		8		// laser total width
#define LASER_H			8		// laser height
#define LASER_TRANS		COL_MAGENTA	// laser transparent color

// 09 Phaser
// format: 4-bit paletted pixel graphics
// image width: 8 pixels
// image height: 8 lines
// image pitch: 4 bytes
extern const u16 PhaserImg_Pal[5] __attribute__ ((aligned(4)));
extern const u8 PhaserImg[32] __attribute__ ((aligned(4)));
#define PHASER_W		8		// phaser width
#define PHASER_WALL		8		// phaser total width
#define PHASER_H		8		// phaser height
#define PHASER_TRANS		COL_MAGENTA	// phaser transparent color

// 10 Rocket
// format: 4-bit paletted pixel graphics
// image width: 64 pixels
// image height: 16 lines
// image pitch: 32 bytes
extern const u16 RocketImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 RocketImg[512] __attribute__ ((aligned(4)));
#define ROCKET_W		32		// rocket width
#define ROCKET_WALL		64		// rocket total width
#define ROCKET_H		16		// rocket height
#define ROCKET_TRANS		COL_MAGENTA	// rocket transparent color

// 11 Sugar Bomb
// format: 4-bit paletted pixel graphics
// image width: 64 pixels
// image height: 16 lines
// image pitch: 32 bytes
extern const u16 SugarbombImg_Pal[16] __attribute__ ((aligned(4)));
extern const u8 SugarbombImg[512] __attribute__ ((aligned(4)));
#define SUGAR_W			16		// sugar bomb width
#define SUGAR_WALL		64		// sugar bomb total width
#define SUGAR_H			16		// sugar bomb height
#define SUGAR_TRANS		COL_MAGENTA	// sugar bomb transparent color

// 12 Hole of Nothingness
// format: 4-bit paletted pixel graphics
// image width: 192 pixels
// image height: 32 lines
// image pitch: 96 bytes
extern const u16 SpiralImg_Pal[8] __attribute__ ((aligned(4)));
extern const u8 SpiralImg[3072] __attribute__ ((aligned(4)));
#define SPIRAL_W		32		// spiral width
#define SPIRAL_WALL		192		// spiral total width
#define SPIRAL_H		32		// spiral height
#define SPIRAL_TRANS		COL_MAGENTA	// spiral transparent color

// initialize actor missiles on new level
void InitMissile();

// add new missile
void AddMissile(const sMissileTemp* temp, int actinx, int x, int y, int dx, int dy, sEnemy* target);

// shift missiles
void MissileShift();

// display missiles
void MissileDisp();

// generate missile
void GenMissile();
