
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	u32 k;
	
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
		k = UsbGetKey();
		if (k != 0)
		{
			printf("key=%02X (%d) modi=%02X char=%c\n",
				k & 0xff, k & 0xff, (k >> 8) & 0xff, (char)(k >> 16));
		}

		// control
		switch (KeyGet())
		{
		// reset to boot loader
		case BTN_C:
			ResetToBootLoader();
			break;
		}
	}
}
