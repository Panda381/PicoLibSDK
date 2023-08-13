
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	// initialize USB
	UsbHostInit();

	// connectint USB keyboard
	printf("\fconnecting USB keyboard...");
	while (!UsbKeyIsMounted()) { }

	// intro screen
	DrawImgRle(IntroImg, IntroImg_Pal, 0, 0, WIDTH, HEIGHT);
	DispUpdate();
	WaitMs(1500);

	// select scene set (returns True = OK, False = Esc)
	while (SetSelect())
	{
		// select scene (returns True = OK, False = Esc)
		while (LevSelect())
		{
			NewGame(True);

			// game loop
			while (GameLoop())
			{
				// continue to next scene
				SceneInx++;
				if (SceneInx >= SceneSetNum) break;
				NewGame(False);
			}
		}
	}

	ResetToBootLoader();
}
