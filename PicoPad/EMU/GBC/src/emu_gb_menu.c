
// ****************************************************************************
//
//                        Game Boy Emulator - Game Menu
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

// display mode
volatile u8 GB_DispMode;		// display mode GB_DISPMODE_*

Bool DoEmuScreenShotReq = False; // request to do screenshot
Bool DoLockedScreenShot = False; // use locked screenshot - game not moving (press B and add A)

// length of save filename
int SaveNameLen;

// snapshot slot
int GB_MenuSlot = 0;

u8 NewVol;

// prepare save filename (0..9, -1=default, -2=ROM file)
void GB_PrepSaveFile(int inx)
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
	if (inx == -2)
	{
		HomePath[i] = '.';
		memcpy(&HomePath[i+1], ProgExt, ProgExtLen+1);
		i += ProgExtLen+1;
	}
	else
	{
		memcpy(&HomePath[i], ".SAV", 5);
		i += 4;

		// set save slot
		if (inx >= 0) HomePath[i-1] = '0' + inx;
	}
	SaveNameLen = i;
}

// unprepare save filename
void GB_UnprepSaveFile()
{
	HomePath[HomePathLen] = 0;
}

// save game (returns True to continue game)
Bool GB_Save(int inx)
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
		GB_TextClear();
		GB_TextSetCol(COL_YELLOW);
		GB_TextY = 3;
		GB_TextPrintCenter("Saving...");

		// mount disk
		if (!DiskAutoMount()) 
		{
			if (repeat) goto GBSaveRepeat;
			msg = "No SD disk";
			goto SaveError;
		}

		// set directory
		if (!SetDir(HomePath))
		{
			if (repeat) goto GBSaveRepeat;
			msg = "Invalid directory";
			goto SaveError;
		}

		// prepare save filename
		GB_PrepSaveFile(inx);

		// delete old file
		FileDelete(HomePath);

		// create file
		res = FileCreate(&f, HomePath);

		// unprepare save filename
		GB_UnprepSaveFile();

		// cannot create file
		if (!res)
		{
			if (repeat) goto GBSaveRepeat;
			msg = "Cannot create file";
			goto SaveError;
		}

		// save RAM memory
		gbContext.savestart = GB_SAVEFILE_VER; // save file version magic
		res = FileWrite(&f, &gbContext.savestart, SAVEFILE_SIZE) == SAVEFILE_SIZE;

		// close file
		FileClose(&f);
		if (res) return True;

		// write error
		if (repeat) goto GBSaveRepeat;
		msg = "Cannot write file";

		// display error
SaveError: 	GB_TextClear();

		GB_TextSetCol(COL_RED);
		GB_TextY = 0;
		GB_TextPrintCenter("File save ERROR");

		GB_TextSetCol(COL_GREEN);
		GB_TextY = 1;
		GB_TextX = 0;
		GB_PrepSaveFile(inx);
		if (SaveNameLen > GB_MSG_WIDTH)
			GB_TextPrint(&HomePath[SaveNameLen - GB_MSG_WIDTH]);
		else
			GB_TextPrintCenter(HomePath);
		GB_UnprepSaveFile();

		GB_TextY = 3;
		GB_TextSetCol(COL_WHITE);
		GB_TextPrintCenter("A=repeat, Y=break");

		GB_TextSetCol(COL_GRAY);
		GB_TextY = 7;
		GB_TextPrintCenter(msg);

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

GBSaveRepeat:
		repeat = False;

		// repeat
		DiskUnmount(); // unmount disk
		DiskMount(); // mount disk
		DirCreate(HomePath); // create directory
	}
}

// load game (returns True to start game)
Bool GB_Load(int inx)
{
	sFile f;
	Bool res;
	const char* msg;
	u8 key;
	int n, i;
	sGB_Cache* c;
	Bool repeat = True;

	while (True)
	{
		// report
		GB_TextClear();
		GB_TextSetCol(COL_YELLOW);
		GB_TextY = 3;
		GB_TextPrintCenter("Loading...");

		// mount disk
		if (!DiskAutoMount()) 
		{
			if (repeat) goto GBLoadRepeat;
			msg = "No SD disk";
			goto LoadError;
		}

		// set directory
		if (!SetDir(HomePath))
		{
			if (repeat) goto GBLoadRepeat;
			msg = "Invalid directory";
			goto LoadError;
		}

		// prepare save filename
		GB_PrepSaveFile(inx);

		// open file
		res = FileOpen(&f, HomePath);

		// unprepare save filename
		GB_UnprepSaveFile();

		// cannot open file
		if (!res)
		{
			if (repeat) goto GBLoadRepeat;
			msg = "Cannot open file";
			goto LoadError;
		}

		// load RAM memory
		n = FileRead(&f, &gbContext.savestart, SAVEFILE_SIZE);

		// close file
		FileClose(&f);

		if (n != SAVEFILE_SIZE)
		{
			if (repeat) goto GBLoadRepeat;
			msg = "Cannot read file";
			if (n > 4)
			{
				if (gbContext.savestart != GB_SAVEFILE_VER) msg = "Incorrect version";
				GB_Setup(); // reinitialize
			}
			goto LoadError;
		}

		// check magic
		if (gbContext.savestart != GB_SAVEFILE_VER) // check save file version magic
		{
			if (repeat) goto GBLoadRepeat;
			GB_Setup(); // reinitialize
			msg = "Incorrect version";
			goto LoadError;
		}

		// update palette
		gbSetPal(gbColorPalSel[gbContext.display.palinx]);

		// clear ROM cache
		c = GB_CacheDesc;
		for (i = 0; i < GB_CACHE_NUM; i++)
		{
			if (c->type == GB_CACHETYPE_VALID) c->type = GB_CACHETYPE_FREE;
			c++;
		}
		memset(GB_CacheROM, GB_CACHEINX_INV, sizeof(GB_CacheROM));

		return True;

		// display error
LoadError: 	GB_TextClear();
		GB_TextSetCol(COL_RED);
		GB_TextY = 0;
		GB_TextPrintCenter("File load ERROR");

		GB_TextSetCol(COL_GREEN);
		GB_TextY = 1;
		GB_TextX = 0;
		GB_PrepSaveFile(inx);
		if (SaveNameLen > GB_MSG_WIDTH)
			GB_TextPrint(&HomePath[SaveNameLen - GB_MSG_WIDTH]);
		else
			GB_TextPrintCenter(HomePath);
		GB_UnprepSaveFile();

		GB_TextY = 3;
		GB_TextSetCol(COL_WHITE);
		GB_TextPrintCenter("A=repeat, Y=break");

		GB_TextSetCol(COL_GRAY);
		GB_TextY = 7;
		GB_TextPrintCenter(msg);

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

GBLoadRepeat:
		repeat = False;

		// repeat
		DiskUnmount(); // unmount disk
	}
}

// enter menu
void GB_MenuEnter()
{
	NewVol = Config.volume;

	// clear text screen
	GB_TextClear();

	// set message display mode
	GB_DispMode = GB_DISPMODE_MSG;
	dmb();
}

// leave menu
void GB_MenuLeave()
{
#if USE_EMUSCREENSHOT		// use emulator screen shots
	// ignore volume on locked screenshot A+B
	if (DoEmuScreenShotReq && DoLockedScreenShot) NewVol = Config.volume;
#endif

#if DEB_WRITECRAM	// 1=debug display write bytes into CRAM (in game menu)
	CRamWriteNum = 0;	// number of writes to CRAM
#endif

	// save volume
	if (NewVol != Config.volume)
	{
		Config.volume = NewVol;
		GB_TermCore1();
		GB_TermSysClk();
		ConfigSave();
		GB_InitSysClk();
		GB_InitCore1();
	}

	// clear text screen
	GB_TextClear();

	// set emlator display mode
	GB_DispMode = GB_DISPMODE_EMU;
	dmb();

#if USE_EMUSCREENSHOT		// use emulator screen shots
	if (DoEmuScreenShotReq)
	{
		u8 oldpause = gbContext.gb_pause;
		if (DoLockedScreenShot) gbContext.gb_pause = 1; // pause program

		while (DoEmuScreenShotReq || DoEmuScreenShot || ScreenShotIsOpen)
		{
			gb_run_frame(&gbContext);
			dmb();
			WaitMs(100);
		}

		if (DoLockedScreenShot) gbContext.gb_pause = oldpause; // unpause program
	}
#endif
}

// draw menu
void GB_MenuDraw()
{
	// clear text screen
	GB_TextClear();

	// set menu color
	GB_TextSetCol(COL_YELLOW);

	// 1) A .. screenshot
	GB_TextPrint("A ... screenshot(+B)\n\r");

	// 2) B ... sound off
	GB_TextPrint("B ... volume ");
	int vol = (NewVol + 5) / 10 * 10;
	if (vol > 100) vol = 100;
	int n = DecNum(DecNumBuf, vol, 0);
	DecNumBuf[n] = '%';
	DecNumBuf[n+1] = 0;
	GB_TextPrint(DecNumBuf);

	// 3) X ... exit
	GB_TextPrint("\n\rX ... exit\n\r");

	// 4) Y ... continue
	GB_TextPrint("Y ... continue\n\r");

	// set slots color
	GB_TextSetCol(COL_GREEN);

	// 5) Le/Ri slot: 0
	GB_TextPrint("Up/Dn slot: ");
	GB_TextPrintCh(GB_MenuSlot+'0');

	// 6) Up/Dn load/save
	GB_TextPrint("\n\rLe/Ri load/save");

	// 7)
	GB_TextY++;

	// set info color
	GB_TextSetCol(COL_LTGRAY);

	// 8) DEV=GBC MBC=1 PAL=23
	GB_TextPrint("\n\rDEV=");
	GB_TextPrint(gbContext.cgb.cgbMode ? "GBC" : "DMG");
	GB_TextPrint(" MBC=");
	GB_TextPrintCh(gbContext.mbc + '0');

#if DEB_WRITECRAM	// 1=debug display write bytes into CRAM (in game menu)
	GB_TextPrint(" W");
	DecNum(DecNumBuf, CRamWriteNum, 0);
#else
	GB_TextPrint(" PAL=");
	DecNum(DecNumBuf, gbContext.display.palinx+1, 0);
#endif
	GB_TextPrint(DecNumBuf);

	// 9) f=4194304Hz @ 252MHz
	GB_TextPrint("\n\rf=");
	int f = 4194304;
	if (gbContext.cgb.doubleSpeed) f *= 2;
	DecNum(DecNumBuf, f, 0);
	GB_TextPrint(DecNumBuf);
	GB_TextPrint("Hz @ ");
	DecNum(DecNumBuf, (ClockGetHz(CLK_SYS)+500000)/1000000, 0);
	GB_TextPrint(DecNumBuf);
	GB_TextPrint("MHz\n\r");

	// 10) GAMENAME_11 CRC=2A23
	GB_TextSetCol(COL_AZURE);
	int i;
	for (i = 0x134; i < 0x13F; i++)
	{
		char ch = gameRom[i];
		if ((ch >= 0x20) && (ch <= 0x7F))
			GB_TextPrintCh(ch);
		else
			break;
	}
	GB_TextX = 12;
	GB_TextPrint("CRC=");
	DecHexNum(DecNumBuf, TitleCrc, 4);
	GB_TextPrint(DecNumBuf);
	GB_TextPrint("\n\r");

	// 11) path
	GB_TextSetCol(COL_MAGENTA);
	GB_PrepSaveFile(GB_MenuSlot); // prepare save filename (0..9, -1=default)
	i = 0;
	if (SaveNameLen > GB_MSG_WIDTH) i = SaveNameLen - GB_MSG_WIDTH;
	GB_TextPrint(&HomePath[i]);
	GB_UnprepSaveFile(); // unprepare save filename
}

// display emulator menu (return True to continue emulation)
Bool GB_Menu()
{
	// enter menu
	GB_MenuEnter();

	// flush keys
	WaitMs(300);
	KeyFlush();
#if USE_USB_HOST_HID
	UsbFlushKey();
#endif

	// draw emulator menu
	GB_MenuDraw();

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
			DoLockedScreenShot = UsbKeyIsPressed(HID_KEY_B); // use locked screenshot - game not moving (press B and add A)
			DoEmuScreenShotReq = True;	// request to do emulator screenshot
			while (UsbKeyIsPressed(HID_KEY_A) || UsbKeyIsPressed(HID_KEY_B)) {}
#endif
			GB_MenuLeave();
			return True;

		// sound
		case HID_KEY_B:
			NewVol = ((NewVol + 5)/10 + 1)*10;
			if (NewVol > 105) NewVol = 0;
			GB_MenuDraw();
			break;

		// continue
		case HID_KEY_ESCAPE:
		case HID_KEY_BACKSPACE:
		case HID_KEY_Y:
			while (UsbKeyIsPressed(HID_KEY_ESCAPE) ||
				UsbKeyIsPressed(HID_KEY_BACKSPACE) ||
				UsbKeyIsPressed(HID_KEY_Y)) {}
			GB_MenuLeave();
			return True;

		// exit
		case HID_KEY_ENTER:
		case HID_KEY_KEYPAD_ENTER:
		case HID_KEY_X:
			GB_MenuLeave();
			return False;

		// down
		case HID_KEY_ARROW_DOWN:
			if (GB_MenuSlot == 0)
				GB_MenuSlot = GB_MENU_SLOTNUM-1;
			else
				GB_MenuSlot--;
			GB_MenuDraw();
			break;

		// up
		case HID_KEY_ARROW_UP:
			if (GB_MenuSlot == GB_MENU_SLOTNUM-1)
				GB_MenuSlot = 0;
			else
				GB_MenuSlot++;
			GB_MenuDraw();
			break;

		// right
		case HID_KEY_ARROW_RIGHT:
			if (GB_Save(GB_MenuSlot))
			{
				while (UsbKeyIsPressed(HID_KEY_ARROW_LEFT)) {}
				GB_MenuLeave();
				return True;
			}
			GB_MenuDraw();
			break;

		// left
		case HID_KEY_ARROW_LEFT:
			if (GB_Load(GB_MenuSlot))
			{
				while (UsbKeyIsPressed(HID_KEY_ARROW_LEFT)) {}
				GB_MenuLeave();
				return True;
			}
			GB_MenuDraw();
			break;
		}
#endif

		// keypad
		switch (KeyGet())
		{
		// screenshot
		case KEY_A:
#if USE_EMUSCREENSHOT		// use emulator screen shots
			DoLockedScreenShot = KeyPressed(KEY_B); // use locked screenshot - game not moving (press B and add A)
			DoEmuScreenShotReq = True;	// request to do emulator screenshot
			while (KeyPressed(KEY_A) || KeyPressed(KEY_B)) {}
#endif
			GB_MenuLeave();
			return True;

		// sound
		case KEY_B:
			NewVol = ((NewVol + 5)/10 + 1)*10;
			if (NewVol > 105) NewVol = 0;
			GB_MenuDraw();
			break;

		// continue
		case KEY_Y:
			while (KeyPressed(KEY_Y)) {}
			GB_MenuLeave();
			return True;

		// exit
		case KEY_X:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			GB_MenuLeave();
			return False;

		// down
		case KEY_DOWN:
			if (GB_MenuSlot == 0)
				GB_MenuSlot = GB_MENU_SLOTNUM-1;
			else
				GB_MenuSlot--;
			GB_MenuDraw();
			break;

		// up
		case KEY_UP:
			if (GB_MenuSlot == GB_MENU_SLOTNUM-1)
				GB_MenuSlot = 0;
			else
				GB_MenuSlot++;
			GB_MenuDraw();
			break;

		// right
		case KEY_RIGHT:
			if (GB_Save(GB_MenuSlot))
			{
				while (KeyPressed(KEY_RIGHT)) {}
				GB_MenuLeave();
				return True;
			}
			GB_MenuDraw();
			break;

		// left
		case KEY_LEFT:
			if (GB_Load(GB_MenuSlot))
			{
				while (KeyPressed(KEY_LEFT)) {}
				GB_MenuLeave();
				return True;
			}
			GB_MenuDraw();
			break;
		}
	}
}
