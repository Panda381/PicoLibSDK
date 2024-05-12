
// ****************************************************************************
//
//                               Enemies
//
// ****************************************************************************

#define ENEMY_SPEED	3		// enemy speed
#define ENEMY_SLOWSPEED	1		// enemy slow speed
#define ENEMY_BORDERY	10		// enemy reserve in Y direction

#define ENEMY_HIT_DIST	20		// enemy hit distance
#define SUGAR_HIT_RADIUS 70		// sugar bomb radius

// move segment descriptor
typedef struct {
	s8		steps;		// number of frame steps (0=endless, stop mark)
	s8		dx;		// X increment
	s8		dy;		// Y increment
	s8		res;		// ... reserved (align)
} sMoveSeg;

// move segment macro (n=steps, 0=endless, stop mark)
#define MOVEL(n)	{ n, -ENEMY_SPEED, 0 }			// left
#define MOVEL_SU(n)	{ n, -ENEMY_SPEED, -ENEMY_SLOWSPEED }	// left, slow up
#define MOVEL_SD(n)	{ n, -ENEMY_SPEED, ENEMY_SLOWSPEED }	// left, slow down
#define MOVELD(n)	{ n, -ENEMY_SPEED, ENEMY_SPEED }	// left down
#define MOVED(n)	{ n, 0, ENEMY_SPEED }			// down
#define MOVERD(n)	{ n, ENEMY_SPEED, ENEMY_SPEED }		// right down
#define MOVER(n)	{ n, ENEMY_SPEED, 0 }			// right
#define MOVERU(n)	{ n, ENEMY_SPEED, -ENEMY_SPEED }	// right up
#define MOVEU(n)	{ n, 0, -ENEMY_SPEED }			// up
#define MOVELU(n)	{ n, -ENEMY_SPEED, -ENEMY_SPEED }	// left up

// move templates
extern const sMoveSeg* Moves[];

// enemy template descriptor
typedef struct {
	const char*	name;		// enemy name
	const u16*	pal;		// palettes
	const u8*	img;		// image
	int		w;		// image width
	int		wall;		// total width
	int		h;		// image height
	COLTYPE		trans;		// transparent color
	int		animmax;	// max. value of animation phase (before shift)
	int		animshift;	// shift of animation phase counter
	int		animmask;	// mask of animation phase after shift
} sEnemyTemp;

// enemy score
extern const s8 EnemyScore[3];

extern const sEnemyTemp	EnemyTemp[ENEMY_NUM]; // enemy templates
extern sEnemyTemp	EnemyNow[ENEMY_LEVNUM]; // current enemy template

// enemy descriptor
typedef struct {
	const sEnemyTemp* temp;		// enemy template, NULL = not used entry
	int		x, y;		// enemy coordinates - middle
	int		anim;		// animation phase
	const sMoveSeg*	move;		// pointer to move segments
	int		step;		// current step in move segment
	int		inx;		// enemy index (0..2)
} sEnemy;

extern sEnemy Enemy[ENEMY_MAX];		// enemies

// explosion descriptor
typedef struct {
	int		anim;		// animation phase (EXPLOSION_PHASES = not used)
	int		x, y;		// explosion coordinates - middle
} sExplosion;

#define EXPLOSIONS_MAX	20		// max. explosions

extern sExplosion Explosion[EXPLOSIONS_MAX];	// explosions

// Explosion
// format: 4-bit paletted pixel graphics
// image width: 256 pixels
// image height: 32 lines
// image pitch: 128 bytes
extern const u16 ExplosionImg_Pal[6] __attribute__ ((aligned(4)));
extern const u8 ExplosionImg[4096] __attribute__ ((aligned(4)));
#define EXLPOSION_W		32		// explosion width
#define EXLPOSION_WALL		256		// explosion total width
#define EXLPOSION_H		32		// explosion height
#define EXLPOSION_TRANS		COL_MAGENTA	// explosion transparent color

#define EXPLOSION_PHASES	8		// number of phases

// 01 Fly
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 27 lines
// image pitch: 100 bytes
extern const u16 FlyImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 FlyImg[2700] __attribute__ ((aligned(4)));
#define FLY_W		50		// fly width
#define FLY_WALL	100		// fly total width
#define FLY_H		27		// fly height
#define FLY_TRANS	COL_MAGENTA	// fly transparent color

// 01 Wasp
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 33 lines
// image pitch: 100 bytes
extern const u16 WaspImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 WaspImg[3300] __attribute__ ((aligned(4)));
#define WASP_W		50		// wasp width
#define WASP_WALL	100		// wasp total width
#define WASP_H		33		// wasp height
#define WASP_TRANS	COL_MAGENTA	// wasp transparent color

// 01 Hornet
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 43 lines
// image pitch: 100 bytes
extern const u16 HornetImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 HornetImg[4300] __attribute__ ((aligned(4)));
#define HORNET_W	50		// hornet width
#define HORNET_WALL	100		// hornet total width
#define HORNET_H	43		// hornet height
#define HORNET_TRANS	COL_MAGENTA	// hornet transparent color

// 02 Dragonfly
// format: 8-bit paletted pixel graphics
// image width: 120 pixels
// image height: 44 lines
// image pitch: 120 bytes
extern const u16 DragonflyImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 DragonflyImg[5280] __attribute__ ((aligned(4)));
#define DRAGONFLY_W	60		// dragonfly width
#define DRAGONFLY_WALL	120		// dragonfly total width
#define DRAGONFLY_H	44		// dragonfly height
#define DRAGONFLY_TRANS	COL_MAGENTA	// dragonfly transparent color

// 02 Moth
// format: 8-bit paletted pixel graphics
// image width: 80 pixels
// image height: 46 lines
// image pitch: 80 bytes
extern const u16 MothImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 MothImg[3680] __attribute__ ((aligned(4)));
#define MOTH_W		40		// moth width
#define MOTH_WALL	80		// moth total width
#define MOTH_H		46		// moth height
#define MOTH_TRANS	COL_MAGENTA	// moth transparent color

// 02 Kestrel
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 45 lines
// image pitch: 100 bytes
extern const u16 KestrelImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KestrelImg[4500] __attribute__ ((aligned(4)));
#define KESTREL_W	50		// kestrel width
#define KESTREL_WALL	100		// kestrel total width
#define KESTREL_H	45		// kestrel height
#define KESTREL_TRANS	COL_MAGENTA	// kestrel transparent color

// 03 Maggot
// format: 8-bit paletted pixel graphics
// image width: 37 pixels
// image height: 50 lines
// image pitch: 37 bytes
extern const u16 MaggotImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 MaggotImg[1850] __attribute__ ((aligned(4)));
#define MAGGOT_W	37		// kestrel width
#define MAGGOT_WALL	37		// kestrel total width
#define MAGGOT_H	50		// kestrel height
#define MAGGOT_TRANS	COL_MAGENTA	// kestrel transparent color

// 03 Eagle
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 53 lines
// image pitch: 100 bytes
extern const u16 EagleImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 EagleImg[5300] __attribute__ ((aligned(4)));
#define EAGLE_W		50		// eagle width
#define EAGLE_WALL	100		// eagle total width
#define EAGLE_H		53		// eagle height
#define EAGLE_TRANS	COL_MAGENTA	// eagle transparent color

// 03 Antman
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 46 lines
// image pitch: 100 bytes
extern const u16 AntmanImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 AntmanImg[4600] __attribute__ ((aligned(4)));
#define ANTMAN_W	50		// antman width
#define ANTMAN_WALL	100		// antman total width
#define ANTMAN_H	46		// antman height
#define ANTMAN_TRANS	COL_MAGENTA	// antman transparent color

// 04 Fish
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 25 lines
// image pitch: 100 bytes
extern const u16 FishImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 FishImg[2500] __attribute__ ((aligned(4)));
#define FISH_W		50		// fish width
#define FISH_WALL	100		// fish total width
#define FISH_H		25		// fish height
#define FISH_TRANS	COL_MAGENTA	// fish transparent color

// 04 Anglerfish
// format: 8-bit paletted pixel graphics
// image width: 120 pixels
// image height: 37 lines
// image pitch: 120 bytes
extern const u16 AnglerfishImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 AnglerfishImg[4440] __attribute__ ((aligned(4)));
#define ANGLERFISH_W		60		// anglerfish width
#define ANGLERFISH_WALL		120		// anglerfish total width
#define ANGLERFISH_H		37		// anglerfish height
#define ANGLERFISH_TRANS	COL_MAGENTA	// anglerfish transparent color

// 04 Shark
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 28 lines
// image pitch: 100 bytes
extern const u16 SharkImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 SharkImg[2800] __attribute__ ((aligned(4)));
#define SHARK_W		50		// shark width
#define SHARK_WALL	100		// shark total width
#define SHARK_H		25		// shark height
#define SHARK_TRANS	COL_MAGENTA	// shark transparent color

// 05 Penguin
// format: 8-bit paletted pixel graphics
// image width: 60 pixels
// image height: 35 lines
// image pitch: 60 bytes
extern const u16 PenguinImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 PenguinImg[2100] __attribute__ ((aligned(4)));
#define PENGUIN_W	30		// penguin width
#define PENGUIN_WALL	60		// penguin total width
#define PENGUIN_H	35		// penguin height
#define PENGUIN_TRANS	COL_MAGENTA	// penguin transparent color

// 05 Polar Bear
// format: 8-bit paletted pixel graphics
// image width: 120 pixels
// image height: 40 lines
// image pitch: 120 bytes
extern const u16 PolarbearImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 PolarbearImg[4800] __attribute__ ((aligned(4)));
#define POLARBEAR_W	60		// polar bear width
#define POLARBEAR_WALL	120		// polar bear total width
#define POLARBEAR_H	40		// polar bear height
#define POLARBEAR_TRANS	COL_MAGENTA	// polar bear transparent color

// 05 Snowflake
// format: 8-bit paletted pixel graphics
// image width: 30 pixels
// image height: 33 lines
// image pitch: 30 bytes
extern const u16 SnowflakeImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 SnowflakeImg[990] __attribute__ ((aligned(4)));
#define SNOWFLAKE_W	30		// snowflake width
#define SNOWFLAKE_WALL	30		// snowflake total width
#define SNOWFLAKE_H	33		// snowflake height
#define SNOWFLAKE_TRANS	COL_MAGENTA	// snowflake transparent color

// 06 Phoenix
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 35 lines
// image pitch: 100 bytes
extern const u16 PhoenixImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 PhoenixImg[3500] __attribute__ ((aligned(4)));
#define PHOENIX_W	50		// phoenix width
#define PHOENIX_WALL	100		// phoenix total width
#define PHOENIX_H	35		// phoenix height
#define PHOENIX_TRANS	COL_MAGENTA	// phoenix transparent color

// 06 Fire Ball
// format: 8-bit paletted pixel graphics
// image width: 60 pixels
// image height: 35 lines
// image pitch: 60 bytes
extern const u16 FireballImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 FireballImg[2100] __attribute__ ((aligned(4)));
#define FIREBALL_W	30		// fire ball width
#define FIREBALL_WALL	60		// fire ball total width
#define FIREBALL_H	35		// fire ball height
#define FIREBALL_TRANS	COL_MAGENTA	// fire ball transparent color

// 06 Evil
// format: 8-bit paletted pixel graphics
// image width: 120 pixels
// image height: 27 lines
// image pitch: 120 bytes
extern const u16 EvilImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 EvilImg[3240] __attribute__ ((aligned(4)));
#define EVIL_W		60		// evil width
#define EVIL_WALL	120		// evil total width
#define EVIL_H		27		// evil height
#define EVIL_TRANS	COL_MAGENTA	// evil transparent color

// 07 Pumpkin
// format: 8-bit paletted pixel graphics
// image width: 60 pixels
// image height: 28 lines
// image pitch: 60 bytes
extern const u16 PumpkinImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 PumpkinImg[1680] __attribute__ ((aligned(4)));
#define PUMPKIN_W	30		// pumpkin width
#define PUMPKIN_WALL	60		// pumpkin total width
#define PUMPKIN_H	28		// pumpkin height
#define PUMPKIN_TRANS	COL_MAGENTA	// pumpkin transparent color

// 07 Zombie
// format: 8-bit paletted pixel graphics
// image width: 30 pixels
// image height: 35 lines
// image pitch: 30 bytes
extern const u16 ZombieImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 ZombieImg[1050] __attribute__ ((aligned(4)));
#define ZOMBIE_W	30		// zombie width
#define ZOMBIE_WALL	30		// zombie total width
#define ZOMBIE_H	35		// zombie height
#define ZOMBIE_TRANS	COL_MAGENTA	// zombie transparent color

// 07 Death
// format: 8-bit paletted pixel graphics
// image width: 130 pixels
// image height: 35 lines
// image pitch: 130 bytes
extern const u16 DeathImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 DeathImg[4550] __attribute__ ((aligned(4)));
#define DEATH_W		65		// death width
#define DEATH_WALL	130		// death total width
#define DEATH_H		35		// death height
#define DEATH_TRANS	COL_MAGENTA	// death transparent color

// 08 Camera
// format: 8-bit paletted pixel graphics
// image width: 35 pixels
// image height: 16 lines
// image pitch: 35 bytes
extern const u16 CameraImg_Pal[230] __attribute__ ((aligned(4)));
extern const u8 CameraImg[560] __attribute__ ((aligned(4)));
#define CAMERA_W	35		// camera width
#define CAMERA_WALL	35		// camera total width
#define CAMERA_H	16		// camera height
#define CAMERA_TRANS	COL_MAGENTA	// camera transparent color

// 08 Explorer
// format: 8-bit paletted pixel graphics
// image width: 60 pixels
// image height: 25 lines
// image pitch: 60 bytes
extern const u16 ExplorerImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 ExplorerImg[1500] __attribute__ ((aligned(4)));
#define EXPLORER_W	30		// explorer width
#define EXPLORER_WALL	60		// explorer total width
#define EXPLORER_H	25		// explorer height
#define EXPLORER_TRANS	COL_MAGENTA	// explorer transparent color

// 08 War Dron
// format: 8-bit paletted pixel graphics
// image width: 50 pixels
// image height: 36 lines
// image pitch: 50 bytes
extern const u16 DronImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 DronImg[1800] __attribute__ ((aligned(4)));
#define DRON_W		50		// war dron width
#define DRON_WALL	50		// war dron total width
#define DRON_H		36		// war dron height
#define DRON_TRANS	COL_MAGENTA	// war dron transparent color

// 09 Starship Enterprise
// format: 8-bit paletted pixel graphics
// image width: 60 pixels
// image height: 17 lines
// image pitch: 60 bytes
extern const u16 EnterpriseImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 EnterpriseImg[1020] __attribute__ ((aligned(4)));
#define ENTERPRISE_W		60		// enterprise width
#define ENTERPRISE_WALL		60		// enterprise total width
#define ENTERPRISE_H		17		// enterprise height
#define ENTERPRISE_TRANS	COL_MAGENTA	// enterprise transparent color

// 09 Klingon Warship
// format: 8-bit paletted pixel graphics
// image width: 50 pixels
// image height: 26 lines
// image pitch: 50 bytes
extern const u16 KlingonImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KlingonImg[1300] __attribute__ ((aligned(4)));
#define KLINGON_W		50		// klingon width
#define KLINGON_WALL		50		// klingon total width
#define KLINGON_H		26		// klingon height
#define KLINGON_TRANS		COL_MAGENTA	// klingon transparent color

// 09 Death Star
// format: 8-bit paletted pixel graphics
// image width: 40 pixels
// image height: 40 lines
// image pitch: 40 bytes
extern const u16 DeathstarImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 DeathstarImg[1600] __attribute__ ((aligned(4)));
#define DEATHSTAR_W		40		// death star width
#define DEATHSTAR_WALL		40		// death star total width
#define DEATHSTAR_H		40		// death star height
#define DEATHSTAR_TRANS		COL_MAGENTA	// death star transparent color

// 10 Alien
// format: 8-bit paletted pixel graphics
// image width: 80 pixels
// image height: 33 lines
// image pitch: 80 bytes
extern const u16 AlienImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 AlienImg[2640] __attribute__ ((aligned(4)));
#define ALIEN_W			40		// alien width
#define ALIEN_WALL		80		// alien total width
#define ALIEN_H			33		// alien height
#define ALIEN_TRANS		COL_MAGENTA	// alien transparent color

// 10 Xenomorph
// format: 8-bit paletted pixel graphics
// image width: 90 pixels
// image height: 48 lines
// image pitch: 90 bytes
extern const u16 XenomorphImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 XenomorphImg[4320] __attribute__ ((aligned(4)));
#define XENOMORPH_W		45		// xenomorph width
#define XENOMORPH_WALL		90		// xenomorph total width
#define XENOMORPH_H		48		// xenomorph height
#define XENOMORPH_TRANS		COL_MAGENTA	// xenomorph transparent color

// 10 Meteor
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 27 lines
// image pitch: 100 bytes
extern const u16 MeteorImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 MeteorImg[2700] __attribute__ ((aligned(4)));
#define METEOR_W		50		// meteor width
#define METEOR_WALL		100		// meteor total width
#define METEOR_H		27		// meteor height
#define METEOR_TRANS		COL_MAGENTA	// meteor transparent color

// 11 Candy Cane
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 26 lines
// image pitch: 100 bytes
extern const u16 CandycaneImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 CandycaneImg[2600] __attribute__ ((aligned(4)));
#define CANDYCANE_W		50		// candy cane width
#define CANDYCANE_WALL		100		// candy cane total width
#define CANDYCANE_H		26		// candy cane height
#define CANDYCANE_TRANS		COL_MAGENTA	// candy cane transparent color

// 11 Lollipop
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 32 lines
// image pitch: 100 bytes
extern const u16 LollipopImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 LollipopImg[3200] __attribute__ ((aligned(4)));
#define LOLLIPOP_W		50		// lollipop width
#define LOLLIPOP_WALL		100		// lollipop total width
#define LOLLIPOP_H		32		// lollipop height
#define LOLLIPOP_TRANS		COL_MAGENTA	// lollipop transparent color

// 11 Fat Man
// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 27 lines
// image pitch: 100 bytes
extern const u16 FatmanImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 FatmanImg[2700] __attribute__ ((aligned(4)));
#define FATMAN_W		50		// fat man width
#define FATMAN_WALL		100		// fat man total width
#define FATMAN_H		27		// fat man height
#define FATMAN_TRANS		COL_MAGENTA	// fat man transparent color

// 12 Cloud
// format: 8-bit paletted pixel graphics
// image width: 35 pixels
// image height: 29 lines
// image pitch: 35 bytes
extern const u16 CloudImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 CloudImg[1015] __attribute__ ((aligned(4)));
#define CLOUD_W			35		// cloud width
#define CLOUD_WALL		35		// cloud total width
#define CLOUD_H			29		// cloud height
#define CLOUD_TRANS		COL_MAGENTA	// cloud transparent color

// 12 Star
// format: 8-bit paletted pixel graphics
// image width: 35 pixels
// image height: 32 lines
// image pitch: 35 bytes
extern const u16 StarImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 StarImg[1120] __attribute__ ((aligned(4)));
#define STAR_W			35		// star width
#define STAR_WALL		35		// star total width
#define STAR_H			32		// star height
#define STAR_TRANS		COL_MAGENTA	// star transparent color

// 12 Moon
// format: 8-bit paletted pixel graphics
// image width: 35 pixels
// image height: 36 lines
// image pitch: 35 bytes
extern const u16 MoonImg_Pal[240] __attribute__ ((aligned(4)));
extern const u8 MoonImg[1260] __attribute__ ((aligned(4)));
#define MOON_W			35		// moon width
#define MOON_WALL		35		// moon total width
#define MOON_H			36		// moon height
#define MOON_TRANS		COL_MAGENTA	// moon transparent color

// initialize enemies on new level
void InitEnemy();

// add new enemy (temp = enemy template index 0..2)
void AddEnemy(const sEnemyTemp* temp, int x, int y, const sMoveSeg* move, int inx);

// add explosion
void AddExplosion(int x, int y);

// generate new enemy
void NewEnemy();

// shift enemies
void EnemyShift();

// shift explosions
void ExplosionShift();

// display enemies
void EnemyDisp();

// display explosions
void ExplosionDisp();

// hit enemy by missile
void EnemyHit();
