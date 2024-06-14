
// ****************************************************************************
//
//                          Game Boy Emulator - Message
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

//	SelFont8x16();

#define GB_MSG_WIDTH	(WIDTH/(2*8))	// message text width of window (1 character = 16 pixels width; = 20)
#define GB_MSG_HEIGHT	((HEIGHT+2*16-1)/(2*16)) // message text height of window (1 character = 32 lined height; = 8)

// text screen buffer (only characters; 160 bytes)
extern u8 GB_TextFrame[GB_MSG_WIDTH*GB_MSG_HEIGHT];

// colors of rows
extern u16 GB_TextColor[GB_MSG_HEIGHT];
extern u16 GB_TextBgColor;

// text coordinates
extern u8 GB_TextX, GB_TextY;

// next row color
extern u16 GB_TextNextCol;

// clear text screen
void GB_TextClear();

// set text coordinates
void GB_TextSetPos(int x, int y);

// set text color
INLINE void GB_TextSetCol(u16 col) { GB_TextNextCol = col; }

// next line
void GB_TextPrintNL(int num);

// print character (incl. CR/LF) at current position, with current color (entire row has the same colour)
void GB_TextPrintCh(char ch);

// print spaces
void GB_TextPrintSpc(int num);

// print ASCIIZ text (incl. CR/LF) at current position, with current color (entire row has the same colour)
void GB_TextPrint(const char* txt);

// print text centred
void GB_TextPrintCenter(const char* txt);

// update text screen on display (called from the GB_LCDRedraw function, do not call it directly)
void GB_TextUpdate();

// error message, repeat or exit (return True = repeat (or ignore), False = ignore or exit)
Bool GB_ErrorMsg(const char* txt, const char* txt2, Bool repeat, Bool update);

// debug - display memory
void DB_HexMsg(int addr, Bool update);
