
// ****************************************************************************
//
//                               Enemies
//
// ****************************************************************************

#ifndef _ENEMY_H
#define _ENEMY_H

// object type
#define ENEMY_NO	0	// not used object
#define ENEMY_DISK	1	// small disk
#define ENEMY_BIG	2	// big enemy
#define ENEMY_BOSS	3	// boss
#define ENEMY_CRASH	4	// crash
#define ENEMY_BOMB	5	// bomb

// points per enemy (no, disk, big, boss)
extern const int EnemyPoint[4];

// one step of the moving
typedef struct {
	short	dx;		// speed in X direction, with (FRAC+3) fraction
	short	dz;		// speed in Z direction, with (FRAC+3) fraction
	short	dt;		// length of step in [ms]
} sStep;

// move track
typedef struct {
	int	startstep;	// index of start step
	int	numstep;	// number of steps
	int	repstep;	// index of first repeated step, -1=do not repeat
} sMove;

// object
typedef struct {
	int	type;		// enemy type (ENEMY_NO = not used object)
	int	move;		// index of move track
	int	istep;		// relative index of current step, on crash = animation step
	u32	starttime;	// start time of move in [us]
	u32	time;		// start time of the step in [us]
	u32	nextbomb;	// time of next bomb
	short	sx, sz;		// coordinates of start of current step, with FRAC fraction
	short	x, z;		// current coordinates, with FRAC fraction
} sObj;

// time of last generated object
extern u32 TimeLastDisk;		// time of last generated small disk
extern u32 TimeLastBig;			// time of last generated big enemy
extern Bool BossIsGen;			// boss is generated
extern int BossLives;			// boss lives counter
extern u32 TimeLastBomb;		// time of last generated bomb

// format: 4-bit paletted pixel graphics
// image width: 76 pixels
// image height: 232 lines
// image pitch: 38 bytes
extern const u16 BossImg_Pal[4] __attribute__ ((aligned(4)));
extern const u8 BossImg[8816] __attribute__ ((aligned(4)));
#define BOSS_IMG_W	76
#define BOSS_IMG_H	29
#define BOSS_IMG_KEY	COL_BLACK

// format: 4-bit paletted pixel graphics
// image width: 40 pixels
// image height: 128 lines
// image pitch: 20 bytes
extern const u16 UfoImg_Pal[5] __attribute__ ((aligned(4)));
extern const u8 UfoImg[2560] __attribute__ ((aligned(4)));
#define UFO_IMG_W	40
#define UFO_IMG_H	16
#define UFO_IMG_KEY	COL_GREEN

// format: 4-bit paletted pixel graphics
// image width: 248 pixels
// image height: 46 lines
// image pitch: 124 bytes
extern const u16 Ufo2Img_Pal[4] __attribute__ ((aligned(4)));
extern const u8 Ufo2Img[5704] __attribute__ ((aligned(4)));
#define UFO2_IMG_W	31
#define UFO2_IMG_TOTW	248
#define UFO2_IMG_H	46
#define UFO2_IMG_KEY	COL_GREEN

// format: 4-bit paletted pixel graphics
// image width: 14 pixels
// image height: 72 lines
// image pitch: 7 bytes
extern const u16 BombImg_Pal[3] __attribute__ ((aligned(4)));
extern const u8 BombImg[504] __attribute__ ((aligned(4)));
#define BOMB_IMG_W	14
#define BOMB_IMG_H	9
#define BOMB_IMG_KEY	COL_BLACK

// speed (in Z<<(FRAC+SPEED_SHIFT) coordinates per ms)
#define SPEED_SHIFT		3	// additional shifts of enemy speed

#define ENEMY_DISK_SPEED	15	// small disk speed << SPEED_SHIFT
#define ENEMY_BIG_SPEED		7	// big enemy speed << SPEED_SHIFT
#define ENEMY_BOSS_SPEED	12	// boss speed << SPEED_SHIFT
#define ENEMY_BOMB_SPEED	15	// bomb speed << SPEED_SHIFT

#define ENEMY_BOSS_LIVES	5	// boss lives
#define ENEMY_DISK_DELAY	500000	// additional delay of disk in group in [us]
#define ENEMY_PROT_DELAY	1000000	// start protection time

// table of steps of moving
extern const sStep Steps[];

// small disk on the left
#define ENEMY_DISK1_STARTSTEP	0	// index of start step
#define ENEMY_DISK1_NUMSTEP	7	// number of steps
#define ENEMY_DISK1_REPSTEP	-1	// index of first repeated step, -1 = do not repeat

// small disk on the right
#define ENEMY_DISK2_STARTSTEP	(ENEMY_DISK1_STARTSTEP+ENEMY_DISK1_NUMSTEP) // index of start step
#define ENEMY_DISK2_NUMSTEP	7	// number of steps
#define ENEMY_DISK2_REPSTEP	-1	// index of first repeated step, -1 = do not repeat

// big enemy
#define ENEMY_BIG_STARTSTEP	(ENEMY_DISK2_STARTSTEP+ENEMY_DISK2_NUMSTEP) // index of start step
#define ENEMY_BIG_NUMSTEP	11	// number of steps
#define ENEMY_BIG_REPSTEP	-1	// index of first repeated step, -1 = do not repeat
#define ENEMY_BIG_NEXTBOMB	3000000	// time of next bomb in [us]

// boss
#define ENEMY_BOSS_STARTSTEP	(ENEMY_BIG_STARTSTEP+ENEMY_BIG_NUMSTEP) // index of start step
#define ENEMY_BOSS_NUMSTEP	8	// number of steps
#define ENEMY_BOSS_REPSTEP	(ENEMY_BOSS_STARTSTEP+2)	// index of first repeated step, -1 = do not repeat
#define ENEMY_BOSS_NEXTBOMB	1000000	// time of next bomb in [us]

// crash
#define ENEMY_CRASH_STARTSTEP	(ENEMY_BOSS_STARTSTEP+ENEMY_BOSS_NUMSTEP) // index of start step
#define ENEMY_CRASH_NUMSTEP	8	// number of steps
#define ENEMY_CRASH_REPSTEP	-1	// index of first repeated step, -1 = do not repeat

// bomb to left
#define ENEMY_BOMBL_STARTSTEP	(ENEMY_CRASH_STARTSTEP+ENEMY_CRASH_NUMSTEP) // index of start step
#define ENEMY_BOMBL_NUMSTEP	1	// number of steps
#define ENEMY_BOMBL_REPSTEP	-1	// index of first repeated step, -1 = do not repeat

// bomb to right
#define ENEMY_BOMBR_STARTSTEP	(ENEMY_BOMBL_STARTSTEP+ENEMY_BOMBL_NUMSTEP) // index of start step
#define ENEMY_BOMBR_NUMSTEP	1	// number of steps
#define ENEMY_BOMBR_REPSTEP	-1	// index of first repeated step, -1 = do not repeat

// bomb down
#define ENEMY_BOMBD_STARTSTEP	(ENEMY_BOMBR_STARTSTEP+ENEMY_BOMBR_NUMSTEP) // index of start step
#define ENEMY_BOMBD_NUMSTEP	1	// number of steps
#define ENEMY_BOMBD_REPSTEP	-1	// index of first repeated step, -1 = do not repeat

// table of move tracks
extern const sMove Moves[10];

// move indices
#define ENEMY_DISK1_MOVE	0	// move index of small disk on the left
#define ENEMY_DISK2_MOVE	1	// move index of small disk on the right
#define ENEMY_BIG_MOVE		2	// move index of big enemy
#define ENEMY_BOSS_MOVE		3	// move index of boss
#define ENEMY_CRASH_MOVE	4	// move index of crash
#define ENEMY_BOMBL_MOVE	5	// move index of bomb to left
#define ENEMY_BOMBR_MOVE	6	// move index of bomb to right
#define ENEMY_BOMBD_MOVE	7	// move index of bomb down

#define ENEMY_CRASH_TIME	200	// enemy animation time in [ms]

#define ENEMY_HIT_DIST		20	// enemy hit distance

// objects
#define OBJ_MAX		20	// max. objects
extern sObj Obj[OBJ_MAX];	// list of objects

// initialize list of objects
void InitObj();

// display objects
void DispObj();

// find free object (NULL = not found)
sObj* GetFreeObj();

// check if no enemy
Bool CheckNoEnemy();

// generate bomb
void GenBomb(int move, short x, short z);

// serve objects
void DoObj();

// check hit enemy by the shot (coordinates are with FRAC; returns True if hit)
Bool EnemyHit(short x, short z);

#endif // _ENEMY_H
