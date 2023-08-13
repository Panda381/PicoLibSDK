
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
	Print("\fSelect to be: A=Device, B=Host, C=Quit\n");
	do {
		key = KeyGet();
		switch (key)
		{
		case BTN_A:	
			UsbDevInit(&UsbDevCdcSetupDesc);
			Print("Connecting as Device...");
			break;

		case BTN_B:
			UsbHostInit();
			Print("Connecting as Host...");
			break;

		case BTN_C:
			ResetToBootLoader();
			break;
		}
	} while ((key != BTN_A) && (key != BTN_B));

	// wait for connection
	while (!UsbCdcIsMounted())
	{
		if (KeyGet() == BTN_C)
		{
			UsbTerm();
			goto Restart;
		}
	}
	Print("OK\n");
	Print("Press A/B/C to send text, A+B restart:\n\27");

	// main loop
	while (True)
	{
		// get key
		key = KeyGet();

		// send text A
		if (key == BTN_A)
		{
			if (KeyPressed(BTN_B))
			{
				UsbTerm();
				goto Restart;
			}
			UsbCdcWriteData("Hello World!\n", 13);
		}

		// send text B
		if (key == BTN_B)
		{		
			if (KeyPressed(BTN_A))
			{
				UsbTerm();
				goto Restart;
			}
			if (UsbHostMode)
				UsbCdcWriteData("Hello from host to device!\n", 27);
			else
				UsbCdcWriteData("Hello from device to host!\n", 27);
		}

		// send text C
		if (key == BTN_C)
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
