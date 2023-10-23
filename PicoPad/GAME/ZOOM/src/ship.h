
// ****************************************************************************
//
//                               Space Ship
//
// ****************************************************************************

#ifndef _SHIP_H
#define _SHIP_H

// ship
#define SHIP_MINX	(30<<FRAC)	// ship min. X coordinate, with FRAF fraction
#define SHIP_MAXX	((WIDTH<<FRAC)-SHIP_MINX) // ship max. X coordinate, with FRAF fraction
#define SHIP_DEFX	((WIDTH/2)<<FRAC) // ship default X coordinate, with FRAF fraction

#define SHIP_SPEEDMIN	(30<<FRAC)	// ship min. speed, with FRAF fraction
#define SHIP_SPEEDMAX	(120<<FRAC)	// ship max. speed, with FRAF fraction
#define SHIP_SPEEDDEF	((SHIP_SPEEDMIN+SHIP_SPEEDMAX)/2) // ship default speed, with FRAF fraction

#define SHIP_CRASH_NUM	8		// number of steps of ship in crash
#define SHIP_CRASH_WAIT	10		// number of steps of ship in crash, wait on end
#define SHIP_ANIM_TIME	300000		// ship animation time in [us]
#define SHIP_NEW_TIME	2000000		// protection time of new ship in [us]

#define SHIP_HIT_DIST		8	// ship hit distance

extern short	ShipX;		// ship X coordinate, with FRAC fraction 
extern short	Speed;		// ship speed, with FRAC fraction 
extern short	ShipInCrash;	// >1 ship is in crash
extern u32	ShipCrashTime;	// ship crash animation time
extern u32	ShipNewTime;	// new ship time (to protect on start)

// format: 4-bit paletted pixel graphics
// image width: 56 pixels
// image height: 60 lines
// image pitch: 28 bytes
extern const u16 ShipImg_Pal[8] __attribute__ ((aligned(4)));
extern const u8 ShipImg[1680] __attribute__ ((aligned(4)));
#define SHIP_IMG_W	56
#define SHIP_IMG_H	15
#define SHIP_IMG_KEY	COL_MAGENTA

#define SHIP_UP		0
#define SHIP_LEFT	1
#define SHIP_RIGHT	2
#define SHIP_SHADOW	3

// display ship
void DispShip();

// crash ship
void ShipCrash();

// check hit ship by enemy
void ShipEnemyCrash();

// ship service
void DoShip(u32 dt, u32 dz);

#endif // _SHIP_H
