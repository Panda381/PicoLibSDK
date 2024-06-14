
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

#include "../include.h"

#define EMU_LCD_WIDTH	320	// LCD display width
#define EMU_LCD_HEIGHT	240	// LCD display height

// text screen buffer (only characters; 160 bytes)
u8 GB_TextFrame[GB_MSG_WIDTH*GB_MSG_HEIGHT];

// colors of rows
u16 GB_TextColor[GB_MSG_HEIGHT];
u16 GB_TextBgColor; // background color

// text coordinates
u8 GB_TextX, GB_TextY;

// next row color
u16 GB_TextNextCol;

// clear text screen
void GB_TextClear()
{
	memset(GB_TextFrame, ' ', sizeof(GB_TextFrame));
	memset(GB_TextColor, 0xff, sizeof(GB_TextColor));
	GB_TextBgColor = COL_BLACK;
	GB_TextX = 0;
	GB_TextY = 0;
	GB_TextNextCol = COL_WHITE;
}

// set text coordinates
void GB_TextSetPos(int x, int y)
{
	if (x < 0) x = 0;
	if (x > GB_MSG_WIDTH) x = GB_MSG_WIDTH;
	GB_TextX = x;

	if (y < 0) y = 0;
	if (y > GB_MSG_HEIGHT) y = GB_MSG_HEIGHT;
	GB_TextY = y;
}

// next lines
void GB_TextPrintNL(int num)
{
	for (; num > 0; num--) GB_TextSetPos(0, GB_TextY+1);
}

// print character (incl. CR/LF) at current position, with current color (entire row has the same colour)
void GB_TextPrintCh(char ch)
{
	// CR
	if (ch == 13)
	{
		GB_TextX = 0;
	}

	// LF
	else if (ch == 10)
	{
		GB_TextSetPos(GB_TextX, GB_TextY+1);
	}

	// printable character
	else if ((GB_TextX < GB_MSG_WIDTH) && (GB_TextY < GB_MSG_HEIGHT))
	{
		GB_TextFrame[GB_MSG_WIDTH*GB_TextY + GB_TextX] = ch;
		if ((u8)ch > 32) GB_TextColor[GB_TextY] = GB_TextNextCol;
		GB_TextSetPos(GB_TextX+1, GB_TextY);
	}
}

// print HEX character
void GB_TextPrintHexCh(u8 n)
{
	n &= 0x0f;
	n += '0';
	if (n > '9') n += 'A' - ('9' + 1);
	GB_TextPrintCh((char)n);
}

// print HEX byte
void GB_TextPrintHexByte(u8 n)
{
	GB_TextPrintHexCh(n >> 4);
	GB_TextPrintHexCh(n);
}

// print spaces
void GB_TextPrintSpc(int num)
{
	for (; num > 0; num--) GB_TextPrintCh(' ');
}

// print ASCIIZ text (incl. CR/LF) at current position, with current color (entire row has the same colour)
void GB_TextPrint(const char* txt)
{
	char ch;
	while ((ch = *txt++) != 0) GB_TextPrintCh(ch);
}

// print text centred
void GB_TextPrintCenter(const char* txt)
{
	GB_TextX = 0;
	int len = StrLen(txt);
	GB_TextPrintSpc((GB_MSG_WIDTH-len)/2);
	GB_TextPrint(txt);
}

// update text screen on display
void GB_TextUpdate()
{
	u8 ch;
	int line, col, pix;
	u8* s = GB_TextFrame;
	u16* c = GB_TextColor;
	u16 cc, ccc;
	u8* s2;
	const u8* f = FontBold8x16;
	u16 bg = GB_TextBgColor;

	// start sending image data
	DispStartImg(0, EMU_LCD_WIDTH, 0, EMU_LCD_HEIGHT);

	// rows
	for (line = 0; line < EMU_LCD_HEIGHT;)
	{
		cc = *c; // color of the row
		s2 = s; // start of text row

		// columns
		for (col = 0; col < GB_MSG_WIDTH; col++)
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
		if (((line & 1) == 0) || (line >= GB_MSG_BTMLINE))
		{
			// shift to next line of the font
			f += 256;

			// next row after 32 lines
			if (((line & 0x1f) == 0) || (((line & 0x0f) == 0) && (line >= GB_MSG_BTMLINE)))
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
Bool GB_ErrorMsg(const char* txt, const char* txt2, Bool repeat, Bool update)
{
	// clear text screen
	GB_TextClear();

	// title
	GB_TextSetCol(COL_RED);
	GB_TextPrintCenter("ERROR");

	// message
	GB_TextSetCol(COL_WHITE);
	GB_TextPrintNL(2);
	GB_TextPrintCenter(txt);

	// message2
	GB_TextSetCol(COL_WHITE);
	GB_TextPrintNL(1);
	GB_TextPrintCenter(txt2);

	// prompt
	GB_TextSetCol(COL_YELLOW);
	GB_TextPrintNL(2);
	if (repeat)
	{
		GB_TextPrintCenter("Press: Y=repeat,");
		GB_TextPrintNL(1);
	}
	GB_TextPrintCenter("B=ignore, X=exit");

	// set message display mode
	GB_DispMode = GB_DISPMODE_MSG;

	// update (do not use from core 1)
	if (update) GB_TextUpdate();

	// wait for a key
	WaitMs(300);
	GB_KeyFlush();
	while (True)
	{
		u8 key;
		if (update)
			key = KeyGet();
		else
			key = GB_KeyGet();

		// exit
		if (key == KEY_X)
		{
			GBC->cpu.stop = True;
			GB_DispMode = GB_DISPMODE_EMU;
			GB_TextClear();
			GB_KeyFlush();
			return False;
		}

		// ignore
		if (key == KEY_B)
		{
			GB_DispMode = GB_DISPMODE_EMU;
			GB_TextClear();
			GB_KeyFlush();
			return !repeat;
		}

		// repeat
		if (key == KEY_Y)
		{
			if (repeat)
			{
				GB_DispMode = GB_DISPMODE_EMU;
				GB_TextClear();
				GB_KeyFlush();
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
	GB_TextClear();

	// print HEX data (256 bytes, 8 x 8)
	int x, y;
	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 8; x++)
		{
			GB_TextPrintHexByte(GBC->cpu.readmem(addr++));
			if ((x & 3) == 3) GB_TextPrintSpc(1);
		}
		GB_TextPrintNL(1);
	}

	// set message display mode
	GB_DispMode = GB_DISPMODE_MSG;

	// update (do not use from core 1)
	if (update) GB_TextUpdate();

	// wait for a key
	GB_KeyFlush();
	while (True)
	{
		u8 key;
		if (update)
			key = KeyGet();
		else
			key = GB_KeyGet();
		if (key != NOKEY) break;
	}

	GB_DispMode = GB_DISPMODE_EMU;
}
*/
