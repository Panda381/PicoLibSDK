
// ****************************************************************************
//
//                                Goals
//
// ****************************************************************************

#ifndef _GOALS_H
#define _GOALS_H

// goals init
extern const u8 GoalsInit[4];
extern int	Goals;		// remaining goals
extern u32	GoalTime;	// last goal time, to wait for end of the sector

// format: 4-bit paletted pixel graphics
// image width: 12 pixels
// image height: 8 lines
// image pitch: 6 bytes
extern const u16 GoalImg_Pal[4] __attribute__ ((aligned(4)));
extern const u8 GoalImg[48] __attribute__ ((aligned(4)));
#define GOAL_IMG_W	12
#define GOAL_IMG_H	8
#define GOAL_IMG_KEY	COL_CYAN

// display goals
void DispGoals();

// decrease goals
void DecGoals();

#endif // _GOALS_H
