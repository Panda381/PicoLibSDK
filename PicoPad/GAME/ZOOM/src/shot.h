
// ****************************************************************************
//
//                               Shooting
//
// ****************************************************************************

#ifndef _SHOT_H
#define _SHOT_H

#define SHOTS		4	// max. number of shots
#define SHOTFREE	-32768	// flag - unused shot
#define SHOTTIME	500000	// shot delta time in [us]
#define SHOTDIST	(50<<FRAC) // shot max. distance, with FRAC fraction

// shot descriptor
typedef struct {
	short	x;	// X coordinate, with FRAC fraction (SHOOTFREE = unused shoot)
	short	z;	// Z coordinate, with FRAC fraction
} sShot;

extern int	ShootSpeed;	// shoot speed
extern sShot	Shots[SHOTS];	// shots
extern u32	LastShotTime;	// time of last shoot

// format: 4-bit paletted pixel graphics
// image width: 13 pixels
// image height: 16 lines
// image pitch: 7 bytes
extern const u16 ShotImg_Pal[4] __attribute__ ((aligned(4)));
extern const u8 ShotImg[112] __attribute__ ((aligned(4)));
#define SHOT_IMG_W	13
#define SHOT_IMG_H	4
#define SHOT_IMG_KEY	COL_MAGENTA

extern const u8 LaserSnd[5244];

// reset shots on start of new scene
void ResetShots();

// display shots
void DispShots();

// shooting service (dt = delta time in [us])
void Shooting(int dt);

#endif // _SHOT_H
