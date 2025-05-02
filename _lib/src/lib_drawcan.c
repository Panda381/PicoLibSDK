
// ****************************************************************************
//
//                             Drawing to canvas
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

// Note: The following 2 switches are not yet defined in the PicoLibSDK at this point, so the global.h file is included.
#if USE_DISPHSTX && DISPHSTX_PICOSDK	// 0=use PicoLibSDK library, 1=use PicoSDK original Raspberry SDK library
#include "disphstx_picolibsk.h"
#else
#include "../../global.h"
#endif

#if USE_DRAWCAN		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

#include "../../_font/_include.h"
#include "../../_sdk/inc/sdk_dma.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../inc/lib_rand.h"
#include "../inc/lib_text.h"
#include "../inc/lib_drawcan.h"
#include "../inc/lib_drawcan1.h"
#include "../inc/lib_drawcan2.h"
#include "../inc/lib_drawcan3.h"
#include "../inc/lib_drawcan4.h"
#include "../inc/lib_drawcan6.h"
#include "../inc/lib_drawcan8.h"
#include "../inc/lib_drawcan12.h"
#include "../inc/lib_drawcan16.h"
#include "../inc/lib_print.h"

#ifndef WIDTH
#define WIDTH		320		// display width
#endif

#ifndef HEIGHT
#define HEIGHT		240		// display height
#endif

// default drawing canvas
sDrawCan DrawCan = {
	// format
#if USE_DRAWCAN16		// 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
	.format = DRAWCAN_FORMAT_16,	// u8	format;		// canvas format CANVAS_*
#else
	.format = DRAWCAN_FORMAT_NONE,	// u8	format;		// canvas format CANVAS_*
#endif

	.colbit = 16,			// u8	colbit;		// number of bits per pixel
	.strip = 0,			// u8	strip;		// current strip index
	.stripnum = 1,			// u8	stripnum;	// number of strips

	// font
	.fontw = FONTW,			// u8	fontw;		// font width (number of pixels, max. 8)
	.fonth = FONTH,			// u8	fonth;		// font height (number of lines)
	.printinv = 0,			// u8	printinv;	// offset added to printed character (print characters 128 = inverted, 0 = normal)
	.printsize = 0,			// u8	printsize;	// font size 0=normal, 1=double-height, 2=double-width, 3=double-size

	// dimension
	.w = WIDTH,			// s16	w;		// width
	.h = HEIGHT,			// s16	h;		// height
	.wb = (WIDTH*2+3)&~3,		// s16	wb;		// pitch (bytes between lines) - aligned to 4 bytes (u32)
	.striph = HEIGHT,		// s16	striph;		// strip height (number of lines)

	// print
	.printposnum = WIDTH/FONTW,	// u16	printposnum;	// number of text positions per row (= w / fontw)
	.printrownum = HEIGHT/FONTH,	// u16	printrownum;	// number of text rows (= h / fonth)
	.printpos = 0,			// u16	printpos;	// console print character position
	.printrow = 0,			// u16	printrow;	// console print character row
	.printcol = COL16_WHITE,	// u16	printcol;	// console print color

	// clipping
	.basey = 0,			// s16	basey;		// base Y of buffer strip
	.clipx1 = 0,			// s16	clipx1;		// clip X min
	.clipx2 = WIDTH,		// s16	clipx2;		// clip X+1 max
	.clipy1 = 0,			// s16	clipy1;		// clip Y min (current, limited by buffer strip)
	.clipy2 = HEIGHT,		// s16	clipy2;		// clip Y+1 max (current, limited by buffer strip)
	.clipy1_orig = 0,		// s16	clipy1_orig;	// clip Y min (original, without strip clipping)
	.clipy2_orig = HEIGHT,		// s16	clipy2_orig;	// clip Y+1 max (original, without strip clipping)

	// dirty window
	.dirtyx1 = 0,			// s16	dirtyx1;	// dirty window start X
	.dirtyx2 = WIDTH,		// s16	dirtyx2;	// dirty window end X
	.dirtyy1 = 0,			// s16	dirtyy1;	// dirty window start Y
	.dirtyy2 = HEIGHT,		// s16	dirtyy2;	// dirty window end Y

	// data buffer
	.buf = NULL,			// u8*	buf;		// image data buffer (back buffer of front buffer)
	.frontbuf = NULL,		// u8*	fronbuf;	// front buffer, or NULL = use only one buffer

	// last system time of auto update
	.autoupdatelast = 0,		// u32	autoupdatelast;	// last system time of auto update

	// font
	.font = FONT,			// const u8* font;	// pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)

	// drawing functions interfaces
	// - Don't set the pointer to the function table, it would increase the size of the Loader
	.drawfnc = NULL, //&DrawCan16Fnc,	// const struct sDrawCanFnc_* drawfnc; // drawing functions
};

// current global drawing canvas
sDrawCan* pDrawCan = &DrawCan;

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

// set default drawing canvas
void SetDrawCan(sDrawCan* can) { pDrawCan = can; }

// drawing function interfaces
const sDrawCanFnc* DrawCanFncList[DRAWCAN_FORMAT_NUM] = {
	NULL,		// DRAWCAN_FORMAT_NONE = 0,	// invalid format

#if USE_DRAWCAN1		// 1=use DrawCan1 1-bit functions, if use drawing canvas
	&DrawCan1Fnc,	// DRAWCAN_FORMAT_1,		// 1-bit per pixel Y1 (8 pixels per byte, MSB first pixel, LSB last pixel)
							//	[7654 3210]
#endif

#if USE_DRAWCAN2		// 1=use DrawCan2 2-bit functions, if use drawing canvas
	&DrawCan2Fnc,	// DRAWCAN_FORMAT_2,		// 2-bits per pixel Y2 (4 pixels per byte, MSB first pixel, LSB last pixel)
							//	[3322 1100]
#endif

#if USE_DRAWCAN3		// 1=use DrawCan3 3-bit functions, if use drawing canvas
	&DrawCan3Fnc,	// DRAWCAN_FORMAT_3,		// 3-bits per pixel at format RGB111 (10 pixels per 32-bit word, LSB fist pixel, MSB last pixel)
							//	[xx99 9888 7776 6655 5444 3332 2211 1000]
#endif

#if USE_DRAWCAN4		// 1=use DrawCan4 4-bit functions, if use drawing canvas
	&DrawCan4Fnc,	// DRAWCAN_FORMAT_4,		// 4-bits per pixel at format YRGB1111 (2 pixels per byte, MSB first pixel, LSB last pixel)
							//	[1111 0000]
#endif

#if USE_DRAWCAN6		// 1=use DrawCan6 6-bit functions, if use drawing canvas
	&DrawCan6Fnc,	// DRAWCAN_FORMAT_6,		// 6-bits per pixel at format RGB222 (5 pixels per 32-bit word, LSB fist pixel, MSB last pixel)
							//	[xx44 4444 3333 3322 2222 1111 1100 0000]
#endif

#if USE_DRAWCAN8		// 1=use DrawCan8 8-bit functions, if use drawing canvas
	&DrawCan8Fnc,	// DRAWCAN_FORMAT_8,		// 8-bits per pixel at format RGB332 (1 pixel per byte)
							//	[RRRGGGBB]
#endif

#if USE_DRAWCAN12		// 1=use DrawCan12 12-bit functions, if use drawing canvas
	&DrawCan12Fnc,	// DRAWCAN_FORMAT_12,		// 12-bits per pixel at format RGB222 (8 pixels per three 32-bit words, or 2 pixels per 3 bytes, LSB fist pixel, MSB last pixel)
							//	[2222 2222 1111 1111 1111 0000 0000 0000]
							//	[5555 4444 4444 4444 3333 3333 3333 2222]
							//	[7777 7777 7777 6666 6666 6666 5555 5555]
#endif

#if USE_DRAWCAN16		// 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
	&DrawCan15Fnc,	// DRAWCAN_FORMAT_15,		// 15-bits per pixel at format RGB555 (1 pixel per 2 bytes, bit 15 is ignored)
							//	[.RRRRRGGGGGBBBBB]
	&DrawCan16Fnc,	// DRAWCAN_FORMAT_16,		// 16-bits per pixel at format RGB565 (1 pixel per 2 bytes)
							//	[RRRRRGGGGGGBBBBB]
#endif
};

#endif // USE_DRAWCAN0

// set dirty all clipping area
void DrawCanDirtyAll(sDrawCan* can)
{
	can->dirtyx1 = can->clipx1;
	can->dirtyx2 = can->clipx2;
	can->dirtyy1 = can->clipy1;
	can->dirtyy2 = can->clipy2;
}
void DispDirtyAll() { DrawCanDirtyAll(pDrawCan); }

// set dirty none
void DrawCanDirtyNone(sDrawCan* can)
{
	can->dirtyx1 = can->clipx2;
	can->dirtyx2 = can->clipx1;
	can->dirtyy1 = can->clipy2;
	can->dirtyy2 = can->clipy1;
}

// update dirty area by point (does not check clipping limits)
void DrawCanDirtyPoint_Fast(sDrawCan* can, int x, int y)
{
	if (x < can->dirtyx1) can->dirtyx1 = x;
	if (x >= can->dirtyx2) can->dirtyx2 = x+1;
	if (y < can->dirtyy1) can->dirtyy1 = y;
	if (y >= can->dirtyy2) can->dirtyy2 = y+1;
}

// update dirty area by point (checks clipping limits)
void DrawCanDirtyPoint(sDrawCan* can, int x, int y)
{
	if ((x >= can->clipx1) && (x < can->clipx2) &&
		(y >= can->clipy1) && (y < can->clipy2))
	{
		if (x < can->dirtyx1) can->dirtyx1 = x;
		if (x >= can->dirtyx2) can->dirtyx2 = x+1;
		if (y < can->dirtyy1) can->dirtyy1 = y;
		if (y >= can->dirtyy2) can->dirtyy2 = y+1;
	}
}

// update dirty area by rectangle (does not check clipping limits)
void DrawCanDirtyRect_Fast(sDrawCan* can, int x, int y, int w, int h)
{
	// update dirty area in X direction
	if (x < can->dirtyx1) can->dirtyx1 = x;
	x += w; if (x > can->dirtyx2) can->dirtyx2 = x;

	// update dirty area in Y direction
	if (y < can->dirtyy1) can->dirtyy1 = y;
	y += h; if (y > can->dirtyy2) can->dirtyy2 = y;
}

// update dirty area by rectangle (checks clipping limits)
void DrawCanDirtyRect(sDrawCan* can, int x, int y, int w, int h)
{
	// flip rectangle
	if (w < 0)
	{
		x += w;
		w = -w;
	}

	if (h < 0)
	{
		y += h;
		h = -h;
	}

	// limit X coordinate and width
	int d = can->clipx1 - x;
	if (d > 0)
	{
		w -= d;
		x += d;
	}
	d = can->clipx2;
	if (x + w > d) w = d - x;
	if (w <= 0) return;

	// limit Y coordinate and height
	d = can->clipy1 - y;
	if (d > 0)
	{
		h -= d;
		y += d;
	}
	d = can->clipy2;
	if (y + h > d) h = d - y;
	if (h <= 0) return;

	// update dirty area in X direction
	if (x < can->dirtyx1) can->dirtyx1 = x;
	x += w; if (x > can->dirtyx2) can->dirtyx2 = x;

	// update dirty area in Y direction
	if (y < can->dirtyy1) can->dirtyy1 = y;
	y += h; if (y > can->dirtyy2) can->dirtyy2 = y;
}

// check if rectangle is clipped and if it is safe to use fast variant of the function
Bool DrawCanRectClipped(sDrawCan* can, int x, int y, int w, int h)
{
	return (x >= can->clipx1) && (y >= can->clipy1) && (w > 0) &&
		(x+w <= can->clipx2) && (h > 0) && (y+h <= can->clipy2);
}

// Display update - transfer image from back buffer to front buffer
// @TODO: So far there is only a simplified update where the whole buffer (without clipping) is transferred using DMA.
void DrawCanUpdateAll(sDrawCan* can)
{
	// check if need update
	if (can->frontbuf == NULL) return;

	// copy buffer data (speed 2 us per 1 KB; update 320x240/16 = 150 KB = 300 us)
#if DISPHSTX_PICOSDK	// 1=use Pico SDK
	memcpy(can->frontbuf, can->buf, can->wb * can->h);
#else
	DMA_MemCopy(can->frontbuf, can->buf, can->wb * can->h);
#endif

	// dirty none
	DrawCanDirtyNone(can);
}

void DispUpdateAll() { DrawCanUpdateAll(pDrawCan); }

// update screen, if dirty
void DrawCanUpdate(sDrawCan* can)
{
	if ((can->dirtyx1 >= can->dirtyx2) || (can->dirtyy1 >= can->dirtyy2)) return;
	DrawCanUpdateAll(can);
}
void DispUpdate() { DrawCanUpdate(pDrawCan); }

// auto update after delta time in [ms] of running program
void DrawCanAutoUpdate(sDrawCan* can, u32 ms)
{
	// interval in [us]
	u32 us = ms*1000;

	// check interval from last update
	u32 t = Time();
	if ((u32)(t - can->autoupdatelast) >= us)
	{
		// update display
		DrawCanUpdate(can);

		// start measure new time interval of running program
		can->autoupdatelast = t;
	}
}
void DispAutoUpdate(u32 ms) { DrawCanAutoUpdate(pDrawCan, ms); }

// scroll screen one row up
void DrawCanScroll(sDrawCan* can)
{
	// drawing buffer
	u8* buf = can->buf;

	// size of one row (in bytes)
	int rowsize = can->wb * can->fonth;

	// move 1 row up
#if DISPHSTX_PICOSDK	// 1=use Pico SDK
	memmove(can->buf, can->buf + rowsize, can->wb * can->h - rowsize);
#else
	DMA_MemCopy(can->buf, can->buf + rowsize, can->wb * can->h - rowsize);
#endif

	// clear last row
	can->drawfnc->pDrawCanRect_Fast(can, 0, can->h - can->fonth, can->w, can->fonth, pDrawCan->drawfnc->col_black);

	// update all screen
	DrawCanUpdateAll(can);
}

void DrawScroll() { DrawCanScroll(pDrawCan); }

// console print character (without display update)
//   Control characters:
//     0x01 '\1' ^A ... set not-inverted text
//     0x02 '\2' ^B ... set inverted text (shift character code by 0x80)
//     0x03 '\3' ^C ... use normal-sized font (default)
//     0x04 '\4' ^D ... use double-height font
//     0x05 '\5' ^E ... use double-width font
//     0x06 '\6' ^F ... use double-sized font
//     0x07 '\a' ^G ... (bell) move cursor 1 position right (no print; uses width of normal-sized font)
//     0x08 '\b' ^H ... (back space) move cursor 1 position left (no print; uses width of normal-sized font)
//     0x09 '\t' ^I ... (tabulator) move cursor to next 8-character position, print spaces (uses width of normal-sized font)
//     0x0A '\n' ^J ... (new line) move cursor to start of next row
//     0x0B '\v' ^K ... (vertical tabulator) move cursor to start of previous row
//     0x0C '\f' ^L ... (form feed) clear screen, reset cursor position and set default color
//     0x0D '\r' ^M ... (carriage return) move cursor to start of current row
//     0x10 '\20' ^P ... set gray text color (COL_GRAY, default)
//     0x11 '\21' ^Q ... set blue text color (COL_AZURE)
//     0x12 '\22' ^R ... set green text color (COL_GREEN)
//     0x13 '\23' ^S ... set cyan text color (COL_CYAN)
//     0x14 '\24' ^T ... set red text color (COL_RED)
//     0x15 '\25' ^U ... set magenta text color (COL_MAGENTA)
//     0x16 '\26' ^V ... set yellow text color (COL_YELLOW)
//     0x17 '\27' ^W ... set white text color (COL_WHITE)
void DrawCanPrintCharRaw(sDrawCan* can, char ch)
{
	switch ((u8)ch)
	{
	// printable characters
	default:
		if ((u8)ch < 32) break; // not printable character

		// double-width or double-sized font
		if (can->printsize >= 2)
		{
			if (can->printsize == 3)
			{
				if (can->printrow >= can->printrownum-1)
				{
					DrawCanScroll(can); // scroll screen one row up
					can->printrow = can->printrownum-2;
				}

				// double-sized font
				can->drawfnc->pDrawCanCharBg(can, (char)(ch + can->printinv), (int)can->printpos*can->fontw,
					(int)can->printrow*can->fonth, can->printcol, can->drawfnc->col_black, 2, 2);
			}
			else
				// double-width font
				can->drawfnc->pDrawCanCharBg(can, (char)(ch + can->printinv), (int)can->printpos*can->fontw,
					(int)can->printrow*can->fonth, can->printcol, can->drawfnc->col_black, 2, 1);

			// move cursor 1 position right
			can->printpos++;
		}
		else
		{
			if (can->printsize == 1)
			{
				if (can->printrow >= can->printrownum-1)
				{
					DrawCanScroll(can); // scroll screen one row up
					can->printrow = can->printrownum-2;
				}

				// double-height font
				can->drawfnc->pDrawCanCharBg(can, (char)(ch + can->printinv), (int)can->printpos*can->fontw,
					(int)can->printrow*can->fonth, can->printcol, can->drawfnc->col_black, 1, 2);
			}
			else
				// normal font size
				can->drawfnc->pDrawCanCharBg(can, (char)(ch + can->printinv), (int)can->printpos*can->fontw,
					(int)can->printrow*can->fonth, can->printcol, can->drawfnc->col_black, 1, 1);
		}

  // ! continue case 0x07 (move right)

	// '\a' ... (bell) move cursor 1 position right (no print)
	case 0x07:
		can->printpos++;
		if (can->printpos < can->printposnum) break;

  // ! continue case 0x0A (new line)

	// '\n' ... (new line) move cursor to start of next row
	case 0x0A:
		can->printpos = 0;
		can->printrow++; // increase row
		if (can->printrow >= can->printrownum)
		{
			DrawCanScroll(can); // scroll screen one row up
			can->printrow = can->printrownum-1;
		}

		// double height
		if ((can->printsize & 1) != 0)
		{
			can->printrow++; // increase row
			if (can->printrow >= can->printrownum)
			{
				DrawCanScroll(can); // scroll screen one row up
				can->printrow = can->printrownum-1;
			}
		}
		break;

	// 'x01' ... set not-inverted text
	case 0x01:
		can->printinv = 0;
		break;

	// 'x02' ... set inverted text (shift character code by 0x80)
	case 0x02:
		can->printinv = 128;
		break;

	// 'x03' ... use normal-sized font (default)
	case 0x03:
		can->printsize = 0; // fonf size: 0=normal, 1=double-height, 2=double-width, 3=double-size
		break;

	// 'x04' ... use double-height font
	case 0x04:
		can->printsize = 1; // fonf size: 0=normal, 1=double-height, 2=double-width, 3=double-size
		break;

	// 'x05' ... use double-width font
	case 0x05:
		can->printsize = 2; // fonf size: 0=normal, 1=double-height, 2=double-width, 3=double-size
		break;

	// 'x06' ... use double-sized font
	case 0x06:
		can->printsize = 3; // fonf size: 0=normal, 1=double-height, 2=double-width, 3=double-size
		break;

	// '\b' ... (back space) move cursor 1 position left (no print)
	case 0x08:
		if (can->printpos > 0) can->printpos--;
		break;

	// '\t' ... (tabulator) move cursor to next 8-character position, print spaces
	case 0x09:
		{
			u8 oldsize = can->printsize; // save current font size
			can->printsize = oldsize & 1; // change double-width or double-size to normal or double-height
			do DrawCanPrintCharRaw(can, ' '); while ((can->printpos & 7) != 0); // print spaces
			can->printsize = oldsize; // restore font size
		}
		break;		

	// '\v' ... (vertical tabulator) move cursor to start of previous row
	case 0x0B:
		if (can->printrow > 0) can->printrow--; // decrease row
		can->printpos = 0;
		break;

	// '\f' ... (form feed) clear screen, reset cursor position and set default color
	case 0x0C:
		can->drawfnc->pDrawCanClear(can); // clear screen
		can->printpos = 0;  // console print character position
		can->printrow = 0;  // console print character row
		can->printinv = 0; // offset added to character (128 = print inverted characters)
		can->printsize = 0; // fonf size: 0=normal, 1=double-width, 2=double-size
		can->printcol = can->drawfnc->col_gray; // console print color
		break;

	// '\r' ... (carriage return) move cursor to start of current row
	case 0x0D:
		can->printpos = 0;
		break;

	// 'x10' ... set gray text color (COL_GRAY, normal color)
	case 0x10:
		can->printcol = can->drawfnc->col_gray;
		break;

	// 'x11' ... set blue text color (COL_AZURE)
	case 0x11:
		can->printcol = can->drawfnc->col_azure;
		break;

	// 'x12' ... set green text color (COL_GREEN)
	case 0x12:
		can->printcol = can->drawfnc->col_green;
		break;

	// 'x13' ... set cyan text color (COL_CYAN)
	case 0x13:
		can->printcol = can->drawfnc->col_cyan;
		break;

	// 'x14' ... set red text color (COL_RED)
	case 0x14:
		can->printcol = can->drawfnc->col_red;
		break;

	// 'x15' ... set magenta text color (COL_MAGENTA)
	case 0x15:
		can->printcol = can->drawfnc->col_magenta;
		break;

	// 'x16' ... set yellow text color (COL_YELLOW)
	case 0x16:
		can->printcol = can->drawfnc->col_yellow;
		break;

	// 'x17' ... set white text color (COL_WHITE)
	case 0x17:
		can->printcol = can->drawfnc->col_white;
		break;
	}
}

void DrawPrintCharRaw(char ch) { DrawCanPrintCharRaw(pDrawCan, ch); }

// console print character (with display update; Control characters - see DrawPrintCharRaw)
void DrawCanPrintChar(sDrawCan* can, char ch)
{
	DrawCanPrintCharRaw(can, ch);
	DrawCanUpdate(can);
}

void DrawPrintChar(char ch) { DrawCanPrintChar(pDrawCan, ch); }

// console print text (Control characters - see DrawPrintCharRaw)
//  If text contains digit after hex numeric code of control character,
//  split text to more parts: use "\4" "1" instead of "\41".
void DrawCanPrintText(sDrawCan* can, const char* txt)
{
	char ch;
	while ((ch = *txt++) != 0) DrawCanPrintCharRaw(can, ch);
	DrawCanUpdate(can);
}

void DrawPrintText(const char* txt) { DrawCanPrintText(pDrawCan, txt); }

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// callback - write data to drawing console
u32 StreamWriteDrawPrint(sStream* str, const void* buf, u32 num)
{
	u32 n = num;
	const char* txt = (const char*)buf;
	for (; n > 0; n--) DrawPrintCharRaw(*txt++);
	return num;
}

// formatted print string to drawing console, with argument list (returns number of characters, without terminating 0)
u32 DrawPrintArg(const char* fmt, va_list args)
{
	// write and read stream
	sStream wstr, rstr;

	// initialize stream to read from
	StreamReadBufInit(&rstr, fmt, StrLen(fmt));

	// initialize stream to write to
	Stream0Init(&wstr); // initialize nul stream
	wstr.write = StreamWriteDrawPrint; // write callback
	
	// print string
	u32 num = StreamPrintArg(&wstr, &rstr, args);

	// display update
	if (num > 0) DispUpdate();
	return num;
}

// formatted print string to drawing console, with variadic arguments (returns number of characters, without terminating 0)
NOINLINE u32 DrawPrint(const char* fmt, ...)
{
	u32 n;
	va_list args;
	va_start(args, fmt);
	n = DrawPrintArg(fmt, args);
	va_end(args);
	return n;
}

#endif // USE_STREAM

/*
DrawPrint test sample:

	SelFont8x16(); // select font 8x16
	DrawPrint(
		"\f"					// clear screen (resolution: 40 characters x 15 rows)

		"- invisible row scrolls up -\n"	// 0: this row will be scrolled-up

		"Hello world?\b!\n"			// 1: first visible print, overprint "?" -> "!"

		"First! row\r" "Second\n"		// 2: overprint "First! row" -> "Second row"

		"\3normal \4height \5width \6sized\3\n"	// 3: double-height row with font of all 4 sizes
		"\3font   \a\a\a\a\a\a\a\5font\3\n"	// 4: second half of previous double-height row

		"non-invert \2invert\1 non-invert\n"	// 5: inverted text

		"0123456701234567012345670123456701234567" // 6: print ruler 40 characters and move to next row (row is 40 columns width)
		"\t123\t1234567\t12345678\tnext row\n"	// 7, 8: move to tab positions and move to next row after all

		"\20 normal \21 blue \22 green \23 cyan \n" // 9: print colors
		"\24 red \25 magenta \26 yellow \27 white \n" // 10:

		"\2"
		"\20 normal \21 blue \22 green \23 cyan \n" // 11: print colors inverted
		"\24 red \25 magenta \26 yellow \27 white \n" // 12:

		"\1\20"					// back to default state
		"%d 0x%08x ALL OK\n"			// 13: print numbers 12345678, 0x12345678

		"\nscrolling up",			// 14: this command scrolls screen up

		12345678, 0x12345678
	);
*/

// select font
//  font ... pointer to current font (256 characters in cells of width 8 pixels, 1-bit format)
//  fontw ... font width (number of pixels, max. 8)
//  fonth ... font height (number of lines)
void DrawCanSelFont(sDrawCan* can, const u8* font, u8 fontw, u8 fonth)
{
	can->font = font;
	can->fontw = fontw;
	can->fonth = fonth;
	can->printposnum = can->w / fontw;
	can->printrownum = can->h / fonth;
}

void DrawCanSelFont8x8(sDrawCan* can)		{ DrawCanSelFont(can, FontBold8x8, 8, 8); }	// sans-serif bold, height 8 lines
void DrawCanSelFont8x14(sDrawCan* can)		{ DrawCanSelFont(can, FontBold8x14, 8, 14); }	// sans-serif bold, height 14 lines
void DrawCanSelFont8x16(sDrawCan* can)		{ DrawCanSelFont(can, FontBold8x16, 8, 16); }	// sans-serif bold, height 16 lines
void DrawCanSelFont8x14Serif(sDrawCan* can)	{ DrawCanSelFont(can, FontBoldB8x14, 8, 14); }	// serif bold, height 14 lines
void DrawCanSelFont8x16Serif(sDrawCan* can)	{ DrawCanSelFont(can, FontBoldB8x16, 8, 16); }	// serif bold, height 16 lines
void DrawCanSelFont6x8(sDrawCan* can)		{ DrawCanSelFont(can, FontCond6x8, 6, 8); }	// condensed font, width 6 pixels, height 8 lines
void DrawCanSelFont8x8Game(sDrawCan* can)	{ DrawCanSelFont(can, FontGame8x8, 8, 8); }	// game font, height 8 lines
void DrawCanSelFont8x8Ibm(sDrawCan* can)	{ DrawCanSelFont(can, FontIbm8x8, 8, 8); }	// IBM charset font, height 8 lines
void DrawCanSelFont8x14Ibm(sDrawCan* can)	{ DrawCanSelFont(can, FontIbm8x14, 8, 14); }	// IBM charset font, height 14 lines
void DrawCanSelFont8x16Ibm(sDrawCan* can)	{ DrawCanSelFont(can, FontIbm8x16, 8, 16); }	// IBM charset font, height 16 lines
void DrawCanSelFont8x8IbmThin(sDrawCan* can)	{ DrawCanSelFont(can, FontIbmTiny8x8, 8, 8); }	// IBM charset thin font, height 8 lines
void DrawCanSelFont8x8Italic(sDrawCan* can)	{ DrawCanSelFont(can, FontItalic8x8, 8, 8); }	// italic, height 8 lines
void DrawCanSelFont8x8Thin(sDrawCan* can)	{ DrawCanSelFont(can, FontThin8x8, 8, 8); }	// thin font, height 8 lines
void DrawCanSelFont5x8(sDrawCan* can)		{ DrawCanSelFont(can, FontTiny5x8, 5, 8); }	// tiny font, width 5 pixels, height 8 lines

#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas

void DrawSelFont(const u8* font, u8 fontw, u8 fonth) { DrawCanSelFont(pDrawCan, font, fontw, fonth); }

void DrawSelFont8x8()		{ DrawCanSelFont8x8(pDrawCan); }		// sans-serif bold, height 8 lines
void DrawSelFont8x14()		{ DrawCanSelFont8x14(pDrawCan); }		// sans-serif bold, height 14 lines
void DrawSelFont8x16()		{ DrawCanSelFont8x16(pDrawCan); }		// sans-serif bold, height 16 lines
void DrawSelFont8x14Serif()	{ DrawCanSelFont8x14Serif(pDrawCan); }		// serif bold, height 14 lines
void DrawSelFont8x16Serif()	{ DrawCanSelFont8x16Serif(pDrawCan); }		// serif bold, height 16 lines
void DrawSelFont6x8()		{ DrawCanSelFont6x8(pDrawCan); }		// condensed font, width 6 pixels, height 8 lines
void DrawSelFont8x8Game()	{ DrawCanSelFont8x8Game(pDrawCan); }		// game font, height 8 lines
void DrawSelFont8x8Ibm()	{ DrawCanSelFont8x8Ibm(pDrawCan); }		// IBM charset font, height 8 lines
void DrawSelFont8x14Ibm()	{ DrawCanSelFont8x14Ibm(pDrawCan); }		// IBM charset font, height 14 lines
void DrawSelFont8x16Ibm()	{ DrawCanSelFont8x16Ibm(pDrawCan); }		// IBM charset font, height 16 lines
void DrawSelFont8x8IbmThin()	{ DrawCanSelFont8x8IbmThin(pDrawCan); }		// IBM charset thin font, height 8 lines
void DrawSelFont8x8Italic()	{ DrawCanSelFont8x8Italic(pDrawCan); }		// italic, height 8 lines
void DrawSelFont8x8Thin()	{ DrawCanSelFont8x8Thin(pDrawCan); }		// thin font, height 8 lines
void DrawSelFont5x8()		{ DrawCanSelFont5x8(pDrawCan); }		// tiny font, width 5 pixels, height 8 lines

// calculate pitch of graphics line from image width (get length of line in bytes, rounded up to 4-byte word)
int DrawPitch(int w) { return pDrawCan->drawfnc->pDrawPitch(w); }

// calculate max. width of image from the pitch (pitch is length of line in bytes, rounded up to 4-byte word)
int DrawMaxWidth(int pitch) { return pDrawCan->drawfnc->pDrawMaxWidth(pitch); }

// ----- Clear canvas

// Clear canvas with color
void DrawCanClearCol(sDrawCan* can, u16 col) { can->drawfnc->pDrawCanClearCol(can, col); }
void DrawClearCol(u16 col) { pDrawCan->drawfnc->pDrawCanClearCol(pDrawCan, col); }

// Clear canvas with black color
void DrawCanClear(sDrawCan* can) { can->drawfnc->pDrawCanClear(can); }
void DrawClear() { pDrawCan->drawfnc->pDrawCanClear(pDrawCan); }

// Fast clear canvas with DMA
void DrawCanClearFast(sDrawCan* can) { 
#if DISPHSTX_PICOSDK	// 1=use Pico SDK
	memset(can->buf, 0, can->wb * can->h);
#else
	DMA_MemFill(can->buf, 0, can->wb * can->h);
#endif
}
void DrawClearFast() { DrawCanClearFast(pDrawCan); }

// ----- Draw point

// Draw point (does not check clipping limits)
void DrawCanPoint_Fast(sDrawCan* can, int x, int y, u16 col) { can->drawfnc->pDrawCanPoint_Fast(can, x, y, col); }
void DrawPoint_Fast(int x, int y, u16 col) { pDrawCan->drawfnc->pDrawCanPoint_Fast(pDrawCan, x, y, col); }

// Draw point (checks clipping limits)
void DrawCanPoint(sDrawCan* can, int x, int y, u16 col) { can->drawfnc->pDrawCanPoint(can, x, y, col); }
void DrawPoint(int x, int y, u16 col) { pDrawCan->drawfnc->pDrawCanPoint(pDrawCan, x, y, col); }

// Draw point inverted (does not check clipping limits)
void DrawCanPointInv_Fast(sDrawCan* can, int x, int y) { can->drawfnc->pDrawCanPointInv_Fast(can, x, y); }
void DrawPointInv_Fast(int x, int y) { pDrawCan->drawfnc->pDrawCanPointInv_Fast(pDrawCan, x, y); }

// Draw point inverted (checks clipping limits)
void DrawCanPointInv(sDrawCan* can, int x, int y) { can->drawfnc->pDrawCanPointInv(can, x, y); }
void DrawPointInv(int x, int y) { pDrawCan->drawfnc->pDrawCanPointInv(pDrawCan, x, y); }

// ----- Get point

// Get point (does not check clipping limits)
u16 DrawCanGetPoint_Fast(sDrawCan* can, int x, int y) { return can->drawfnc->pDrawCanGetPoint_Fast(can, x, y); }
u16 DrawGetPoint_Fast(int x, int y) { return pDrawCan->drawfnc->pDrawCanGetPoint_Fast(pDrawCan, x, y); }

// Get point (checks clipping limits)
u16 DrawCanGetPoint(sDrawCan* can, int x, int y) { return can->drawfnc->pDrawCanGetPoint(can, x, y); }
u16 DrawGetPoint(int x, int y) { return pDrawCan->drawfnc->pDrawCanGetPoint(pDrawCan, x, y); }

// ----- Draw rectangle

// Draw rectangle (does not check clipping limits; dimensions must be > 0)
void DrawCanRect_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col) { can->drawfnc->pDrawCanRect_Fast(can, x, y, w, h, col); }
void DrawRect_Fast(int x, int y, int w, int h, u16 col) { pDrawCan->drawfnc->pDrawCanRect_Fast(pDrawCan, x, y, w, h, col); }

// Draw rectangle (checks clipping limits; negative dimensions flip rectangle)
void DrawCanRect(sDrawCan* can, int x, int y, int w, int h, u16 col) { can->drawfnc->pDrawCanRect(can, x, y, w, h, col); }
void DrawRect(int x, int y, int w, int h, u16 col) { pDrawCan->drawfnc->pDrawCanRect(pDrawCan, x, y, w, h, col); }

// Draw rectangle inverted (does not check clipping limits; dimensions must be > 0)
void DrawCanRectInv_Fast(sDrawCan* can, int x, int y, int w, int h) { can->drawfnc->pDrawCanRectInv_Fast(can, x, y, w, h); }
void DrawRectInv_Fast(int x, int y, int w, int h) { pDrawCan->drawfnc->pDrawCanRectInv_Fast(pDrawCan, x, y, w, h); }

// Draw rectangle inverted (checks clipping limits; negative dimensions flip rectangle)
void DrawCanRectInv(sDrawCan* can, int x, int y, int w, int h) { can->drawfnc->pDrawCanRectInv(can, x, y, w, h); }
void DrawRectInv(int x, int y, int w, int h) { pDrawCan->drawfnc->pDrawCanRectInv(pDrawCan, x, y, w, h); }

// ----- Draw horizontal line

// Draw horizontal line (does not check clipping limits; width must be > 0)
void DrawCanHLine_Fast(sDrawCan* can, int x, int y, int w, u16 col) { can->drawfnc->pDrawCanHLine_Fast(can, x, y, w, col); }
void DrawHLine_Fast(int x, int y, int w, u16 col) { pDrawCan->drawfnc->pDrawCanHLine_Fast(pDrawCan, x, y, w, col); }

// Draw horizontal line (checks clipping limits; negative width flip line)
void DrawCanHLine(sDrawCan* can, int x, int y, int w, u16 col) { can->drawfnc->pDrawCanHLine(can, x, y, w, col); }
void DrawHLine(int x, int y, int w, u16 col) { pDrawCan->drawfnc->pDrawCanHLine(pDrawCan, x, y, w, col); }

// Draw horizontal line inverted (does not check clipping limits; width must be > 0)
void DrawCanHLineInv_Fast(sDrawCan* can, int x, int y, int w) { can->drawfnc->pDrawCanHLineInv_Fast(can, x, y, w); }
void DrawHLineInv_Fast(int x, int y, int w) { pDrawCan->drawfnc->pDrawCanHLineInv_Fast(pDrawCan, x, y, w); }

// Draw horizontal line inverted (checks clipping limits; negative width flip line)
void DrawCanHLineInv(sDrawCan* can, int x, int y, int w) { can->drawfnc->pDrawCanHLineInv(can, x, y, w); }
void DrawHLineInv(int x, int y, int w) { pDrawCan->drawfnc->pDrawCanHLineInv(pDrawCan, x, y, w); }

// ----- Draw vertical line

// Draw vertical line (does not check clipping limits; height must be > 0)
void DrawCanVLine_Fast(sDrawCan* can, int x, int y, int h, u16 col) { can->drawfnc->pDrawCanVLine_Fast(can, x, y, h, col); }
void DrawVLine_Fast(int x, int y, int h, u16 col) { pDrawCan->drawfnc->pDrawCanVLine_Fast(pDrawCan, x, y, h, col); }

// Draw vertical line (checks clipping limits; negative height flip line)
void DrawCanVLine(sDrawCan* can, int x, int y, int h, u16 col) { can->drawfnc->pDrawCanVLine(can, x, y, h, col); }
void DrawVLine(int x, int y, int h, u16 col) { pDrawCan->drawfnc->pDrawCanVLine(pDrawCan, x, y, h, col); }

// Draw vertical line inverted (does not check clipping limits; height must be > 0)
void DrawCanVLineInv_Fast(sDrawCan* can, int x, int y, int h) { can->drawfnc->pDrawCanVLineInv_Fast(can, x, y, h); }
void DrawVLineInv_Fast(int x, int y, int h) { pDrawCan->drawfnc->pDrawCanVLineInv_Fast(pDrawCan, x, y, h); }

// Draw vertical line inverted (checks clipping limits; negative height flip line)
void DrawCanVLineInv(sDrawCan* can, int x, int y, int h) { can->drawfnc->pDrawCanVLineInv(can, x, y, h); }
void DrawVLineInv(int x, int y, int h) { pDrawCan->drawfnc->pDrawCanVLineInv(pDrawCan, x, y, h); }

// ----- Draw frame

// Draw 1-pixel frame (does not check clipping limits; dimensions must be > 0)
void DrawCanFrame_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark) { can->drawfnc->pDrawCanFrame_Fast(can, x, y, w, h, col_light, col_dark); }
void DrawFrame_Fast(int x, int y, int w, int h, u16 col_light, u16 col_dark) { pDrawCan->drawfnc->pDrawCanFrame_Fast(pDrawCan, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame (checks clipping limits; negative dimensions flip frame)
void DrawCanFrame(sDrawCan* can, int x, int y, int w, int h, u16 col_light, u16 col_dark) { can->drawfnc->pDrawCanFrame(can, x, y, w, h, col_light, col_dark); }

// For backward compatibility with Draw version 1
//void DrawFrame(int x, int y, int w, int h, u16 col_light, u16 col_dark) { pDrawCan->drawfnc->pDrawCanFrame(pDrawCan, x, y, w, h, col_light, col_dark); }
void DrawFrame3D(int x, int y, int w, int h, u16 col_light, u16 col_dark) { pDrawCan->drawfnc->pDrawCanFrame(pDrawCan, x, y, w, h, col_light, col_dark); }

// Draw 1-pixel frame inverted (does not check clipping limits; dimensions must be > 0)
void DrawCanFrameInv_Fast(sDrawCan* can, int x, int y, int w, int h) { can->drawfnc->pDrawCanFrameInv_Fast(can, x, y, w, h); }
void DrawFrameInv_Fast(int x, int y, int w, int h) { pDrawCan->drawfnc->pDrawCanFrameInv_Fast(pDrawCan, x, y, w, h); }

// Draw 1-pixel frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCanFrameInv(sDrawCan* can, int x, int y, int w, int h) { can->drawfnc->pDrawCanFrameInv(can, x, y, w, h); }
void DrawFrameInv(int x, int y, int w, int h) { pDrawCan->drawfnc->pDrawCanFrameInv(pDrawCan, x, y, w, h); }

// Draw thick frame (does not check clipping limits; dimensions and thick must be > 0)
void DrawCanFrameW_Fast(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick) { can->drawfnc->pDrawCanFrameW_Fast(can, x, y, w, h, col, thick); }
void DrawFrameW_Fast(int x, int y, int w, int h, u16 col, int thick) { pDrawCan->drawfnc->pDrawCanFrameW_Fast(pDrawCan, x, y, w, h, col, thick); }

// Draw thick frame (checks clipping limits; negative dimensions flip frame)
void DrawCanFrameW(sDrawCan* can, int x, int y, int w, int h, u16 col, int thick) { can->drawfnc->pDrawCanFrameW(can, x, y, w, h, col, thick); }
void DrawFrameW(int x, int y, int w, int h, u16 col, int thick) { pDrawCan->drawfnc->pDrawCanFrameW(pDrawCan, x, y, w, h, col, thick); }

// Draw thick frame inverted (does not check clipping limits; dimensions and thick must be > 0)
void DrawCanFrameWInv_Fast(sDrawCan* can, int x, int y, int w, int h, int thick) { can->drawfnc->pDrawCanFrameWInv_Fast(can, x, y, w, h, thick); }
void DrawFrameWInv_Fast(int x, int y, int w, int h, int thick) { pDrawCan->drawfnc->pDrawCanFrameWInv_Fast(pDrawCan, x, y, w, h, thick); }

// Draw thick frame inverted (checks clipping limits; negative dimensions flip frame)
void DrawCanFrameWInv(sDrawCan* can, int x, int y, int w, int h, int thick) { can->drawfnc->pDrawCanFrameWInv(can, x, y, w, h, thick); }
void DrawFrameWInv(int x, int y, int w, int h, int thick) { pDrawCan->drawfnc->pDrawCanFrameWInv(pDrawCan, x, y, w, h, thick); }

// ----- Draw line

// Draw line with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
//  over ... draw overlapped pixels
void DrawCanLineOver_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over) { can->drawfnc->pDrawCanLineOver_Fast(can, x1, y1, x2, y2, col, over); }
void DrawLineOver_Fast(int x1, int y1, int x2, int y2, u16 col, Bool over) { pDrawCan->drawfnc->pDrawCanLineOver_Fast(pDrawCan, x1, y1, x2, y2, col, over); }

// Draw line (does not check clipping limits)
void DrawCanLine_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { can->drawfnc->pDrawCanLine_Fast(can, x1, y1, x2, y2, col); }
void DrawLine_Fast(int x1, int y1, int x2, int y2, u16 col) { pDrawCan->drawfnc->pDrawCanLine_Fast(pDrawCan, x1, y1, x2, y2, col); }

// Draw line with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCanLineOver(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, Bool over) { can->drawfnc->pDrawCanLineOver(can, x1, y1, x2, y2, col, over); }
void DrawLineOver(int x1, int y1, int x2, int y2, u16 col, Bool over) { pDrawCan->drawfnc->pDrawCanLineOver(pDrawCan, x1, y1, x2, y2, col, over); }

// Draw line (checks clipping limits)
void DrawCanLine(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col) { can->drawfnc->pDrawCanLine(can, x1, y1, x2, y2, col); }
void DrawLine(int x1, int y1, int x2, int y2, u16 col) { pDrawCan->drawfnc->pDrawCanLine(pDrawCan, x1, y1, x2, y2, col); }

// Draw line inverted with overlapped pixels (does not check clipping limits; function used internally by the "draw thick line" function)
void DrawCanLineOverInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over) { can->drawfnc->pDrawCanLineOverInv_Fast(can, x1, y1, x2, y2, over); }
void DrawLineOverInv_Fast(int x1, int y1, int x2, int y2, Bool over) { pDrawCan->drawfnc->pDrawCanLineOverInv_Fast(pDrawCan, x1, y1, x2, y2, over); }

// Draw line inverted (does not check clipping limits)
void DrawCanLineInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2) { can->drawfnc->pDrawCanLineInv_Fast(can, x1, y1, x2, y2); }
void DrawLineInv_Fast(int x1, int y1, int x2, int y2) { pDrawCan->drawfnc->pDrawCanLineInv_Fast(pDrawCan, x1, y1, x2, y2); }

// Draw line inverted with overlapped pixels (checks clipping limits; function used internally by the "draw thick line" function)
void DrawCanLineOverInv(sDrawCan* can, int x1, int y1, int x2, int y2, Bool over) { can->drawfnc->pDrawCanLineOverInv(can, x1, y1, x2, y2, over); }
void DrawLineOverInv(int x1, int y1, int x2, int y2, Bool over) { pDrawCan->drawfnc->pDrawCanLineOverInv(pDrawCan, x1, y1, x2, y2, over); }

// Draw line inverted (checks clipping limits)
void DrawCanLineInv(sDrawCan* can, int x1, int y1, int x2, int y2) { can->drawfnc->pDrawCanLineInv(can, x1, y1, x2, y2); }
void DrawLineInv(int x1, int y1, int x2, int y2) { pDrawCan->drawfnc->pDrawCanLineInv(pDrawCan, x1, y1, x2, y2); }

// Draw thick line (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCanLineW_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { can->drawfnc->pDrawCanLineW_Fast(can, x1, y1, x2, y2, col, thick, round); }
void DrawLineW_Fast(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { pDrawCan->drawfnc->pDrawCanLineW_Fast(pDrawCan, x1, y1, x2, y2, col, thick, round); }

// Draw thick line (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCanLineW(sDrawCan* can, int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { can->drawfnc->pDrawCanLineW(can, x1, y1, x2, y2, col, thick, round); }
void DrawLineW(int x1, int y1, int x2, int y2, u16 col, int thick, Bool round) { pDrawCan->drawfnc->pDrawCanLineW(pDrawCan, x1, y1, x2, y2, col, thick, round); }

// Draw thick line inverted (does not check clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCanLineWInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int thick) { can->drawfnc->pDrawCanLineWInv_Fast(can, x1, y1, x2, y2, thick); }
void DrawLineWInv_Fast(int x1, int y1, int x2, int y2, int thick) { pDrawCan->drawfnc->pDrawCanLineWInv_Fast(pDrawCan, x1, y1, x2, y2, thick); }

// Draw thick line inverted (checks clipping limits)
//  thick ... thick of line in pixels
//  round ... draw round ends
void DrawCanLineWInv(sDrawCan* can, int x1, int y1, int x2, int y2, int thick) { can->drawfnc->pDrawCanLineWInv(can, x1, y1, x2, y2, thick); }
void DrawLineWInv(int x1, int y1, int x2, int y2, int thick) { pDrawCan->drawfnc->pDrawCanLineWInv(pDrawCan, x1, y1, x2, y2, thick); }

// ----- Draw round (filled circle)
//  d ... diameter of the round (radius = d/2)
//  mask ... hide parts of the round with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole round

// Draw round (does not check clipping limits; diameter must be > 0)
void DrawCanRound_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask) { can->drawfnc->pDrawCanRound_Fast(can, x, y, d, col, mask); }
void DrawRound_Fast(int x, int y, int d, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanRound_Fast(pDrawCan, x, y, d, col, mask); }

// Draw round (checks clipping limits)
void DrawCanRound(sDrawCan* can, int x, int y, int d, u16 col, u8 mask) { can->drawfnc->pDrawCanRound(can, x, y, d, col, mask); }
void DrawRound(int x, int y, int d, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanRound(pDrawCan, x, y, d, col, mask); }

// Draw round inverted (does not check clipping limits; diameter must be > 0)
void DrawCanRoundInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask) { can->drawfnc->pDrawCanRoundInv_Fast(can, x, y, d, mask); }
void DrawRoundInv_Fast(int x, int y, int d, u8 mask) { pDrawCan->drawfnc->pDrawCanRoundInv_Fast(pDrawCan, x, y, d, mask); }

// Draw round inverted (checks clipping limits)
void DrawCanRoundInv(sDrawCan* can, int x, int y, int d, u8 mask) { can->drawfnc->pDrawCanRoundInv(can, x, y, d, mask); }
void DrawRoundInv(int x, int y, int d, u8 mask) { pDrawCan->drawfnc->pDrawCanRoundInv(pDrawCan, x, y, d, mask); }

// ----- Draw circle
//  d ... diameter of the circle (radius = d/2)
//  mask ... draw circle arcs, use combination of DRAWCAN_CIRCLE_*; use DRAWCAN_CIRCLE_ALL or 0xff to draw whole circle

// Draw circle or arc (does not check clipping limits)
void DrawCanCircle_Fast(sDrawCan* can, int x, int y, int d, u16 col, u8 mask) { can->drawfnc->pDrawCanCircle_Fast(can, x, y, d, col, mask); }
void DrawCircle_Fast(int x, int y, int d, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanCircle_Fast(pDrawCan, x, y, d, col, mask); }

// Draw circle or arc (checks clipping limits)
void DrawCanCircle(sDrawCan* can, int x, int y, int d, u16 col, u8 mask) { can->drawfnc->pDrawCanCircle(can, x, y, d, col, mask); }

// For backward compatibility with Draw version 1
//void DrawCircle(int x, int y, int d, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanCircle(pDrawCan, x, y, d, col, mask); }
void DrawCircleArc(int x, int y, int d, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanCircle(pDrawCan, x, y, d, col, mask); }

// Draw circle or arc inverted (does not check clipping limits)
void DrawCanCircleInv_Fast(sDrawCan* can, int x, int y, int d, u8 mask) { can->drawfnc->pDrawCanCircleInv_Fast(can, x, y, d, mask); }
void DrawCircleInv_Fast(int x, int y, int d, u8 mask) { pDrawCan->drawfnc->pDrawCanCircleInv_Fast(pDrawCan, x, y, d, mask); }

// Draw circle or arc inverted (checks clipping limits)
void DrawCanCircleInv(sDrawCan* can, int x, int y, int d, u8 mask) { can->drawfnc->pDrawCanCircleInv(can, x, y, d, mask); }

// For backward compatibility with Draw version 1
//void DrawCircleInv(int x, int y, int d, u8 mask) { pDrawCan->drawfnc->pDrawCanCircleInv(pDrawCan, x, y, d, mask); }
void DrawCircleArcInv(int x, int y, int d, u8 mask) { pDrawCan->drawfnc->pDrawCanCircleInv(pDrawCan, x, y, d, mask); }

// ----- Draw ring
//  d ... outer diameter of the ring (outer radius = d/2)
//  din ... inner diameter of the ring (inner radius = din/2)
//  mask ... hide parts of the ring with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ring

// Draw ring (does not check clipping limits)
void DrawCanRing_Fast(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask) { can->drawfnc->pDrawCanRing_Fast(can, x, y, d, din, col, mask); }
void DrawRing_Fast(int x, int y, int d, int din, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanRing_Fast(pDrawCan, x, y, d, din, col, mask); }

// Draw ring (checks clipping limits)
void DrawCanRing(sDrawCan* can, int x, int y, int d, int din, u16 col, u8 mask) { can->drawfnc->pDrawCanRing(can, x, y, d, din, col, mask); }

// For backward compatibility with Draw version 1
//void DrawRing(int x, int y, int d, int din, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanRing(pDrawCan, x, y, d, din, col, mask); }
void DrawRingMask(int x, int y, int d, int din, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanRing(pDrawCan, x, y, d, din, col, mask); }

// Draw ring inverted (does not check clipping limits)
void DrawCanRingInv_Fast(sDrawCan* can, int x, int y, int d, int din, u8 mask) { can->drawfnc->pDrawCanRingInv_Fast(can, x, y, d, din, mask); }
void DrawRingInv_Fast(int x, int y, int d, int din, u8 mask) { pDrawCan->drawfnc->pDrawCanRingInv_Fast(pDrawCan, x, y, d, din, mask); }

// Draw ring inverted (checks clipping limits)
void DrawCanRingInv(sDrawCan* can, int x, int y, int d, int din, u8 mask) { can->drawfnc->pDrawCanRingInv(can, x, y, d, din, mask); }

// For backward compatibility with Draw version 1
//void DrawRingInv(int x, int y, int d, int din, u8 mask) { pDrawCan->drawfnc->pDrawCanRingInv(pDrawCan, x, y, d, din, mask); }
void DrawRingInvMask(int x, int y, int d, int din, u8 mask) { pDrawCan->drawfnc->pDrawCanRingInv(pDrawCan, x, y, d, din, mask); }

// ----- Draw triangle

// Draw triangle (does not check clipping limits)
void DrawCanTriangle_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { can->drawfnc->pDrawCanTriangle_Fast(can, x1, y1, x2, y2, x3, y3, col); }
void DrawTriangle_Fast(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { pDrawCan->drawfnc->pDrawCanTriangle_Fast(pDrawCan, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle (checks clipping limits)
void DrawCanTriangle(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { can->drawfnc->pDrawCanTriangle(can, x1, y1, x2, y2, x3, y3, col); }
void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, u16 col) { pDrawCan->drawfnc->pDrawCanTriangle(pDrawCan, x1, y1, x2, y2, x3, y3, col); }

// Draw triangle inverted (does not check clipping limits)
void DrawCanTriangleInv_Fast(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3) { can->drawfnc->pDrawCanTriangleInv_Fast(can, x1, y1, x2, y2, x3, y3); }
void DrawTriangleInv_Fast(int x1, int y1, int x2, int y2, int x3, int y3) { pDrawCan->drawfnc->pDrawCanTriangleInv_Fast(pDrawCan, x1, y1, x2, y2, x3, y3); }

// Draw triangle inverted (checks clipping limits)
void DrawCanTriangleInv(sDrawCan* can, int x1, int y1, int x2, int y2, int x3, int y3) { can->drawfnc->pDrawCanTriangleInv(can, x1, y1, x2, y2, x3, y3); }
void DrawTriangleInv(int x1, int y1, int x2, int y2, int x3, int y3) { pDrawCan->drawfnc->pDrawCanTriangleInv(pDrawCan, x1, y1, x2, y2, x3, y3); }

// ----- Draw fill area

// Draw fill area
void DrawCanFill(sDrawCan* can, int x, int y, u16 col) { can->drawfnc->pDrawCanFill(can, x, y, col); }
void DrawFill(int x, int y, u16 col) { pDrawCan->drawfnc->pDrawCanFill(pDrawCan, x, y, col); }

// ----- Draw character
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw character with transparent background (does not check clipping limits)
void DrawCanChar_Fast(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley) { can->drawfnc->pDrawCanChar_Fast(can, ch, x, y, col, scalex, scaley); }
void DrawChar_Fast(char ch, int x, int y, u16 col, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanChar_Fast(pDrawCan, ch, x, y, col, scalex, scaley); }

// Draw character with transparent background (checks clipping limits)
void DrawCanChar(sDrawCan* can, char ch, int x, int y, u16 col, int scalex, int scaley) { can->drawfnc->pDrawCanChar(can, ch, x, y, col, scalex, scaley); }

// For backward compatibility with Draw version 1
//void DrawChar(char ch, int x, int y, u16 col, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanChar(pDrawCan, ch, x, y, col, scalex, scaley); }
void DrawCharSize(char ch, int x, int y, u16 col, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanChar(pDrawCan, ch, x, y, col, scalex, scaley); }

// Draw character with background (does not check clipping limits)
void DrawCanCharBg_Fast(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { can->drawfnc->pDrawCanCharBg_Fast(can, ch, x, y, col, bgcol, scalex, scaley); }
void DrawCharBg_Fast(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanCharBg_Fast(pDrawCan, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character with background (checks clipping limits)
void DrawCanCharBg(sDrawCan* can, char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { can->drawfnc->pDrawCanCharBg(can, ch, x, y, col, bgcol, scalex, scaley); }

// For backward compatibility with Draw version 1
//void DrawCharBg(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanCharBg(pDrawCan, ch, x, y, col, bgcol, scalex, scaley); }
void DrawCharBgSize(char ch, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanCharBg(pDrawCan, ch, x, y, col, bgcol, scalex, scaley); }

// Draw character inverted (does not check clipping limits)
void DrawCanCharInv_Fast(sDrawCan* can, char ch, int x, int y, int scalex, int scaley) { can->drawfnc->pDrawCanCharInv_Fast(can, ch, x, y, scalex, scaley); }
void DrawCharInv_Fast(char ch, int x, int y, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanCharInv_Fast(pDrawCan, ch, x, y, scalex, scaley); }

// Draw character inverted (checks clipping limits)
void DrawCanCharInv(sDrawCan* can, char ch, int x, int y, int scalex, int scaley) { can->drawfnc->pDrawCanCharInv(can, ch, x, y, scalex, scaley); }
void DrawCharInv(char ch, int x, int y, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanCharInv(pDrawCan, ch, x, y, scalex, scaley); }

// ----- Draw text
//  len ... text length, or -1 to detect ASCIIZ length
//  scalex ... scale size in X direction (> 0)
//  scaley ... scale size in Y direction (> 0)

// Draw text with transparent background (does not check clipping limits)
void DrawCanText_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { can->drawfnc->pDrawCanText_Fast(can, text, len, x, y, col, scalex, scaley); }
void DrawText_Fast(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanText_Fast(pDrawCan, text, len, x, y, col, scalex, scaley); }

// Draw text with transparent background (checks clipping limits)
void DrawCanText(sDrawCan* can, const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { can->drawfnc->pDrawCanText(can, text, len, x, y, col, scalex, scaley); }

// For backward compatibility with Draw version 1
//void DrawText(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanText(pDrawCan, text, len, x, y, col, scalex, scaley); }
void DrawTextSize(const char* text, int len, int x, int y, u16 col, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanText(pDrawCan, text, len, x, y, col, scalex, scaley); }

// Draw text with background (does not check clipping limits)
void DrawCanTextBg_Fast(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { can->drawfnc->pDrawCanTextBg_Fast(can, text, len, x, y, col, bgcol, scalex, scaley); }
void DrawTextBg_Fast(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanTextBg_Fast(pDrawCan, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text with background (checks clipping limits)
void DrawCanTextBg(sDrawCan* can, const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { can->drawfnc->pDrawCanTextBg(can, text, len, x, y, col, bgcol, scalex, scaley); }

// For backward compatibility with Draw version 1
//void DrawTextBg(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanTextBg(pDrawCan, text, len, x, y, col, bgcol, scalex, scaley); }
void DrawTextBgSize(const char* text, int len, int x, int y, u16 col, u16 bgcol, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanTextBg(pDrawCan, text, len, x, y, col, bgcol, scalex, scaley); }

// Draw text inverted (does not check clipping limits)
void DrawCanTextInv_Fast(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley) { can->drawfnc->pDrawCanTextInv_Fast(can, text, len, x, y, scalex, scaley); }
void DrawTextInv_Fast(const char* text, int len, int x, int y, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanTextInv_Fast(pDrawCan, text, len, x, y, scalex, scaley); }

// Draw text inverted (checks clipping limits)
void DrawCanTextInv(sDrawCan* can, const char* text, int len, int x, int y, int scalex, int scaley) { can->drawfnc->pDrawCanTextInv(can, text, len, x, y, scalex, scaley); }
void DrawTextInv(const char* text, int len, int x, int y, int scalex, int scaley) { pDrawCan->drawfnc->pDrawCanTextInv(pDrawCan, text, len, x, y, scalex, scaley); }

// ----- Draw ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... draw ellipse arcs, use combination of DRAWCAN_ELLIPSE_*; use DRAWCAN_ELLIPSE_ALL or 0x0f to draw whole ellipse

// Draw ellipse (does not check clipping limits)
void DrawCanEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask) { can->drawfnc->pDrawCanEllipse_Fast(can, x, y, dx, dy, col, mask); }
void DrawEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanEllipse_Fast(pDrawCan, x, y, dx, dy, col, mask); }

// Draw ellipse (checks clipping limits)
void DrawCanEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask) { can->drawfnc->pDrawCanEllipse(can, x, y, dx, dy, col, mask); }
void DrawEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanEllipse(pDrawCan, x, y, dx, dy, col, mask); }

// Draw ellipse inverted (does not check clipping limits)
void DrawCanEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask) { can->drawfnc->pDrawCanEllipseInv_Fast(can, x, y, dx, dy, mask); }
void DrawEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { pDrawCan->drawfnc->pDrawCanEllipseInv_Fast(pDrawCan, x, y, dx, dy, mask); }

// Draw ellipse inverted (checks clipping limits)
void DrawCanEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask) { can->drawfnc->pDrawCanEllipseInv(can, x, y, dx, dy, mask); }
void DrawEllipseInv(int x, int y, int dx, int dy, u8 mask) { pDrawCan->drawfnc->pDrawCanEllipseInv(pDrawCan, x, y, dx, dy, mask); }

// ----- Draw filled ellipse
//  dx ... diameter of ellipse in X direction, range 1..430 (X radius = dx/2)
//  dy ... diameter of ellipse in Y direction, range 1..430 (Y radius = dy/2)
//  mask ... hide parts of the ellipse with DRAWCAN_ROUND_* (or their combination); use DRAWCAN_ROUND_ALL or 0 to draw whole ellipse

// Draw filled ellipse (does not check clipping limits)
void DrawCanFillEllipse_Fast(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask) { can->drawfnc->pDrawCanFillEllipse_Fast(can, x, y, dx, dy, col, mask); }
void DrawFillEllipse_Fast(int x, int y, int dx, int dy, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanFillEllipse_Fast(pDrawCan, x, y, dx, dy, col, mask); }

// Draw filled ellipse (checks clipping limits)
void DrawCanFillEllipse(sDrawCan* can, int x, int y, int dx, int dy, u16 col, u8 mask) { can->drawfnc->pDrawCanFillEllipse(can, x, y, dx, dy, col, mask); }
void DrawFillEllipse(int x, int y, int dx, int dy, u16 col, u8 mask) { pDrawCan->drawfnc->pDrawCanFillEllipse(pDrawCan, x, y, dx, dy, col, mask); }

// Draw filled ellipse inverted (does not check clipping limits)
void DrawCanFillEllipseInv_Fast(sDrawCan* can, int x, int y, int dx, int dy, u8 mask) { can->drawfnc->pDrawCanFillEllipseInv_Fast(can, x, y, dx, dy, mask); }
void DrawFillEllipseInv_Fast(int x, int y, int dx, int dy, u8 mask) { pDrawCan->drawfnc->pDrawCanFillEllipseInv_Fast(pDrawCan, x, y, dx, dy, mask); }

// Draw filled ellipse inverte (checks clipping limits)
void DrawCanFillEllipseInv(sDrawCan* can, int x, int y, int dx, int dy, u8 mask) { can->drawfnc->pDrawCanFillEllipseInv(can, x, y, dx, dy, mask); }
void DrawFillEllipseInv(int x, int y, int dx, int dy, u8 mask) { pDrawCan->drawfnc->pDrawCanFillEllipseInv(pDrawCan, x, y, dx, dy, mask); }

// ----- Draw image
//  can ... destination canvas
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  src ... source image
//  xs ... source X coordinate
//  ys ... source Y coordinate
//  w ... source width to draw
//  h ... source height to draw
//  wbs ... pitch of source image (length of line in bytes)
//  col ... key transparent color

// Draw image with the same format as destination
void DrawCanImg(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { can->drawfnc->pDrawCanImg(can, xd, yd, src, xs, ys, w, h, wbs); }

// For backward compatibility with Draw version 1
//void DrawImg(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { pDrawCan->drawfnc->pDrawCanImg(pDrawCan, xd, yd, src, xs, ys, w, h, wbs); }
void DrawImg2(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { pDrawCan->drawfnc->pDrawCanImg(pDrawCan, xd, yd, src, xs, ys, w, h, wbs); }

// Draw image inverted with the same format as destination
void DrawCanImgInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { can->drawfnc->pDrawCanImgInv(can, xd, yd, src, xs, ys, w, h, wbs); }
void DrawImgInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs) { pDrawCan->drawfnc->pDrawCanImgInv(pDrawCan, xd, yd, src, xs, ys, w, h, wbs); }

// Draw transparent image with the same format as destination
void DrawCanBlit(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { can->drawfnc->pDrawCanBlit(can, xd, yd, src, xs, ys, w, h, wbs, col); }

// For backward compatibility with Draw version 1
//void DrawBlit(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { pDrawCan->drawfnc->pDrawCanBlit(pDrawCan, xd, yd, src, xs, ys, w, h, wbs, col); }
void DrawBlit2(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { pDrawCan->drawfnc->pDrawCanBlit(pDrawCan, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image inverted with the same format as destination
void DrawCanBlitInv(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { can->drawfnc->pDrawCanBlitInv(can, xd, yd, src, xs, ys, w, h, wbs, col); }
void DrawBlitInv(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col) { pDrawCan->drawfnc->pDrawCanBlitInv(pDrawCan, xd, yd, src, xs, ys, w, h, wbs, col); }

// Draw transparent image with the same format as destination, with substitute color
void DrawCanBlitSubs(sDrawCan* can, int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { can->drawfnc->pDrawCanBlitSubs(can, xd, yd, src, xs, ys, w, h, wbs, col, fnd, subs); }

// For backward compatibility with Draw version 1
void DrawBlitSubs(int xd, int yd, const void* src, int xs, int ys, int w, int h, int wbs, u16 col, u16 fnd, u16 subs) { pDrawCan->drawfnc->pDrawCanBlitSubs(pDrawCan, xd, yd, src, xs, ys, w, h, wbs, col, fnd, subs); }

// Get image from canvas to buffer
//  can ... source canvas
//  xs ... source X coordinate
//  ys ... source Y coordinate
//  w ... width
//  h ... height
//  dst ... destination buffer
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  wbd ... pitch of destination buffer (length of line in bytes)
void DrawCanGetImg(const sDrawCan* can, int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { can->drawfnc->pDrawCanGetImg(can, xs, ys, w, h, dst, xd, yd, wbd); }

// For backward compatibility with Draw version 1
//void DrawGetImg(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { pDrawCan->drawfnc->pDrawCanGetImg(pDrawCan, xs, ys, w, h, dst, xd, yd, wbd); }
void DrawGetImg2(int xs, int ys, int w, int h, void* dst, int xd, int yd, int wbd) { pDrawCan->drawfnc->pDrawCanGetImg(pDrawCan, xs, ys, w, h, dst, xd, yd, wbd); }

// Draw image with 2D transformation matrix
//  can ... destination canvas
//  xd ... destination X coordinate
//  yd ... destination Y coordinate
//  wd ... destination width
//  hd ... destination height
//  src ... source image
//  ws ... source image width
//  hs ... source image height
//  wbs ... pitch of source image (length of line in bytes)
//  m ... transformation matrix (should be prepared using PrepDrawImg function)
//  mode ... draw mode DRAWIMG_*
//  color ... key or border color (DRAWIMG_PERSP mode: horizon offset)
// Note to wrap and perspective mode: Width and height of source image should be power of 2, or it will render slower.
void DrawCanImgMat(sDrawCan* can, int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ can->drawfnc->pDrawCanImgMat(can, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

// For backward compatibility with Draw version 1
void DrawImgMat2D(int xd, int yd, int wd, int hd, const void* src, int ws, int hs, int wbs, const sMat2D* m, int mode, u16 color)
	{ pDrawCan->drawfnc->pDrawCanImgMat(pDrawCan, xd, yd, wd, hd, src, ws, hs, wbs, m, mode, color); }

// ----- Colors

// convert RGB888 color to pixel color
u16 DrawCanColRgb(sDrawCan* can, u8 r, u8 g, u8 b) { return can->drawfnc->pColRgb(r, g, b); }
u16 DrawColRgb(u8 r, u8 g, u8 b) { return pDrawCan->drawfnc->pColRgb(r, g, b); }

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// random pixel color
u16 DrawCanColRand(sDrawCan* can) { return can->drawfnc->pColRand(); }
u16 DrawColRand() { return pDrawCan->drawfnc->pColRand(); }
#endif

// base colors
u16 DrawCanColBlack(sDrawCan* can) { return can->drawfnc->col_black; }
u16 DrawColBlack() { return pDrawCan->drawfnc->col_black; }

u16 DrawCanColBlue(sDrawCan* can) { return can->drawfnc->col_blue; }
u16 DrawColBlue() { return pDrawCan->drawfnc->col_blue; }

u16 DrawCanColGreen(sDrawCan* can) { return can->drawfnc->col_green; }
u16 DrawColGreen() { return pDrawCan->drawfnc->col_green; }

u16 DrawCanColCyan(sDrawCan* can) { return can->drawfnc->col_cyan; }
u16 DrawColCyan() { return pDrawCan->drawfnc->col_cyan; }

u16 DrawCanColRed(sDrawCan* can) { return can->drawfnc->col_red; }
u16 DrawColRed() { return pDrawCan->drawfnc->col_red; }

u16 DrawCanColMagenta(sDrawCan* can) { return can->drawfnc->col_magenta; }
u16 DrawColMagenta() { return pDrawCan->drawfnc->col_magenta; }

u16 DrawCanColYellow(sDrawCan* can) { return can->drawfnc->col_yellow; }
u16 DrawColYellow() { return pDrawCan->drawfnc->col_yellow; }

u16 DrawCanColWhite(sDrawCan* can) { return can->drawfnc->col_white; }
u16 DrawColWhite() { return pDrawCan->drawfnc->col_white; }

u16 DrawCanColGray(sDrawCan* can) { return can->drawfnc->col_gray; }
u16 DrawColGray() { return pDrawCan->drawfnc->col_gray; }

u16 DrawCanColDkBlue(sDrawCan* can) { return can->drawfnc->col_dkblue; }
u16 DrawColDkBlue() { return pDrawCan->drawfnc->col_dkblue; }

u16 DrawCanColDkGreen(sDrawCan* can) { return can->drawfnc->col_dkgreen; }
u16 DrawColDkGreen() { return pDrawCan->drawfnc->col_dkgreen; }

u16 DrawCanColDkCyan(sDrawCan* can) { return can->drawfnc->col_dkcyan; }
u16 DrawColDkCyan() { return pDrawCan->drawfnc->col_dkcyan; }

u16 DrawCanColDkRed(sDrawCan* can) { return can->drawfnc->col_dkred; }
u16 DrawColDkRed() { return pDrawCan->drawfnc->col_dkred; }

u16 DrawCanColDkMagenta(sDrawCan* can) { return can->drawfnc->col_dkmagenta; }
u16 DrawColDkMagenta() { return pDrawCan->drawfnc->col_dkmagenta; }

u16 DrawCanColDkYellow(sDrawCan* can) { return can->drawfnc->col_dkyellow; }
u16 DrawColDkYellow() { return pDrawCan->drawfnc->col_dkyellow; }

u16 DrawCanColDkWhite(sDrawCan* can) { return can->drawfnc->col_dkwhite; }
u16 DrawColDkWhite() { return pDrawCan->drawfnc->col_dkwhite; }

u16 DrawCanColDkGray(sDrawCan* can) { return can->drawfnc->col_dkgray; }
u16 DrawColDkGray() { return pDrawCan->drawfnc->col_dkgray; }

u16 DrawCanColLtBlue(sDrawCan* can) { return can->drawfnc->col_ltblue; }
u16 DrawColLtBlue() { return pDrawCan->drawfnc->col_ltblue; }

u16 DrawCanColLtGreen(sDrawCan* can) { return can->drawfnc->col_ltgreen; }
u16 DrawColLtGreen() { return pDrawCan->drawfnc->col_ltgreen; }

u16 DrawCanColLtCyan(sDrawCan* can) { return can->drawfnc->col_ltcyan; }
u16 DrawColLtCyan() { return pDrawCan->drawfnc->col_ltcyan; }

u16 DrawCanColLtRed(sDrawCan* can) { return can->drawfnc->col_ltred; }
u16 DrawColLtRed() { return pDrawCan->drawfnc->col_ltred; }

u16 DrawCanColLtMagenta(sDrawCan* can) { return can->drawfnc->col_ltmagenta; }
u16 DrawColLtMagenta() { return pDrawCan->drawfnc->col_ltmagenta; }

u16 DrawCanColLtYellow(sDrawCan* can) { return can->drawfnc->col_ltyellow; }
u16 DrawColLtYellow() { return pDrawCan->drawfnc->col_ltyellow; }

u16 DrawCanColLtGray(sDrawCan* can) { return can->drawfnc->col_ltgray; }
u16 DrawColLtGray() { return pDrawCan->drawfnc->col_ltgray; }

u16 DrawCanColAzure(sDrawCan* can) { return can->drawfnc->col_azure; }
u16 DrawColAzure() { return pDrawCan->drawfnc->col_azure; }

u16 DrawCanColOrange(sDrawCan* can) { return can->drawfnc->col_orange; }
u16 DrawColOrange() { return pDrawCan->drawfnc->col_orange; }

u16 DrawCanColBrown(sDrawCan* can) { return can->drawfnc->col_brown; }
u16 DrawColBrown() { return pDrawCan->drawfnc->col_brown; }

#endif // USE_DRAWCAN0

#endif // USE_DRAWCAN
