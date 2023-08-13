
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
	s8 dx;

Restart:

	// initialize USB
	UsbDevInit(&UsbDevHidSetupDesc2);

	// connecting
	DrawPrint("\fconnecting to the host...");
	while (!UsbDevHidIsMounted()) { }

	// help
	DrawPrint("connected\n");
	DrawPrint("C, B ..... move mouse\n");
	DrawPrint("A ........ keyboard text\n");
	DrawPrint("C+B ...... quit\n");

	while (True)
	{
		// lost connection
		if (!UsbDevHidIsMounted()) goto Restart;

		// send mouse (returns False on error)
		dx = 0;
		if (KeyPressed(BTN_C)) dx = -MOUSE_SPEED;
		if (KeyPressed(BTN_B)) dx = MOUSE_SPEED;
		UsbDevHidSendMouse(dx, 0, False, False, False);

		// reset
		if (KeyPressed(BTN_C) && KeyPressed(BTN_B)) ResetToBootLoader();

		// keyboard
		switch (KeyGet())
		{
		case BTN_A:
			UsbDevHidSendText(" Hello from Pico ");
			break;
		}
	}
}
