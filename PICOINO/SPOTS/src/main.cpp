
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
		int r = RandU8MinMax(5,25);
		int r2 = r*r;

		// random center coordinate
		int xc = RandU16MinMax(r,WIDTH-r);
		int yc = RandU16MinMax(r,HEIGHT-r);

		// random color
		u8 col = RandU8();

		// loop Y
		int x, y, dist;
		u8* d;
		for (y = -r; y < r; y++)
		{
			// pointer into draw box
			d = &FrameBuf[WIDTH*(yc+y) + xc-r];

			// loop X
			for (x = -r; x < r; x++)
			{
				// distance from center
				dist = x*x + y*y;

				// draw pixel, near center there is more probability
				if (dist < RandU16Max(r2)) *d = col;

				// increase destination pointer
				d++;
			}
		}

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
