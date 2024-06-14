
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

#define GB_MENU_SLOTNUM	10	// number of snapshot slots

// snapshot slot
extern int GB_MenuSlot;

// enter menu (stop emulator, save videomode, initialize debug videomode)
//   Returns True if emulator was running.
Bool GB_MenuEnter();

// leave menu (restore videomode, run emulator if required)
void GB_MenuLeave(Bool run);

// display emulator menu (return True to continue emulation)
Bool GB_Menu();
