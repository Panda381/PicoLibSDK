
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define MOUSE_SPEED	2	// mouse speed

// keyboard report structure
sUsbHidKey KeyRep;

int main()
{
	s8 dx, dy;

Restart:

	// initialize USB
	UsbDevInit(&UsbDevHidSetupDesc2);

	// connecting
	DrawPrint("\fconnecting to the host...");
	while (!UsbDevHidIsMounted()) { }

	// help
	DrawPrint("connected\n");
	DrawPrint("arrows ... move mouse\n");
	DrawPrint("A ........ left button\n");
	DrawPrint("B ........ right button\n");
	DrawPrint("X ........ keyboard text\n");
	DrawPrint("Y ........ quit\n");

	while (True)
	{
		// lost connection
		if (!UsbDevHidIsMounted()) goto Restart;

		// send mouse (returns False on error)
		dx = 0;
		if (KeyPressed(KEY_LEFT)) dx = -MOUSE_SPEED;
		if (KeyPressed(KEY_RIGHT)) dx = MOUSE_SPEED;
		dy = 0;
		if (KeyPressed(KEY_UP)) dy = -MOUSE_SPEED;
		if (KeyPressed(KEY_DOWN)) dy = MOUSE_SPEED;
		UsbDevHidSendMouse(dx, dy, KeyPressed(KEY_A), KeyPressed(KEY_B), False);

		// keyboard
		switch (KeyGet())
		{
		case KEY_X:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			UsbDevHidSendText(" Hello from Pico ");
			break;

		// reset to boot loader
		case KEY_Y:
			ResetToBootLoader();
			break;
		}
	}
}
