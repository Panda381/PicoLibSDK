
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	int i;
	char ch;

	// initialize USB
	UsbHostInit();

	// connectint USB keyboard
	printf("\fconnecting USB keyboard...");
	while (!UsbKeyIsMounted()) { }

	LastTime = Time();

	// loop with open screen
	MaxScore = 0;
	while (True)
	{
		// open screen (break with a valid key)
		OpenScreen();

		// open new game
		OpenGame();
	}
}
