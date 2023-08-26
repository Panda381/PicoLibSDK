
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
#if RENDER_BOTHCORES	// 1 = use both cores to render, 0 = use only core 0

	// start render on core 1
	VgaCore1Exec(Render3D);

	// leaving little head start for core 1 so that both lines can be displayed simultaneously
	WaitMs(100);

#endif

	// render image without antialiasing
	Render3D();

	// main loop
	while (True)
	{
		// indicate work done
		LedOn(LED1);
		WaitMs(125);
		LedOff(LED1);
		WaitMs(125);

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
