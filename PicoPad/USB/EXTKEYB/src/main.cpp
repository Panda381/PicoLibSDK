
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	u32 k;
	u8 ch;
	
Restart:
	// initialize USB
	UsbHostInit();

	// connectint USB keyboard
	printf("\fconnecting USB keyboard...");
	while (!UsbKeyIsMounted()) { }
	printf("connected:\27\n");

	while(True)
	{
		// disconnected
		if (!UsbKeyIsMounted()) goto Restart;

		// get keyboard key
		k = UsbGetKeyRel();
		if (k != 0)
		{
			if ((k & B24) != 0)
				printf("key=0x%02X (%d) modi=0x%02X RELEASE\n",
					k & 0xff, k & 0xff, (k >> 8) & 0xff);
			else
			{
				ch = (u8)(k >> 16);
				if (ch < 32)
					if (ch == 0)
						printf("key=0x%02X (%d) modi=0x%02X\n",
							k & 0xff, k & 0xff, (k >> 8) & 0xff);
					else
						printf("key=0x%02X (%d) modi=0x%02X char=0x%02X\n",
							k & 0xff, k & 0xff, (k >> 8) & 0xff, ch);
				else
					printf("key=0x%02X (%d) modi=0x%02X char=%c\n",
						k & 0xff, k & 0xff, (k >> 8) & 0xff, (char)ch);
			}
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
