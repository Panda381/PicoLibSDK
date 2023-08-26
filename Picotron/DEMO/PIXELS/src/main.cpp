
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	int x, y;
	u8 col;

	// main loop
	while (True)
	{
		x = RandU16Max(WIDTH-1);
		y = RandU16Max(HEIGHT-1);
		col = RandU8Max(15);
		DrawPoint(x, y, col);
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
