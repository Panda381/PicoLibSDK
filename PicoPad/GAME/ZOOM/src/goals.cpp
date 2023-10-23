
// ****************************************************************************
//
//                                Goals
//
// ****************************************************************************

#include "include.h"

// goals init
const u8 GoalsInit[4] = { 10, 15, 25, 10 };
int	Goals;		// remaining goals
u32	GoalTime;	// last goal time, to wait for end of the sector

// display goals
void DispGoals()
{
	int i;
	for (i = 0; i < Goals; i++)
		DrawBlit4Pal(GoalImg, GoalImg_Pal, 0, 0, i*GOAL_IMG_W, 23,
			GOAL_IMG_W, GOAL_IMG_H, GOAL_IMG_W, GOAL_IMG_KEY);
}

// decrease goals
void DecGoals()
{
	if (Goals > 0)
	{
		Goals--;
		GoalTime = Time();
	}
}
