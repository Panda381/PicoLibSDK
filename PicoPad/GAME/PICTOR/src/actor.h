
// ****************************************************************************
//
//                                  Actors
//
// ****************************************************************************

// actor indices
#define ACT_JILL	0	// Jill's index
#define ACT_BIRD	1	// Bird's index
#define ACT_PARROT	2	// Parrot's index
#define ACT_SCARABEUS	3	// Scarabeus index
#define ACT_STINGRAY	4	// Stingray index
#define ACT_ICEBIRD	5	// Ice Bird's index
#define ACT_DRAGON	6	// Dragon's index
#define ACT_GHOST	7	// Ghost's index
#define ACT_R2D2	8	// R2D2's index
#define ACT_UFO		9	// UFO index
#define ACT_SQUIRREL	10	// Squirrel's index
#define ACT_GINGER	11	// Gingerbread Man's index
#define ACT_DEVILBIRD	12	// Devil Bird's index

#define ACT_MINX	3	// actor's left minimal X coordinate
#define ACT_MAXX	(WIDTH-20) // actor's right maximal X coordinate
#define ACT_MINY	(TIT_HEIGHT+3) // actor's top minimal Y coordinate
#define ACT_MAXY	(HEIGHT-FOOT_HEIGHT-3) // actor's bottom maximal Y coordinate
#define ACT_SPEEDX	5	// actor's speed in X direction
#define ACT_SPEEDY	5	// actor's speed in Y direction
#define ACT_SHADOWY	210	// actor's shadow Y
#define ACT_JUMPSPEED	10	// actor's jump speed (jump height = 10+9+8... = n*(n+1)/2 = 10*11/2 = 55)
#define ACT_ENEMYZONE	30	// Y zone range to search enemies

#define ACT_HIT_DIST	20	// actor hit distance
#define BLOOD_TIME	5	// blood time

#define ACT_HIT_FLY	6	// actor hit points - flying
#define ACT_HIT_WALK	3	// actor hit points - walking

// actor
extern int ActInx;		// index of current actor (0=Jill)
extern int ActorPhase;		// actor animation phase
extern int ActorX;		// actor middle X coordinate
extern int ActorY;		// actor middle Y coordinate
extern int ActorMinX, ActorMaxX; // actor's middle min/max X coordinate
extern int ActorMinY, ActorMaxY; // actor's middle min/max Y coordinate
extern int ActorJumpSpeed;	// actor's current jump speed
extern int ActorReload;		// counter to reload actor's weapon
extern int BloodTime;		// blood timer (0 = none)
extern int BloodX, BloodY;	// blood coordinate

// actor template descriptor
typedef struct {
	const char*	name;		// actor name
	const char*	missile;	// missile name
	const u16*	pal;		// palettes
	const u8*	img;		// image
	int		w;		// image width
	int		wall;		// total width
	int		h;		// image height
	COLTYPE		trans;		// transparent color
	Bool		walk;		// walking actor
} sActorTemp;

extern const sActorTemp	ActorTemp[ACT_NUM]; // actor templates
extern sActorTemp	Actor;		// current actor template

// Actor shadow
// format: 1-bit pixel graphics
// image width: 32 pixels
// image height: 7 lines
// image pitch: 4 bytes
extern const u8 ShadowImg[28] __attribute__ ((aligned(4)));
#define SHADOW_W	32		// shadow width
#define SHADOW_WS	32		// shadow total width
#define SHADOW_H	7		// shadow height

// Blood
// format: 4-bit paletted pixel graphics
// image width: 16 pixels
// image height: 16 lines
// image pitch: 8 bytes
extern const u16 BloodImg_Pal[6] __attribute__ ((aligned(4)));
extern const u8 BloodImg[128] __attribute__ ((aligned(4)));
#define BLOOD_W		16		// blood width
#define BLOOD_WS	16		// blood total width
#define BLOOD_H		16		// blood height
#define BLOOD_TRANS	COL_MAGENTA	// blood transparent color

// 00 Jill
// format: 8-bit paletted pixel graphics
// image width: 704 pixels
// image height: 100 lines
// image pitch: 704 bytes
extern const u16 JillImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 JillImg[70400] __attribute__ ((aligned(4)));
#define JILL_H		100		// Jill height
#define JILL_W		64		// Jill width
#define JILL_WALL	704		// Jill total width
#define JILL_RUNNUM	8		// Jill run phases
#define JILL_JUMPUP	8		// Jill jump up phase
#define JILL_JUMPDN	9		// Jill jump down phase
#define JILL_STAND	10		// Jill stand phase
#define JILL_TRANS	COL_BLACK	// Jill transparent color

// 01 Bird
// format: 8-bit paletted pixel graphics
// image width: 400 pixels
// image height: 56 lines
// image pitch: 400 bytes
extern const u16 BirdImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 BirdImg[22400] __attribute__ ((aligned(4)));
#define BIRD_W		100		// bird width
#define BIRD_WALL	400		// bird total width
#define BIRD_H		56		// bird height
#define BIRD_TRANS	COL_GREEN	// bird transparent color

// 02 Parrot
// format: 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 59 lines
// image pitch: 320 bytes
extern const u16 ParrotImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 ParrotImg[18880] __attribute__ ((aligned(4)));
#define PARROT_W	80		// parrot width
#define PARROT_WALL	320		// parrot total width
#define PARROT_H	59		// parrot height
#define PARROT_TRANS	COL_MAGENTA	// parrot transparent color

// 03 Scarabeus
// format: 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 59 lines
// image pitch: 320 bytes
extern const u16 ScarabeusImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 ScarabeusImg[18880] __attribute__ ((aligned(4)));
#define SCARABEUS_W	80		// scarabeus width
#define SCARABEUS_WALL	320		// scarabeus total width
#define SCARABEUS_H	59		// scarabeus height
#define SCARABEUS_TRANS	COL_MAGENTA	// scarabeus transparent color

// 04 Stingray
// format: 8-bit paletted pixel graphics
// image width: 400 pixels
// image height: 48 lines
// image pitch: 400 bytes
extern const u16 StingrayImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 StingrayImg[19200] __attribute__ ((aligned(4)));
#define STINGRAY_W	100		// stingray width
#define STINGRAY_WALL	400		// stingray total width
#define STINGRAY_H	48		// stingray height
#define STINGRAY_TRANS	COL_MAGENTA	// stingray transparent color

// 05 Icebird
// format: 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 44 lines
// image pitch: 320 bytes
extern const u16 IcebirdImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 IcebirdImg[14080] __attribute__ ((aligned(4)));
#define ICEBIRD_W	80		// icebird width
#define ICEBIRD_WALL	320		// icebird total width
#define ICEBIRD_H	44		// icebird height
#define ICEBIRD_TRANS	COL_MAGENTA	// icebird transparent color

// 06 Dragon
// format: 8-bit paletted pixel graphics
// image width: 400 pixels
// image height: 58 lines
// image pitch: 400 bytes
extern const u16 DragonImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 DragonImg[23200] __attribute__ ((aligned(4)));
#define DRAGON_W	100		// dragon width
#define DRAGON_WALL	400		// dragon total width
#define DRAGON_H	58		// dragon height
#define DRAGON_TRANS	COL_MAGENTA	// dragon transparent color

// 07 Ghost
// format: 8-bit paletted pixel graphics
// image width: 200 pixels
// image height: 55 lines
// image pitch: 200 bytes
extern const u16 GhostImg_Pal[251] __attribute__ ((aligned(4)));
extern const u8 GhostImg[11000] __attribute__ ((aligned(4)));
#define GHOST_W		50		// ghost width
#define GHOST_WALL	200		// ghost total width
#define GHOST_H		55		// ghost height
#define GHOST_TRANS	COL_MAGENTA	// ghost transparent color

// 08 R2D2
// format: 8-bit paletted pixel graphics
// image width: 200 pixels
// image height: 61 lines
// image pitch: 200 bytes
extern const u16 R2D2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 R2D2Img[12200] __attribute__ ((aligned(4)));
#define R2D2_W		50		// R2D2 width
#define R2D2_WALL	200		// R2D2 total width
#define R2D2_H		61		// R2D2 height
#define R2D2_TRANS	COL_MAGENTA	// R2D2 transparent color

// 09 UFO
// format: 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 50 lines
// image pitch: 320 bytes
extern const u16 UfoImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 UfoImg[16000] __attribute__ ((aligned(4)));
#define UFO_W		80		// UFO width
#define UFO_WALL	320		// UFO total width
#define UFO_H		50		// UFO height
#define UFO_TRANS	COL_MAGENTA	// UFO transparent color

// 10 Squirrel
// format: 8-bit paletted pixel graphics
// image width: 240 pixels
// image height: 60 lines
// image pitch: 240 bytes
extern const u16 SquirrelImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 SquirrelImg[14400] __attribute__ ((aligned(4)));
#define SQUIRREL_W	60		// squirrel width
#define SQUIRREL_WALL	240		// squirrel total width
#define SQUIRREL_H	60		// squirrel height
#define SQUIRREL_TRANS	COL_MAGENTA	// squirrel transparent color

// 11 Gingerbread Man
// format: 8-bit paletted pixel graphics
// image width: 220 pixels
// image height: 68 lines
// image pitch: 220 bytes
extern const u16 GingerbreadImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 GingerbreadImg[14960] __attribute__ ((aligned(4)));
#define GINGERBREAD_W		55		// gingerbread man width
#define GINGERBREAD_WALL	220		// gingerbread man total width
#define GINGERBREAD_H		68		// gingerbread man height
#define GINGERBREAD_TRANS	COL_MAGENTA	// gingerbread man transparent color

// 12 Devil Bird
// format: 8-bit paletted pixel graphics
// image width: 400 pixels
// image height: 72 lines
// image pitch: 400 bytes
extern const u16 DevilImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 DevilImg[28800] __attribute__ ((aligned(4)));
#define DEVIL_W		100		// gingerbread man width
#define DEVIL_WALL	400		// gingerbread man total width
#define DEVIL_H		72		// gingerbread man height
#define DEVIL_TRANS	COL_MAGENTA	// gingerbread man transparent color

// activate blood
void SetBlood(int x, int y);

// display blood
void DispBlood();

// shift blood
void ShiftBlood();

// switch actor
void SetActor(int actinx);

// initialize actor on a new game
void InitActor();

// display actor
void DispActor();

// shift actor animation
void ShiftActor();

// falling actor
void FallActor();

// control actor (returns False to break game)
Bool CtrlActor();

// hit actor
void HitActor();
