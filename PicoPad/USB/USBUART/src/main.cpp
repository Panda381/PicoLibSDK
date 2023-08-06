
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// random texts
const char* RandText[] = {
	"How are you?\n",
	"Don't worry!\n",
	"PicoPad is cool!\n",
	"\21Nice blue text\27\n",
	"\22Full \23row \24of \25nice \26colors\27\n",
	"\2\26 This \21 text \24 is \22 inverted \1\27\n",
	"\5\26Double-width text\27\3\n",
	"\4\24Double-height text\27\n\3",
	"\6\23Very \2\26BIG\25\1 text\27\n\3",
};

int main()
{
	u8 key;
	int i, n;

Restart:
	// select menu
	Print("\fSelect to be: A=Device, B=Host, Y=Quit\n");
	do {
		key = KeyGet();
		switch (key)
		{
		case KEY_A:	
			UsbDevInit(&UsbDevCdcSetupDesc);
			Print("Connecting as Device...");
			break;

		case KEY_B:
			UsbHostInit();
			Print("Connecting as Host...");
			break;

		case KEY_Y:
			ResetToBootLoader();
			break;
		}
	} while ((key != KEY_A) && (key != KEY_B));

	// wait for connection
	while (!UsbCdcIsMounted())
	{
		if (KeyGet() == KEY_Y)
		{
			UsbTerm();
			goto Restart;
		}
	}
	Print("OK\n");
	Print("Press A/B/X to send text, Y restart:\n\27");

	// main loop
	while (True)
	{
		// get key
		key = KeyGet();

		// restart
		if (key == KEY_Y)
		{
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			UsbTerm();
			goto Restart;
		}

		// send text A
		if (key == KEY_A)
		{
			UsbCdcWriteData("Hello World!\n", 13);
		}

		// send text B
		if (key == KEY_B)
		{		
			if (UsbHostMode)
				UsbCdcWriteData("Hello from host to device!\n", 27);
			else
				UsbCdcWriteData("Hello from device to host!\n", 27);
		}

		// send text X
		if (key == KEY_X)
		{		
			i = RandU8Max(count_of(RandText)-1);
			n = StrLen(RandText[i]);
			UsbCdcWriteData(RandText[i], n);
		}

		// receive data
		key = UsbCdcReadChar();
		if (key != 0) PrintChar(key);
	}
}
