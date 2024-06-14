
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

#define GB_MENU_SLOTNUM	10	// number of snapshot slots

// display mode
#define GB_DISPMODE_MSG		0	// display message text window
#define GB_DISPMODE_EMU		1	// display emulated window
extern volatile u8 GB_DispMode;		// display mode GB_DISPMODE_*

extern Bool DoEmuScreenShotReq; // request to do screenshot
extern Bool DoLockedScreenShot; // use locked screenshot - game not moving (press B and add A)

// length of save filename
extern int SaveNameLen;

// snapshot slot
extern int GB_MenuSlot;

// prepare save filename (0..9, -1=default, -2=ROM file)
void GB_PrepSaveFile(int inx);

// unprepare save filename
void GB_UnprepSaveFile();

// enter menu
void GB_MenuEnter();

// leave menu
void GB_MenuLeave();

// display emulator menu (return True to continue emulation)
Bool GB_Menu();
