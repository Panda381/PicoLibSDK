
// ****************************************************************************
//
//                        Game Boy Emulator - display
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

/*
Display:

0x8000-0x9FFF ... VRAM (GB: 8 KB, GBC: 16 KB with 8 KB bank switching)
  0x8000-0x97FF ... character data (6 KB; 1 byte lower dot data + 1 byte upper dot data;
			GBC: switchable bank 0 and bank 1)
  0x9800-0x9BFF ... display data 1 (1 KB; 1 byte character + 1 byte attributes)
  0x9C00-0x9FFF ... display data 2 (1 KB; 1 byte character + 1 byte attributes)

  Resolution: 160 x 144 pixels
  Block structure: 8 x 8 pixels
  Object: 8 x 8 pixels or 8 x 16 pixels
  Number of usable characters: 256 (GBC: 512)
  Number of usable objects 8 x 8: 256 (GBC: 512)
    or number of usable objects 8 x 16: 128 (GBC: 256)
  Grayscale, window: 4 shades, 1 palette (GBC: 4 colors, 8 palettes)
  Grayscale, object: 3 shades, 2 palettes (GBC: 3 colors, 8 palettes)
  Object priority: smallest X, or lowest OBJ number if same X (GBC: lowest X)

  OBJ character:
    character code: 0..0xFF
    X coordinate: 0..0xFF
    Y coordinate: 0..0xFF

  Character data on 0x8000-0x8800:
    128 characters (0x00..0x7F), every character is 16 bytes (2 KB total)
         One bit of two subsequent bytes specifies the brightness 0..3 of one pixel.
         GBC - bank 1 adds 2 next bits, representing color 0..15
  OBJ code and BG code on 0x8800-0x8FFF:
         0x80-0xFF dot data
  BG code on 0x9000-0x97FF:
         0x00-0x7F dot data


BG display data: 32 x 32 character codes (256 x 256 dots), 0x9800-0x9C00 BG display data
Data for 20 x 18 character codes (160 x 144 dots) are displayed to LCD screen
Screen can be scrolled.


*/

#define GB_NUM_SPRITES	40		// max. number of sprites

// display mode
#define GB_DISPMODE_MSG		0	// display message text window
#define GB_DISPMODE_EMU		1	// display emulated window
extern volatile u8 GB_DispMode;		// display mode GB_DISPMODE_*

// redraw display to LCD
void FASTCODE GB_LCDRedraw();

// render current line
void FASTCODE NOFLASH(GB_RenderLine)();
