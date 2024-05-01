
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

#define PC_MENU_W	20	// width of menu screen (number of columns)
#define PC_MENU_H	15	// height of menu screen (number of rows)
#define PC_MENU_SLOTNUM	10	// number of snapshot slots

// saved current videomode
extern sPC_Vmode PC_VmodeSave;

// snapshot slot
extern int PC_MenuSlot;

// menu print cursor coordinate
extern int PC_MenuX, PC_MenuY;

// enter menu (stop emulator, save videomode, initialize debug videomode)
//   Returns True if emulator was running.
Bool PC_MenuEnter();

// leave menu (restore videomode, run emulator if required)
void PC_MenuLeave(Bool run);

// clear menu screen
void PC_MenuClr();

// print character to menu screen
void PC_MenuCh(char ch, u8 col);

// print spaces
void PC_MenuSpc(int num, u8 col);

// new lines
void PC_MenuNL(int num);

// print ASCIIZ text to menu screen
void PC_MenuText(const char* txt, u8 col);

// display emulator menu (return True to continue emulation)
Bool PC_Menu();
