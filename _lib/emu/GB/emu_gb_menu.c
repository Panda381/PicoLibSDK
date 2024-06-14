
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

// >>> Keep tables and functions in RAM (without "const") for faster access.

// snapshot slot
int GB_MenuSlot = 0;

// enter menu (stop emulator, save videomode, initialize debug videomode)
//   Returns True if emulator was running.
Bool GB_MenuEnter()
{
	// check if emulator is running
	Bool running = GB_IsRunning();
	if (running) GB_Stop();

	// clear text screen
	GB_TextClear();

	// set message display mode
	GB_DispMode = GB_DISPMODE_MSG;

	return running;
}

// leave menu (restore videomode, run emulator if required)
void GB_MenuLeave(Bool run)
{
	// clear text screen
	GB_TextClear();

	// set emlator display mode
	GB_DispMode = GB_DISPMODE_EMU;

	// continue emulator
	if (run)
		GB_Cont();
	else
		// stop emulator
		GB_Stop();
}

// draw menu
void GB_MenuDraw()
{
	// clear text screen
	GB_TextClear();

	// set menu color
	GB_TextSetCol(COL_YELLOW);

	// 1) A .. screenshot
	GB_TextPrint("A ... screenshot\n\r");

	// 2) B ... sound off
	GB_TextPrint("B ... sound ");
	GB_TextPrint(GlobalSoundOff ? "off" : "ON");

	// 3) X ... exit
	GB_TextPrint("\n\rX ... exit\n\r");

	// 4) Y ... continue
	GB_TextPrint("Y ... continue\n\r");

	// set slots color
	GB_TextSetCol(COL_GREEN);

	// 5) Le/Ri slot: 0
	GB_TextPrint("Le/Ri slot: ");
	GB_TextPrintCh(GB_MenuSlot+'0');

	// 6) Up/Dn load/save
	GB_TextPrint("\n\rUp/Dn load/save");

	// set info color
	GB_TextSetCol(COL_LTGRAY);

	// 7) IP=1234 f=4194304Hz
	GB_TextPrint("\n\rPC=");
	DecHexNum(DecNumBuf, GBC->cpu.pc, 4);
	GB_TextPrint(DecNumBuf);
	GB_TextPrint(" f=");
	DecNum(DecNumBuf, GB_RealFreq, 0);
	GB_TextPrint(DecNumBuf);
	GB_TextPrint("Hz");

	// 8) MBC=1 sys=180000kHz
	GB_TextPrint("\n\rMBC=");

	GB_TextPrintCh(GBC->mbc + '0');
//	DecNum(DecNumBuf, GBC->hram[GB_IO_LCDC] /*GBC->cpu.readmem(GB_IO_LY+0xff00)*/ /*readmem(0xff44)*/, 0);	GB_TextPrint(DecNumBuf);

	GB_TextPrint(" sys=");
	DecNum(DecNumBuf, (ClockGetHz(CLK_SYS)+500)/1000, 0);
	GB_TextPrint(DecNumBuf);
	GB_TextPrint("kHz");

	// update text screen on display
	GB_TextUpdate();
}

// display emulator menu (return True to continue emulation)
Bool GB_Menu()
{
	// enter menu
	GB_MenuEnter();
	
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
			DoEmuScreenShot = True;	// request to do emulator screenshot
#endif
			GB_MenuLeave(True);
			return True;

		// sound
		case HID_KEY_B:
			GlobalSoundOff = !GlobalSoundOff;
			GB_MenuDraw();
			break;

		// continue
		case HID_KEY_ESCAPE:
		case HID_KEY_PAUSE:
		case HID_KEY_Y:
			GB_MenuLeave(True);
			return True;

		// exit
		case HID_KEY_RETURN:
		case HID_KEY_X:
			GB_MenuLeave(False);
			return False;

		// Left
		case HID_KEY_ARROW_LEFT:
			if (GB_MenuSlot == 0)
				GB_MenuSlot = GB_MENU_SLOTNUM-1;
			else
				GB_MenuSlot--;
			GB_MenuDraw();
			break;

		// Right
		case HID_KEY_ARROW_RIGHT:
			if (GB_MenuSlot == GB_MENU_SLOTNUM-1)
				GB_MenuSlot = 0;
			else
				GB_MenuSlot++;
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
			DoEmuScreenShot = True;	// request to do emulator screenshot
#endif
			GB_MenuLeave(True);
			return True;

		// sound
		case KEY_B:
			GlobalSoundOff = !GlobalSoundOff;
			GB_MenuDraw();
			break;

		// continue
		case KEY_Y:
			GB_MenuLeave(True);
			return True;

		// exit
		case KEY_X:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			GB_MenuLeave(False);
			return False;

		// Left
		case KEY_LEFT:
			if (GB_MenuSlot == 0)
				GB_MenuSlot = GB_MENU_SLOTNUM-1;
			else
				GB_MenuSlot--;
			GB_MenuDraw();
			break;

		// Right
		case KEY_RIGHT:
			if (GB_MenuSlot == GB_MENU_SLOTNUM-1)
				GB_MenuSlot = 0;
			else
				GB_MenuSlot++;
			GB_MenuDraw();
			break;
		}
	}
}
