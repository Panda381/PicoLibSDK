
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

#define DIRPATH	"/SLIDE/"

int main()
{
	int i;
	char name[30];
	sFile file;

	while (True)
	{
RESTART:
		// mounting SD disk
		while (!DiskMount())
		{
			DrawClear();
			DrawText2("Insert SD Card", (WIDTH-14*8*2)/2, (HEIGHT-16)/2, COL_WHITE);
			if (KeyGet() == BTN_C) ResetToBootLoader();
			WaitMs(1000);
		}

		// check file
		while (!FileExist(DIRPATH "1.BMP"))
		{
			DrawClear();
			DrawText("Canot find " DIRPATH "1.BMP", (WIDTH-23*8)/2, (HEIGHT-8)/2, COL_WHITE);
			WaitMs(1000);
			DiskUnmount();
			goto RESTART;
		}

		// show files
		for (i = 1; i < 10000; i++)
		{
			// prepare filename
			MemPrint(name, 30, "%s%d.BMP", DIRPATH, i);

			// open file
			if (!FileOpen(&file, name)) goto RESTART;

			// skip header
			FileSeek(&file, 0x46);

			// load image
			FileRead(&file, FrameBuf, FRAMESIZE*sizeof(FRAMETYPE));

			// close vile
			FileClose(&file);

			// exit
			if (KeyGet() == BTN_C) ResetToBootLoader();

			// wait
			WaitMs(2000);
		}
	}
}
