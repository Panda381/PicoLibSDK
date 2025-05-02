
// ****************************************************************************
//
//                                 Stars
//
// ****************************************************************************

#ifndef _STARS_H
#define _STARS_H

#define STARS		100	// max. number of stars
#define STARFREE	-32768	// flag - unused star
#define STARTIME	50000	// delta time to generate next star, in [us]

#if USE_PICOPADVGA || USE_PICOPADHSTX
#define STARMIN		(500/5)	// star min. speed
#define STARMAX		(800/5)	// star max. speed
#else
#define STARMIN		500	// star min. speed
#define STARMAX		800	// star max. speed
#endif

// star descriptor
typedef struct {
	short	x;	// X coordinate, with FRAC fraction (STARFREE = unused star)
	short	y;	// Y coordinate, with FRAC fraction 
	short	dx;	// delta X coordinate, with FRAC fraction
	short	dy;	// delta Y coordinate, with FRAC fraction
} sStar;

extern sStar	Stars[STARS];	// stars
extern u32	LastStarTime;	// time of last star

// generate one new star
void NewStar1();

// generate new stars
void NewStars();

// initialize stars
void InitStars();

// shift stars
void ShiftStars();

// display stars
void DispStars();

#endif // _STARS_H
