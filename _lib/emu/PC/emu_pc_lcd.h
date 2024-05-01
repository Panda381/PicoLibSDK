
// ****************************************************************************
//
//                       IBM PC Emulator - output to LCD
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

// current zoom level (0=full screen)
extern u8 PC_LCDZoom;

// vertical sync wait
extern int PC_VSyncWait;

// VSYNC flag
extern volatile Bool PC_VSync;

#if USE_EMU_PC_LCD		// use output PC to LCD display

// max. zoom level by videomode (0=full screen)
extern u8 PC_LCDZoomMax[PC_VMODE_NUM];

// shift zoom level
void PC_LCDShiftZoom();

// redraw display to LCD
void FASTCODE NOFLASH(PC_LCDRedraw)();

#else // USE_EMU_PC_LCD

// shift zoom level
INLINE void PC_LCDShiftZoom() {}

// redraw display to LCD
INLINE void PC_LCDRedraw() {}

#endif // USE_EMU_PC_LCD
