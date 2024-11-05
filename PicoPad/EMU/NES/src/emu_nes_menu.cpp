
// ****************************************************************************
//
//                         NES Emulator - Game Menu
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

volatile Bool DoEmuScreenShotReq = False; // request to do screenshot

// length of save filename
int SaveNameLen;

// snapshot slot
int NES_MenuSlot = 0;

u8 NewVol;

// prepare save filename (0..9, -1=default)
void NES_PrepSaveFile(int inx)
{
	// add path separator '/'
	int i = HomePathLen;
	if (HomePath[i-1] != '/')
	{
		HomePath[i] = '/';
		i++;
	}

	// add program name
	memcpy(&HomePath[i], ProgName, ProgNameLen);
	i += ProgNameLen;

	// add extension
	memcpy(&HomePath[i], ".SAV", 5);
	i += 4;

	// set save slot
	if (inx >= 0) HomePath[i-1] = '0' + inx;
	SaveNameLen = i;
}

// unprepare save filename
void NES_UnprepSaveFile()
{
	HomePath[HomePathLen] = 0;
}

// save game (returns True to continue game)
Bool NES_Save(int inx)
{
	sFile f;
	Bool res;
	const char* msg;
	u8 key;
	int i;
	Bool repeat = True;

	// reset after error
	while (True)
	{
		// report
		NES_TextClear();
		NES_TextSetCol(COL_YELLOW);
		NES_TextY = 3;
		NES_TextPrintCenter("Saving...");

		// mount disk
		if (!DiskAutoMount()) 
		{
			if (repeat) goto NesSaveRepeat;
			msg = "No SD disk";
			goto SaveError;
		}

		// set directory
		if (!SetDir(HomePath))
		{
			if (repeat) goto NesSaveRepeat;
			msg = "Invalid directory";
			goto SaveError;
		}

		// prepare save filename
		NES_PrepSaveFile(inx);

		// delete old file
		FileDelete(HomePath);

		// create file
		res = FileCreate(&f, HomePath);

		// unprepare save filename
		NES_UnprepSaveFile();

		// cannot create file
		if (!res)
		{
			if (repeat) goto NesSaveRepeat;
			msg = "Cannot create file";
			goto SaveError;
		}

		// save RAM memory
		NES_Unlink(); // unlink NES
		NES->savestart = NES_SAVEFILE_VER; // save file version magic
		res = FileWrite(&f, NES, SAVEFILE_SIZE) == SAVEFILE_SIZE;
		NES_Link(); // link NES

		// close file
		FileClose(&f);
		if (res) return True;

		// write error
		if (repeat) goto NesSaveRepeat;
		msg = "Cannot write file";

		// display error
SaveError: 	NES_TextClear();

		NES_TextSetCol(COL_RED);
		NES_TextY = 0;
		NES_TextPrintCenter("File save ERROR");

		NES_TextSetCol(COL_GREEN);
		NES_TextY = 1;
		NES_TextX = 0;
		NES_PrepSaveFile(inx);
		if (SaveNameLen > NES_MSG_WIDTH)
			NES_TextPrint(&HomePath[SaveNameLen - NES_MSG_WIDTH]);
		else
			NES_TextPrintCenter(HomePath);
		NES_UnprepSaveFile();

		NES_TextY = 3;
		NES_TextSetCol(COL_WHITE);
		NES_TextPrintCenter("A=repeat, Y=break");

		NES_TextSetCol(COL_GRAY);
		NES_TextY = 7;
		NES_TextPrintCenter(msg);

		// wait a key
		KeyFlush();
#if USE_USB_HOST_HID
		UsbFlushKey();
#endif
		while (True)
		{
			key = KeyGet();
			if (key == KEY_Y) return False;
			if (key == KEY_A) break;

#if USE_USB_HOST_HID
			key = UsbGetKey() & 0xff;
			if ((key == HID_KEY_ESCAPE) ||
				(key == HID_KEY_BACKSPACE) ||
				(key == HID_KEY_Y)) return False;
			if (key == HID_KEY_A) break;
#endif
		}

NesSaveRepeat:
		repeat = False;

		// repeat
		DiskUnmount(); // unmount disk
		DiskMount(); // mount disk
		DirCreate(HomePath); // create directory
	}
}

// load game (returns True to start game)
Bool NES_Load(int inx)
{
	sFile f;
	Bool res;
	const char* msg;
	u8 key;
	int n, i;
	Bool repeat = True;

	while (True)
	{
		// report
		NES_TextClear();
		NES_TextSetCol(COL_YELLOW);
		NES_TextY = 3;
		NES_TextPrintCenter("Loading...");

		// mount disk
		if (!DiskAutoMount()) 
		{
			if (repeat) goto NesLoadRepeat;
			msg = "No SD disk";
			goto LoadError;
		}

		// set directory
		if (!SetDir(HomePath))
		{
			if (repeat) goto NesLoadRepeat;
			msg = "Invalid directory";
			goto LoadError;
		}

		// prepare save filename
		NES_PrepSaveFile(inx);

		// open file
		res = FileOpen(&f, HomePath);

		// unprepare save filename
		NES_UnprepSaveFile();

		// cannot open file
		if (!res)
		{
			if (repeat) goto NesLoadRepeat;
			msg = "Cannot open file";
			goto LoadError;
		}

		// load RAM memory
		n = FileRead(&f, NES, SAVEFILE_SIZE);
		NES_Link(); // link NES

		// close file
		FileClose(&f);

		if (n != SAVEFILE_SIZE)
		{
			if (repeat) goto NesLoadRepeat;
			msg = "Cannot read file";
			if (n > 4)
			{
				if (NES->savestart != NES_SAVEFILE_VER) msg = "Incorrect version";

				// reinitialize
				NES_Term();
				NES_Init();
			}
			goto LoadError;
		}

		// check magic
		if (NES->savestart != NES_SAVEFILE_VER) // check save file version magic
		{
			if (repeat) goto NesLoadRepeat;

			// reinitialize
			NES_Term();
			NES_Init();

			msg = "Incorrect version";
			goto LoadError;
		}

		NES_OldSyncTime = Time();
		return True;

		// display error
LoadError: 	NES_TextClear();
		NES_TextSetCol(COL_RED);
		NES_TextY = 0;
		NES_TextPrintCenter("File load ERROR");

		NES_TextSetCol(COL_GREEN);
		NES_TextY = 1;
		NES_TextX = 0;
		NES_PrepSaveFile(inx);
		if (SaveNameLen > NES_MSG_WIDTH)
			NES_TextPrint(&HomePath[SaveNameLen - NES_MSG_WIDTH]);
		else
			NES_TextPrintCenter(HomePath);
		NES_UnprepSaveFile();

		NES_TextY = 3;
		NES_TextSetCol(COL_WHITE);
		NES_TextPrintCenter("A=repeat, Y=break");

		NES_TextSetCol(COL_GRAY);
		NES_TextY = 7;
		NES_TextPrintCenter(msg);

		// wait a key
		KeyFlush();
#if USE_USB_HOST_HID
		UsbFlushKey();
#endif
		while (True)
		{
			key = KeyGet();
			if (key == KEY_Y) return False;
			if (key == KEY_A) break;

#if USE_USB_HOST_HID
			key = UsbGetKey() & 0xff;
			if ((key == HID_KEY_ESCAPE) ||
				(key == HID_KEY_BACKSPACE) ||
				(key == HID_KEY_Y)) return False;
			if (key == HID_KEY_A) break;
#endif
		}

NesLoadRepeat:
		repeat = False;

		// repeat
		DiskUnmount(); // unmount disk
	}
}

// enter menu
void NES_MenuEnter()
{
	NewVol = Config.volume;

	// clear text screen
	NES_TextClear();

	// set message display mode
	NES_DispMode = NES_DISPMODE_MSG;
	dmb();
}

// leave menu
void NES_MenuLeave()
{
	// save volume
	if (NewVol != Config.volume)
	{
		Config.volume = NewVol;
		NES_TermCore1();
		NES_TermSysClk();
		ConfigSave();
		NES_InitSysClk();
		NES_InitCore1();
	}

	// clear text screen
	NES_TextClear();

	// set emlator display mode
	NES_DispMode = NES_DISPMODE_EMU;
	dmb();

#if USE_EMUSCREENSHOT		// use emulator screen shots
	if (DoEmuScreenShotReq)
	{
		WaitMs(100);
		while (DoEmuScreenShotReq || DoEmuScreenShot || ScreenShotIsOpen)
		{
			WaitMs(10);
		}
	}
#endif
}

// draw menu
void NES_MenuDraw()
{
	// clear text screen
	NES_TextClear();

	// set menu color
	NES_TextSetCol(COL_YELLOW);

	// 1) A .. screenshot
	NES_TextPrint("A ... screenshot\n\r");

	// 2) B ... sound off
#ifdef DEB_REMAP			// Debug - enable mapper number incrementing using B key in menu
	NES_TextPrint("B ... DEBUG REMAP!");
#else
	NES_TextPrint("B ... volume ");
	int vol = (NewVol + 5) / 10 * 10;
	if (vol > 100) vol = 100;
	int n = DecNum(DecNumBuf, vol, 0);
	DecNumBuf[n] = '%';
	DecNumBuf[n+1] = 0;
	NES_TextPrint(DecNumBuf);
#endif

	// 3) X ... exit
	NES_TextPrint("\n\rX ... exit\n\r");

	// 4) Y ... continue
	NES_TextPrint("Y ... continue\n\r");

	// set slots color
	NES_TextSetCol(COL_GREEN);

	// 5) Le/Ri slot: 0
	NES_TextPrint("Up/Dn slot: ");
	NES_TextPrintCh(NES_MenuSlot+'0');

	// 6) Up/Dn load/save
	NES_TextPrint("\n\rLe/Ri load/save");

	// 7)
	NES_TextY++;


#if 0
	// set info color
	NES_TextSetCol(COL_LTGRAY);

	NES_TextPrint("\r");
	DecHexNum(DecNumBuf, K6502_Read(NES->reg_pc), 2);
	NES_TextPrint(DecNumBuf);

	NES_TextPrint(" ");
	DecHexNum(DecNumBuf, K6502_Read(NES->reg_pc+1), 2);
	NES_TextPrint(DecNumBuf);

	NES_TextPrint(" ");
	DecHexNum(DecNumBuf, K6502_Read(NES->reg_pc+2), 2);
	NES_TextPrint(DecNumBuf);

	NES_TextPrint(" ");
	DecHexNum(DecNumBuf, K6502_Read(NES->reg_pc+3), 2);
	NES_TextPrint(DecNumBuf);

	NES_TextPrint(" ");
	DecHexNum(DecNumBuf, K6502_Read(NES->reg_pc+4), 2);
	NES_TextPrint(DecNumBuf);

	NES_TextPrint(" ");
	DecHexNum(DecNumBuf, K6502_Read(NES->reg_pc+5), 2);
	NES_TextPrint(DecNumBuf);
#endif

	// set info color
	NES_TextSetCol(COL_LTGRAY);

	// 8) MBC=023
	NES_TextPrint("\n\rMBC=");
	NES_TextPrintCh(MapperNo/100 + '0');
	NES_TextPrintCh((MapperNo/10) % 10 + '0');
	NES_TextPrintCh(MapperNo % 10 + '0');

#if 0
	NES_TextPrint(" PC=");
	DecHexNum(DecNumBuf, NES->reg_pc, 4);
	NES_TextPrint(DecNumBuf);
	NES_TextPrint(" (");
	DecHexNum(DecNumBuf, K6502_Read(NES->reg_pc), 2);
	NES_TextPrint(DecNumBuf);
	NES_TextPrint(")");
#else
	NES_TextPrint(" CRC=");
	DecHexNum(DecNumBuf, NES_RomCrc, 8);
	NES_TextPrint(DecNumBuf);
#endif

	// 9) f=1789773Hz @ 252MHz
	NES_TextPrint("\n\rf=");
	int f = EMU_FREQ;
	DecNum(DecNumBuf, f, 0);
	NES_TextPrint(DecNumBuf);
	NES_TextPrint("Hz @ ");
	DecNum(DecNumBuf, (ClockGetHz(CLK_SYS)+500000)/1000000, 0);
	NES_TextPrint(DecNumBuf);
	NES_TextPrint("MHz\n\r");

	// 10) path
	NES_TextSetCol(COL_MAGENTA);
	NES_PrepSaveFile(NES_MenuSlot); // prepare save filename (0..9, -1=default)
	int i = 0;
	if (SaveNameLen > NES_MSG_WIDTH) i = SaveNameLen - NES_MSG_WIDTH;
	NES_TextPrint(&HomePath[i]);
	NES_UnprepSaveFile(); // unprepare save filename
}

// display emulator menu (return True to continue emulation)
Bool NES_Menu()
{
	// enter menu
	NES_MenuEnter();

	// flush keys
	WaitMs(300);
	KeyFlush();
#if USE_USB_HOST_HID
	UsbFlushKey();
#endif

	// draw emulator menu
	NES_MenuDraw();

	while (True)
	{
#if USE_USB_HOST_HID
		// get USB key
		u32 k = UsbGetKey();
		u8 ch = (u8)(k & 0xff);

		// quit
		switch (ch)
		{
		// screenshot
		case HID_KEY_A:
#if USE_EMUSCREENSHOT		// use emulator screen shots
			DoEmuScreenShotReq = True;	// request to do emulator screenshot
			while (UsbKeyIsPressed(HID_KEY_A)) {}
#endif
			NES_MenuLeave();
			return True;

		// sound
		case HID_KEY_B:
			NewVol = ((NewVol + 5)/10 + 1)*10;
			if (NewVol > 105) NewVol = 0;
			NES_MenuDraw();
			break;

		// continue
		case HID_KEY_ESCAPE:
		case HID_KEY_BACKSPACE:
		case HID_KEY_Y:
			while (UsbKeyIsPressed(HID_KEY_ESCAPE) ||
				UsbKeyIsPressed(HID_KEY_BACKSPACE) ||
				UsbKeyIsPressed(HID_KEY_Y)) {}
			NES_MenuLeave();
			return True;

		// exit
		case HID_KEY_ENTER:
		case HID_KEY_KEYPAD_ENTER:
		case HID_KEY_X:
			NES_MenuLeave();
			return False;

		// down
		case HID_KEY_ARROW_DOWN:
			if (NES_MenuSlot == 0)
				NES_MenuSlot = NES_MENU_SLOTNUM-1;
			else
				NES_MenuSlot--;
			NES_MenuDraw();
			break;

		// up
		case HID_KEY_ARROW_UP:
			if (NES_MenuSlot == NES_MENU_SLOTNUM-1)
				NES_MenuSlot = 0;
			else
				NES_MenuSlot++;
			NES_MenuDraw();
			break;

		// right
		case HID_KEY_ARROW_RIGHT:
			if (NES_Save(NES_MenuSlot))
			{
				while (UsbKeyIsPressed(HID_KEY_ARROW_LEFT)) {}
				NES_MenuLeave();
				return True;
			}
			NES_MenuDraw();
			break;

		// left
		case HID_KEY_ARROW_LEFT:
			if (NES_Load(NES_MenuSlot))
			{
				while (UsbKeyIsPressed(HID_KEY_ARROW_LEFT)) {}
				NES_MenuLeave();
				return True;
			}
			NES_MenuDraw();
			break;
		}
#endif

		// keypad
		switch (KeyGet())
		{
		// screenshot
		case KEY_A:
#if USE_EMUSCREENSHOT		// use emulator screen shots
			DoEmuScreenShotReq = True;	// request to do emulator screenshot
			while (KeyPressed(KEY_A)) {}
#endif
			NES_MenuLeave();
			return True;

		// sound
		case KEY_B:
#ifdef DEB_REMAP			// Debug - enable mapper number incrementing using B key in menu
			NES_MenuLeave();
			NES_Term();
			NES_Init();
			return True;
#else
			NewVol = ((NewVol + 5)/10 + 1)*10;
			if (NewVol > 105) NewVol = 0;
			NES_MenuDraw();
			break;
#endif

		// continue
		case KEY_Y:
			while (KeyPressed(KEY_Y)) {}
			NES_MenuLeave();
			return True;

		// exit
		case KEY_X:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			NES_MenuLeave();
			return False;

		// down
		case KEY_DOWN:
			if (NES_MenuSlot == 0)
				NES_MenuSlot = NES_MENU_SLOTNUM-1;
			else
				NES_MenuSlot--;
			NES_MenuDraw();
			break;

		// up
		case KEY_UP:
			if (NES_MenuSlot == NES_MENU_SLOTNUM-1)
				NES_MenuSlot = 0;
			else
				NES_MenuSlot++;
			NES_MenuDraw();
			break;

		// right
		case KEY_RIGHT:
			if (NES_Save(NES_MenuSlot))
			{
				while (KeyPressed(KEY_RIGHT)) {}
				NES_MenuLeave();
				return True;
			}
			NES_MenuDraw();
			break;

		// left
		case KEY_LEFT:
			if (NES_Load(NES_MenuSlot))
			{
				while (KeyPressed(KEY_LEFT)) {}
				NES_MenuLeave();
				return True;
			}
			NES_MenuDraw();
			break;
		}
	}
}
