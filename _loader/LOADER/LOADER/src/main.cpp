
// ****************************************************************************
//
//                                Boot3 Loader
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include "../include.h"

// boot loader resident segment
u8 __attribute__((section(".bootloaderdata"))) LoaderData[BOOTLOADER_DATA];

// custom frame buffer - used as buffer to load UF2 program into RAM
ALIGNED FRAMETYPE FrameBuf[CUSTOM_FRAMEBUF_SIZE];

// display
COLTYPE FgCol, BgCol; // foreground and background color
int DispX, DispY; // X and Y text coordinate

// temporary buffer
#define TEMPBUF 1024 // size of temporary buffer
char TempBuf[TEMPBUF+1];
int TempBufNum; // number of bytes in temporary buffer
int TempBufInx; // current index in temporary buffer

// files
char Path[PATHMAX+1]; // current path (with terminating 0)
int PathLen; // length of path
u32 LastMount; // last mount time
Bool Remount; // remount request
sFileDesc FileDesc[MAXFILES]; // file descriptors
int FileNum; // number of files
int FileCur; // index of current file
int FileTop; // index of first visible file
sFile FileF; // search structure
sFileInfo FileI; // search file info

// seek cursor to last name
char LastName[8];
int LastNameLen; // length of last name, 0 = not used
u8 LastNameDir; // ATTR_DIR flag of last name

// preview
enum  {
	PREV_START,	// waiting for start
	PREV_TXT_LOAD,	// loading text
	PREV_BMP_START,	// waiting for bitmap start
	PREV_BMP_LOAD,	// loading bitmap
	PREV_STOP,	// preview not active
};	

u8 PrevState; // current preview state
sFileDesc* PrevFD; // preview file descriptor
int PrevLine; // current preview line
sFile PrevFile; // preview file (name[0] = 0 if not open)
int PrevW, PrevH; // size of preview image

// set home position
void DispHome()
{
	DispX = 0;
	DispY = 0;
}

// set print colors
void DispCol(COLTYPE fgcol, COLTYPE bgcol)
{
	FgCol = fgcol;
	BgCol = bgcol;
}

// display text
void DispText(const char* text)
{
	SelFont8x16();
	DrawTextBg(text, DispX*FONTW, DispY*FONTH, FgCol, BgCol);
	DispX += StrLen(text);
}

// display small text
void DispSmallText(const char* text)
{
	SelFont6x8();
	DrawTextBg(text, DispX*FONTW2, DispY*FONTH2, FgCol, BgCol);
	DispX += StrLen(text);
}

// print character
void DispChar(char ch)
{
	char buf[2];
	buf[0] = ch;
	buf[1] = 0;
	DispText(buf);
}

// print small character
void DispSmallChar(char ch)
{
	char buf[2];
	buf[0] = ch;
	buf[1] = 0;
	DispSmallText(buf);
}

// print space
void DispSpc()
{
	DispChar(' ');
}

// display character repeated
void DispCharRep(char ch, int num)
{
	for (; num > 0; num--) DispChar(ch);
}

// display space repeated
void DispSpcRep(int num)
{
	for (; num > 0; num--) DispSpc();
}

// save boot loader data
void SaveBootData()
{
	if ((uint)FileCur < (uint)FileNum)
	{
		sFileDesc* fd;
		fd = &FileDesc[FileCur];
		LoaderData[0] = fd->len;
		LoaderData[1] = fd->attr & ATTR_DIR;
		memcpy(LoaderData+2, fd->name, 8);
		SetDir(Path);
		*(u32*)&LoaderData[12] = CurDirClust;
		*(u32*)&LoaderData[16] = FileTop;
		*(u32*)&LoaderData[20] = Crc32ADMA(LoaderData, 20);
	}
}

// load boot loader data
void LoadBootData()
{
	if (*(u32*)&LoaderData[20] == Crc32ADMA(LoaderData, 20))
	{
		LastNameLen = LoaderData[0];
		LastNameDir = LoaderData[1];
		memcpy(LastName, LoaderData+2, 8);
		CurDirClust = *(u32*)&LoaderData[12];
		FileTop = *(u32*)&LoaderData[16];
		PathLen = GetDir(Path, PATHMAX);
	}
	LoaderData[0] = 0;
}

// display frame of file list
void FrameFileList()
{
	// reset cursor
	DispHome();

	// top frame
	DispCol(COL_FILEBG, COL_FILEFG); // invert color
	DispChar(0x9c); // left frame
	DispCharRep(0x95, FILECOLW-2); // row
	DispChar(0x99); // right frame
	DispY++;

	// inner rows
	do {
		DispX = 0; // reset column
		DispChar(0x9a); // left frame
		DispCol(COL_FILEFG, COL_FILEBG); // normal color
		DispSpcRep(FILECOLW-2); // row
		DispCol(COL_FILEBG, COL_FILEFG); // invert color
		DispChar(0x9a); // right frame
		DispY++; // increment row
	} while (DispY != TEXTH-1);

	// bottom frame
	DispX = 0; // reset column
	DispChar(0x96); // left frame
	DispCharRep(0x95, FILECOLW-2); // row
	DispChar(0x93); // right frame
}

// display current path
void PathFileList()
{
	// prepare path length
	int len = PathLen;

	// reset cursor
	DispY = 0;
	DispX = 1;

// len <= FILECOLW-4 ... | top_line + space + path + space + top_line |
// len == FILECOLW-3 ... | path + space |
// len == FILECOLW-2 ... | path |
// len > FILECOLW-2 ... | "..." short_path |

	// left part of top line, left space
	if (len <= FILECOLW-4)
	{
		// left part of top line
		DispCol(COL_FILEBG, COL_FILEFG); // invert color
		DispCharRep(0x95, (FILECOLW-4 - len)/2); // line

		// left space
		DispCol(COL_TITLEFG, COL_TITLEBG);
		DispSpc();
	}

	// path
	DispCol(COL_TITLEFG, COL_TITLEBG);
	if (len <= FILECOLW-2)
	{
		// full path
		DispText(Path);
	}
	else
	{
		// short path
		DispCharRep('.', 3);
		DispText(Path + len - (FILECOLW-2-3));
	}

	// right space
	if (len <= FILECOLW-3) DispSpc();
	
	// right part of top line
	if (len <= FILECOLW-4)
	{
		// right part of top line
		DispCol(COL_FILEBG, COL_FILEFG); // invert color
		DispCharRep(0x95, (FILECOLW-4 - len + 1)/2); // line
	}
}

// display current index of selected file
void InxFileList()
{
	if (FileNum == 0) return;

	// prepare text
	int n = DecNum(TempBuf, FileCur+1, 0);
	TempBuf[n++] = '/';
	n += DecNum(&TempBuf[n], FileNum, 0);

	// reset cursor
	DispY = TEXTH-1;
	DispX = 1;

	// left part of bottom line
	DispCol(COL_FILEBG, COL_FILEFG); // invert color
	DispCharRep(0x95, (FILECOLW-4 - n)/2); // line

	// left space
	DispCol(COL_FILEFG, COL_FILEBG);
	DispSpc();

	// text
	DispText(TempBuf);

	// right space
	DispSpc();
	
	// right part of bottom line
	DispCol(COL_FILEBG, COL_FILEFG); // invert color
	DispCharRep(0x95, (FILECOLW-4 - n + 1)/2); // line
}

// dispay file list
void DispFileList()
{
	// reset cursor
	DispY = 1;

	// display files
	int i, j;
	char ch;
	Bool dir;
	sFileDesc* fd = &FileDesc[FileTop];
	for (i = 0; i < FILEROWS; i++)
	{
		DispX = 1;

		// set normal color
		DispCol(COL_FILEFG, COL_FILEBG);

		// entry is valid
		j = i + FileTop;
		if (j < FileNum)
		{
			// check directory
			dir = ((fd->attr & ATTR_DIR) != 0);

			// set directory color
			if (dir) DispCol(COL_DIRFG, COL_DIRBG);

			// set cursor color
			if (j == FileCur) DispCol(COL_CURFG, COL_CURBG);

			// directory mark '['
			ch = dir ? '[' : ' ';
			DispChar(ch);

			// decode entry name
			for (j = 0; j < fd->len; j++) DispChar(fd->name[j]);

			// directory mark ']'
			ch = dir ? ']' : ' ';
			DispChar(ch);

			// display spaces
			DispSpcRep(9 - fd->len);

			// TXT mark
			if ((fd->attr & ATTR_TXT) != 0)
			{
				DispText("TXT");
			}
			else
				DispSpcRep(3);

			// space
			DispSpc();

			// BMP mark
			if ((fd->attr & ATTR_BMP) != 0)
			{
				DispText("BMP");
			}
			else
				DispSpcRep(3);
		}
		else
			// clear invalid row
			DispSpcRep(FILECOLW-2);

		// increase file
		DispY++;
		fd++;
	}

	// display current index
	InxFileList();

	// restart preview state
	PrevState = PREV_START;
}

// load files (programs with UF2 extension, directory without extension)
void LoadFileList()
{
	// clear file list
	FileNum = 0;
	FileCur = 0;

	// set current directory
	if (!SetDir(Path)) return;

	// open search 
	if (!FindOpen(&FileF, "")) return;

	// load files
	int inx, i, len;
	sFileDesc* fd = FileDesc;
	char ch;
	char* name;
	Bool dir;
	for (inx = 0; inx < MAXFILES; inx++)
	{
		// find next file
		if (!FindNext(&FileF, &FileI, ATTR_DIR_MASK, "*.*")) break;

		// check directory
		dir = (FileI.attr & ATTR_DIR) != 0;

		// skip directory "."
		len = FileI.namelen;
		name = FileI.name;
		if (dir && (len == 1) && (name[0] == '.')) continue;

		// get attributes
		fd->attr = FileI.attr & ATTR_MASK;

		// copy directory ".."
		if (dir && (len == 2) && (name[0] == '.') && (name[1] == '.'))
		{
			fd->len = len;
			fd->name[0] = '.';
			fd->name[1] = '.';
			fd++;
			FileNum++;
		}
		else
		{
			// skip hidden entry
			if ((FileI.attr & ATTR_HID) != 0) continue;

			// directory
			if (dir)
			{
				// copy directory name (without extension)
				for (i = 0; (i < len) && (i < 8); i++)
				{
					ch = name[i];
					if (ch == '.') break;
					fd->name[i] = ch;
				}

				// directory is valid only if has no extension
				if (i == len)
				{
					fd->len = len;
					fd++;
					FileNum++;
				}
			}

			// file
			else
			{
				// check extension "UF2"
				if ((len > 4) && (name[len-4] == '.') && (name[len-3] == 'U') &&
					 (name[len-2] == 'F') && (name[len-1] == '2'))
				{
					// copy file name (without extension)
					fd->len = len-4;
					memcpy(fd->name, name, len-4);
					fd++;
					FileNum++;
				}
			}
		}
	}

	// open search again, to check info files
	if (FindOpen(&FileF, ""))
	{
		// find next file
		while (FindNext(&FileF, &FileI, ATTR_DIR_MASK, "*.*"))
		{
			// skip directory
			if ((FileI.attr & ATTR_DIR) != 0) continue;

			// check extension "TXT"
			len = FileI.namelen;
			if (len > 4)
			{
				len -= 4;

				name = FileI.name;
				if ((name[len] == '.') && (name[len+1] == 'T') &&
					 (name[len+2] == 'X') && (name[len+3] == 'T'))
				{
					// search this file name
					sFileDesc* fd = FileDesc;
					for (inx = 0; inx < FileNum; inx++)
					{
						// compare file name length
						if (fd->len == len)
						{
							// compare file names, set TXT flag
							if (memcmp(fd->name, name, len) == 0) fd->attr |= ATTR_TXT;
						}
						fd++;
					}
				}
				else
				{
					// check extension "BMP"
					if ((name[len] == '.') && (name[len+1] == 'B') &&
						 (name[len+2] == 'M') && (name[len+3] == 'P'))
					{
						// search this file name
						sFileDesc* fd = FileDesc;
						for (inx = 0; inx < FileNum; inx++)
						{
							// compare file name length
							if (fd->len == len)
							{
								// compare file names, set BMP flag
								if (memcmp(fd->name, name, len) == 0) fd->attr |= ATTR_BMP;
							}
							fd++;
						}
					}
				}
			}
		}
	}

	// sort files (using bubble sort)
	fd = FileDesc;
	for (inx = 0; inx < FileNum-1;)
	{
		Bool ok = True;

		// directory '..' must be at first place
		if ((fd[1].len == 2) && (fd[1].name[0] == '.') && (fd[1].name[1] == '.')) ok = False;

		// directory must be before the files
		if (((fd[0].attr & ATTR_DIR) == 0) && ((fd[1].attr & ATTR_DIR) != 0)) ok = False;

		// entry of the same group
		if (((fd[0].attr ^ fd[1].attr) & ATTR_DIR) == 0)
		{
			// compare names
			len = fd[0].len;
			if (fd[1].len < fd[0].len) len = fd[1].len;
			for (i = 0; i < len; i++)
			{
				if (fd[0].name[i] != fd[1].name[i]) break;
			}

			if (i < len) // names are different
			{
				if (fd[0].name[i] > fd[1].name[i]) ok = False;
			}
			else // names are equal, check name lengths
			{
				if (fd[1].len < fd[0].len) ok = False;
			}
		}

		// exchange files
		if (!ok)
		{
			ch = fd[0].attr;
			fd[0].attr = fd[1].attr;
			fd[1].attr = ch;

			ch = fd[0].len;
			fd[0].len = fd[1].len;
			fd[1].len = ch;

			for (i = 0; i < 8; i++)
			{
				ch = fd[0].name[i];
				fd[0].name[i] = fd[1].name[i];
				fd[1].name[i] = ch;
			}

			// shift index down
			if (inx > 0)
			{
				inx -= 2;
				fd -= 2;
			}
		}

		// shift index up
		inx++;
		fd++;
	}
}

// request to reload current directory
void Reload()
{
	// reset file list
	FileNum = 0; // no entry
	FileCur = 0; // reset cursor
	FileTop = 0; // reset top entry

	// preview state not active
	PrevState = PREV_STOP;

	// display frame of file list
	FrameFileList();

	// display current path
	PathFileList();

	// set flag - disk need to be remounted
	Remount = True;
}

// reset to root
void ResetRoot()
{
	// reset path to root
	Path[0] = PATHCHAR;
	Path[1] = 0;
	PathLen = 1;

	// request to reload current directory
	Reload();
}

// set cursor to last name
void SetLastName()
{
	sFileDesc* fd;
	int i;

	// no last name required
	if (LastNameLen == 0) return;

	// search last name in file list
	for (FileCur = FileNum-1; FileCur > 0; FileCur--)
	{
		fd = &FileDesc[FileCur];
		if ((fd->len == LastNameLen) &&
			((fd->attr & ATTR_DIR) == LastNameDir) &&
			(memcmp(LastName, fd->name, LastNameLen) == 0)) break;
	}

	// limit max. top entry
	if (FileTop > FileCur) FileTop = FileCur;

	// limit min. top entry
	i = FileCur - (FILEROWS-1);
	if (i < 0) i = 0;
	if (FileTop < i) FileTop = i;

	// delele last name request
	LastNameLen = 0;
}

// display info text
void DispInfo(const char* text)
{
	// prepare length of into text
	int len = StrLen(text);

	// set text color and coordinated
	DispCol(COL_INFOFG, COL_INFOBG);
	DispY = 2;
	DispX = (FILECOLW - len)/2;

	// display info text
	DispText(text);
}

// loading next byte from temporary buffer (returns 0 if no next byte)
u8 PrevChar()
{
	// check if buffer need to be loaded
	if (TempBufInx >= TempBufNum)
	{
		// check if file is open
		if (!FileIsOpen(&PrevFile)) return 0;

		// read next temporary buffer
		TempBufInx = 0;
		TempBufNum = FileRead(&PrevFile, TempBuf, TEMPBUF);

		// end of file
		if (TempBufNum < TEMPBUF) FileClose(&PrevFile);

		// no data
		if (TempBufNum == 0) return 0;
	}

	// get next byte
	return TempBuf[TempBufInx++];
}

// loading next byte from temporary buffer, skip CR (returns 0 if no next byte)
char PrevCharCR()
{
	char ch;
	do {
		ch = PrevChar();
	} while (ch == CH_CR);
	return ch;
}

// clear preview panel
void PreviewClr()
{
	DrawRect(WIDTH/2, 0, WIDTH/2, HEIGHT, COL_BLACK);
}

// display preview
void Preview()
{
	int i, j;
	char ch;
	u8 inv;
	sBmp* bmp;
	COLTYPE* dst;

	switch(PrevState)
	{
	// waiting for start
	case PREV_START:

		// close old preview file
		FileClose(&PrevFile);

		// clear preview panel
		PreviewClr();

		// check if current file is valid
		if ((uint)FileCur >= (uint)FileNum)
		{
			// no current file
			PrevState = PREV_STOP;
			return;
		}

		// prepare file descriptor
		PrevFD = &FileDesc[FileCur];

		// no text file
		if ((PrevFD->attr & ATTR_TXT) == 0)
		{
			// waitting for bitmap start
			PrevState = PREV_BMP_START;
			return;
		}

		// prepare filename of text file
		memcpy(TempBuf, PrevFD->name, PrevFD->len);
		memcpy(&TempBuf[PrevFD->len], ".TXT", 5);

		// open text file
		SetDir(Path);
		if (!FileOpen(&PrevFile, TempBuf))
		{
			// cannot open text file, waitting for bitmap start
			PrevState = PREV_BMP_START;
			return;
		}
		TempBufNum = 0; // no data in temporary buffer
		TempBufInx = 0; // reset current index in temporary buffer
		PrevLine = 0; // reset current preview line

		// loading text file
		PrevState = PREV_TXT_LOAD;
		break;

	// loading text
	//	- text width = 26 characters
	//	- ^ is prefix of control characters
	//		^^ ... print ^ character
	//		^1..^9 ... print character with code 1..9
	//		^A..^V ... print character with code 10..31
	//		^0 ... normal gray text
	//		^W ... white text
	//		^X ... green text
	//		^Y ... yellow text
	//		^Z ... red text
	//		^[ ... start invert
	//		^] ... stop invert
	case PREV_TXT_LOAD:

		// set normal text color
		DispCol(COL_TEXTFG, COL_TEXTBG);
		inv = 0;

		// set coordinates
		DispX = (TEXTW2+1)/2;
		DispY = PrevLine;

		// decode one row (i = relative character position)
		for (i = 0; i < TEXTW2/2;)
		{
			// load next character, skip CR characters
			ch = PrevCharCR();

			// LF end of line or NUL end of file
			if ((ch == CH_LF) || (ch == 0)) break;

			// tabulator (modulo 8)
			if (ch == CH_TAB)
			{
				do {
					DispSmallChar(' ' ^ inv);
					i++; // increase character position
				} while ((i < TEXTW2/2) && ((i & 7) != 0));
			}
			else
			{
				// prefix character ^
				if (ch == '^')
				{
					// load next character
					ch = PrevCharCR();

					// LF end of line or NUL end of file
					if ((ch == CH_LF) || (ch == 0))
						break;

					// set white text
					else if (ch == 'W')
						DispCol(COL_WHITE, COL_TEXTBG);

					// set green text
					else if (ch == 'X')
						DispCol(COL_GREEN, COL_TEXTBG);

					// set yellow text
					else if (ch == 'Y')
						DispCol(COL_YELLOW, COL_TEXTBG);
						
					// set red text
					else if (ch == 'Z')
						DispCol(COL_RED, COL_TEXTBG);
						
					// set normal text
					else if (ch == '0')
						DispCol(COL_TEXTFG, COL_TEXTBG);

					// print character with code 1..9
					else if ((ch >= '1') && (ch <= '9'))
					{
						DispSmallChar((ch - '0') ^ inv);
						i++; // increase character position
					}

					// print character with code 10..31
					else if ((ch >= 'A') && (ch <= 'V'))
					{
						DispSmallChar((ch - 'A' + 10) ^ inv);
						i++; // increase character position
					}

					// start invert
					else if (ch == '[')
						inv = 0x80;

					// stop invert
					else if (ch == ']')
						inv = 0;

					// print ^ character or other normal character
					else
					{
						DispSmallChar(ch ^ inv);
						i++; // increase character position
					}
				}
				else
				{
					// display one character
					DispSmallChar(ch ^ inv);
					i++; // increase character position
				}
			}
		}

		// skip rest of the row
		if (i == TEXTW2/2)
		{
			// find LF end of line or NUL end of file
			do {
				ch = PrevCharCR();
			} while ((ch != CH_LF) && (ch != 0));
		}

		// prepare number of rows
		i = ((PrevFD->attr & ATTR_BMP) == 0) ? TEXTH2 : (TEXTH2/2);

		// increase curren row
		PrevLine++;
		if (PrevLine >= i)
		{
			// close text file
			FileClose(&PrevFile);

			// start loading bitmap
			PrevState = PREV_BMP_START;
		}
		break;

	// waiting for bitmap start
	case PREV_BMP_START:

		// no bitmap file
		if ((PrevFD->attr & ATTR_BMP) == 0)
		{
			// stop loading
			PrevState = PREV_STOP;
			return;
		}

		// prepare filename of bitmap file
		memcpy(TempBuf, PrevFD->name, PrevFD->len);
		memcpy(&TempBuf[PrevFD->len], ".BMP", 5);

		// open bitmap file
		SetDir(Path);
		if (!FileOpen(&PrevFile, TempBuf))
		{
			// cannot open bitmap file, stop
			PrevState = PREV_STOP;
			return;
		}
		TempBufNum = 0; // no data in temporary buffer
		TempBufInx = 0; // reset current index in temporary buffer

		// load bitmap header
		i = FileRead(&PrevFile, TempBuf, sizeof(sBmp));

		// get size of preview image
		bmp = (sBmp*)TempBuf;
		PrevW = (bmp->biWidth + 1) & ~1; // width aligned to DWORD
		PrevH = bmp->biHeight;
		if (PrevH < 0) PrevH = -PrevH; // negative height -> flip row order

		// check bitmap header
		if ((i != sizeof(sBmp)) ||
			(PrevW < 8) || (PrevW > 2048) ||
			(PrevH < 8) || (PrevH > 2048) ||
			(bmp->bfType != 0x4D42) ||
			(bmp->bfSize < 100) || (bmp->bfSize > 5000000) ||
			(bmp->bfOffBits < 54) || (bmp->bfOffBits > 2000) ||
			(bmp->biPlanes != 1) ||
			(bmp->biBitCount != COLBITS) /*||
			(bmp->biCompression != 0)*/)
		{
			FileClose(&PrevFile);

			// cannot open bitmap file, stop
			PrevState = PREV_STOP;
			return;
		}

		// set start of first line
		FileSeek(&PrevFile, bmp->bfOffBits);

		// prepare first video line
		PrevLine = ((PrevFD->attr & ATTR_TXT) == 0) ? 0 : (HEIGHT/2);

		// loading bitmap file
		PrevState = PREV_BMP_LOAD;
		break;

	// loading bitmap
	case PREV_BMP_LOAD:

		// prepare address in video memory
#if COLBITS == 4
		dst = &FrameBuf[PrevLine*WIDTHLEN + WIDTH/4];
#else
		dst = &FrameBuf[PrevLine*WIDTH + WIDTH/2];
#endif

		// prepare size of data to read from one line
		i = (PrevW > (WIDTH/2)) ? (WIDTH/2) : PrevW;

		// read one video line
		FileRead(&PrevFile, dst, (i*COLBITS+7)/8);
		DispDirtyRect(WIDTH/2, PrevLine, WIDTH/2, 1);

		// skip rest of line
		if (PrevW > (WIDTH/2)) FileSeek(&PrevFile, FilePos(&PrevFile) + (PrevW - (WIDTH/2))*COLBITS/8);

		// increase line
		PrevLine++;
		PrevH--;

		// end of image
		if ((PrevLine >= HEIGHT) || (PrevH <= 0))
		{
			// close preview file
			FileClose(&PrevFile);

			// stop preview
			PrevState = PREV_STOP;
		}
		break;

	// preview not active
	// PREV_STOP
	default:
		break;
	}
}

// check application in memory
Bool CheckApp()
{
	// start of application
	const u32* app = (const u32*)(XIP_BASE + BOOTLOADER_SIZE);

	// application header
	const u32* h = &app[48]; // 16+32 vectors

	// check header base
	if (h[0] != 0x44415050) return False; // check magic "PPAD"

	// get application size
	int len = h[1];
	if ((len < 10) || (len > 2*1024*1024)) return False;

	// check application CRC
	u32 crc = Crc32ADMA(&app[51], len);
	return crc == h[2];
}

// runtime terminate
extern "C" void RuntimeTerm();

// run application
extern "C" void GoToApp();

// run application
void RunApp()
{
	// wait for no key pressed
	KeyWaitNoPressed();

	// runtime terminate
	RuntimeTerm();

	// run application
	GoToApp();
}

// display progress bar
void Progress(int i, int n, int y, u16 col)
{
	// wait for VSync scanline
	VgaWaitVSync();

#define PROGRESS_X 32
#define PROGRESS_W 256
#define PROGRESS_H 16

	int w = i*PROGRESS_W/n;
	if (w > PROGRESS_W) w = PROGRESS_W;

	DrawFrame(PROGRESS_X-2, y-2, PROGRESS_W+4, PROGRESS_H+4, COL_WHITE);
	DrawRect(PROGRESS_X, y, PROGRESS_W, PROGRESS_H, COL_GRAY);
	DrawRect(PROGRESS_X, y, w, PROGRESS_H, col);
	DispUpdate();
}

// display big info text
void DispBigInfo(const char* text)
{
	DrawClear();
	int len = StrLen(text);
	SelFont8x16();
	DrawTextBg2(text, (WIDTH - len*16)/2, 88, COL_BIGINFOFG, COL_BIGINFOBG);
	DispUpdate();
}

// clear program memory
void ClearApp()
{
	// display info
	DispBigInfo("Erasing...");

	// find end of memory (last 4 KB are reserver for config)
	u32 addr = XIP_BASE + BOOTLOADER_SIZE;
	u32 count = 2*1024*1024 - 4096 - BOOTLOADER_SIZE;
	const u32* s = (const u32*)(addr + count);
	while (count >= 4)
	{
		if (s[-1] != (u32)-1) break;
		s--;
		count -= 4;
	}

	// align up to to 32 KB
	count = (count + 0x8000-4) & ~0x7fff;

	// limit to protect config on last 4 KB page
	if (count > 2*1024*1024 - 4096 - BOOTLOADER_SIZE) count = 2*1024*1024 - 4096 - BOOTLOADER_SIZE;

	// erase memory
	int n = count;
	int k;
	while (count >= 0x1000)
	{
		Progress(n - count, n, 122, COL_ORANGE);

		k = count;
		if (((addr & 0xffff) == 0x8000) && (count >= 0x8000))
			k = 0x8000;
		else
			if (k >= 0x10000) k = 0x10000;

		FlashErase(addr - XIP_BASE, k);
		addr += k;
		count -= k;
	}
	Progress(n, n, 122, COL_ORANGE);
	WaitMs(50);
}

// display big error text
void DispBigErr(const char* text)
{
	FileClose(&PrevFile);

	DrawClear();
	int len = StrLen(text);
	SelFont8x16();
	DrawTextBg2(text, (WIDTH - len*16)/2, (HEIGHT-32)/2, COL_BIGERRFG, COL_BIGERRBG);
	DispUpdate();

	KeyFlush();
	while (KeyGet() == NOKEY) {}

	PreviewClr();
	FrameFileList();
	DispFileList();
	DispUpdate();
}

#ifdef KEY_X

char BatTxt[] = "Battery 0.00V";

void Battery()
{
	DrawClear();
	KeyFlush();
	u8 key;
	Bool cfg = False;

	do {
		// display battery voltage
		int bat = (GetBatInt() + 5)/10;
		int i = bat / 10;
		BatTxt[11] = bat - i*10 + '0';
		bat = i / 10;
		BatTxt[10] = i - bat*10 + '0';
		BatTxt[8] = bat + '0';
		SelFont8x16();
		DrawTextBg2(BatTxt, (WIDTH - 13*16)/2, 23, COL_YELLOW, COL_BLACK);

		// display battery voltage progress bar
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
		bat = GetBatInt() - ConfigGetBatEmptyInt();
		if (bat < 0) bat = 0;
		if (bat > ConfigGetBatFullInt() - ConfigGetBatEmptyInt()) bat = ConfigGetBatFullInt() - ConfigGetBatEmptyInt();
		Progress(bat, ConfigGetBatFullInt() - ConfigGetBatEmptyInt(), 57, COL_GREEN);
#else
		bat = GetBatInt() - BATTERY_EMPTY_INT;
		if (bat < 0) bat = 0;
		if (bat > BATTERY_FULL_INT - BATTERY_EMPTY_INT) bat = BATTERY_FULL_INT - BATTERY_EMPTY_INT;
		Progress(bat, BATTERY_FULL_INT - BATTERY_EMPTY_INT, 57, COL_GREEN);
#endif

		// volume
		DrawText("Volume UP/DOWN:", PROGRESS_X, 95, COL_WHITE);
		COLTYPE col = COL_GREEN;
		u8 vol = ConfigGetVolume();
		if (vol > CONFIG_VOLUME_FULL) col = COL_YELLOW;
		if (vol > (CONFIG_VOLUME_FULL+CONFIG_VOLUME_MAX)/2) col = COL_RED;
		Progress(vol, CONFIG_VOLUME_MAX, 111, col);
		DrawText("0%    50%   100%  150%  200%  250%", 26, 131, COL_WHITE);

		// bright
		DrawText("Backlight LEFT/RIGHT:", PROGRESS_X, 160, COL_WHITE);
		Progress(ConfigGetBacklight(), CONFIG_BACKLIGHT_MAX, 176, COL_GREEN);
		DrawText("0   1   2   3   4   5   6   7   8", 26, 196, COL_WHITE);

		// help
#if USE_PICOINO10 || USE_PICOTRON
		DrawText("Press Z=BOOTSEL, Back=back...", (WIDTH-29*8)/2, HEIGHT-16, COL_WHITE);
#else
		DrawText("Press B=BOOTSEL, Y=back...", (WIDTH-26*8)/2, HEIGHT-16, COL_WHITE);
#endif

		DispUpdate();
		WaitMs(100);

		key = KeyGet();
		switch (key)
		{

		// BOOTSEL
#ifdef KEY_B
		case KEY_B:
#else
		case BTN_B:
#endif
			if (cfg) ConfigSave();
			ResetUsb(0, 0);

		// Up volume up
		case KEY_UP:
			ConfigIncVolume();
			KeyFlush();
			cfg = True;
			break;

		// Down volume down
		case KEY_DOWN:
			ConfigDecVolume();
			KeyFlush();
			cfg = True;
			break;

		// Left backlight down
		case KEY_LEFT:
			if (Config.backlight > CONFIG_BACKLIGHT_MIN)
				ConfigDecBacklight();
			KeyFlush();
			cfg = True;
			break;

		// Right backlight up
		case KEY_RIGHT:				
			ConfigIncBacklight();
			KeyFlush();
			cfg = True;
			break;	
		}

	} while (key != KEY_Y);
	
	if (cfg) ConfigSave();

	PreviewClr();
	FrameFileList();
	DispFileList();
	DispUpdate();
}

#endif // KEY_X

// run application in RAM
void RunRAM(int num)
{
	// wait for no key pressed
	KeyWaitNoPressed();

	// runtime terminate
	RuntimeTerm();

	// copy program to RAM
	u8* s = (u8*)FrameBuf;
	u8* d = (u8*)SRAM_BASE;
	for (; num > 0; num--) *d++ = *s++;

	// run application
	((void(*)(void))(SRAM_BASE+1))();
}

int main()
{
	int i, j, k, m, n;
	u32 t;
	sFileDesc* fd;

	// clear screen
	memset(FrameBuf, 0, sizeof(FrameBuf));
	DispDirtyAll();

	// reset to root
	LastNameLen = 0; // no last name
	FileInit(&PrevFile); // initialize file structure of preview file
	ResetRoot();

	// display frame of file list
	FrameFileList();
	DispInfo("Insert SD card");

	// initialize remount
	LastMount = Time()-2000000; // invalidate last mount time = current time - 2 seconds
	Remount = True; // remount request

	// check loader magic
	Bool loader = (WATCHDOG_SCRATCH[4] == WATCHDOG_LOADER_MAGIC);
	WATCHDOG_SCRATCH[4] = 0;

	// try to mount disk
	if (!DiskMount())
	{
		// cannot mount disk, try to run current application ... not if watchdog reset from application
		if (!loader && CheckApp()) RunApp();
	}
	else
	{
		FileTop = 0;

		// load boot loader data
		LoadBootData();

		// display info text
		DispInfo("Loading files...");

		// load files
		Remount = False; // clear flag - disk already remounted
		LoadFileList(); // load file list

		// set cursor to last name
		SetLastName();

		// display new directory
		DispFileList();
	}

	while (True)
	{
		// display update
		DispUpdate();

		// check last mount time (interval 1 second)
		t = Time();
		if ((int)(t - LastMount) > 1000000)
		{
			// save new last mount time
			LastMount = t;

			// disk removed, request to remount disk next time
			if (!DiskMount())
			{
				// clear directories if not done yet
				if (!Remount) // if remount request yet not set
				{
					// clear preview panel
					PreviewClr();

					// reset to root
					ResetRoot();

					// display info text
					DispInfo("Insert SD card");
				}
			}
			else
			{
				// reload directories if disk need to be remounted
				if (Remount)
				{
//					FileTop = 0;

					// clear preview panel
					PreviewClr();

					// display info text
					DispInfo("Loading files...");

					// load files
					Remount = False; // clear flag - disk already remounted
					LoadFileList(); // load file list

					// set cursor to last name
					SetLastName();

					// display new directory
					DispFileList();
				}
			}
		}

		// keyboard service
		while (True)
		{
			u8 ch = KeyGet();
			if (ch == NOKEY) break;

			// key switch
			switch (ch)
			{
			// Down
			case KEY_DOWN:
#if USE_DEMOVGA
				// restart program
				if (KeyPressed(KEY_UP) && CheckApp())
				{
					SaveBootData();
					RunApp();
				}
#endif // USE_DEMOVGA

				if (FileCur < FileNum-1)
				{
					// increase cursor
					FileCur++;

					// prepare max. top entry
					i = FileCur - (FILEROWS-1);
					if (i < 0) i = 0;

					// limit min. top entry
					if (FileTop < i) FileTop = i;

					// repaint display
					DispFileList();
					//SaveBootData();
				}
				break;

#ifdef KEY_RIGHT
			// PageDown and Right
			case KEY_RIGHT:
				if (FileCur < FileNum-1)
				{
					// max. top entry
					i = FileNum - FILEROWS;
					if (i < 0) i = 0;

					// max. top entry not reached - move page
					if (FileTop < i)
					{
						j = FILEROWS - 1; // page size
						FileTop += j; // shift top file
						if (FileTop > i) // max. top entry exceeded?
						{
							j -= FileTop - i; // reduce page size
							FileTop = i; // reduce top entry
						}
						FileCur += j; // shift cursor
					}

					// max. top entry reached - only shift cursor
					else
						FileCur = FileNum-1;

					// repaint display
					DispFileList();
					//SaveBootData();
				}
				break;
#endif

			// Up
			case KEY_UP:
#if USE_DEMOVGA
				// restart program
				if (KeyPressed(KEY_DOWN) && CheckApp())
				{
					SaveBootData();
					RunApp();
				}
#endif // USE_DEMOVGA

				if (FileCur > 0)
				{
					// decrease cursor
					FileCur--;

					// limit top entry
					if (FileTop > FileCur) FileTop = FileCur;

					// repaint display
					DispFileList();
					//SaveBootData();
				}
				break;

#ifdef KEY_LEFT
			// PagUp and Left
			case KEY_LEFT:
				if (FileCur > 0)
				{
					// min. top entry not reached - move page
					if (FileTop > 0)
					{
						j = FILEROWS - 1; // page size
						FileTop -= j; // shift top file
						if (FileTop < 0) // min. top entry exceeded?
						{
							j += FileTop; // reduce page size
							FileTop = 0; // reduce top entry
						}
						FileCur -= j; // shift cursor
					 }

					// min. top entry reached - only shift cursor
					else
						FileCur = 0;


					// repaint display
					DispFileList();
					//SaveBootData();
				}
				break;
#endif

			// Enter
			case KEY_A:
				if ((uint)FileCur < (uint)FileNum)
				{
					fd = &FileDesc[FileCur];

					// change directory
					if ((fd->attr & ATTR_DIR) != 0)
					{
						// going to up-directory
						if ((fd->len == 2) && (fd->name[0] == '.') && (fd->name[1] == '.'))
						{
							// delete last directory
							i = PathLen;
							while (Path[PathLen-1] != PATHCHAR) PathLen--;

							// save last name
							LastNameLen = i - PathLen;
							memcpy(LastName, &Path[PathLen], LastNameLen);
							LastNameDir = ATTR_DIR;

							// delete path separator if not root
							if (PathLen > 1) PathLen--;
						}

						// going to sub-directory
						else
						{
							// check path length
							if (PathLen + 1 + fd->len <= PATHMAX)
							{
								// add path separator if not root
								if (PathLen != 1) Path[PathLen++] = PATHCHAR;

								// add path name
								memcpy(&Path[PathLen], fd->name, fd->len);
								PathLen += fd->len;
							}
						}

						// set terminating 0
						Path[PathLen] = 0;

						// request to reload current directory
						FileTop = 0;
						Reload();

						// invalidate last mount time = current time - 2 seconds
						LastMount = Time()-2000000;
					}
					else
					{
						// close old preview file
						FileClose(&PrevFile);

						// prepare filename of the file
						memcpy(TempBuf, fd->name, fd->len);
						memcpy(&TempBuf[fd->len], ".UF2", 5);

						// open file
						DispBigInfo("Loading...");
						SetDir(Path);
						if (!FileOpen(&PrevFile, TempBuf))
						{
							DispBigErr("Cannot open file");
						}
						else
						{
							// read and check UF2 header
							TempBufNum = FileRead(&PrevFile, TempBuf, 32);
							sUf2* uf = (sUf2*)TempBuf;
							Bool ram = (uf->target_addr == 0x20000000);
							if (!ram && (uf->target_addr != 0x10000000))
							{
								DispBigErr("Unsupported address");
								FileClose(&PrevFile);
							}
							else
							{
								// loading into RAM
								if (ram)
								{
									DispBigInfo("Loading into RAM...");
									j = 32;
									m = 0;
									while (m <= CUSTOM_FRAMEBUF_SIZE - 256)
									{
										FileSeek(&PrevFile, j);
										TempBufNum = FileRead(&PrevFile, ((u8*)FrameBuf) + m, 256);
										if (TempBufNum <= 0) break;
										m += TempBufNum;
										j += 512;
									}
									FileClose(&PrevFile);

									// run application in RAM
									SaveBootData();
									RunRAM(m);
								}
								else
								{
									// erase memory
									ClearApp();

									DispBigInfo("Loading...");

									// read and check application header
									j = BOOTLOADER_SIZE*2+32;
									FileSeek(&PrevFile, j);
									TempBufNum = FileRead(&PrevFile, TempBuf, 256);
									m = 256;
									u32* h = (u32*)&TempBuf[48*4];
									i = h[1]; 			// application length
									if ((TempBufNum != 256) ||	// check segment length
										(h[0] != 0x44415050) ||	// check magix
										(i < 10) || (i > 2*1024*1024 - 4096 - BOOTLOADER_SIZE - 51*4)) // check program length
									{
										// error - incompatible program
										DispBigErr("Loading Error");
										FileClose(&PrevFile);
									}
									else
									{
										i += 51*4;
										n = i;
										k = BOOTLOADER_SIZE;

										// loading program into memory
										do {
											// load next program block
											while ((m <= TEMPBUF-256) && (i > 0))
											{
												j += 512;
												FileSeek(&PrevFile, j);
												TempBufNum = FileRead(&PrevFile, &TempBuf[m], 256);
												m += 256;
												i -= 256;
											}

											// progress bar
											Progress(k - BOOTLOADER_SIZE, n, 122, COL_GREEN);

											// program four 256-byte pages
											FlashProgram(k, (const u8*)TempBuf, m);
											k += m;
											if (i <= 0) break;
											m = 0;

										} while (TempBufNum > 0);

										FileClose(&PrevFile);

										// try to run the application
										if (CheckApp())
										{
											SaveBootData();
											RunApp();
										}
										else
											DispBigErr("Loading Error");
									}
								}
							}
						}
					}
				}
				break;

#ifdef KEY_B
			// out of directory
			case KEY_B:
				// not root directory yet
				if (PathLen > 1)
				{
					// delete last directory
					i = PathLen;
					while (Path[PathLen-1] != PATHCHAR) PathLen--;

					// save last name
					LastNameLen = i - PathLen;
					memcpy(LastName, &Path[PathLen], LastNameLen);
					LastNameDir = ATTR_DIR;

					// delete path separator if not root
					if (PathLen > 1) PathLen--;

					// set terminating 0
					Path[PathLen] = 0;

					// request to reload current directory
					FileTop = 0;
					Reload();

					// invalidate last mount time = current time - 2 seconds
					LastMount = Time()-2000000;

					//SaveBootData();
				}
				break;
#endif

#ifdef KEY_Y
			// restart program
			case KEY_Y:
				if (CheckApp())
				{
					SaveBootData();
					RunApp();
				}
				break;
#endif // KEY_Y

#ifdef KEY_X
			// battery
			case KEY_X:
#if USE_SCREENSHOT		// use screen shots
				ScreenShot();
#else
				Battery();
#endif
				break;
#endif // KEY_X
			}
		}

		// preview
		Preview();
	}
}
