
// ****************************************************************************
//                                 
//                         FileSel - File selection
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

#include "../../global.h"	// globals

#if USE_FILESEL		// use file selection (lib_filesel.c, lib_filesel.h)

#include "../../_display/_include.h"	// display interface (colors)
#include "../../_devices/key.h"		// key interface

#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/usb_inc/sdk_usb_host_hid.h"

#include "../inc/lib_decnum.h"
#include "../inc/lib_draw.h"
#include "../inc/lib_filesel.h"

// text of file panel
#define FILESEL_FONTW	8	// file panel font width
#define FILESEL_FONTH	16	// file panel font height
#define FILESEL_TEXTW	(WIDTH/FILESEL_FONTW) // number of characters per row (=320/8=40)
#define FILESEL_TEXTH	(HEIGHT/FILESEL_FONTH) // number of rows (=240/16=15)

// text of preview
#define FILESEL_FONTW2	6	// preview font width
#define FILESEL_FONTH2	8	// preview font height
#define FILESEL_TEXTW2	(WIDTH/FILESEL_FONTW2) // number of characters per row (=320/6=53)
#define FILESEL_TEXTH2	(HEIGHT/FILESEL_FONTH2) // number of rows (=240/8=30)

// files
#define FILESEL_MAXFILES	256	// max files in one directory
#define FILESEL_NAMELEN		9	// max. length of filename without extension and with terminating 0
#define FILESEL_FILEROWS	(FILESEL_TEXTH-2) // rows of files (=13)
#define FILESEL_FILECOLW	20	// width of file panel

// file entry descriptor (10 bytes)
//#define ATTR_RO	B0	// Read only
//#define ATTR_HID	B1	// Hidden
//#define ATTR_SYS	B2	// System
//#define ATTR_VOL	B3	// Volume label
//#define ATTR_DIR	B4	// Directory
//#define ATTR_ARCH	B5	// Archive
#define ATTR_TXT	B6	// internal: text file TXT present
#define ATTR_BMP	B7	// internal: bitmap indicator BMP present
#define ATTR_MASK	0x3F	// mask of valid bits

typedef struct {
	u32	size;		// file size
	u8	attr;		// attributes ATTR_*
	u8	len;		// file name length
	u8	ext;		// index of file extension 0..2
	char	name[8+1];	// file name (without extension, with terminating zero)
} sFileDesc;

/*
// BMP file header (size 70 bytes)
#pragma pack(push,1)
typedef struct {
	// BMP file header (size 14 bytes)
	u16	bfType;			// 0x00: magic, 'B' 'M' = 0x4D42
	u32	bfSize;			// 0x02: file size, aligned to DWORD = 70 + 320*240*2 + 2 = 153672 = 0x25848
	u16	bfReserved1;		// 0x06: = 0
	u16	bfReserved2;		// 0x08: = 0
	u32	bfOffBits;		// 0x0A: offset of data bits after file header = 70 (0x46)
	// BMP info header (size 40 bytes)
	u32	biSize;			// 0x0E: size of this info header = 56 (0x38)
	s32	biWidth;		// 0x12: width = 320 (0x140)
	s32	biHeight;		// 0x16: height, negate if flip row order = -240 (0xFFFFFF10)
	u16	biPlanes;		// 0x1A: planes = 1
	u16	biBitCount;		// 0x1C: number of bits per pixel = 16
	u32	biCompression;		// 0x1E: compression = 3 (BI_BITFIELDS)
	u32	biSizeImage;		// 0x22: size of data of image + aligned file = 320*240*2 + 2 = 153602 (0x25802)
	s32	biXPelsPerMeter;	// 0x26: X pels per meter = 2834 (= 0xB12)
	s32	biYPelsPerMeter;	// 0x2A: Y pels per meter = 2834 (= 0xB12)
	u32	biClrUsed;		// 0x2E: number of user colors (0 = all)
	u32	biClrImportant;		// 0x32: number of important colors (0 = all)
	// BMP color bit mask (size 16 bytes)
	u32	biRedMask;		// 0x36: red mask = 0x0000F800
	u32	biGreenMask;		// 0x3A: green mask = 0x000007E0
	u32	biBlueMask;		// 0x3E: blue mask = 0x0000001F
	u32	biAlphaMask;		// 0x42: alpha mask = 0x00000000
					// 0x46
} sBmp;
#pragma pack(pop)
*/

// current path
char FileSelPath[FILESEL_PATHMAX+1]; // current path (with terminating 0, without last path character if not root)
int FileSelPathLen; // length of path
int FileSelRootLen; // length of root

// title (used instead of root path)
char FileSelTitle[FILESEL_FILECOLW+1]; // title (with terminating 0)
int FileSelTitleLen; // length of title

// file extension
char FileSelExt[4]; // file extension in uppercase (1 to 3 characters, with terminating 0)
int FileSelExtLen; // length of file extension (1 to 3 characters)

char FileSelExt2[4]; // file extension 2 in uppercase (0 to 3 characters, with terminating 0)
int FileSelExtLen2; // length of file extension 2 (0 to 3 characters)

char FileSelExt3[4]; // file extension 3 in uppercase (0 to 3 characters, with terminating 0)
int FileSelExtLen3; // length of file extension 3 (0 to 3 characters)

// colors template - blue theme
const sFileSelColors FileSelColBlue = {
#if USE_DISPHSTXMINI && !USE_DISPHSTX_DISPBUF // 1=use HSTX Display Mini driver
	.titlefg = COL_BLACK & DISPHSTX_VGA_MASK,	// title foreground color - current directory or description
	.titlebg = COL_WHITE & DISPHSTX_VGA_MASK,	// title background color - current directory or description
	.filefg = COL_CYAN & DISPHSTX_VGA_MASK,		// file foreground color
	.filebg = COL_BLUE & DISPHSTX_VGA_MASK,		// file background color
	.dirfg = COL_WHITE & DISPHSTX_VGA_MASK,		// directory foreground color
	.dirbg = COL_BLUE & DISPHSTX_VGA_MASK,		// directory background color
	.curfg = COL_BLACK & DISPHSTX_VGA_MASK,		// cursor coreground color
	.curbg = COL_CYAN & DISPHSTX_VGA_MASK,		// cursor background color
	.infofg = COL_GREEN & DISPHSTX_VGA_MASK,	// info text foreground color
	.infobg = COL_BLUE & DISPHSTX_VGA_MASK,		// info text background color
	.textfg = COL_GRAY & DISPHSTX_VGA_MASK,		// text foreground color
	.textbg = COL_BLACK & DISPHSTX_VGA_MASK,	// text background color
	.biginfofg = COL_YELLOW & DISPHSTX_VGA_MASK,	// big info text foreground color
	.biginfobg = COL_BLACK & DISPHSTX_VGA_MASK,	// big info text background color
	.bigerrfg = COL_YELLOW & DISPHSTX_VGA_MASK,	// big error foreground color
	.bigerrbg = COL_RED & DISPHSTX_VGA_MASK,	// big error background color
#else // USE_DISPHSTXMINI
	.titlefg = COL_BLACK,		// title foreground color - current directory or description
	.titlebg = COL_WHITE,		// title background color - current directory or description
	.filefg = COL_CYAN,		// file foreground color
	.filebg = COL_BLUE,		// file background color
	.dirfg = COL_WHITE,		// directory foreground color
	.dirbg = COL_BLUE,		// directory background color
	.curfg = COL_BLACK,		// cursor coreground color
	.curbg = COL_CYAN,		// cursor background color
	.infofg = COL_GREEN,		// info text foreground color
	.infobg = COL_BLUE,		// info text background color
	.textfg = COL_GRAY,		// text foreground color
	.textbg = COL_BLACK,		// text background color
	.biginfofg = COL_YELLOW,	// big info text foreground color
	.biginfobg = COL_BLACK,		// big info text background color
	.bigerrfg = COL_YELLOW,		// big error foreground color
	.bigerrbg = COL_RED,		// big error background color
#endif // USE_DISPHSTXMINI
};

// colors template - green theme
const sFileSelColors FileSelColGreen = {
#if USE_DISPHSTXMINI && !USE_DISPHSTX_DISPBUF // 1=use HSTX Display Mini driver
	.titlefg = COL_BLACK & DISPHSTX_VGA_MASK,	// title foreground color - current directory or description
	.titlebg = COL_WHITE & DISPHSTX_VGA_MASK,	// title background color - current directory or description
	.filefg = COL_WHITE & DISPHSTX_VGA_MASK,	// file foreground color
	.filebg = COL_DKGREEN & DISPHSTX_VGA_MASK,	// file background color
	.dirfg = COL_YELLOW & DISPHSTX_VGA_MASK,	// directory foreground color
	.dirbg = COL_DKGREEN & DISPHSTX_VGA_MASK,	// directory background color
	.curfg = COL_BLACK & DISPHSTX_VGA_MASK,		// cursor coreground color
	.curbg = COL_WHITE & DISPHSTX_VGA_MASK,		// cursor background color
	.infofg = COL_YELLOW & DISPHSTX_VGA_MASK,	// info text foreground color
	.infobg = COL_DKGREEN & DISPHSTX_VGA_MASK,	// info text background color
	.textfg = COL_GRAY & DISPHSTX_VGA_MASK,		// text foreground color
	.textbg = COL_BLACK & DISPHSTX_VGA_MASK,	// text background color
	.biginfofg = COL_YELLOW & DISPHSTX_VGA_MASK,	// big info text foreground color
	.biginfobg = COL_BLACK & DISPHSTX_VGA_MASK,	// big info text background color
	.bigerrfg = COL_YELLOW & DISPHSTX_VGA_MASK,	// big error foreground color
	.bigerrbg = COL_RED & DISPHSTX_VGA_MASK,	// big error background color
#else // USE_DISPHSTXMINI
	.titlefg = COL_BLACK,		// title foreground color - current directory or description
	.titlebg = COL_WHITE,		// title background color - current directory or description
	.filefg = COL_WHITE,		// file foreground color
	.filebg = COL_DKGREEN,		// file background color
	.dirfg = COL_YELLOW,		// directory foreground color
	.dirbg = COL_DKGREEN,		// directory background color
	.curfg = COL_BLACK,		// cursor coreground color
	.curbg = COL_WHITE,		// cursor background color
	.infofg = COL_YELLOW,		// info text foreground color
	.infobg = COL_DKGREEN,		// info text background color
	.textfg = COL_GRAY,		// text foreground color
	.textbg = COL_BLACK,		// text background color
	.biginfofg = COL_YELLOW,	// big info text foreground color
	.biginfobg = COL_BLACK,		// big info text background color
	.bigerrfg = COL_YELLOW,		// big error foreground color
	.bigerrbg = COL_RED,		// big error background color
#endif // USE_DISPHSTXMINI
};

// current color theme
const sFileSelColors* FileSelColors = &FileSelColGreen;

// display
COLTYPE FileSelFgCol, FileSelBgCol; // foreground and background color
int FileSelDispX, FileSelDispY; // X and Y text coordinate

// temporary buffer
char FileSelTempBuf[FILESEL_TEMPBUF+1];
int FileSelTempBufNum; // number of bytes in temporary buffer
int FileSelTempBufInx; // current index in temporary buffer

// files
u32 FileSelLastMount; // last mount time
Bool FileSelRemount; // remount request
sFileDesc FileSelList[FILESEL_MAXFILES]; // file list
int FileSelFileNum; // number of files
int FileSelFileCur; // index of current file
int FileSelFileTop; // index of first visible file
sFile FileSelSearchFile; // search structure
sFileInfo FileSelFileInfo; // search file info

// selected last name
char FileSelLastName[9]; // last selected name (without extension, with terminating 0)
int FileSelLastNameLen = 0; // length of last name (without extension), 0 = not used
int FileSelLastNameTop = 0; // top file of last name
u8 FileSelLastNameAttr; // attributes of last name
u8 FileSelLastNameExt; // index of extension of last name 0..2
u32 FileSelLastNameSize; // size of last name

// preview
enum  {
	PREV_START,	// waiting for start
	PREV_TXT_LOAD,	// loading text
	PREV_BMP_START,	// waiting for bitmap start
	PREV_BMP_LOAD,	// loading bitmap
	PREV_STOP,	// preview not active
};	

u8 FileSelPrevState; // current preview state
sFileDesc* FileSelPrevFD; // pointer to preview file descriptor
int FileSelPrevLine; // current preview line
sFile FileSelPrevFile; // preview file (name[0] = 0 if not open)
int FileSelPrevW, FileSelPrevH; // size of preview image

// set home position
void FileSelDispHome()
{
	FileSelDispX = 0;
	FileSelDispY = 0;
}

// set print colors
void FileSelDispCol(COLTYPE fgcol, COLTYPE bgcol)
{
	FileSelFgCol = fgcol;
	FileSelBgCol = bgcol;
}

// display text on file panel
void FileSelDispText(const char* text)
{
	SelFont8x16();
	DrawTextBg(text, FileSelDispX*FILESEL_FONTW, FileSelDispY*FILESEL_FONTH, FileSelFgCol, FileSelBgCol);
	FileSelDispX += StrLen(text);
}

// display small text on preview panel
void FileSelDispSmallText(const char* text)
{
	SelFont6x8();
	DrawTextBg(text, FileSelDispX*FILESEL_FONTW2, FileSelDispY*FILESEL_FONTH2, FileSelFgCol, FileSelBgCol);
	FileSelDispX += StrLen(text);
}

// print character on file panel
void FileSelDispChar(char ch)
{
	SelFont8x16();
	DrawCharBg(ch, FileSelDispX*FILESEL_FONTW, FileSelDispY*FILESEL_FONTH, FileSelFgCol, FileSelBgCol);
	FileSelDispX++;
}

// print small character on preview panel
void FileSelDispSmallChar(char ch)
{
	SelFont6x8();
	DrawCharBg(ch, FileSelDispX*FILESEL_FONTW2, FileSelDispY*FILESEL_FONTH2, FileSelFgCol, FileSelBgCol);
	FileSelDispX++;
}

// print space on file panel
void FileSelDispSpc()
{
	FileSelDispChar(' ');
}

// display character on file panel repeated
void FileSelDispCharRep(char ch, int num)
{
	for (; num > 0; num--) FileSelDispChar(ch);
}

// display space on file panel repeated
void FileSelDispSpcRep(int num)
{
	for (; num > 0; num--) FileSelDispSpc();
}

// display frame of file list
void FileSelFrameFileList()
{
	// reset cursor
	FileSelDispHome();

	// top frame
	FileSelDispCol(FileSelColors->filebg, FileSelColors->filefg); // invert color
	FileSelDispChar(0x9c); // left frame
	FileSelDispCharRep(0x95, FILESEL_FILECOLW-2); // row
	FileSelDispChar(0x99); // right frame
	FileSelDispY++;

	// inner rows
	do {
		FileSelDispX = 0; // reset column
		FileSelDispChar(0x9a); // left frame
		FileSelDispCol(FileSelColors->filefg, FileSelColors->filebg); // normal color
		FileSelDispSpcRep(FILESEL_FILECOLW-2); // row
		FileSelDispCol(FileSelColors->filebg, FileSelColors->filefg); // invert color
		FileSelDispChar(0x9a); // right frame
		FileSelDispY++; // increment row
	} while (FileSelDispY != FILESEL_TEXTH-1);

	// bottom frame
	FileSelDispX = 0; // reset column
	FileSelDispChar(0x96); // left frame
	FileSelDispCharRep(0x95, FILESEL_FILECOLW-2); // row
	FileSelDispChar(0x93); // right frame
}

// display current path or title (on top row of file panel)
void FileSelPathFileList()
{
	// prepare path or title
	const char* txt = FileSelPath;
	int len = FileSelPathLen;
	if (len <= FileSelRootLen)
	{
		txt = FileSelTitle;
		len = FileSelTitleLen;
	}

	// reset cursor
	FileSelDispY = 0;
	FileSelDispX = 1;

// len <= FILECOLW-4 ... | top_line + space + path + space + top_line |
// len == FILECOLW-3 ... | path + space |
// len == FILECOLW-2 ... | path |
// len > FILECOLW-2 ... | "..." short_path |

	// left part of top line, left space
	if (len <= FILESEL_FILECOLW-4)
	{
		// left part of top line
		FileSelDispCol(FileSelColors->filebg, FileSelColors->filefg); // invert color
		FileSelDispCharRep(0x95, (FILESEL_FILECOLW-4 - len)/2); // line

		// left space
		FileSelDispCol(FileSelColors->titlefg, FileSelColors->titlebg);
		FileSelDispSpc();
	}

	// path
	FileSelDispCol(FileSelColors->titlefg, FileSelColors->titlebg); // normal title color
	if (len <= FILESEL_FILECOLW-2)
	{
		// full path
		FileSelDispText(txt);
	}
	else
	{
		// short path
		FileSelDispCharRep('.', 3); // "..."
		FileSelDispText(txt + len - (FILESEL_FILECOLW-2-3));
	}

	// right space
	if (len <= FILESEL_FILECOLW-3) FileSelDispSpc();
	
	// right part of top line
	if (len <= FILESEL_FILECOLW-4)
	{
		// right part of top line
		FileSelDispCol(FileSelColors->filebg, FileSelColors->filefg); // invert color
		FileSelDispCharRep(0x95, (FILESEL_FILECOLW-4 - len + 1)/2); // line
	}
}

// display current index of selected file (on bottom row of file panel)
void FileSelInxFileList()
{
	// check if any files
	if (FileSelFileNum == 0) return;

	// prepare text
	int n = DecNum(FileSelTempBuf, FileSelFileCur+1, 0);
	FileSelTempBuf[n++] = '/';
	n += DecNum(&FileSelTempBuf[n], FileSelFileNum, 0);

	// set cursor
	FileSelDispY = FILESEL_TEXTH-1;
	FileSelDispX = 1;

	// left part of bottom line
	FileSelDispCol(FileSelColors->filebg, FileSelColors->filefg); // invert color
	FileSelDispCharRep(0x95, (FILESEL_FILECOLW-4 - n)/2); // line

	// left space
	FileSelDispCol(FileSelColors->filefg, FileSelColors->filebg); // normal color
	FileSelDispSpc();

	// text
	FileSelDispText(FileSelTempBuf);

	// right space
	FileSelDispSpc();
	
	// right part of bottom line
	FileSelDispCol(FileSelColors->filebg, FileSelColors->filefg); // invert color
	FileSelDispCharRep(0x95, (FILESEL_FILECOLW-4 - n + 1)/2); // line
}

// dispay file list
void FileSelDispFileList()
{
	// reset cursor
	FileSelDispY = 1;

	// display files
	int i, j;
	char ch;
	Bool dir;
	sFileDesc* fd = &FileSelList[FileSelFileTop];
	for (i = 0; i < FILESEL_FILEROWS; i++)
	{
		FileSelDispX = 1;

		// set normal color
		FileSelDispCol(FileSelColors->filefg, FileSelColors->filebg);

		// entry is valid
		j = i + FileSelFileTop;
		if (j < FileSelFileNum)
		{
			// check directory
			dir = ((fd->attr & ATTR_DIR) != 0);

			// set directory color
			if (dir) FileSelDispCol(FileSelColors->dirfg, FileSelColors->dirbg);

			// set cursor color
			if (j == FileSelFileCur) FileSelDispCol(FileSelColors->curfg, FileSelColors->curbg);

			// directory mark '['
			ch = dir ? '[' : ' ';
			FileSelDispChar(ch);

			// decode entry name
			for (j = 0; j < fd->len; j++) FileSelDispChar(fd->name[j]);


#if FILESEL_DISPSIZE	// 1=display size instead of TXT+BMP flags

			// directory mark ']'
			if (dir)
			{
				FileSelDispChar(']');
				FileSelDispSpcRep(16 - fd->len);
			}
			else
			{
				// prepare file size (max. 9 characters)
				if (fd->size < 1000000)
					j = DecUNum(DecNumBuf, fd->size, '\'');
				else
					j = DecUNum(DecNumBuf, fd->size, 0);

				// print spaces
				FileSelDispSpcRep(17 - fd->len - j);

				// print size
				FileSelDispText(DecNumBuf);
			}

#else // FILESEL_DISPSIZE

			// directory mark ']'
			ch = dir ? ']' : ' ';
			FileSelDispChar(ch);

			// display spaces
			FileSelDispSpcRep(9 - fd->len);

			// TXT mark
			if ((fd->attr & ATTR_TXT) != 0)
				FileSelDispText("TXT");
			else
				FileSelDispSpcRep(3);

			// space
			FileSelDispSpc();

			// BMP mark
			if ((fd->attr & ATTR_BMP) != 0)
				FileSelDispText("BMP");
			else
				FileSelDispSpcRep(3);
#endif // FILESEL_DISPSIZE

		}
		else
			// clear invalid row
			FileSelDispSpcRep(FILESEL_FILECOLW-2);

		// increase file
		FileSelDispY++;
		fd++;
	}

	// display current index
	FileSelInxFileList();

	// display current path
	FileSelPathFileList();

	// restart preview state
	FileSelPrevState = PREV_START;
}

// load files
void FileSelLoadFileList()
{
	// clear file list
	FileSelFileNum = 0;
	FileSelFileCur = 0;

	// set current directory
	if (!SetDir(FileSelPath)) return;

	// open search 
	if (!FindOpen(&FileSelSearchFile, "")) return;

	// load files
	int inx, i, len;
	sFileDesc* fd = FileSelList;
	char ch;
	char* name;
	Bool dir;
	for (inx = 0; inx < FILESEL_MAXFILES; inx++)
	{
		// find next file
		if (!FindNext(&FileSelSearchFile, &FileSelFileInfo, ATTR_DIR_MASK, "*.*")) break;

		// check directory
		dir = (FileSelFileInfo.attr & ATTR_DIR) != 0;

		// skip directory "."
		len = FileSelFileInfo.namelen;
		name = FileSelFileInfo.name;
		if (dir && (len == 1) && (name[0] == '.')) continue;

		// get attributes
		fd->attr = FileSelFileInfo.attr & ATTR_MASK;

		// get size
		fd->size = FileSelFileInfo.size;

		// default extension
		fd->ext = 0;

		// copy directory ".."
		if (dir && (len == 2) && (name[0] == '.') && (name[1] == '.'))
		{
			// disable on root level
			if (FileSelPathLen > FileSelRootLen)
			{
				fd->len = len;
				fd->name[0] = '.';
				fd->name[1] = '.';
				fd->name[2] = 0;
				fd++;
				FileSelFileNum++;
			}
		}
		else
		{
			// skip hidden entry
			if ((FileSelFileInfo.attr & ATTR_HID) != 0) continue;

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
				fd->name[i] = 0;

				// directory is valid only if has no extension
				if (i == len)
				{
					fd->len = len;
					fd++;
					FileSelFileNum++;
				}
			}

			// file
			else
			{
				// check file extension
				if ((len > FileSelExtLen+1) && (name[len-FileSelExtLen-1] == '.')
					&& (memcmp(&name[len-FileSelExtLen], FileSelExt, FileSelExtLen) == 0))
				{
					// copy file name (without extension)
					fd->len = len-FileSelExtLen-1;
					memcpy(fd->name, name, len-FileSelExtLen-1);
					fd->name[fd->len] = 0;
					fd->ext = 0;
					fd++;
					FileSelFileNum++;
				}

				// check file extension 2
				else if ((FileSelExtLen2 != 0) &&	
					(len > FileSelExtLen2+1) && (name[len-FileSelExtLen2-1] == '.')
					&& (memcmp(&name[len-FileSelExtLen2], FileSelExt2, FileSelExtLen2) == 0))
				{
					// copy file name (without extension)
					fd->len = len-FileSelExtLen2-1;
					memcpy(fd->name, name, len-FileSelExtLen2-1);
					fd->name[fd->len] = 0;
					fd->ext = 1;
					fd++;
					FileSelFileNum++;
				}

				// check file extension 3
				else if ((FileSelExtLen3 != 0) &&	
					(len > FileSelExtLen3+1) && (name[len-FileSelExtLen3-1] == '.')
					&& (memcmp(&name[len-FileSelExtLen3], FileSelExt3, FileSelExtLen3) == 0))
				{
					// copy file name (without extension)
					fd->len = len-FileSelExtLen3-1;
					memcpy(fd->name, name, len-FileSelExtLen3-1);
					fd->name[fd->len] = 0;
					fd->ext = 2;
					fd++;
					FileSelFileNum++;
				}
			}
		}
	}

	// open search again, to check info files
	if (FindOpen(&FileSelSearchFile, ""))
	{
		// find next file
		while (FindNext(&FileSelSearchFile, &FileSelFileInfo, ATTR_DIR_MASK, "*.*"))
		{
			// skip directory
			if ((FileSelFileInfo.attr & ATTR_DIR) != 0) continue;

			// search extensions "TXT" and "BMP"
			len = FileSelFileInfo.namelen;
			if (len > 4)
			{
				len -= 4;

				name = FileSelFileInfo.name;

				// check extension "TXT"
				if ((name[len] == '.') && (name[len+1] == 'T') &&
					 (name[len+2] == 'X') && (name[len+3] == 'T'))
				{
					// search this file name
					sFileDesc* fd = FileSelList;
					for (inx = 0; inx < FileSelFileNum; inx++)
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
						sFileDesc* fd = FileSelList;
						for (inx = 0; inx < FileSelFileNum; inx++)
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
	fd = FileSelList;
	for (inx = 0; inx < FileSelFileNum-1;)
	{
		Bool ok = True;

		// directory '..' must be at first place
		if ((fd[1].len == 2) && (fd[1].name[0] == '.') && (fd[1].name[1] == '.')) ok = False;

		// directory must be before files
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
				if (fd[1].len < fd[0].len)
					ok = False;

				// lengths are equal, check extension
				else if ((fd[1].len == fd[0].len) && (fd[1].ext < fd[0].ext))
					ok = False;
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

			ch = fd[0].ext;
			fd[0].ext = fd[1].ext;
			fd[1].ext = ch;

			int k = fd[0].size;
			fd[0].size = fd[1].size;
			fd[1].size = k;

			for (i = 0; i < 9; i++)
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
void FileSelReload()
{
	// reset file list
	FileSelFileNum = 0; // no entry
	FileSelFileCur = 0; // reset cursor
	FileSelFileTop = 0; // reset top entry

	// preview state not active
	FileSelPrevState = PREV_STOP;

	// display frame of file list
	FileSelFrameFileList();

	// display current path
	FileSelPathFileList();

	// set flag - disk need to be remounted
	FileSelRemount = True;
}

// reset to root
void FileSelResetRoot()
{
	// reset path to root
	FileSelPathLen = FileSelRootLen;
	FileSelPath[FileSelRootLen] = 0;

	// request to reload current directory
	FileSelReload();
}

// set cursor to last name
void FileSelSetLastName()
{
	sFileDesc* fd;
	int i;

	// no last name required
	if (FileSelLastNameLen == 0) return;

	// search last name in file list
	for (FileSelFileCur = FileSelFileNum-1; FileSelFileCur > 0; FileSelFileCur--)
	{
		fd = &FileSelList[FileSelFileCur];
		if ((fd->len == FileSelLastNameLen) &&
			(fd->ext == FileSelLastNameExt) &&
			((fd->attr & ATTR_DIR) == (FileSelLastNameAttr & ATTR_DIR)) &&
			(memcmp(FileSelLastName, fd->name, FileSelLastNameLen) == 0)) break;
	}

	// limit max. top entry
	FileSelFileTop = FileSelLastNameTop;
	if (FileSelFileTop > FileSelFileCur) FileSelFileTop = FileSelFileCur;

	// limit min. top entry
	i = FileSelFileCur - (FILESEL_FILEROWS-1);
	if (i < 0) i = 0;
	if (FileSelFileTop < i) FileSelFileTop = i;

	// delele last name request
	FileSelLastNameLen = 0;
}

// display info text
void FileSelDispInfo(const char* text)
{
	// prepare length of into text
	int len = StrLen(text);

	// set text color and coordinated
	FileSelDispCol(FileSelColors->infofg, FileSelColors->infobg);
	FileSelDispY = 2;
	FileSelDispX = (FILESEL_FILECOLW - len)/2;

	// display info text
	FileSelDispText(text);
}

// loading next byte from temporary buffer (returns 0 if no next byte)
u8 FileSelPrevChar()
{
	// check if buffer need to be loaded
	if (FileSelTempBufInx >= FileSelTempBufNum)
	{
		// check if file is open
		if (!FileIsOpen(&FileSelPrevFile)) return 0;

		// read next temporary buffer
		FileSelTempBufInx = 0;
		FileSelTempBufNum = FileRead(&FileSelPrevFile, FileSelTempBuf, FILESEL_TEMPBUF);

		// end of file
		if (FileSelTempBufNum < FILESEL_TEMPBUF) FileClose(&FileSelPrevFile);

		// no data
		if (FileSelTempBufNum == 0) return 0;
	}

	// get next byte
	return FileSelTempBuf[FileSelTempBufInx++];
}

// loading next byte from temporary buffer, skip CR (returns 0 if no next byte)
char FileSelPrevCharCR()
{
	char ch;
	do {
		ch = FileSelPrevChar();
	} while (ch == CH_CR);
	return ch;
}

// clear preview panel
void FileSelPreviewClr()
{
	DrawRect(WIDTH/2, 0, WIDTH/2, HEIGHT, COL_BLACK);
}

#if USE_DISPHSTXMINI && !USE_DISPHSTX_DISPBUF // 1=use HSTX Display Mini driver
u16 FileSelLineBuf[WIDTH/2];
#endif

// display preview
void FileSelPreview()
{
	int i, j;
	char ch;
	u8 inv;
	sBmp* bmp;
	COLTYPE* dst;

	switch(FileSelPrevState)
	{
	// waiting for start
	case PREV_START:

		// close old preview file
		FileClose(&FileSelPrevFile);

		// clear preview panel
		FileSelPreviewClr();

		// check if current file is valid
		if ((uint)FileSelFileCur >= (uint)FileSelFileNum)
		{
			// no current file
			FileSelPrevState = PREV_STOP;
			return;
		}

		// prepare file descriptor
		FileSelPrevFD = &FileSelList[FileSelFileCur];

		// no text file
		if ((FileSelPrevFD->attr & ATTR_TXT) == 0)
		{
			// waitting for bitmap start
			FileSelPrevState = PREV_BMP_START;
			return;
		}

		// prepare filename of text file
		memcpy(FileSelTempBuf, FileSelPrevFD->name, FileSelPrevFD->len);
		memcpy(&FileSelTempBuf[FileSelPrevFD->len], ".TXT", 5);

		// open text file
		SetDir(FileSelPath);
		if (!FileOpen(&FileSelPrevFile, FileSelTempBuf))
		{
			// cannot open text file, waiting for bitmap start
			FileSelPrevState = PREV_BMP_START;
			return;
		}
		FileSelTempBufNum = 0; // no data in temporary buffer
		FileSelTempBufInx = 0; // reset current index in temporary buffer
		FileSelPrevLine = 0; // reset current preview line

		// loading text file
		FileSelPrevState = PREV_TXT_LOAD;
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
		FileSelDispCol(FileSelColors->textfg, FileSelColors->textbg);
		inv = 0;

		// set coordinates
		FileSelDispX = (FILESEL_TEXTW2+1)/2;
		FileSelDispY = FileSelPrevLine;

		// decode one row (i = relative character position)
		for (i = 0; i < FILESEL_TEXTW2/2;)
		{
			// load next character, skip CR characters
			ch = FileSelPrevCharCR();

			// LF end of line or NUL end of file
			if ((ch == CH_LF) || (ch == 0)) break;

			// tabulator (modulo 8)
			if (ch == CH_TAB)
			{
				do {
					FileSelDispSmallChar(' ' ^ inv);
					i++; // increase character position
				} while ((i < FILESEL_TEXTW2/2) && ((i & 7) != 0));
			}
			else
			{
				// prefix character ^
				if (ch == '^')
				{
					// load next character
					ch = FileSelPrevCharCR();

					// LF end of line or NUL end of file
					if ((ch == CH_LF) || (ch == 0))
						break;

#if USE_DISPHSTXMINI && !USE_DISPHSTX_DISPBUF // 1=use HSTX Display Mini driver
					// set white text
					else if (ch == 'W')
						FileSelDispCol(COL_WHITE & DISPHSTX_VGA_MASK, FileSelColors->textbg);

					// set green text
					else if (ch == 'X')
						FileSelDispCol(COL_GREEN & DISPHSTX_VGA_MASK, FileSelColors->textbg);

					// set yellow text
					else if (ch == 'Y')
						FileSelDispCol(COL_YELLOW & DISPHSTX_VGA_MASK, FileSelColors->textbg);
						
					// set red text
					else if (ch == 'Z')
						FileSelDispCol(COL_RED & DISPHSTX_VGA_MASK, FileSelColors->textbg);
#else // USE_DISPHSTXMINI
					// set white text
					else if (ch == 'W')
						FileSelDispCol(COL_WHITE, FileSelColors->textbg);

					// set green text
					else if (ch == 'X')
						FileSelDispCol(COL_GREEN, FileSelColors->textbg);

					// set yellow text
					else if (ch == 'Y')
						FileSelDispCol(COL_YELLOW, FileSelColors->textbg);

					// set red text
					else if (ch == 'Z')
						FileSelDispCol(COL_RED, FileSelColors->textbg);
#endif // USE_DISPHSTXMINI
					// set normal text
					else if (ch == '0')
						FileSelDispCol(FileSelColors->textfg, FileSelColors->textbg);

					// print character with code 1..9
					else if ((ch >= '1') && (ch <= '9'))
					{
						FileSelDispSmallChar((ch - '0') ^ inv);
						i++; // increase character position
					}

					// print character with code 10..31
					else if ((ch >= 'A') && (ch <= 'V'))
					{
						FileSelDispSmallChar((ch - 'A' + 10) ^ inv);
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
						FileSelDispSmallChar(ch ^ inv);
						i++; // increase character position
					}
				}
				else
				{
					// display one character
					FileSelDispSmallChar(ch ^ inv);
					i++; // increase character position
				}
			}
		}

		// skip rest of the row
		if (i == FILESEL_TEXTW2/2)
		{
			// find LF end of line or NUL end of file
			do {
				ch = FileSelPrevCharCR();
			} while ((ch != CH_LF) && (ch != 0));
		}

		// prepare number of rows
		i = ((FileSelPrevFD->attr & ATTR_BMP) == 0) ? FILESEL_TEXTH2 : (FILESEL_TEXTH2/2);

		// increase curren row
		FileSelPrevLine++;
		if (FileSelPrevLine >= i)
		{
			// close text file
			FileClose(&FileSelPrevFile);

			// start loading bitmap
			FileSelPrevState = PREV_BMP_START;
		}
		break;

	// waiting for bitmap start
	case PREV_BMP_START:

		// no bitmap file
		if ((FileSelPrevFD->attr & ATTR_BMP) == 0)
		{
			// stop loading
			FileSelPrevState = PREV_STOP;
			return;
		}

		// prepare filename of bitmap file
		memcpy(FileSelTempBuf, FileSelPrevFD->name, FileSelPrevFD->len);
		memcpy(&FileSelTempBuf[FileSelPrevFD->len], ".BMP", 5);

		// open bitmap file
		SetDir(FileSelPath);
		if (!FileOpen(&FileSelPrevFile, FileSelTempBuf))
		{
			// cannot open bitmap file, stop
			FileSelPrevState = PREV_STOP;
			return;
		}
		FileSelTempBufNum = 0; // no data in temporary buffer
		FileSelTempBufInx = 0; // reset current index in temporary buffer

		// load bitmap header
		i = FileRead(&FileSelPrevFile, FileSelTempBuf, sizeof(sBmp));

		// get size of preview image
		bmp = (sBmp*)FileSelTempBuf;
		FileSelPrevW = (bmp->biWidth + 1) & ~1; // width aligned to DWORD
		FileSelPrevH = bmp->biHeight;
		if (FileSelPrevH < 0) FileSelPrevH = -FileSelPrevH; // negative height -> flip row order

		// check bitmap header
		if ((i != sizeof(sBmp)) ||
			(FileSelPrevW < 8) || (FileSelPrevW > 2048) ||
			(FileSelPrevH < 8) || (FileSelPrevH > 2048) ||
			(bmp->bfType != 0x4D42) ||
			(bmp->bfSize < 100) || (bmp->bfSize > 5000000) ||
			(bmp->bfOffBits < 54) || (bmp->bfOffBits > 2000) ||
			(bmp->biPlanes != 1) ||
			(bmp->biBitCount != COLBITS) /*||
			(bmp->biCompression != 0)*/)
		{
			FileClose(&FileSelPrevFile);

			// cannot open bitmap file, stop
			FileSelPrevState = PREV_STOP;
			return;
		}

		// set start of first line
		FileSeek(&FileSelPrevFile, bmp->bfOffBits);

		// prepare first video line
		FileSelPrevLine = ((FileSelPrevFD->attr & ATTR_TXT) == 0) ? 0 : (HEIGHT/2);

		// loading bitmap file
		FileSelPrevState = PREV_BMP_LOAD;
		break;

	// loading bitmap
	case PREV_BMP_LOAD:

		// prepare address in video memory
#if COLBITS == 4
		dst = &FrameBuf[FileSelPrevLine*WIDTHLEN + WIDTH/4];
#else
		dst = &FrameBuf[FileSelPrevLine*WIDTH + WIDTH/2];
#endif

		// prepare size of data to read from one line
		i = (FileSelPrevW > (WIDTH/2)) ? (WIDTH/2) : FileSelPrevW;

		// read one video line
#if USE_DISPHSTXMINI && !USE_DISPHSTX_DISPBUF // 1=use HSTX Display Mini driver
		{
			u16* s = FileSelLineBuf;
			FileRead(&FileSelPrevFile, s, (i*COLBITS+7)/8);
			for (; i > 0; i--) *dst++ = *s++ & DISPHSTX_VGA_MASK;
		}
#else
		FileRead(&FileSelPrevFile, dst, (i*COLBITS+7)/8);
#endif

		DispDirtyRect(WIDTH/2, FileSelPrevLine, WIDTH/2, 1);

		// skip rest of line
		if (FileSelPrevW > (WIDTH/2)) FileSeek(&FileSelPrevFile, FilePos(&FileSelPrevFile) + (FileSelPrevW - (WIDTH/2))*COLBITS/8);

		// increase line
		FileSelPrevLine++;
		FileSelPrevH--;

		// end of image
		if ((FileSelPrevLine >= HEIGHT) || (FileSelPrevH <= 0))
		{
			// close preview file
			FileClose(&FileSelPrevFile);

			// stop preview
			FileSelPrevState = PREV_STOP;
		}
		break;

	// preview not active
	// PREV_STOP
	default:
		break;
	}
}

// display big info text
void FileSelDispBigInfo(const char* text)
{
	DrawClear();
	int len = StrLen(text);
	SelFont8x16();
	DrawTextBg2(text, (WIDTH - len*16)/2, 88, FileSelColors->biginfofg, FileSelColors->biginfobg);
	DispUpdate();
}

// display big error text, wait for key press
void FileSelDispBigErr(const char* text)
{
	FileClose(&FileSelPrevFile);

	DrawClear();
	int len = StrLen(text);
	SelFont8x16();
	DrawTextBg2(text, (WIDTH - len*16)/2, (HEIGHT-32)/2, FileSelColors->bigerrfg, FileSelColors->bigerrbg);
	DispUpdate();

	KeyFlush();

#if USE_USB_HOST_HID	// use USB HID Human Interface Device, value = number of interfaces (host)
	UsbFlushKey();
	while ((KeyGet() == NOKEY) && (UsbGetKey() == 0)) {}
#else
	while (KeyGet() == NOKEY) {}
#endif

	FileSelPreviewClr();
	FileSelFrameFileList();
	FileSelDispFileList();
	DispUpdate();
}

// initialize file selection
//  root ... path to root directory (terminated with 0; max. 240 characters)
//   - use uppercase
//   - use '/' character as path separator
//   - start path with '/' root entry
//   - do not end path with '/' character (except root path)
//  title... title on file panel, used instead of root path (terminated with 0; max. 20 characters)
//  ext ... file extension (1 to 3 characters in uppercase, terminated with 0)
//  ... ext2 ... file extension 2 (1 to 3 characters in uppercase, terminated with 0, or 0 characters if not used)
//  ... ext3 ... file extension 3 (1 to 3 characters in uppercase, terminated with 0, or 0 characters if not used)
//  col ... colors (recommended &FileSelColGreen or &FileSelColBlue)
void FileSelInit3(const char* root, const char* title, const char* ext, const char* ext2, const char* ext3, const sFileSelColors* col)
{
	int n;

	FileSelLastNameLen = 0; // no last name

	// get path
	n = StrLen(root); // get length of the path
	memcpy(FileSelPath, root, n+1); // copy path
	FileSelPathLen = n; // length of path
	FileSelRootLen = n; // length of root

	// get title
	n = StrLen(title); // get length of the title
	memcpy(FileSelTitle, title, n+1); // copy title
	FileSelTitleLen = n; // length of the title

	// get file extension
	n = StrLen(ext); // get length of file extension
	memcpy(FileSelExt, ext, n+1); // copy file extension
	FileSelExtLen = n; // length of file extension

	// get file extension 2
	if (ext2 == NULL)
	{
		FileSelExt2[0] = 0;
		FileSelExtLen2 = 0;
	}
	else
	{
		n = StrLen(ext2); // get length of file extension
		memcpy(FileSelExt2, ext2, n+1); // copy file extension
		FileSelExtLen2 = n; // length of file extension
	}

	// get file extension 3
	if (ext3 == NULL)
	{
		FileSelExt3[0] = 0;
		FileSelExtLen3 = 0;
	}
	else
	{
		n = StrLen(ext3); // get length of file extension
		memcpy(FileSelExt3, ext3, n+1); // copy file extension
		FileSelExtLen3 = n; // length of file extension
	}

	// colors
	FileSelColors = col;
}

#if USE_SCREENSHOT		// use screen shots
void ScreenShot();
#endif

// select file (returns True if file selected, or False to break)
Bool FileSel()
{
	int i, j, k, m, n;
	u32 t;
	sFileDesc* fd;

	FileInit(&FileSelPrevFile); // initialize file structure of preview file

	// display frame of file list
	FileSelFrameFileList();
	FileSelPathFileList();
	FileSelDispInfo("Insert SD card");

	// initialize remount
	FileSelLastMount = Time()-2000000; // invalidate last mount time = current time - 2 seconds
	FileSelRemount = True; // remount request

	// loading files
	if (DiskMount())
	{
		FileSelFileTop = 0;

		// display info text
		FileSelDispInfo("Loading files...");

		// load files
		FileSelRemount = False; // clear flag - disk already remounted
		FileSelLoadFileList(); // load file list

		// set cursor to last name
		FileSelSetLastName();

		// display new directory
		FileSelDispFileList();
	}

	// flush keyboard
	KeyFlush();
#if USE_USB_HOST_HID	// use USB HID Human Interface Device, value = number of interfaces (host)
	UsbFlushKey();
#endif

	// main loop
	while (True)
	{
		// display update
		DispUpdate();

		// check last mount time (interval 1 second)
		t = Time();
		if ((int)(t - FileSelLastMount) > 1000000)
		{
			// save new last mount time
			FileSelLastMount = t;

			// disk removed, request to remount disk next time
			if (!DiskMount())
			{
				// clear directories if not done yet
				if (!FileSelRemount) // if remount request yet not set
				{
					// clear preview panel
					FileSelPreviewClr();

					// reset to root
					FileSelResetRoot();

					// display info text
					FileSelDispInfo("Insert SD card");
				}
			}
			else
			{
				// reload directories if disk need to be remounted
				if (FileSelRemount)
				{
					// clear preview panel
					FileSelPreviewClr();

					// display info text
					FileSelDispInfo("Loading files...");

					// load files
					FileSelRemount = False; // clear flag - disk already remounted
					FileSelLoadFileList(); // load file list

					// set cursor to last name
					FileSelSetLastName();

					// display new directory
					FileSelDispFileList();
				}
			}
		}

		// keyboard service
		while (True)
		{
			u8 ch = KeyGet();

#if USE_USB_HOST_HID	// use USB HID Human Interface Device, value = number of interfaces (host)
			if (ch == NOKEY)
			{
				ch = (u8)(UsbGetKey() & 0xff);
				switch (ch)
				{
				case HID_KEY_ARROW_UP:
					ch = KEY_UP;
					break;

				case HID_KEY_ARROW_DOWN:
					ch = KEY_DOWN;
					break;

				case HID_KEY_ARROW_RIGHT:
					ch = KEY_RIGHT;
					break;

				case HID_KEY_ARROW_LEFT:
					ch = KEY_LEFT;
					break;

				case HID_KEY_Y:
				case HID_KEY_ESCAPE:
					ch = KEY_Y;
					break;

				case HID_KEY_X:
					ch = KEY_X;
					break;

				case HID_KEY_A:
				case HID_KEY_ENTER:
				case HID_KEY_SPACE:
					ch = KEY_A;
					break;

				case HID_KEY_B:
					ch = KEY_B;
					break;

				default:
					ch = NOKEY;
				}
			}
#endif

			if (ch == NOKEY) break;

			// key switch
			switch (ch)
			{
			// Down
			case KEY_DOWN:
				if (FileSelFileCur < FileSelFileNum-1)
				{
					// increase cursor
					FileSelFileCur++;

					// prepare max. top entry
					i = FileSelFileCur - (FILESEL_FILEROWS-1);
					if (i < 0) i = 0;

					// limit min. top entry
					if (FileSelFileTop < i) FileSelFileTop = i;

					// repaint display
					FileSelDispFileList();
				}
				break;

			// PageDown and Right
			case KEY_RIGHT:
				if (FileSelFileCur < FileSelFileNum-1)
				{
					// max. top entry
					i = FileSelFileNum - FILESEL_FILEROWS;
					if (i < 0) i = 0;

					// max. top entry not reached - move page
					if (FileSelFileTop < i)
					{
						j = FILESEL_FILEROWS - 1; // page size
						FileSelFileTop += j; // shift top file
						if (FileSelFileTop > i) // max. top entry exceeded?
						{
							j -= FileSelFileTop - i; // reduce page size
							FileSelFileTop = i; // reduce top entry
						}
						FileSelFileCur += j; // shift cursor
					}

					// max. top entry reached - only shift cursor
					else
						FileSelFileCur = FileSelFileNum-1;

					// repaint display
					FileSelDispFileList();
				}
				break;

			// Up
			case KEY_UP:
				if (FileSelFileCur > 0)
				{
					// decrease cursor
					FileSelFileCur--;

					// limit top entry
					if (FileSelFileTop > FileSelFileCur) FileSelFileTop = FileSelFileCur;

					// repaint display
					FileSelDispFileList();
				}
				break;

			// PagUp and Left
			case KEY_LEFT:
				if (FileSelFileCur > 0)
				{
					// min. top entry not reached - move page
					if (FileSelFileTop > 0)
					{
						j = FILESEL_FILEROWS - 1; // page size
						FileSelFileTop -= j; // shift top file
						if (FileSelFileTop < 0) // min. top entry exceeded?
						{
							j += FileSelFileTop; // reduce page size
							FileSelFileTop = 0; // reduce top entry
						}
						FileSelFileCur -= j; // shift cursor
					 }

					// min. top entry reached - only shift cursor
					else
						FileSelFileCur = 0;

					// repaint display
					FileSelDispFileList();
				}
				break;

			// Enter
			case KEY_A:
				if ((uint)FileSelFileCur < (uint)FileSelFileNum)
				{
					fd = &FileSelList[FileSelFileCur];

					// change directory
					if ((fd->attr & ATTR_DIR) != 0)
					{
						// going to up-directory
						if ((fd->len == 2) && (fd->name[0] == '.') && (fd->name[1] == '.'))
						{
							// delete last directory
							i = FileSelPathLen;
							while (FileSelPath[FileSelPathLen-1] != PATHCHAR) FileSelPathLen--;

							// save last name
							FileSelLastNameLen = i - FileSelPathLen;
							memcpy(FileSelLastName, &FileSelPath[FileSelPathLen], FileSelLastNameLen);
							FileSelLastNameAttr = ATTR_DIR;
							FileSelLastNameExt = 0;

							// delete path separator if not root
							if (FileSelPathLen > 1) FileSelPathLen--;
						}

						// going to sub-directory
						else
						{
							// check path length
							if (FileSelPathLen + 1 + fd->len <= FILESEL_PATHMAX)
							{
								// add path separator if not root
								if (FileSelPathLen != 1) FileSelPath[FileSelPathLen++] = PATHCHAR;

								// add path name
								memcpy(&FileSelPath[FileSelPathLen], fd->name, fd->len);
								FileSelPathLen += fd->len;
							}
						}

						// set terminating 0
						FileSelPath[FileSelPathLen] = 0;

						// request to reload current directory
						FileSelLastNameTop = 0;
						FileSelFileTop = 0;
						FileSelReload();

						// invalidate last mount time = current time - 2 seconds
						FileSelLastMount = Time()-2000000;
					}
					else
					{
						// close old preview file
						FileClose(&FileSelPrevFile);

						// prepare filename of the file
						memcpy(FileSelTempBuf, fd->name, fd->len);
						FileSelTempBuf[fd->len] = '.';
						if (fd->ext == 0)
							memcpy(&FileSelTempBuf[fd->len+1], FileSelExt, FileSelExtLen+1);
						else if (fd->ext == 1)
							memcpy(&FileSelTempBuf[fd->len+1], FileSelExt2, FileSelExtLen2+1);
						else
							memcpy(&FileSelTempBuf[fd->len+1], FileSelExt3, FileSelExtLen3+1);

						// save last name
						memcpy(FileSelLastName, fd->name, fd->len+1);
						FileSelLastNameLen = fd->len;
						FileSelLastNameTop = FileSelFileTop;
						FileSelLastNameAttr = fd->attr;
						FileSelLastNameSize = fd->size; // size of last name
						FileSelLastNameExt = fd->ext;

						// prepare file
						FileSelDispBigInfo("Loading...");
						SetDir(FileSelPath);
						return True;
					}
				}
				break;

			// out of directory
			case KEY_B:
				// not root directory yet
				if (FileSelPathLen > FileSelRootLen)
				{
					// delete last directory
					i = FileSelPathLen;
					while (FileSelPath[FileSelPathLen-1] != PATHCHAR) FileSelPathLen--;

					// save last name
					FileSelLastNameLen = i - FileSelPathLen;
					memcpy(FileSelLastName, &FileSelPath[FileSelPathLen], FileSelLastNameLen);
					FileSelLastNameAttr = ATTR_DIR;
					FileSelLastNameExt = 0;

					// delete path separator if not root
					if (FileSelPathLen > 1) FileSelPathLen--;

					// set terminating 0
					FileSelPath[FileSelPathLen] = 0;

					// request to reload current directory
					FileSelLastNameTop = 0;
					FileSelFileTop = 0;
					FileSelReload();

					// invalidate last mount time = current time - 2 seconds
					FileSelLastMount = Time()-2000000;
				}
				break;

			case KEY_X:
#if USE_SCREENSHOT		// use screen shots
				ScreenShot();
#endif
				break;

			// exit
			case KEY_Y:
				return False;

			}
		}

		// preview
		FileSelPreview();
	}
}

#endif // USE_FILESEL		// use file selection (lib_filesel.c, lib_filesel.h)
