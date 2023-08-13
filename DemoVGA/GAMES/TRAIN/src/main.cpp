
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	int i;

	// initialize USB
	UsbHostInit();

	// connectint USB keyboard
	printf("\fconnecting USB keyboard...");
	while (!UsbKeyIsMounted()) { }

	// clear screen
	DrawClear();

	// set first scene
	Level = LEVFIRST;
	Score = 0;

	// loop with demo scene
	while (True)
	{
		// demo
		i = Level; // save current level

		while (True)
		{
			// play demo scene 0
			Level = 0;
			if (HelpLevel()) break; // break

#ifdef AUTOMODE // automode - run levels to check solutions (help completes scene)
			break;
#endif

			// play random scene
			Level = RandU8MinMax(1, LEVNUM);
			if (HelpLevel()) break; // break
		}

		if (UsbKeyIsPressed(HID_KEY_ESCAPE)) ResetToBootLoader();

		// return current level
		Level = i;
		InitLevel();

		// main loop
		while (True)
		{
			// game loop - called every single game step
			if (GameLoop()) break;

			// wait step
			WaitStep();
		}
	}
}
