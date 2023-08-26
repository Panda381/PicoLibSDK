
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	// draw text
#if USE_PICOINO10
	DrawText("Press Z=BOOTSEL, Back=quit...", (WIDTH-29*8)/2, (HEIGHT-8)/2, COL_WHITE);
#else
	DrawText("Press B=BOOTSEL, Y=quit...", (WIDTH-26*8)/2, (HEIGHT-8)/2, COL_WHITE);
#endif

	DispUpdate();

	// reset to boot loader
	while (True)
	{
		switch (KeyGet())
		{
		case KEY_B:
			ResetUsb(0, 0);

		case KEY_Y:
			ResetToBootLoader();

#if USE_SCREENSHOT		// use screen shots
		case KEY_X:
			ScreenShot();
			break;
#endif
		}
	}
}
