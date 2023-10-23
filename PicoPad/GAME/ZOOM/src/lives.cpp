
// ****************************************************************************
//
//                                Lives
//
// ****************************************************************************

#include "include.h"

int	Lives;		// number of lives

// display number of lives
void DispLives()
{
	int i;
	for (i = 0; (i < Lives) && (i < 5); i++)
		DrawBlit4Pal(LiveImg, LiveImg_Pal, 0, 0, 200 + i*24, 10,
			LIVE_IMG_W, LIVE_IMG_H, LIVE_IMG_W, LIVE_IMG_KEY);
}
