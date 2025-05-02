
// ****************************************************************************
//
//                              NES Emulator - Message
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

#define EMU_LCD_WIDTH	WIDTH	// LCD display width
#define EMU_LCD_HEIGHT	HEIGHT	// LCD display height

// text screen buffer (only characters; 160 bytes)
u8 NES_TextFrame[NES_MSG_WIDTH*NES_MSG_HEIGHT];

// colors of rows
u16 NES_TextColor[NES_MSG_HEIGHT];
u16 NES_TextBgColor; // background color

// text coordinates
u8 NES_TextX, NES_TextY;

// next row color
u16 NES_TextNextCol;

// clear text screen
void NES_TextClear()
{
	memset(NES_TextFrame, ' ', sizeof(NES_TextFrame));
	memset(NES_TextColor, 0xff, sizeof(NES_TextColor));
	NES_TextBgColor = COL_BLACK;
	NES_TextX = 0;
	NES_TextY = 0;
	NES_TextNextCol = COL_WHITE;
}

// set text coordinates
void NES_TextSetPos(int x, int y)
{
	if (x < 0) x = 0;
	if (x > NES_MSG_WIDTH) x = NES_MSG_WIDTH;
	NES_TextX = x;

	if (y < 0) y = 0;
	if (y > NES_MSG_HEIGHT) y = NES_MSG_HEIGHT;
	NES_TextY = y;
}

// next lines
void NES_TextPrintNL(int num)
{
	for (; num > 0; num--) NES_TextSetPos(0, NES_TextY+1);
}

// print character (incl. CR/LF) at current position, with current color (entire row has the same colour)
void NES_TextPrintCh(char ch)
{
	// CR
	if (ch == 13)
	{
		NES_TextX = 0;
	}

	// LF
	else if (ch == 10)
	{
		NES_TextSetPos(NES_TextX, NES_TextY+1);
	}

	// printable character
	else if ((NES_TextX < NES_MSG_WIDTH) && (NES_TextY < NES_MSG_HEIGHT))
	{
		NES_TextFrame[NES_MSG_WIDTH*NES_TextY + NES_TextX] = ch;
		if ((u8)ch > 32) NES_TextColor[NES_TextY] = NES_TextNextCol;
		NES_TextSetPos(NES_TextX+1, NES_TextY);
	}
}

// print HEX character
void NES_TextPrintHexCh(u8 n)
{
	n &= 0x0f;
	n += '0';
	if (n > '9') n += 'A' - ('9' + 1);
	NES_TextPrintCh((char)n);
}

// print HEX byte
void NES_TextPrintHexByte(u8 n)
{
	NES_TextPrintHexCh(n >> 4);
	NES_TextPrintHexCh(n);
}

// print spaces
void NES_TextPrintSpc(int num)
{
	for (; num > 0; num--) NES_TextPrintCh(' ');
}

// print ASCIIZ text (incl. CR/LF) at current position, with current color (entire row has the same colour)
void NES_TextPrint(const char* txt)
{
	char ch;
	while ((ch = *txt++) != 0) NES_TextPrintCh(ch);
}

// print text centred
void NES_TextPrintCenter(const char* txt)
{
	NES_TextX = 0;
	int len = StrLen(txt);
	NES_TextPrintSpc((NES_MSG_WIDTH-len)/2);
	NES_TextPrint(txt);
}

// update text screen on display
void NES_TextUpdate()
{
	u8 ch;
	int line, col, pix;
	u8* s = NES_TextFrame;
	u16* c = NES_TextColor;
	u16 cc, ccc;
	u8* s2;
	const u8* f = FontBold8x16;
	u16 bg = NES_TextBgColor;

	// start sending image data
	DispStartImg(0, EMU_LCD_WIDTH, 0, EMU_LCD_HEIGHT);

	// rows
	for (line = 0; line < EMU_LCD_HEIGHT;)
	{
		cc = *c; // color of the row
		s2 = s; // start of text row

		// columns
		for (col = 0; col < NES_MSG_WIDTH; col++)
		{
			// get character
			ch = *s2++;

			// get font pixels
			ch = f[ch];

			// draw pixels
			for (pix = 8; pix > 0; pix--)
			{
				// send color of the pixel (or black color of the background)
				ccc = ((ch & 0x80) != 0) ? cc : bg;
				DispSendImg2(ccc);
				DispSendImg2(ccc);
				ch <<= 1;
			}
		}

		// increase line
		line++;

		// odd line - do nothing (repeat)
		if (((line & 1) == 0) || (line >= NES_MSG_BTMLINE))
		{
			// shift to next line of the font
			f += 256;

			// next row after 32 lines
			if (((line & 0x1f) == 0) || (((line & 0x0f) == 0) && (line >= NES_MSG_BTMLINE)))
			{
				c++; // color of the row
				s = s2; // start of text row
				f = FontBold8x16; // font
			}
		}
	}

	// stop sending data
	DispStopImg();
}

/*
// error message, repeat or exit (return True = repeat (or ignore), False = ignore or exit)
Bool NES_ErrorMsg(const char* txt, const char* txt2, Bool repeat, Bool update)
{
	// clear text screen
	NES_TextClear();

	// title
	NES_TextSetCol(COL_RED);
	NES_TextPrintCenter("ERROR");

	// message
	NES_TextSetCol(COL_WHITE);
	NES_TextPrintNL(2);
	NES_TextPrintCenter(txt);

	// message2
	NES_TextSetCol(COL_WHITE);
	NES_TextPrintNL(1);
	NES_TextPrintCenter(txt2);

	// prompt
	NES_TextSetCol(COL_YELLOW);
	NES_TextPrintNL(2);
	if (repeat)
	{
		NES_TextPrintCenter("Press: Y=repeat,");
		NES_TextPrintNL(1);
	}
	NES_TextPrintCenter("B=ignore, X=exit");

	// set message display mode
	NES_DispMode = NES_DISPMODE_MSG;

	// update (do not use from core 1)
	if (update) NES_TextUpdate();

	// wait for a key
	WaitMs(300);
	NES_KeyFlush();
	while (True)
	{
		u8 key;
		if (update)
			key = KeyGet();
		else
			key = NES_KeyGet();

		// exit
		if (key == KEY_X)
		{
			GBC->cpu.stop = True;
			NES_DispMode = NES_DISPMODE_EMU;
			NES_TextClear();
			NES_KeyFlush();
			return False;
		}

		// ignore
		if (key == KEY_B)
		{
			NES_DispMode = NES_DISPMODE_EMU;
			NES_TextClear();
			NES_KeyFlush();
			return !repeat;
		}

		// repeat
		if (key == KEY_Y)
		{
			if (repeat)
			{
				NES_DispMode = NES_DISPMODE_EMU;
				NES_TextClear();
				NES_KeyFlush();
				return True;
			}
		}
	}
}
*/

/*
// debug - display memory
void DB_HexMsg(int addr, Bool update)
{
	// clear text screen
	NES_TextClear();

	// print HEX data (256 bytes, 8 x 8)
	int x, y;
	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 8; x++)
		{
			NES_TextPrintHexByte(GBC->cpu.readmem(addr++));
			if ((x & 3) == 3) NES_TextPrintSpc(1);
		}
		NES_TextPrintNL(1);
	}

	// set message display mode
	NES_DispMode = NES_DISPMODE_MSG;

	// update (do not use from core 1)
	if (update) NES_TextUpdate();

	// wait for a key
	NES_KeyFlush();
	while (True)
	{
		u8 key;
		if (update)
			key = KeyGet();
		else
			key = NES_KeyGet();
		if (key != NOKEY) break;
	}

	NES_DispMode = NES_DISPMODE_EMU;
}
*/
