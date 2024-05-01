
// ****************************************************************************
//
//                       IBM PC Emulator - Load program
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

// >>> Keep tables and functions in RAM (without "const") for faster access.

// EXE file header
sExeFile PC_ExeFile;
Bool PC_ExeFileOK; // program is in EXE format (or COM otherwise)

// configuration file
int PC_CfgFileNum; // size of configuration file

// display progress bar
//  i ... current position
//  n ... max. position
void PC_Progress(int i, int n, int y, u16 col)
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

#define COL_BIGINFOFG	COL_YELLOW	// big info text foreground color
#define COL_BIGINFOBG	COL_BLACK	// big info text background color

// display big info text
void PC_DispBigInfo(const char* text)
{
	DrawClear();
	int len = StrLen(text);
	DrawTextBg2(text, (WIDTH - len*16)/2, 88, COL_BIGINFOFG, COL_BIGINFOBG);
	DispUpdate();
}

// clear program memory
void PC_ClearApp()
{
	// display info
	PC_DispBigInfo("Erasing...");

	// find end of memory (last 4 KB are reserver for config)
	u32 addr = (u32)PC_ROM_BASE;
	u32 count = 2*1024*1024 - 4096 - (addr - XIP_BASE);
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
	if (count > 2*1024*1024 - 4096 - (addr - XIP_BASE)) count = 2*1024*1024 - 4096 - (addr - XIP_BASE);

	// erase
	FlashErase((u32)PC_ROM_BASE - XIP_BASE, count);

	// wait a while
	WaitMs(50);
}

// load program to Flash memory (loads program and adds program to memory pages; returns False on error)
//  file ... open file
//  size ... file size in bytes
//  PC_ROM_BASE ... base address to load program to the flash (must be aligned to 4 KB)
//  PC_RAM_BASE ... base RAM address to load temporary data
//  PC_RAM_PAGE_NUM*PC_PAGE_SIZE ... max. size of RAM
Bool PC_LoadProg(sFile* file, int size)
{
	// clear program memory
	PC_ClearApp();

	// load file
	PC_DispBigInfo("Loading...");

	// progress bar
	PC_Progress(0, size, 122, COL_GREEN);

	// load EXE file header
	int i = FileRead(file, &PC_ExeFile, sizeof(sExeFile));

	// check EXE header
	int relnum = 0;
	if ((i < sizeof(sExeFile)) || (PC_ExeFile.sign[0] != 'M') || (PC_ExeFile.sign[1] != 'Z'))
	{
		// check COM file size
		if (size > PC_COM_SIZE_MAX) return False;

		// prepare to load COM file
		FileSeek(file, 0); // reset seek pointer back to start of file
		PC_ExeFileOK = False; // COM format
	}

	// prepare to load EXE file
	else
	{
		// check size of relocation table (need 1 page of RAM to load program)
		relnum = PC_ExeFile.relnum;
		int relsize = relnum * 4;
		if (relsize > (PC_RAM_PAGE_NUM-1)*PC_PAGE_SIZE) return False;

		// seek to start of relocation table
		FileSeek(file, PC_ExeFile.reloff);

		// read relocation table
		i = FileRead(file, PC_RAM_BASE, relsize);
		if (i != relsize) return False;

		// seek to start of program
		FileSeek(file, PC_ExeFile.header*16);
		size -= PC_ExeFile.header*16; // program size
		PC_ExeFileOK = True; // EXE format
	}

	// check program size
	if (size > PC_PROG_MAX) size = PC_PROG_MAX;

	// align size up to page 256 bytes
	int n = (size + 0xff) & ~0xff;

	// temporary buffer
	u8* buf = &PC_RAM_BASE[relnum * 4];

	// load file
	int carryL = 0; // carry from low odd address
	int carryH = 0; // carry to high odd address
	int k;
	u32 a = (u32)PC_ROM_BASE - XIP_BASE;
	for (i = 0; i < n;)
	{
		// load temporary buffer
		k = n - i;
		if (k > PC_PAGE_SIZE) k = PC_PAGE_SIZE;
		if (FileRead(file, buf, k) == 0) return False;

		// relocate entries
		int offmin = i - 1; // minimal offset: last odd entry in previous block
		int offmax = i + k - 1; // maximal offset: last odd entry in this block
		carryH = 0; // preset - no carry to high odd address
		int num;
		u16* s = (u16*)PC_RAM_BASE;
		for (num = relnum; num > 0; num--)
		{
			// relative offset of the entry
			int entry = s[0] + ((u32)s[1] << 4);
			if ((entry >= offmin) && (entry <= offmax))
			{
				// relocate odd entry HIGH
				if (entry == offmin)
				{
					buf[0] += carryL + (u8)(PC_PROG_SEG >> 8);
				}

				// relocate odd entry LOW
				else if (entry == offmax)
				{
					int rr = (int)(u8)buf[entry - i] + (u8)PC_PROG_SEG;
					buf[entry - i] = (u8)rr;
					carryH = (rr > 255) ? 1 : 0;
				}

				// relocate inner entry
				else
				{
					entry -= i;
					u16 bb = buf[entry] + ((u16)buf[entry+1] << 8);
					bb += PC_PROG_SEG;
					buf[entry] = (u8)bb;
					buf[entry+1] = (u8)(bb >> 8);
				}
			}

			// shift to next entry
			s += 2;
		}
		carryL = carryH; // shift carry

		// shift offset
		i += k;

		// progress bar
		PC_Progress(i, n, 122, COL_GREEN);
		
		// program
		FlashProgram(a, buf, k);
		a += k;
	}

	// clear screen
	DrawClear();
	DispUpdate();

	// add program to pages
	PC_MemMapSetupRom(size);

	return True;
}

// load program configuration into RAM memory (returns False on error - can use parser, but will return default values)
//  filename ... filename can be in the same RAM as later file content
//  PC_RAM_BASE ... base RAM address to load temporary data
//  PC_RAM_PAGE_NUM*PC_PAGE_SIZE ... max. size of RAM
Bool PC_LoadCfg(const char* filename)
{
	sFile f;

	// open file
	if (!FileOpen(&f, filename)) return False;

	// load file
	PC_CfgFileNum = FileRead(&f, PC_RAM_BASE, PC_RAM_PAGE_NUM*PC_PAGE_SIZE);

	// close file
	FileClose(&f);
	return True;
}

// get config parameter
int PC_GetCfg(const char* name, int def)
{
	// prepare length of parameter
	int len = StrLen(name);
	const char* t = (const char*)PC_RAM_BASE;
	const char* tend = t + PC_CfgFileNum;

	// while not end
	while (t < tend)
	{
		// compare parameter
		if ((memcmp(t, name, len) == 0) && (t[len] == '='))
		{
			// parse parameter
			return StrToInt(t + len + 1);
		}

		// find next row
		while ((t < tend) && (*t != CH_LF)) t++;
		while ((t < tend) && ((u8)*t <= 32)) t++;
	}
	return def;
}
