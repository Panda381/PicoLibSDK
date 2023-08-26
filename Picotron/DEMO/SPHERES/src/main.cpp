
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// main function
int main()
{
	// main loop
	while (True)
	{
		// random radius
		int r = RandU8MinMax(20,50);
		int r2 = r*r;
		int r3 = (r+r/4)*(r+r/4);

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

				// inside sphere
				if (dist <= r2)
				{
					// new center if left top edge
					int x2 = x + r/4;
					int y2 = y + r/4;
					dist = x2*x2 + y2*y2;

					x2 = x+xc;
					y2 = y+yc;

					// draw color or black
					if (dist < RandU16Max(r3))
					{
						DrawPoint(x2, y2, (((x2 + y2) & 1) == 0) ? col : col2);
					}
					else
						DrawPoint(x2, y2, COL_BLACK);
				}
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
