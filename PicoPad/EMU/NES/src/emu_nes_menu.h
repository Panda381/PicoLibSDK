
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

#define NES_MENU_SLOTNUM	10	// number of snapshot slots

#define NES_SAVEFILE_VER	'1'	// save file version magic

extern volatile Bool DoEmuScreenShotReq; // request to do screenshot

// length of save filename
extern int SaveNameLen;

// snapshot slot
extern int NES_MenuSlot;

// prepare save filename (0..9, -1=default)
void NES_PrepSaveFile(int inx);

// unprepare save filename
void NES_UnprepSaveFile();

// enter menu
void NES_MenuEnter();

// leave menu
void NES_MenuLeave();

// display emulator menu (return True to continue emulation)
Bool NES_Menu();
