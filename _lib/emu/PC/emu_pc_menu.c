
// ****************************************************************************
//
//                       IBM PC Emulator - Emulator menu
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

#if USE_SCREENSHOT		// use screen shots
void ScreenShot();
#endif

// saved current videomode
sPC_Vmode PC_VmodeSave;
u8 PC_LCDZoomSave;
u16 PC_CurPal16Save[16];

// snapshot slot
int PC_MenuSlot = 0;

// menu print cursor coordinate
int PC_MenuX, PC_MenuY;

// enter menu (stop emulator, save videomode, initialize debug videomode)
//   Returns True if emulator was running.
Bool PC_MenuEnter()
{
	// check if emulator is running
	Bool running = PC_IsRunning();
	if (running) PC_Stop();

	// save current videomode
	memcpy(&PC_VmodeSave, &PC_Vmode, sizeof(sPC_Vmode));
	memcpy(PC_CurPal16Save, PC_CurPal16, sizeof(PC_CurPal16));
	PC_LCDZoomSave = PC_LCDZoom;

	// setup menu videomode (old video mode remains allocated)
	memcpy(PC_CurPal16, PC_Pal16, sizeof(PC_Pal16));
	sPC_Vmode* m = &PC_Vmode;
	m->vmode = PC_VMODE_EMU;	// current videomode
	m->vclass = PC_VCLASS_TEXT;	// text class
	m->cols = 40;			// number of text columns
	m->rows = 15;			// number of text rows
	m->pages = 1;			// number of pages
	m->page = 0;			// current pag
	m->pagemask = 0;		// mask of page number
	m->blink = False;		// blinking
	m->fonth = 8;			// font height
	m->font = PC_Font8;		// font
	m->curstart = 6;		// cursor start line
	m->curend = 7;			// cursor end line
	m->segm = PC_EMUL_VRAM_SEG;	// base segment address
	m->planesize = 1024;		// plane size
	m->off = 0;			// offset in video-RAM
	m->mask = 0x3ff;		// mask in video-RAM
	m->wb = 40*2;			// width of row
	m->pal = (u16*)PC_Pal16;	// palettes (using u16* is ok - we will not modify it)
	m->base = PC_EMUL_VRAM;		// pointer to current video-RAM base address
	m->cur[0].curx = 0;
	m->cur[0].cury = 15;		// invisible cursor
	PC_LCDZoom = 0;			// no zoom

	// clear menu screen
	PC_MenuClr();

	return running;
}

// leave menu (restore videomode, run emulator if required)
void PC_MenuLeave(Bool run)
{
	// restore videomode (old video mode remained allocated)
	memcpy(&PC_Vmode, &PC_VmodeSave, sizeof(sPC_Vmode));
	memcpy(PC_CurPal16, PC_CurPal16Save, sizeof(PC_CurPal16));
	PC_LCDZoom = PC_LCDZoomSave;

	// continue emulator
	if (run)
	{
		PC_LCDRedraw();
		PC_Cont();
	}
	else
		// stop emulator
		PC_Stop();
}

// clear menu screen
void PC_MenuClr()
{
	memset(PC_EMUL_VRAM, 0, PC_MENU_W*PC_MENU_H*2);
	PC_MenuX = 0;
	PC_MenuY = 0;
}

// print character to menu screen
void PC_MenuCh(char ch, u8 col)
{
	if ((PC_MenuX < PC_MENU_W) && (PC_MenuY < PC_MENU_H))
	{
		u8* d = PC_EMUL_VRAM + 2*(PC_MenuX + PC_MenuY*PC_MENU_W);
		d[0] = ch;
		d[1] = col;
	}
	PC_MenuX++;
}

// print spaces
void PC_MenuSpc(int num, u8 col)
{
	for (; num > 0; num--) PC_MenuCh(' ', col);
}

// new lines
void PC_MenuNL(int num)
{
	PC_MenuX = 0;
	PC_MenuY += num;
}

// print ASCIIZ text to menu screen
void PC_MenuText(const char* txt, u8 col)
{
	char ch;
	for (;;)
	{
		ch = *txt++;
		if (ch == 0) break; // end of text
		if (ch == CH_LF)
			PC_MenuNL(1); // new line
		else
			PC_MenuCh(ch, col); // print character
	}
}

// colors
#define PC_MENU_COL_KEY		PC_COLOR(PC_BLACK, PC_YELLOW)	// menu key
#define PC_MENU_COL_MENU	PC_COLOR(PC_BLACK, PC_WHITE)	// menu label
#define PC_MENU_COL_INFO	PC_COLOR(PC_BLACK, PC_LTGRAY)	// info text

// draw emulator menu
void PC_MenuDraw()
{
	PC_MenuClr();
	
	PC_MenuText("A", PC_MENU_COL_KEY);
	PC_MenuText(" .... screenshot\n", PC_MENU_COL_MENU);

	PC_MenuText("B", PC_MENU_COL_KEY);
	PC_MenuText(" .... sound: ", PC_MENU_COL_MENU);
	PC_MenuText(GlobalSoundOff ? "off" : "ON", PC_MENU_COL_MENU);
	PC_MenuNL(1);

	PC_MenuText("X", PC_MENU_COL_KEY);
	PC_MenuText(" .... exit\n", PC_MENU_COL_MENU);

	PC_MenuText("Y", PC_MENU_COL_KEY);
	PC_MenuText(" .... continue\n", PC_MENU_COL_MENU);

	PC_MenuNL(1);

//	PC_MenuText("Le/Ri", PC_MENU_COL_KEY);
//	PC_MenuText("  slot: ", PC_MENU_COL_MENU);
//	PC_MenuCh(PC_MenuSlot+'0', PC_MENU_COL_MENU);
	PC_MenuNL(1);

//	PC_MenuText("Up", PC_MENU_COL_KEY);
//	PC_MenuText(" ... load shot\n", PC_MENU_COL_MENU);

//	PC_MenuText("Dn", PC_MENU_COL_KEY);
//	PC_MenuText(" ... save shot\n", PC_MENU_COL_MENU);

	PC_MenuNL(1);

	int f = (PC_VmemPageRAM - PC_NextPageRAM) << (PC_PAGE_SHIFT - 10); // free RAM in KB
	DecNum(DecNumBuf, f, 0);
	PC_MenuText("Free:", PC_MENU_COL_INFO);
	PC_MenuText(DecNumBuf, PC_MENU_COL_INFO);
	PC_MenuText("KB", PC_MENU_COL_INFO);

	PC_MenuText(" Video:", PC_MENU_COL_INFO);
	DecNum(DecNumBuf, PC_VmodeSave.vmode, 0);
	PC_MenuText(DecNumBuf, PC_MENU_COL_INFO);

	PC_MenuNL(1);

	PC_MenuText("CS:IP=0x", PC_MENU_COL_INFO);
	DecHexNum(DecNumBuf, PC_Cpu->cs, 4);
	PC_MenuText(DecNumBuf, PC_MENU_COL_INFO);
	PC_MenuText(":0x", PC_MENU_COL_INFO);
	DecHexNum(DecNumBuf, PC_Cpu->ip, 4);
	PC_MenuText(DecNumBuf, PC_MENU_COL_INFO);
	PC_MenuNL(1);

	PC_MenuText("Freq:", PC_MENU_COL_INFO);
	DecNum(DecNumBuf, (PC_RealFreq+500)/1000, 0);
	PC_MenuText(DecNumBuf, PC_MENU_COL_INFO);
	PC_MenuText("kHz\n", PC_MENU_COL_INFO);

	PC_MenuText("SysClock:", PC_MENU_COL_INFO);
	DecNum(DecNumBuf, (ClockGetHz(CLK_SYS)+500)/1000, 0);
	PC_MenuText(DecNumBuf, PC_MENU_COL_INFO);
	PC_MenuText("kHz\n", PC_MENU_COL_INFO);
}

// emulator menu (return True to continue emulation)
Bool PC_Menu()
{
	// enter menu
	PC_MenuEnter();
	
	// draw emulator menu
	PC_MenuDraw();

	while (True)
	{
		PC_LCDRedraw();

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
			DoEmuScreenShot = True;	// request to do emulator screenshot
#endif
			PC_MenuLeave(True);
			return True;

		// sound
		case HID_KEY_B:
			GlobalSoundOff = !GlobalSoundOff;
			PC_MenuDraw();
			break;

		// continue
		case HID_KEY_ESCAPE:
		case HID_KEY_PAUSE:
		case HID_KEY_Y:
			PC_MenuLeave(True);
			return True;

		// exit
		case HID_KEY_RETURN:
		case HID_KEY_X:
			// pause on exit
			PC_PauseExit = False;
			PC_MenuLeave(False);
			return False;

		// Left
		case HID_KEY_ARROW_LEFT:
			if (PC_MenuSlot == 0)
				PC_MenuSlot = PC_MENU_SLOTNUM-1;
			else
				PC_MenuSlot--;
			PC_MenuDraw();
			break;

		// Right
		case HID_KEY_ARROW_RIGHT:
			if (PC_MenuSlot == PC_MENU_SLOTNUM-1)
				PC_MenuSlot =0;
			else
				PC_MenuSlot++;
			PC_MenuDraw();
			break;
		}
#endif
		
		// keypad
		switch (KeyGet())
		{
		// screenshot
		case KEY_A:
#if USE_EMUSCREENSHOT		// use emulator screen shots
			DoEmuScreenShot = True;	// request to do emulator screenshot
#endif
			PC_MenuLeave(True);
			return True;

		// sound
		case KEY_B:
			GlobalSoundOff = !GlobalSoundOff;
			PC_MenuDraw();
			break;

		// continue
		case KEY_Y:
			PC_MenuLeave(True);
			return True;

		// exit
		case KEY_X:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			// pause on exit
			PC_PauseExit = False;
			PC_MenuLeave(False);
			return False;

		// Left
		case KEY_LEFT:
			if (PC_MenuSlot == 0)
				PC_MenuSlot = PC_MENU_SLOTNUM-1;
			else
				PC_MenuSlot--;
			PC_MenuDraw();
			break;

		// Right
		case KEY_RIGHT:
			if (PC_MenuSlot == PC_MENU_SLOTNUM-1)
				PC_MenuSlot =0;
			else
				PC_MenuSlot++;
			PC_MenuDraw();
			break;
		}
	}
}
