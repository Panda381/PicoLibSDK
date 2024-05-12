
// ****************************************************************************
//
//                               Game menu
//
// ****************************************************************************

#include "../include.h"

// request to do screenshot
Bool ReqScreenShot = False;

// home directory
char HomePath[APPPATH_PATHMAX+1];
int HomePathLen;

sTop Top[TOP_NUM];	// top list

// load top list (returns max. score)
s32 LoadTop()
{
	// reset top list
	int i;
	memset(Top, ' ', sizeof(Top));
	for (i = 0; i < TOP_NUM; i++) Top[i].score = 0;

	// mount disk if not mounted
	DiskAutoMount();

	// get home path (and set is as current directory)
	HomePathLen = GetHomePath(HomePath, HOMEPATH);

	// load top file
	sFile file;
	if (FileOpen(&file, TOPLIST_NAME))
	{
		FileRead(&file, Top, sizeof(Top));
		FileClose(&file);
	}

	// get max. score
	return Top[0].score;
}

// save top list (returns False on error)
Bool SaveTop()
{
	// mount disk if not mounted
	DiskAutoMount();

	// set home directory
	SetDir(HomePath);

	// write top file
	sFile file;
	if (!FileOpen(&file, TOPLIST_NAME) && !FileCreate(&file, TOPLIST_NAME)) return False;

	// write file
	int k = FileWrite(&file, Top, sizeof(Top));

	// close file
	FileClose(&file);

	return (k == sizeof(Top));
}

// display top score
void DispTop()
{
#define TOPBUFN 30
	char buf[TOPBUFN];
	char namebuf[TOPNAME_LEN+1];

	// display top list
	int y = 105; // -> y = 120 - 15 = 105
	DrawTextW("Top List", (WIDTH - 8*16)/2, y, COL_GREEN);
	y += 15;

	int i; // y=240-8*15 = 120
	for (i = 0; i < TOP_NUM; i++)
	{
		memcpy(namebuf, Top[i].name, TOPNAME_LEN);
		namebuf[TOPNAME_LEN] = 0;
		MemPrint(buf, TOPBUFN, "%d: %07d %s", i+1, Top[i].score, namebuf);
		DrawTextW(buf, 0, y, COL_WHITE);
		y += 15;
	}
}

// game menu (returns False to exit)
Bool GameMenu()
{
	int strip;

	// stop sounds
	StopAllSound();

	// flush keys
	KeyFlush();

	while (True)
	{
		for (strip = DISP_STRIP_NUM; strip > 0; strip--)
		{
			DispSetStripNext();

			// clear screen
			DrawClear();

			// display title
			int y = 0;
			DrawText2("Game Menu", (WIDTH - 9*16)/2, y, COL_GREEN);
			y += 32; // 32

			// display menu
			DrawTextW("Y", 0, y, MENU_COL_KEY);
			DrawTextW("... Continue", 32, y, MENU_COL_MENU);
			y += 15; // 47

			DrawTextW("X", 0, y, MENU_COL_KEY);
			DrawTextW("... Exit", 32, y, MENU_COL_MENU);
			y += 15; // 62

			DrawTextW("B", 0, y, MENU_COL_KEY);
			DrawTextW("... Sound is", 32, y, MENU_COL_MENU);
			DrawTextW((SoundMode == SOUNDMODE_OFF) ? "off" : ((SoundMode == SOUNDMODE_SOUND) ? "ON" : "ALL"), 15*16, y, MENU_COL_MENU);
			y += 15; // 77

			DrawTextW("A", 0, y, MENU_COL_KEY);
			DrawTextW("... Screenshot", 32, y, MENU_COL_MENU);
			//y += 15; // 92

			// display top list
			DispTop();

			// display update
			DispUpdate();
		}

		// keyboard
		switch (KeyGet())
		{
		// exit
		case KEY_X:
			return False;

		// sound
		case KEY_B:
			SoundMode++;
			if (SoundMode > 2) SoundMode = 0;
			break;

		// screen shot
		case KEY_A:
			// request to do screenshot
			ReqScreenShot = True;
			while (KeyPressed(KEY_A)) {}
			return True;

		// continue
		case KEY_Y:
			return True;
		}
	}
}
