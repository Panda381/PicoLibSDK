
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

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
	DrawTextBg(text, DispX*FONTW, DispY*FONTH, FgCol, BgCol);
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

// display frame of file list
void FrameFileList()
{
	// reset cursor
	DispX = 0;
	DispY = FILEROW1;

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
	} while (DispY != FILEROW1+1+FILEROWS);

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
	DispY = FILEROW1;
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
	DispY = FILEROW1+1+FILEROWS;
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
	DispY = FILEROW1+1;

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
			if (dir)
			{
				DispChar(']');
				DispSpcRep(16 - fd->len);
			}
			else
			{
				// prepare file size (max. 9 characters)
				if (fd->size < 1000000)
					j = DecUNum(DecNumBuf, fd->size, '\'');
				else
					j = DecUNum(DecNumBuf, fd->size, 0);

				// print spaces
				DispSpcRep(17 - fd->len - j);

				// print size
				DispText(DecNumBuf);
			}
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

	// display current path
	PathFileList();
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
	int inx, i, len, k;
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

		// get size
		fd->size = FileI.size;

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

			k = fd[0].size;
			fd[0].size = fd[1].size;
			fd[1].size = k;

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
	DispY = 2+FILEROW1;
	DispX = (FILECOLW - len)/2;

	// display info text
	DispText(text);
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
	DrawTextBg2(text, (WIDTH - len*16)/2, 88, COL_BIGINFOFG, COL_BIGINFOBG);
	DispUpdate();
}

// clear program memory (full = erase full memory, including config)
void ClearApp(Bool full)
{
	// display info
	DispBigInfo("Erasing...");

	// find end of memory (last 4 KB are reserved for config)
	u32 addr = XIP_BASE;
	u32 count = FLASHSIZE;
	if (!full) count -= 4096; // without config
	const u32* s = (const u32*)(addr + count);
	while (count >= 4)
	{
		if (s[-1] != (u32)-1) break;
		s--;
		count -= 4;
	}

	// align up to to 64 KB
	count = (count + 0x10000-4) & ~0xffff;

	// limit to protect config on last 4 KB page
	if (!full && (count > FLASHSIZE - 4096)) count = FLASHSIZE - 4096;

	// erase memory
	int n = count;
	int k;
	while (count >= 0x1000)
	{
		Progress(n - count, n, 122, COL_ORANGE);

		k = count;
		if (k >= 0x10000) k = 0x10000;
		FlashErase(addr - XIP_BASE, k);
		addr += k;
		count -= k;
	}
	Progress(n, n, 122, COL_ORANGE);
	WaitMs(50);
}

u32 ProgLen;		// total program length to save
u32 ProgLenReal;	// real program length
Bool AppOK;		// application is OK
u32 AppLen;		// application length
u32 AppCRC;		// application CRC

// display info screen
void DispInfoScr()
{
	int i, y;
	const u8* s;

	// clear screen
	DrawRect(WIDTH/2, 0, WIDTH/2, HEIGHT, COL_BLACK);

	// draw text
	DrawText("SD-Card Boot Loader", WIDTH/2+4, 0, COL_YELLOW);
	DrawText("A=load UF2 to flash", WIDTH/2+4, 12*FONTH, COL_YELLOW);
	DrawText("B=save flash to UF2", WIDTH/2+4, 13*FONTH, COL_YELLOW);
	DrawText("X=save all flash", WIDTH/2+4, 14*FONTH, COL_YELLOW);

	// flash content
	DrawText2("IN FLASH", WIDTH/2+(WIDTH/2-8*16)/2, 22, COL_AZURE);
	y = 58;

	// config
	i = ConfigLoad() + 1;
	if (i > 0)
	{
		MemPrint(TempBuf, TEMPBUF, "config: %d entries", i);
		s = (const u8*)CONFIG_ADDR;
	}
	else
	{
		MemPrint(TempBuf, TEMPBUF, "config: NO");
		s = (const u8*)(XIP_BASE + FLASHSIZE);
	}
	DrawText(TempBuf, WIDTH/2+8, y, COL_WHITE); y += FONTH;
	y += FONTH/2;

	// check application
	AppOK = CheckApp(&AppLen, &ProgLen, &AppCRC);

	// check program size
	while (((u32)s > (u32)XIP_BASE) && (s[-1] == 0xff)) s--;
	ProgLenReal = (u32)s - XIP_BASE;
	if (!AppOK) ProgLen = ProgLenReal;

	// print program size to save
	MemPrint(TempBuf, TEMPBUF, "size: %'u B", ProgLen);
	DrawText(TempBuf, WIDTH/2+8, y, COL_WHITE); y += FONTH;

	// application
	if (AppOK)
	{
		// print real program size in memory (not saved)
		MemPrint(TempBuf, TEMPBUF, "real: %'u B", ProgLenReal);
		DrawText(TempBuf, WIDTH/2+8, y, COL_WHITE); y += FONTH;
		y += FONTH/2;

		// print loader info
		DrawText("loader: PicoLibSDK", WIDTH/2+8, y, COL_WHITE); y += FONTH;
		MemPrint(TempBuf, TEMPBUF, "length: %'u B", BOOTLOADER_SIZE);
		DrawText(TempBuf, WIDTH/2+8, y, COL_WHITE); y += FONTH;

		// print application info
		MemPrint(TempBuf, TEMPBUF, "AppLen: %'u B", AppLen);
		DrawText(TempBuf, WIDTH/2+8, y, COL_WHITE); y += FONTH;
		MemPrint(TempBuf, TEMPBUF, "AppCRC: 0x%08X", AppCRC);
		DrawText(TempBuf, WIDTH/2+8, y, COL_WHITE); y += FONTH;
	}

	// display update
	DispUpdate();
}

// display big error text
void DispBigErr(const char* text)
{
	DrawClear();
	int len = StrLen(text);
	DrawTextBg2(text, (WIDTH - len*16)/2, (HEIGHT-32)/2, COL_BIGERRFG, COL_BIGERRBG);
	DispUpdate();

	KeyFlush();
	while (KeyGet() == NOKEY) {}

	FrameFileList();
	DispFileList();
	DispInfoScr();
}

// save flash
void SaveFlash(Bool full)
{
	int i, n, num, k, len;
	sFile f; // open file
	sUf2* d; // destination sector
	const u8* s; // source address
	char fn[20]; // filename

	// program size to save
	len = full ? FLASHSIZE : ProgLen;

	// search not existing file
	for (i = 1; i < 1000; i++)
	{
		// prepare full filename
		MemPrint(fn, 20, "\\FLASH%d.UF2", i);

		// check if file exists
		if (!FileExist(fn)) break;
	}

	// create file
	if ((i == 1000) || !FileCreate(&f, fn)) 
	{
		// display big error text
		DispBigErr("Cannot create file");
		return;
	}

	// display info
	DispBigInfo("Saving...");

	// write file
	i = 0;	// data offset
	n = 0;	// block index
	num = (len + 255) >> 8; // number of blocks
	s = (const u8*)XIP_BASE; // source data
	d = (sUf2*)TempBuf; // destination sector
	while (i < len)
	{
		// display progress
		Progress(i, len, 122, COL_GREEN);

		// data size of this sector
		k = len - i;
		if (k > 256) k = 256;

		// prepare one sector
		memset(d, 0, sizeof(sUf2));
		d->magic_start0 = UF2_MAGIC_START0;	// magic 1
		d->magic_start1 = UF2_MAGIC_START1;	// magic 2
		d->flags = UF2_FLAG_FAMILY_ID_PRESENT;	// flags
		d->target_addr = (u32)s;		// address
		d->payload_size = k;			// data size
		d->block_no = n;			// block number
		d->num_blocks = num;			// number of blocks
		d->file_size = RP2040_FAMILY_ID;	// family ID
		memcpy(d->data, s, k);			// copy data
		d->magic_end = UF2_MAGIC_END;		// magic end

		// save sector
		FileWrite(&f, d, sizeof(sUf2));

		// shift to next sector
		i += 256;
		s += 256;
		n++;
	}
	Progress(len, len, 122, COL_GREEN);
	WaitMs(50);

	// close file
	FileClose(&f);

	// display info
	MemPrint(TempBuf, TEMPBUF, "Saved %s", fn);
	DispBigInfo(TempBuf);
	KeyFlush();
	while (KeyGet() == NOKEY) {}

	// reload file list
	i = StrLen(fn+1) - 4;;
	LastNameLen = i;
	memcpy(LastName, fn+1, i);
	LastNameDir = 0;
	Remount = True;

	// redraw
	LastMount = Time()-2000000;
	FrameFileList();
//	DispFileList();
	DispInfoScr();
}

// load flash
void LoadFlash()
{
	int i, j, m;
	sFile f; // open file
	u32 d = 0; // destination offset
	sUf2* s = (sUf2*)TempBuf; // source sector

	// file
	sFileDesc* fd = &FileDesc[FileCur];

	// prepare filename of the file
	memcpy(TempBuf, fd->name, fd->len);
	memcpy(&TempBuf[fd->len], ".UF2", 5);

	// open file
	DispBigInfo("Loading...");
	SetDir(Path);
	if (!FileOpen(&f, TempBuf))
	{
		// display big error text
		DispBigErr("Cannot open file");
		return;
	}

	// number of pages
	int n = fd->size/512;
	if (n > FLASHSIZE/256) n = FLASHSIZE/256;

	// check if erase full memory
	Bool full = n > ((FLASHSIZE - 4096)/256);

	// read header of first sector and check address
	FileRead(&f, TempBuf, 32);
	if (s->target_addr != XIP_BASE)
	{
		// display big error text
		DispBigErr("Incorrect address");
		return;
	}

	// clear program memory
	ClearApp(full);

	// load file
	DispBigInfo("Loading...");
	j = 32; // file offset
	for (i = 0; i < n;)
	{
		// load 4 sectors into buffer
		m = 0; // data in buffer
		while ((m <= TEMPBUF-256) && (i < n))
		{
			// set file offset
			FileSeek(&f, j);

			// read sector
			FileRead(&f, &TempBuf[m], 256);

			// shift to next sector
			m += 256; // data in buffer
			i++; // sector index
			j += 512; // file offset
		}

		// progress bar
		Progress(i, n, 122, COL_GREEN);

		// program four 256-byte pages
		FlashProgram(d, (const u8*)TempBuf, m);
		d += m; // destination offset
	}

	// close file
	FileClose(&f);

	// display info
	MemPrint(TempBuf, TEMPBUF, "Loaded %s", fd->name);
	DispBigInfo(TempBuf);
	KeyFlush();
	while (KeyGet() == NOKEY) {}

	// redraw
	FrameFileList();
	DispFileList();
	DispInfoScr();
}

// main function
int main()
{
	int i, j, k, m, n;
	u32 t;
	sFileDesc* fd;

	// initialize Flash interface (clkdiv = clock divider, must be even number, 4 is default)
	SSI_InitFlash(4);

	// reset to root
	LastNameLen = 0; // no last name
	ResetRoot();

	// display info screen
	DispInfoScr();

	// display frame of file list
	FrameFileList();
	DispInfo("Insert SD card");

	// initialize remount
	LastMount = Time()-2000000; // invalidate last mount time = current time - 2 seconds
	Remount = True; // remount request

	// try to mount disk
	if (DiskMount())
	{
		FileTop = 0;

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
	KeyFlush();

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
				}
				break;

			// PageDown
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
				}
				break;

			// Up
			case KEY_UP:
				if (FileCur > 0)
				{
					// decrease cursor
					FileCur--;

					// limit top entry
					if (FileTop > FileCur) FileTop = FileCur;

					// repaint display
					DispFileList();
				}
				break;

			// PagUp
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
				}
				break;

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
						// load flash
						LoadFlash();
					}
				}
				break;

			// save program
			case KEY_B:
				if (DiskMounted()) SaveFlash(False);
				break;

			// save flash
			case KEY_X:
				if (DiskMounted()) SaveFlash(True);
				break;

			// quit
			case KEY_Y:
#if USE_SCREENSHOT		// use screen shots
				ScreenShot();
#endif
				ResetToBootLoader();
				break;
			}
		}
	}
}
