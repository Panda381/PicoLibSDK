
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

// main function
int main()
{
	// main loop
	while (True)
	{
		// random radius
		int r = RandU8MinMax(20,50);
		int r2 = r*r;

		// random center coordinate
		int xc = RandU16MinMax(r,WIDTH-r);
		int yc = RandU16MinMax(r,HEIGHT-r);

		// random color
		u8 col = RandU8Max(15);
		u8 col2 = RandU8Max(15);

		// loop Y
		int x, y, dist;
		u8* d;
		for (y = -r; y < r; y++)
		{
			// loop X
			for (x = -r; x < r; x++)
			{
				// distance from center
				dist = x*x + y*y;

				// draw pixel, near center there is more probability
				int x2 = x+xc;
				int y2 = y+yc;
				if (dist < RandU16Max(r2))
					DrawPoint(x2, y2, (((x2 + y2) & 1) == 0) ? col : col2);
			}
		}

		DispUpdate();

		switch (KeyGet())
		{
		// reset to boot loader
		case KEY_Y: ResetToBootLoader();

#if USE_SCREENSHOT		// use screen shots
		case KEY_X:
			ScreenShot();
			break;
#endif
		}
	}
}
