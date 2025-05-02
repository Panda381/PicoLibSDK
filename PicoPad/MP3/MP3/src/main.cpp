
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// MP3 player
sMP3Player MP3Player;
u8 ALIGNED MP3PlayerInBuf[MP3PLAYER_INSIZE];
u8 ALIGNED MP3PlayerOutBuf[MP3PLAYER_OUTSIZE];

Bool UpdateVol = False; // update volume config

Bool InfoMode; // info mode (no image, display MP3 info)

#define MAXFILES 500
#define ROWN	(HEIGHT/16)	// number of rows (= 240/16 = 15)
#define COLN	(WIDTH/(10*8))	// number of columns (= 320/80 = 4)
#define PAGEN	(ROWN*COLN)	// number of entries per page (= 15*4 = 60)

// Sound file
typedef struct {
	char	name[12+1];	// Sound file name
	u8	namelen;	// length of file name
	u8	dir;		// directory (without extension)
} sSoundFile;

// subdirectory
#define MAXDIR 10
#define SOUNDPATH_MAX	(APPPATH_PATHMAX+10)
char SoundPath[SOUNDPATH_MAX+1]; // path with sounds (without tailing '/')
int SoundPathLen;	// length of the path
sSoundFile DirList[MAXDIR]; // levels of subdirectories
int DirTop[MAXDIR];	// save top entry
int DirSel[MAXDIR];	// save selected entry
int SoundDirNum = 0; // current subdirectory level

// list of video files
sSoundFile FileList[MAXFILES];
int FileListNum = 0;
int FileListTop = 0;
int FileListSel = 0;
s8 Volume = CONFIG_VOLUME_FULL; // current volume

// wait mounting SD card (return False to break)
Bool WaitMount()
{
	// clear display
	DrawClear();

	// set font 8x16
	SelFont8x16();

	// display update
	DispUpdate();

	// waiting for mounting SD card
	while (!DiskMount())
	{
		// prompt
		DrawText2("Insert SD card", (WIDTH-14*8*2)/2, (HEIGHT-8*2)/2, COL_YELLOW);

		// display update
		DispUpdate();

		// break
		if (KeyGet() == KEY_Y) return False;
		WaitMs(200);
	}

	return True;
}

// load file list (returns False to break)
Bool LoadFileList()
{
	sFile find;
	sFileInfo info;
	sSoundFile* snd;
	sSoundFile tmp;
	int i;

	// waiting for mounting SD card
	if (!WaitMount()) return False;

	// clear display
	DrawClear();
	DispUpdate();
	SelFont8x16();

	// set directory
	if (!SetDir(SoundPath))
	{
		char t[50];
		MemPrint(t, 50, "No directory %d", SoundPath);
		DrawText(t, 0, 0, COL_YELLOW);
		DispUpdate();
		WaitMs(2000);
		return False;
	}

	// open search files
	if (!FindOpen(&find, ""))
	{
		DrawText("No MP3 files", 0, 0, COL_YELLOW);
		DispUpdate();
		WaitMs(2000);
		return False;
	}

	// add up-dir
	FileListNum = 0;
	if (SoundDirNum != 0)
	{
		snd = &FileList[FileListNum];
		memcpy(snd->name, "..", 3);
		snd->namelen = 2;
		snd->dir = True;
		FileListNum++;
	}

	// find subdirectories
	for (; FileListNum < MAXFILES;)
	{
		// find next file
		if (!FindNext(&find, &info, ATTR_DIR, "*.")) break;

		// fill up info (skip '..' and '.' directory)
		if ((info.name[0] != '.') && ((info.attr & ATTR_DIR) != 0))
		{
			snd = &FileList[FileListNum];
			memcpy(snd->name, info.name, 12+1);
			snd->namelen = info.namelen;
			snd->name[snd->namelen] = 0;
			snd->dir = True;
			FileListNum++;
		}
	}

	// find re-open
	FindClose(&find);
	if (FindOpen(&find, ""))
	{
		// find files
		for (; FileListNum < MAXFILES;)
		{
			// find next file
			if (!FindNext(&find, &info, ATTR_ARCH, "*.MP3")) break;

			// fill up info
			snd = &FileList[FileListNum];
			memcpy(snd->name, info.name, 12+1);
			snd->namelen = info.namelen - 4; // cut .MP3
			snd->name[snd->namelen] = 0;
			snd->dir = False;
			FileListNum++;
		}

		// find close
		FindClose(&find);
	}

	if (FileListSel >= FileListNum) FileListSel = FileListNum-1;
	if (FileListSel < 0) FileListSel = 0;

	// no MP3 and no directory
	if (FileListNum == 0)
	{
		DrawText("No MP3 files", 0, 0, COL_YELLOW);
		DispUpdate();
		WaitMs(2000);
		return False;
	}

	// sort files
	for (i = 0; i < FileListNum-1;)
	{
		if (	(FileList[i+1].name[0] == '.') ||
			(!FileList[i].dir && FileList[i+1].dir) ||
			((FileList[i].dir == FileList[i+1].dir) &&
				(FileList[i].name[0] != '.') &&
				(strcmp(FileList[i].name, FileList[i+1].name) > 0)))
		{
			memcpy(&tmp, &FileList[i], sizeof(sSoundFile));
			memcpy(&FileList[i], &FileList[i+1], sizeof(sSoundFile));
			memcpy(&FileList[i+1], &tmp, sizeof(sSoundFile));
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
	int i, namelen;
	u16 fgcol, bgcol;
	char buf[31];
	sSoundFile* snd;

	// set font 8x16
	SelFont8x16();

	// no file
	if (FileListNum == 0)
	{
		DrawClear();
		DrawText("No MP3 files", 0, 0, COL_YELLOW);
		DispUpdate();
		return;
	}

	// top correction
	i = (FileListNum+ROWN-1)/ROWN; // number of columns
	i -= COLN; // max. top column
	i *= ROWN; // max. top entry
	if (FileListTop > i) FileListTop = i; // limit max. top entry
	while (FileListSel < FileListTop) FileListTop -= ROWN;
	if (FileListTop < 0) FileListTop = 0;
	while (FileListSel >= FileListTop + PAGEN) FileListTop += ROWN;

	// display files
	int x = 0;
	int y = 0;
	for (i = FileListTop; i < FileListTop + PAGEN; i++)
	{
		// prepare color
		fgcol = COL_WHITE;
		bgcol = COL_BLACK;
		if ((i < FileListNum) && FileList[i].dir) fgcol = COL_YELLOW; // directory

		if (i == FileListSel)
		{
			fgcol = COL_BLACK;
			bgcol = COL_WHITE;
		}

		// display name
		if (i < FileListNum)
		{
			snd = &FileList[i];
			if (snd->dir)
				MemPrint(buf, 30, "[%-8s]", snd->name);
			else
				MemPrint(buf, 30, " %-8s ", snd->name);
			DrawTextBg(buf, x, y, fgcol, bgcol);
		}
		else
			DrawTextBg("          ", x, y, fgcol, bgcol);

		y += 16;
		if (y > HEIGHT-16)
		{
			x += 10*8;
			y = 0;
		}		
	}
	DispUpdate();
}

// up-dir
void PlayUpDir()
{
	SoundDirNum--;
	sSoundFile* s = &DirList[SoundDirNum];
	SoundPathLen -= s->namelen;
	if (SoundPathLen > 1) SoundPathLen--; // skip '/'
	SoundPath[SoundPathLen] = 0;
	FileListTop = DirTop[SoundDirNum];
	FileListSel = DirSel[SoundDirNum];
}

// select directory
void PlayDir()
{
	// selected directory
	sSoundFile* s = &FileList[FileListSel];	

	// up-dir
	if ((SoundDirNum > 0) && (s->name[0] == '.'))
	{
		PlayUpDir();
		return;
	}

	if (SoundDirNum >= MAXDIR) return;

	// sub-dir
	if (SoundPathLen > 1)
	{
		SoundPath[SoundPathLen] = PATHCHAR;
		SoundPathLen++;
	}

	memcpy(&SoundPath[SoundPathLen], s->name, s->namelen);
	SoundPathLen += s->namelen;	
	SoundPath[SoundPathLen] = 0;

	memcpy(&DirList[SoundDirNum], s, sizeof(sSoundFile));
	DirTop[SoundDirNum] = FileListTop;
	DirSel[SoundDirNum] = FileListSel;
	SoundDirNum++;

	FileListTop = 0;
	FileListSel = 0;
}

// display sound position
void DispSoundPos(Bool pause)
{
	char buf[20];
	sMP3Player* mp3 = &MP3Player;

	// get total length and position in seconds
	int len = MP3GetTimeLen(mp3);
	int pos = MP3GetTimePos(mp3);
#define MAXSOUNDPOS (99*60+59)
	if (len > MAXSOUNDPOS) len = MAXSOUNDPOS;
	if (pos > MAXSOUNDPOS) pos = MAXSOUNDPOS;

	// display current position (11 characters, width 88 pixels) 
	if (pause)
		MemPrint(buf, 20, "%02d:%02d/PAUSE", pos/60, pos%60);
	else
		MemPrint(buf, 20, "%02d:%02d/%02d:%02d", pos/60, pos%60, len/60, len%60);
	SelFont8x8();
	DrawTextBg(buf, 0, HEIGHT-8, COL_WHITE, COL_BLACK);

	// progress frame
	int x = 11*8;
	int w = WIDTH - 5*8 - x;
	DrawFrame(x, HEIGHT-8, w, 8, COL_BLACK);
	DrawFrame(x+1, HEIGHT-7, w-2, 6, COL_WHITE);
	x += 2;
	w -= 4;

	pos = mp3->pos;
	len = mp3->frames;
	if (pos > len) pos = len;
	if (len <= 0)
		pos = 0;
	else
		pos = (pos*w + len/2) /  len;
	DrawRect(x, HEIGHT-6, pos, 4, COL_AZURE);
	DrawRect(x + pos, HEIGHT-6, w - pos, 4, COL_BLACK);
}

// display volume
void DispVol()
{
	char buf[20];
	u8 vol = ConfigGetVolume();
	MemPrint(buf, 20, "\12%03d%%", (vol*100 + CONFIG_VOLUME_FULL/2) / CONFIG_VOLUME_FULL);
	SelFont8x8();
	u16 col = COL_WHITE;
	if (vol > CONFIG_VOLUME_FULL) col = COL_YELLOW;
	if (vol > (CONFIG_VOLUME_FULL+CONFIG_VOLUME_MAX)/2) col = COL_RED;
	DrawTextBg(buf, WIDTH-5*8, HEIGHT-8, col, COL_BLACK);
}

// display title
void DispTitle()
{
	sMP3Player* mp3 = &MP3Player;

	// prepare artist name
	char* art = mp3->id3_artist;
	int artlen = mp3->id3_artist_len;

	// prepare song name
	char* nam = mp3->id3_title;
	int namlen = mp3->id3_title_len;

	// prepare title
	char tit[53+1];
	int titlen = artlen;
	memcpy(tit, art, artlen); // add artist
	if ((artlen != 0) && (namlen != 0))
	{
		memcpy(tit + titlen, " - ", 3); // add " - " separator
		titlen += 3;
	}
	if (titlen + namlen > 53) namlen = 53 - titlen;
	memcpy(tit + titlen, nam, namlen); // add song name
	titlen += namlen;
	if (titlen == 0)
	{
		memcpy(tit, FileList[FileListSel].name, FileList[FileListSel].namelen);
		titlen = FileList[FileListSel].namelen;
	}
	tit[titlen] = 0;

	// display title
	if (titlen <= 38)
	{
		SelFont8x8();
		int x = (WIDTH - titlen*8)/2;
		DrawRect(x - 2, 0, titlen*8+4, 8, COL_BLACK);
		DrawText(tit, x, 0, COL_GREEN);
	}
	else
	{
		SelFont6x8();
		int x = (WIDTH - titlen*6)/2;
		DrawRect(x - 2, 0, titlen*6+4, 8, COL_BLACK);
		DrawText(tit, x, 0, COL_GREEN);
	}
}

// display frame
void DispFrame(int y, int h, const char* title, u16 col)
{
	SelFont8x8();
	DrawFrame(4, y + 4, WIDTH - 8, h - 8, col);
	DrawTextBg(title, 8, y, col, COL_BLACK);
}

// MPEG version
const char* MpegVerTxt[4] = {
	"MPEG1",
	"MPEG2",
	"MPEG2.5",
	"-"
};

// display info (info mode)
void DispInfo()
{
	int i;
	int y = 8 + 4;
	SelFont8x16();
	char buf[45];
	sMP3Player* mp3 = &MP3Player;

	int x1 = 8;

// ---- File

	// File frame
	DispFrame(y, 5*8, "File", COL_CYAN); y += 8;
	SelFont8x8();
	int x2 = 6*8+2;

	// File path
	DrawText("Path:", x1, y, COL_YELLOW);
	DrawText(SoundPath, x2, y, COL_WHITE);
	y += 8;

	// File name
	DrawText("Name:", x1, y, COL_YELLOW);
	sSoundFile* s = &FileList[FileListSel];
	const char* filename = s->name;
	i = s->namelen;
	memcpy(buf, filename, i);
	memcpy(buf+i, ".MP3", 5);
	DrawText(buf, x2, y, COL_WHITE);
	y += 8;

	// File size
	DrawText("Size:", x1, y, COL_YELLOW);
	MemPrint(buf, 12, "%dKB", mp3->file.size/1024);
	DrawText(buf, x2, y, COL_WHITE);
	y += 8+8;

// ---- ID3

	// ID3 frame
	DispFrame(y, 7*16, "ID3", COL_MAGENTA); y += 8;
	SelFont8x16();
	x2 = 9*8+2;

	// title
	DrawText("  Title:", x1, y, COL_YELLOW);
	DrawText(mp3->id3_title, x2, y, COL_WHITE);
	y += 16;
	
	// artist
	DrawText(" Artist:", x1, y, COL_YELLOW);
	DrawText(mp3->id3_artist, x2, y, COL_WHITE);
	y += 16;
	
	// album
	DrawText("  Album:", x1, y, COL_YELLOW);
	DrawText(mp3->id3_album, x2, y, COL_WHITE);
	y += 16;

	// comment
	DrawText("Comment:", x1, y, COL_YELLOW);
	DrawText(mp3->id3_comment, x2, y, COL_WHITE);
	y += 16;

	// genre
	DrawText("  Genre:", x1, y, COL_YELLOW);
	i = mp3->id3v1.genre;
	if (i < MP3_GENRELIST_NUM) DrawText(MP3GenreList[i], x2, y, COL_WHITE);
	y += 16;

	// year
	DrawText("   Year:", x1, y, COL_YELLOW);
	DrawText(mp3->id3_year, x2, y, COL_WHITE);

	// track
	DrawText("Track:", 18*8, y, COL_YELLOW);
	i = mp3->id3v1.track;
	if (i > 99) i = 99;
	MemPrint(buf, 12, "%02d", mp3->id3v1.track);
	DrawText(buf, 18*8+6*8+2, y, COL_WHITE);

	// Mono
	DrawText((mp3->info.nChans == 1) ? "MONO" : "STEREO", 32*8, y, COL_WHITE);
	y += 16+8;

// ---- Format

	// Format frame
	DispFrame(y, 8*8, "Format", COL_GRAY); y += 8;
	SelFont8x8();

	// Version
	DrawText("Version:", x1, y, COL_YELLOW);
	DrawText(MpegVerTxt[mp3->info.version], x2, y, COL_WHITE);

	// Layer
	DrawText("Layer:", 23*8, y, COL_YELLOW);
	DrawChar('0' + mp3->info.layer, 23*8+6*8+2, y, COL_WHITE);
	y += 8;

	// Bitrate
	DrawText("Bitrate:", x1, y, COL_YELLOW);
	MemPrint(buf, 12, "%dkbps", mp3->bitrateavg/1000);
	DrawText(buf, x2, y, COL_WHITE);

	// Sample rate
	DrawText("SampRate:", 20*8, y, COL_YELLOW);
	MemPrint(buf, 12, "%dHz", mp3->info.samprate);
	DrawText(buf, 20*8+9*8+2, y, COL_WHITE);
	y += 8;

	// Frames
	DrawText(" Frames:", x1, y, COL_YELLOW);
	MemPrint(buf, 12, "%d", mp3->frames);
	DrawText(buf, x2, y, COL_WHITE);

	// Frame time
	DrawText("FrameTime:", 19*8, y, COL_YELLOW);
	MemPrint(buf, 12, "%dus", mp3->frametime);
	DrawText(buf, 19*8+10*8+2, y, COL_WHITE);
	y += 8;

	// Frame In size	
	DrawText("FrameIn:", x1, y, COL_YELLOW);
	MemPrint(buf, 12, "%dB", mp3->framesizeavg);
	DrawText(buf, x2, y, COL_WHITE);

	// Frame Out size	
	DrawText("FrameOut:", 20*8, y, COL_YELLOW);
	MemPrint(buf, 12, "%dB", mp3->sampnumavg*2);
	DrawText(buf, 20*8+9*8+2, y, COL_WHITE);
	y += 8;

	// Poll load
	SelFont8x8();
	DrawText("   Poll:", x1, y, COL_YELLOW);
	DrawText("buf:", 30*8, y, COL_YELLOW);
	y += 8;

	// Decode load
	DrawText(" Decode:", x1, y, COL_YELLOW);

}

u32 DispLoadInfoOldTime;

// display load info (info mode)
void DispLoadInfo()
{
	// check interval
	u32 t = Time();
	if ((u32)(t - DispLoadInfoOldTime) < 1000000) return;
	DispLoadInfoOldTime = t;

	// get load info
	sMP3Player* mp3 = &MP3Player;
	di();
	u32 decode_time = mp3->decode_time;
	int decode_num = mp3->decode_num;
	int decode_outrem = mp3->decode_outrem;
	int decode_inrem = mp3->decode_inrem;
	u32 poll_time = mp3->poll_time;
	int poll_len = mp3->poll_len;
	int poll_rem = mp3->poll_rem;
	mp3->decode_time = 0;
	mp3->decode_num = 0;
	mp3->decode_outrem = mp3->outsize;
	mp3->decode_inrem = mp3->insize;
	mp3->poll_time = 0;
	mp3->poll_len = 0;
	mp3->poll_rem = mp3->insize;
	ei();

	SelFont8x8();
	char buf[30];
	int y = 25*8 + 4;

	// average Poll time
	if (poll_len < 1) poll_len = 1;
	poll_time = (int)((s64)poll_time * mp3->framesizeavg / poll_len);
	int i = MemPrint(buf, 30, "%dus/frame (%d%%)", poll_time, poll_time*100/mp3->frametime);
	DrawRect(9*8+i*8, y, 21*8-i*8, 8, COL_BLACK);
	DrawTextBg(buf, 9*8+2, y, COL_WHITE, COL_BLACK);

	// Poll buffer
	i = MemPrint(buf, 30, "%d%%", poll_rem*100/mp3->insize);
	DrawRect(34*8+i*8, y, 5*8-i*8, 8, COL_BLACK);
	DrawTextBg(buf, 34*8+2, y, COL_WHITE, COL_BLACK);
	y += 8;

	// average Decode time
	if (decode_num < 1) decode_num = 1;
	decode_time = (decode_time + decode_num/2) / decode_num;
	i = MemPrint(buf, 30, "%dus/frame (%d%%)", decode_time, decode_time*100/mp3->frametime);
	DrawRect(9*8+i*8, y, 21*8-i*8, 8, COL_BLACK);
	DrawTextBg(buf, 9*8+2, y, COL_WHITE, COL_BLACK);

	// decode IN buffer, decode OUT buffer
	i = MemPrint(buf, 30, "%d%%,%d%%", decode_inrem*100/mp3->insize, decode_outrem*100/mp3->outsize);
	DrawRect(30*8+i*8, y, 9*8-i*8, 8, COL_BLACK);
	DrawTextBg(buf, 30*8, y, COL_WHITE, COL_BLACK);
}

// load BMP image
void PlayBMP()
{
	sBmp bmp;

	// selected file
	sSoundFile* s = &FileList[FileListSel];
	const char* filename = s->name;
	int len = s->namelen;

	// load image
	char buf[20];
	memcpy(buf, filename, len);
	memcpy(buf+len, ".BMP", 5);

	InfoMode = True; // info mode (no image, display MP3 info)

	sFile f;
	if (FileOpen(&f, buf)) // open BMP file
	{
		// read BMP header
		int n = FileRead(&f, &bmp, sizeof(sBmp));

		// base check BMP
		if (	(n == sizeof(sBmp)) &&
			(bmp.bfType == 0x4D42) &&
			(bmp.biBitCount == 16) &&
			(bmp.biWidth == WIDTH))
		{
			// seek to data
			FileSeek(&f, bmp.bfOffBits);

			// load image
			n = FileRead(&f, FrameBuf, WIDTH*HEIGHT*2);
			if (n == WIDTH*HEIGHT*2) InfoMode = False;
		}
		FileClose(&f);
	}
	if (InfoMode) DrawClear();
}

// play selected sound
void PlayMedia()
{
	// waiting for mounting SD card
	if (!WaitMount()) return;

	// set directory
	if (!SetDir(SoundPath)) return;

PlayNextSound:

	// selected file
	sSoundFile* s = &FileList[FileListSel];
	const char* filename = s->name;
	int len = s->namelen;

	// load BMP image
	PlayBMP();

	// open MP3 file
	sMP3Player* mp3 = &MP3Player;
	char buf[20];
	memcpy(buf, filename, len);
	memcpy(buf+len, ".MP3", 5);
	int r = MP3PlayerInit(mp3, buf, MP3PlayerInBuf, MP3PLAYER_INSIZE, MP3PlayerOutBuf, MP3PLAYER_OUTSIZE, 100);

	// error
	if (r != ERR_MP3_NONE)
	{
		DrawClear();
		SelFont8x16();
		DrawText("Error opening MP3 file", 0, 0, COL_YELLOW);
		DispUpdate();
		WaitMs(2000);
		return;
	}

	// start playing
	MP3Play(mp3, 0, False);

	// display volume
	DispVol();

	// display title
	DispTitle();

	// display info (info mode)
	if (InfoMode) DispInfo();

	DispLoadInfoOldTime = Time();

	KeyFlush();

	// play sound
	Bool pause = False;
	while (MP3Playing(mp3) || pause)
	{
		// polling MP3
		MP3Poll(mp3);

		// display load info (info mode)
		if (InfoMode) DispLoadInfo();

		// display sound position
		DispSoundPos(pause);
		DispUpdate();

		// keyboard control
		switch (KeyGet())
		{
		// left (rewind back)
		case KEY_LEFT:
			MP3SeekTime(mp3, MP3GetTimePos(mp3) - 10);
			KeyFlush();
			break;

		// right (rewind forward)
		case KEY_RIGHT:
			MP3SeekTime(mp3, MP3GetTimePos(mp3) + 10);
			KeyFlush();
			break;

		// up (volume up)
		case KEY_UP:
			ConfigIncVolume();
			DispVol();
			KeyFlush();
			UpdateVol = True;
			break;

		// down (volume down)
		case KEY_DOWN:
			ConfigDecVolume();
			DispVol();
			KeyFlush();
			UpdateVol = True;
			break;

		// previous
		case KEY_A:
			MP3PlayerTerm(mp3);
			do {
				FileListSel--;
				if (FileListSel < 0) FileListSel = FileListNum-1;
			} while (FileList[FileListSel].dir);
			KeyFlush();
			goto PlayNextSound;

		// next
		case KEY_B:
			MP3PlayerTerm(mp3);
			do {
				FileListSel++;
				if (FileListSel >= FileListNum) FileListSel = 0;
			} while (FileList[FileListSel].dir);
			KeyFlush();
			goto PlayNextSound;

		// pause, info, screen shot
		case KEY_X:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif

			if (pause)
			{
				MP3Play(mp3, 0, False);
				pause = False;
			}
			else
			{
				MP3Stop(mp3);
				pause = True;

				if (InfoMode)
				{
					// load BMP image
					PlayBMP();
				}
				else
				{
					DrawClear();
					InfoMode = True;
				}

				// display volume
				DispVol();

				// display title
				DispTitle();

				// display info (info mode)
				if (InfoMode)
				{
					DispInfo();
					DispLoadInfo();
				}
			}
			KeyFlush();
			break;

		// quit
		case KEY_Y:
			if (UpdateVol)
			{
				ConfigSave();
				UpdateVol = False;
			}
			MP3PlayerTerm(mp3);
			KeyFlush();
			return;
		}
	}

	// save volume config
	if (UpdateVol)
	{
		ConfigSave();
		UpdateVol = False;
	}

	// flush buffers
	WaitMs(150);

	// close sound
	MP3PlayerTerm(mp3);

	// next sound
	do {
		FileListSel++;
		if (FileListSel >= FileListNum) FileListSel = 0;
	} while (FileList[FileListSel].dir);
	goto PlayNextSound;
}

int main()
{
	FileListSel = 0;
	SoundDirNum = 0;
	Bool ok;
	int k;

#if !USE_PICOPADHSTX		// use PicoPadHSTX device configuration
	ClockPllSysFreqVolt(252000);
#endif

	// get path with sounds
	SoundPathLen = GetHomePath(SoundPath, "/MP3");

	while (True)
	{
		// load file list (returns False to exit)
		while (True)
		{
			if (LoadFileList()) break;
			if (SoundDirNum == 0) ResetToBootLoader();
			PlayUpDir(); // go up-dir
		}

		// display file list
		DispFileList();

		// sound selection
		ok = False;
		while (!ok)
		{
			switch (KeyGet())
			{
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
				FileListSel -= ROWN;
				if (FileListSel < 0) FileListSel = 0;
				DispFileList();
				break;

			// RIGHT
			case KEY_RIGHT:
				FileListSel += ROWN;
				if (FileListSel >= FileListNum) FileListSel = FileListNum - 1;
				if (FileListSel < 0) FileListSel = 0;
				DispFileList();
				break;

			// A run
			case KEY_A:
				if (FileListSel < FileListNum) ok = True;
				break;

			// Y exit
			case KEY_Y:
				if ((SoundDirNum > 0) && (FileListNum > 0))
				{
					FileListSel = 0;
					ok = True;
				}
				else
					ResetToBootLoader();
				break;

			// B play first
			case KEY_B:
				for (k = 0; k < FileListNum; k++)
				{
					if (!FileList[k].dir)
					{
						FileListSel = k;
						ok = True;
						break;
					}
				}
				KeyFlush();
				break;
			}
		}

		// play selected sound or select directory
		if (FileList[FileListSel].dir)
			PlayDir();
		else
			PlayMedia();
	}
}
