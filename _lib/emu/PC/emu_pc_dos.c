
// ****************************************************************************
//
//                       IBM PC Emulator - DOS functions
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

// extended character buffer (0 = no character)
u8 PC_Int21fncBuf = 0;

// number of characters in input buffer AH=0Ah
u8 PC_Int21InNum = 0;

// open files
sFile PC_DOS_OpenFile[PC_DOS_FILES];

// filename
char PC_DOS_FileName[PC_DOS_MAXFILENAME+1];

// Initialize PSP (Program Segment Prefix)
//  0x00F00 - 0x00FFF: 256 bytes PSP Program Segment Prefix of user COM program (0x00F0:0x0000 - 0x00F0:0x00FF)
//  endseg ... segment of address beyond program
void PC_InitPSP(u16 endseg)
{
	// pointer to PSP
	u8* psp = &PC_RAM_BASE[0x00F00];

	// clear PSP
	memset(psp, 0, 256);

	// INT 20h instruction (terminate program with RET instruction)
	psp[0x00] = 0xCD;
	psp[0x01] = 0x20;

	// end of allocated memory
	*(u16*)&psp[0x02] = endseg;

	// far jump to INT 21h
	psp[0x05] = 0xEA;
	*(u16*)&psp[0x06] = 0x21; // offset of INT 21h trap
	*(u16*)&psp[0x08] = PC_TRAP_ADDR >> 4; // segment of INT 21h trap

	// old address of INT 22h (program termination)
	*(u16*)&psp[0x0A] = 0x22; // offset of INT 22h trap
	*(u16*)&psp[0x0C] = PC_TRAP_ADDR >> 4; // segment of INT 22h trap

	// old address of INT 23h (Ctrl+Break)
	*(u16*)&psp[0x0E] = 0x23; // offset of INT 23h trap
	*(u16*)&psp[0x10] = PC_TRAP_ADDR >> 4; // segment of INT 23h trap

	// old address of INT 24h (error handler)
	*(u16*)&psp[0x12] = 0x24; // offset of INT 24h trap
	*(u16*)&psp[0x14] = PC_TRAP_ADDR >> 4; // segment of INT 24h trap

	// segment of previous PSP (emulator: not used, pointing to self)
	*(u16*)&psp[0x16] = 0x00f0;

	// environment segment (emulator: not used)
	// *(u16*)&psp[0x2C] = 0;

	// emulator: media descriptor (fixed disk)
	psp[PC_DOS_MEDIA] = 0xf0;

	// Unix-like far call entry into DOS (INT 21h + RETF)
	psp[0x50] = 0xCD; // INT 21h
	psp[0x51] = 0x21;
	psp[0x52] = 0xCB; // RETF

	// unopened first FCB (16 bytes)
	//psp[0x5C]

	// unopened second FCB (16 bytes)
	//psp[0x6C]

	// command line length
	//psp[0x80] = 0;

	// command line (max. 127 bytes)
	//psp[0x81] = ...
}

// close DOS files
void PC_CloseFiles()
{
	// close files
	int i;
	for (i = 0; i < PC_DOS_FILES; i++) FileClose(&PC_DOS_OpenFile[i]);
}

// load DOS filename (path is not supported)
void PC_LoadFileName(u16 seg, u16 off)
{
	sI8086* cpu = PC_Cpu;
	int i;
	for (i = 0; i < PC_DOS_MAXFILENAME;)
	{
		char ch = I8086_GetByte(cpu, seg, off++);
		if (ch == 0) break;
		if ((ch == ':') || (ch == '\\') || (ch == '/'))
			i = 0;
		else
			PC_DOS_FileName[i++] = ch;
	}
	PC_DOS_FileName[i] = 0;
}

// DOS output character to console
void PC_DosOutCh(u8 ch)
{
	PC_WriteTty(ch, PC_Vmode.page, PC_DEF_COL, True); // write character
}

// INT 21h - set error
void PC_Int21Error()
{
	// pointer to CPU
	sI8086* cpu = PC_Cpu;

	// error code - not supported
	cpu->ax = 1;

	// set error flag
	u16 f = I8086_GetWord(cpu, cpu->ss, cpu->sp+4);
	f |= I8086_CF;
	I8086_SetWord(cpu, cpu->ss, cpu->sp+4, f);
}

// INT 21h - clear error
void PC_Int21Ok()
{
	// pointer to CPU
	sI8086* cpu = PC_Cpu;

	// clear error flag
	u16 f = I8086_GetWord(cpu, cpu->ss, cpu->sp+4);
	f &= ~I8086_CF;
	I8086_SetWord(cpu, cpu->ss, cpu->sp+4, f);
}

// INT 21h service - function; returns instruction code I8086_INS_IRET if continue, or I8086_INS_NOP if waiting in loop
u8 PC_Int21fnc()
{
	// temporary disk buffer
#define TMP_DISKBUF	64
	u8 diskbuf[TMP_DISKBUF];

	// return instruction IRET
	u8 res = I8086_INS_IRET;

	// pointer to CPU
	sI8086* cpu = PC_Cpu;

	// get AH function
	u8 ah = cpu->ah;

	// flush input buffer
	if (ah == 0x0c)
	{
		// flush keyboard buffer
		PC_Int21fncBuf = 0; // extended character buffer (0 = no character)
		PC_Int21InNum = 0; // number of characters in input buffer AH=0Ah
		*(u16*)&PC_RAM_BASE[0x041A] = *(u16*)&PC_RAM_BASE[0x041C]; // write pointer -> read pointer

		// check valid function
		ah = cpu->al;
		if ((ah == 0x01) || (ah == 0x06) || (ah == 0x07) || (ah == 0x08) || (ah == 0x0A))
		{
			cpu->ah = ah;
		}

		// invalid function
		else
		{
			cpu->al = 0;
			return res;
		}
	}

	switch (ah)
	{
	// terminate program
	case 0x00:
	case 0x31:
	case 0x4C:
		cpu->stop = 1;
		break;

	// input from keyboard with echo
	case 0x01:

		// DOS Ctrl+Break or Ctrl+C
		PC_DosBreak();

		// check character saved in buffer
		if (PC_Int21fncBuf != 0)
		{
			cpu->al = PC_Int21fncBuf;
			PC_Int21fncBuf = 0;
		}
		else
		{
			// input key into cpu->AX
			//   ext ... use extended code
			// Returns: I8086_INS_NOP if no key is ready, or I8086_INS_IRET if key was loaded into cpu->AX
			res = PC_InCh(False);

			// no key is ready, wait
			if (res == I8086_INS_NOP) break;

			// control character, save extended code for later
			u8 al = cpu->al;
			if (al == 0)
				PC_Int21fncBuf = cpu->ah;
			else
				// or echo ASCII character
				PC_DosOutCh(al);

			// restore command
			cpu->ah = ah;
		}
		break;

	// output character
	case 0x02:
		// DOS Ctrl+Break or Ctrl+C
		PC_DosBreak();

		// output character
		PC_DosOutCh(cpu->dl);
		break;

	// input/output character
	case 0x06:
		// input character
		if (cpu->dl == 0xff)
		{
			// load flags
			u16 f = I8086_GetWord(cpu, cpu->ss, cpu->sp+4);

			// check character saved in buffer
			if (PC_Int21fncBuf != 0)
			{
				cpu->al = PC_Int21fncBuf;
				PC_Int21fncBuf = 0; // clear buffer
				f &= ~I8086_ZF; // clear zero flag
			}
			else
			{
				// input key into cpu->AX
				//   ext ... use extended code
				// Returns: I8086_INS_NOP if no key is ready, or I8086_INS_IRET if key was loaded into cpu->AX
				u8 res2 = PC_InCh(False);

				// character not ready
				if (res2 == I8086_INS_NOP)
				{
					// invalidate code
					cpu->al = 0;

					// set zero flag
					f |= I8086_ZF;
				}
				else
				{
					// control character, save extended code for later
					if (cpu->al == 0) PC_Int21fncBuf = cpu->ah;

					// clear zero flag
					f &= ~I8086_ZF;
				}
			}

			// write new flags
			I8086_SetWord(cpu, cpu->ss, cpu->sp+4, f);

			// restore command
			cpu->ah = ah;
		}

		// output character
		else
			PC_DosOutCh(cpu->dl);
		break;

	// input from keyboard without echo, with Break
	case 0x08:
		// DOS Ctrl+Break or Ctrl+C
		PC_DosBreak();

	// input from keyboard without echo, without Break
	case 0x07:
		// check character saved in buffer
		if (PC_Int21fncBuf != 0)
		{
			cpu->al = PC_Int21fncBuf;
			PC_Int21fncBuf = 0;
		}
		else
		{
			// input key into cpu->AX
			//   ext ... use extended code
			// Returns: I8086_INS_NOP if no key is ready, or I8086_INS_IRET if key was loaded into cpu->AX
			res = PC_InCh(False);

			// no key is ready, wait
			if (res == I8086_INS_NOP) break;

			// control character, save extended code for later
			if (cpu->al == 0) PC_Int21fncBuf = cpu->ah;

			// restore command
			cpu->ah = ah;
		}
		break;

	// print string
	case 0x09:
		{
			// pointer to text DS:DX
			u16 ds = cpu->ds;
			u16 dx = cpu->dx;
			u8 ch;

			for (;;)
			{
				// DOS Ctrl+Break or Ctrl+C
				PC_DosBreak();

				// get character
				ch = I8086_GetByte(cpu, ds, dx++);
				if (ch == '$') break;

				// output character
				PC_DosOutCh(ch);
			}
		}
		break;

	// input text
	case 0x0A:
		{
			// pointer to buffer DS:DX
			u16 ds = cpu->ds;
			u16 dx = cpu->dx;

			// read buffer length
			u8 len = I8086_GetByte(cpu, ds, dx++);

			// incorrect zero buffer length
			if (len == 0)
			{
				// write result length 0
				I8086_SetByte(cpu, ds, dx, 0);
				break;
			}
			len--; // max. length

			// get current length
			u8 n = PC_Int21InNum;
			if (n > len) n = len; // limit current length

			// input loop
			for (;;)
			{
				// DOS Ctrl+Break or Ctrl+C
				PC_DosBreak();

				// input character into AX
				if (PC_InCh(False) == I8086_INS_NOP)
				{
					cpu->ah = ah; // restore function code
					PC_Int21InNum = n; // save new length
					return I8086_INS_NOP; // wait
				}

				// get character
				u8 ch = cpu->al;

				// CR - end of input
				if (ch == CH_CR)
				{
					// write result length
					I8086_SetByte(cpu, ds, dx, n);

					// write CR character
					I8086_SetByte(cpu, ds, dx + 1 + n, ch);

					PC_Int21InNum = 0; // end of input
					break;
				}

				// BS - delete last character
				if ((ch == CH_BS) || (ch == CH_DEL))
				{
					if (n > 0)
					{
						n--;
						PC_DosOutCh(CH_BS);
					}
				}

				// valid character
				else if (ch >= 32)
				{
					if (n < len)
					{
						I8086_SetByte(cpu, ds, dx + 1 + n, ch);
						PC_DosOutCh(ch);
						n++;
					}
				}
			}
		}
		break;

	// check input character
	case 0x0B:
		{
			// get pointers
			u16 r = *(u16*)&PC_RAM_BASE[0x041A]; // read pointer
			u16 w = *(u16*)&PC_RAM_BASE[0x041C]; // write pointer

			// check if read pointer == write pointer
			cpu->al = (r == w) ? 0 : 0xff;
		}
		break;

	// select current disk (emulator: only one disk PC_DOS_DISK is supported)
	case 0x0E:
		cpu->al = PC_DOS_DISK + 1; // number of disks
		break;

	// open FCB (emulator: not supported)
	case 0x0F:
	// close FCB (emulator: not supported)
	case 0x10:
	// find first file FCB (emulator: not supported)
	case 0x11:
	// find next file FCB (emulator: not supported)
	case 0x12:
	// delete file FCB (emulator: not supported)
	case 0x13:
	// read file FCB (emulator: not supported)
	case 0x14:
	// write file FCB (emulator: not supported)
	case 0x15:
	// create file FCB (emulator: not supported)
	case 0x16:
	// rename file FCB (emulator: not supported)
	case 0x17:
	// read file FCB (emulator: not supported)
	case 0x21:
	// write file FCB (emulator: not supported)
	case 0x22:
	// get FCB file size (emulator: not supported)
	case 0x23:
	// seek file FCB (emulator: not supported)
	case 0x24:
	// read file FCB (emulator: not supported)
	case 0x27:
	// write file FCB (emulator: not supported)
	case 0x28:
	// parse filename (emulator: not supported)
	case 0x29:
	// set system date (emulator: not supported)
	case 0x2B:
	// set system time (emulator: not supported)
	case 0x2D:
		cpu->al = 0xff;
		break;

	// get current disk
	case 0x19:
		cpu->al = PC_DOS_DISK;
		break;

	// get specified disk info
	case 0x1C:
		if ((cpu->dl != 0) && (cpu->dl != PC_DOS_DISK+1))
		{
			cpu->al = 0xff; // invalid disk
			break;
		}
	// get current disk info
	case 0x1B:
		cpu->al = 64; // number of sectors per cluster
		cpu->cx = 512; // sector size
		cpu->dx = 32000; // total number of clusters (= total disk size 1 GB)
		cpu->bx = PC_DOS_MEDIA; // offset of media descriptor in PSP
		cpu->ds = 0x00f0; // segment of PSP
		break;

	// set interrupt vector
	case 0x25:
		*(u16*)&PC_RAM_BASE[cpu->al*4] = cpu->dx;	// offset
		*(u16*)&PC_RAM_BASE[cpu->al*4+2] = cpu->ds;	// segment
		break;

	// get system date (emulator: not supported)
	case 0x2A:
		cpu->al = 0;
		cpu->cx = 2000;
		cpu->dh = 1;
		cpu->dl = 1;
		break;

	// get system time (emulator: not supported)
	case 0x2C:
		cpu->ch = 12;
		cpu->cl = 0;
		cpu->dh = 0;
		cpu->dl = 0;
		break;

	// get DTA
	case 0x2F:
		cpu->bx = 0x80;
		cpu->es = 0x00f0;
		break;

	// get DOS version
	case 0x30:
		cpu->al = 3;
		cpu->ah = 0;
		break;

	// set/get BREAK state
	case 0x33:
		if (cpu->al == 0) cpu->dl = 1;
		break;

	// get interrupt vector
	case 0x35:
		cpu->bx = *(u16*)&PC_RAM_BASE[cpu->al*4];	// offset
		cpu->es = *(u16*)&PC_RAM_BASE[cpu->al*4+2];	// segment
		break;

	// get disk free space
	case 0x36:
		if ((cpu->dl != 0) && (cpu->dl != PC_DOS_DISK+1))
		{
			cpu->ax = 0xffff; // invalid disk
			break;
		}
		cpu->ax = 64; // number of sectors per cluster
		cpu->bx = 32000; // free clusters
		cpu->cx = 512; // sector size
		cpu->dx = 32000; // total number of clusters (= total disk size 1 GB)
		break;

	// get country info (emulator: not supported)
	case 0x38:
	// create directory (emulator: not supported)
	case 0x39:
	// delete directory (emulator: not supported)
	case 0x3A:
	// change directory (emulator: not supported)
	case 0x3B:
	// create file (emulator: not supported)
	case 0x3C:
	// write file (emulator: not supported)
	case 0x40:
	// delete file (emulator: not supported)
	case 0x41:
	// file attribute (emulator: not supported)
	case 0x43:
	// IOCTL (emulator: not supported)
	case 0x44:
	// double handle (emulator: not supported)
	case 0x45:
	case 0x46:
	// get current directory (emulator: not supported)
	case 0x47:
	// execute program (emulator: not supported)
	case 0x4B:
	// get code (emulator: not supported)
	case 0x4D:
	// rename file (emulator: not supported)
	case 0x56:
	// get date (emulator: not supported)
	case 0x57:
	// get error code (emulator: not supported)
	case 0x59:
	// create temporary file (emulator: not supported)
	case 0x5A:
	// create file (emulator: not supported)
	case 0x5B:
	// lock file (emulator: not supported)
	case 0x5C:
	// device name (emulator: not supported)
	case 0x5E:
	// redirection (emulator: not supported)
	case 0x5F:

	default:
		{
			// INT 21h - set error
			PC_Int21Error();
		}
		break;

	// open file
	case 0x3D:
		{
			// load DOS filename (path is not supported)
			PC_LoadFileName(cpu->ds, cpu->dx);

			// find free file descriptor
			int inx;
			for (inx = 0; inx < PC_DOS_FILES; inx++)
			{
				if (PC_DOS_OpenFile[inx].name[0] == 0) break;
			}

			// open file
			if ((inx >= PC_DOS_FILES) || !FileOpen(&PC_DOS_OpenFile[inx], PC_DOS_FileName))
			{
				// set error
				PC_Int21Error();
				break;
			}

			// ok
			cpu->ax = inx + PC_DOS_FILE1;

			// clear error flag
			PC_Int21Ok();
			break;
		}
		break;

	// close file
	case 0x3E:
		{
			// check handle
			int inx = (int)cpu->bx - PC_DOS_FILE1;
			if ((inx < 0) || (inx >= PC_DOS_FILES) || (PC_DOS_OpenFile[inx].name[0] == 0))
			{
				// set error
				PC_Int21Error();
			}
			else
			{
				// close file
				FileClose(&PC_DOS_OpenFile[inx]);

				// clear error flag
				PC_Int21Ok();
			}
		}
		break;

	// read file
	case 0x3F:
		{
			// check handle
			int inx = (int)cpu->bx - PC_DOS_FILE1;
			if ((inx < 0) || (inx >= PC_DOS_FILES) || (PC_DOS_OpenFile[inx].name[0] == 0))
			{
				// set error
				PC_Int21Error();
			}
			else
			{
				// get number of bytes and destination address
				u16 cx = cpu->cx;
				u16 ds = cpu->ds;
				u16 dx = cpu->dx;
				u16 ax = 0;

				// read data
				while ((cx > 0) && (cpu->stop == 0))
				{
					// number of bytes, limited to buffer
					u32 n = cx;
					if (n > TMP_DISKBUF) n = TMP_DISKBUF;

					// read data
					u32 n2 = FileRead(&PC_DOS_OpenFile[inx], diskbuf, n);
					if (n2 == 0) break;
					if (n2 > n) n2 = n;
					cx -= n2;
					ax += n2;

					// write data
					n = 0;
					while ((n2 > 0) && (cpu->stop == 0))
					{
						I8086_SetByte(cpu, ds, dx++, diskbuf[n]);
						n2--;
						n++;
					}
				}

				// clear error flag
				PC_Int21Ok();
				cpu->ax = ax;
			}
		}
		break;

	// seek file pointer
	case 0x42:
		{
			// check handle
			int inx = (int)cpu->bx - PC_DOS_FILE1;
			if ((inx < 0) || (inx >= PC_DOS_FILES) || (PC_DOS_OpenFile[inx].name[0] == 0))
			{
				// set error
				PC_Int21Error();
			}
			else
			{
				// prepare new offset
				u32 off = ((u32)cpu->cx << 16) | cpu->dx;
				if (cpu->al == 1)
					// seek from current position
					off += PC_DOS_OpenFile[inx].off;
				else if (cpu->al == 2)
					// seek from file end
					off += PC_DOS_OpenFile[inx].size;
		
				// seek file read/write pointer (returns False on error)
				if (!FileSeek(&PC_DOS_OpenFile[inx], off))
				{
					// set error
					PC_Int21Error();
				}
				else
				{
					// clear error flag
					PC_Int21Ok();

					// return new position
					off = PC_DOS_OpenFile[inx].off;
					cpu->ax = (u16)off;
					cpu->dx = (u16)(off >> 16);
				}
			}
		}
		break;

	// allocate memory
	case 0x48:
		{
			// required memory block in paragraphs
			u16 req = cpu->bx;

			// available RAM memory
			u16 av = ((PC_VmemPageRAM - PC_NextPageRAM) << PC_PAGE_SHIFT) / 16; // empty RAM

			// memory error
			if (req > av)
			{
				cpu->bx = av; // max. memory block

				// INT 21h - set error
				PC_Int21Error();
			}
			else
			{
				// clear error flag
				PC_Int21Ok();

				// allocate memory block
				cpu->ax = PC_AllocEnd; // address of memory block
				PC_AllocEnd += req; // shift end of memory
			}
		}
		break;

	// free memory (emulator: ignored, return OK)
	case 0x49:
		{
			// clear error flag
			PC_Int21Ok();
		}
		break;

	// resize memory (emulator: ignored, return OK)
	case 0x4A:
		{
			// check PSP segment, save new program size
			if (cpu->es == PC_PSP_SEG) PC_AllocEnd = PC_PSP_SEG + cpu->bx;

			// clear error flag
			PC_Int21Ok();
		}
		break;

// !!!!!!! @TODO

	// find file
	case 0x4E:
	// find next file
	case 0x4F:
		{
			// INT 21h - set error
			PC_Int21Error();
		}
		break;

	// get VERIFY state
	case 0x54:
		cpu->al = 1;
		break;

	// get PSP
	case 0x62:
		cpu->bx = PC_PSP_SEG;
		break;
	}

	return res;
}
