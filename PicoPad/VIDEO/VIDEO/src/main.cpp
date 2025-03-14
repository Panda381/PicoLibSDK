
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define PATH "/VIDEO" // video path

#define MAXFILES 14

// video file
typedef struct {
	char	name[12+1];	// video file name
	u8	namelen;	// length of file name
	int	len;		// length in seconds
} sVideoFile;

// list of video files
sVideoFile FileList[MAXFILES];
int FileListNum = 0;
int FileListSel = 0;
Bool CtrlIsOn = False; // control is ON
Bool MuteIsOn = False; // mute is ON
s8 Volume = VIDEO_VOLUMEDEF; // current volume

// wait mounting SD card (return False to break)
Bool WaitMount()
{
	// clear display
	DrawClear();

	// set font 8x16
	SelFont8x16();

	// prompt
	DrawText2("Insert SD card", (WIDTH-14*8*2)/2, (HEIGHT-8*2)/2, COL_WHITE);

	// display update
	DispUpdate();

	// waiting for mounting SD card
	while (!DiskMount())
	{
		// break
		if (KeyGet() == KEY_Y) return False;
	}

	return True;
}

// load file list (returns False to break)
Bool LoadFileList()
{
	sFile find;
	sFileInfo info;
	sVideoFile* vid;
	sVideoFile tmp;
	int i;

	// waiting for mounting SD card
	if (!WaitMount()) return False;

	// clear display
	DrawClear();
	DispUpdate();
	SelFont8x16();

	// set directory
	if (!SetDir(PATH))
	{
		DrawText("No directory " PATH, 0, 0, COL_WHITE);
		DispUpdate();
		WaitMs(2000);
		return False;
	}

	// open search files
	if (!FindOpen(&find, ""))
	{
		DrawText("No video files", 0, 0, COL_WHITE);
		DispUpdate();
		WaitMs(2000);
		return False;
	}

	// find files
	for (FileListNum = 0; FileListNum < MAXFILES; FileListNum++)
	{
		// find next file
		if (!FindNext(&find, &info, ATTR_ARCH, "*.VID")) break;

		// fill up info
		vid = &FileList[FileListNum];
		memcpy(vid->name, info.name, 12+1);
		vid->namelen = info.namelen;
		vid->len = (info.size / VIDEO_FRAMESIZE) / VIDEO_FPS;
	}

	// find close
	FindClose(&find);

	if (FileListSel >= FileListNum) FileListSel = FileListNum-1;
	if (FileListSel < 0) FileListSel = 0;

	// sort files
	for (i = 0; i < FileListNum-1;)
	{
		if (strcmp(FileList[i].name, FileList[i+1].name) > 0)
		{
			memcpy(&tmp, &FileList[i], sizeof(sVideoFile));
			memcpy(&FileList[i], &FileList[i+1], sizeof(sVideoFile));
			memcpy(&FileList[i+1], &tmp, sizeof(sVideoFile));
			if (i > 0)
				i--;
			else
				i++;
		}
		else
			i++;
	}

	return True;
}

// display file list
void DispFileList()
{
	int i, len;
	u16 fgcol, bgcol;
	char buf[31];
	sVideoFile* vf;

	// clear display
	DrawClear();

	// set font 8x16
	SelFont8x16();

	// no file
	if (FileListNum == 0)
	{
		DrawText("No video files", 0, 0, COL_WHITE);
		DispUpdate();
	}

	// display files
	for (i = 0; i < FileListNum; i++)
	{
		// prepare color
		fgcol = COL_WHITE;
		bgcol = COL_BLACK;

		if (i == FileListSel)
		{
			fgcol = COL_BLACK;
			bgcol = COL_WHITE;
		}

		// display name
		vf = &FileList[i];
		len = vf->len;
		if (len > 5999) len = 5999;
		MemPrint(buf, 30, " % 12s %02d:%02d ", vf->name, len/60, len%60);
		DrawTextBg(buf, 0, i*16, fgcol, bgcol);
	}
	DispUpdate();
}

// play one video
void PlayVideo(const char* filename)
{
	// waiting for mounting SD card
	if (!WaitMount()) return;

	// clear display
	DrawClear();
	DispUpdate();

	// open video
	sVideo video;
	if (SetDir(PATH) && VideoOpen(&video, filename))
	{
		VideoSetCtrl(&video, CtrlIsOn);	
		VideoSetVol(&video, Volume);
		VideoSetMute(&video, MuteIsOn);

		// play video
		while (video.frame < video.frames)
		{
			// play one frame (if not paused)
			if (!video.pause)
			{
				if (!VideoPlayFrame(&video)) break;
			}

			// keyboard control
			switch (KeyGet())
			{
			// left (rewind back)
			case KEY_LEFT:
				VideoShiftPos(&video, -15);
				KeyFlush();
				break;

			// right (rewind forward)
			case KEY_RIGHT:
				VideoShiftPos(&video, +15);
				KeyFlush();
				break;

			// up (volume up)
			case KEY_UP:
				VideoSetVol(&video, video.volume + 1);
				Volume = video.volume;
				break;

			// down (volume down)
			case KEY_DOWN:
				VideoSetVol(&video, video.volume - 1);
				Volume = video.volume;
				break;

			// pause
			case KEY_A:
				VideoSetPause(&video, !video.pause);
				break;

			// mute
			case KEY_B:
				VideoSetMute(&video, !video.mute);
				MuteIsOn = video.mute;
				break;

			// display control
			case KEY_X:
#if USE_SCREENSHOT		// use screen shots
				ScreenShot();
#endif
				VideoSetCtrl(&video, !video.ctrl);
				CtrlIsOn = video.ctrl;
				break;

			// quit
			case KEY_Y:
				video.frame = video.frames;
				break;
			}
		}

		// close video
		VideoClose(&video);
	}

	// cannot open input file
	else
	{
		// set font 8x16
		SelFont8x16();

		// prompt
		DrawTextBg("Cannot open video file", (WIDTH-22*8)/2, (HEIGHT-8)/2, COL_YELLOW, COL_RED);
		DispUpdate();

		// wait for a key
		WaitMs(500);
		KeyFlush();
		while (KeyGet() == NOKEY) {}
	}

	DrawClear();
	DispUpdate();
}

int main()
{
	FileListSel = 0;
	Bool ok;

	while (True)
	{
		// load file list (returns False to break)
		if (!LoadFileList()) ResetToBootLoader();

		// display file list
		DispFileList();

		// video selection
		ok = False;
		while (!ok)
		{
			switch (KeyGet())
			{
			// Y exit
			case KEY_Y:
				ResetToBootLoader();
				break;

			// UP
			case KEY_UP:
				FileListSel--;
				if (FileListSel < 0) FileListSel = FileListNum-1;
				DispFileList();
				break;

			// DOWN
			case KEY_DOWN:
				FileListSel++;
				if (FileListSel >= FileListNum) FileListSel = 0;
				DispFileList();
				break;

			// LEFT
			case KEY_LEFT:
				FileListSel = 0;
				DispFileList();
				break;

			// RIGHT
			case KEY_RIGHT:
				FileListSel = FileListNum - 1;
				if (FileListSel < 0) FileListSel = 0;
				DispFileList();
				break;

			// A run
			case KEY_A:
				if (FileListSel < FileListNum) ok = True;
				break;
			}
		}

		// play selected video
		PlayVideo(FileList[FileListSel].name);
	}
}
