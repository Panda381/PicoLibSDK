
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	u32 k;
	int x = 0;
	int y = 0;
	
Restart:
	// initialize USB
	UsbHostInit();

	// connectint USB mouse
	printf("\fconnecting USB mouse...");
	while (!UsbMouseIsMounted()) { }
	printf("connected:\27\n");

	x = 0;
	y = 0;
	
	while(True)
	{
		// disconnected
		if (!UsbMouseIsMounted()) goto Restart;

		// get mouse
		k = UsbGetMouse();
		if (k != 0)
		{
			x += (s8)(k >> 8);
			y += (s8)(k >> 16);

			printf("btn=%05b X=%d Y=%d\n", k & 0x1f, x, y);
		}

		// control
		switch (KeyGet())
		{
#if USE_SCREENSHOT		// use screen shots
		case KEY_X:
			ScreenShot();
			break;
#endif

		// reset to boot loader
		case KEY_Y:
			ResetToBootLoader();
			break;
		}
	}
}
