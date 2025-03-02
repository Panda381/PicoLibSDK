
// ****************************************************************************
//
//  DVI (HDMI) and VGA display driver for Pico2 RP2350, using HSTX peripheral
//                                   Demo
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico/Pico2 and RP2040/RP2350
// Copyright (c) 2023-2025 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include "../include.h"

#define DISPVMODEINFO		0	// 1=display videomode info (debug)

// halt on memory error, blinking internal LED on Pico2 board (not on Wifi version)
#define CHECK_ERR() while (res != DISPHSTX_ERR_OK) { GPIO_Flip(LED_PIN); WaitMs(100); }

#if DISPVMODEINFO	// 1=display videomode info (debug)
// Display videomode info (debug)
void DispVModeInfo()
{
	// Draw border marks (to check image clipping)
	DrawTextBg("[*]", 0, 0, COL_WHITE, COL_BLACK);			// left-top mark
	DrawTextBg("[*]", pDrawCan->w - 3*8, 0, COL_WHITE, COL_BLACK);	// right-top mark
	DrawTextBg("[*]", pDrawCan->w - 3*8, pDrawCan->h - 16, COL_WHITE, COL_BLACK); // right-bottom mark

	// Draw videomode info
	char tt[100];
	MemPrint(tt, 100, "%s %dx%d/%db %dMHz %'d bytes",
		(DispHstxDispMode == DISPHSTX_DISPMODE_VGA) ? "VGA" : "DVI",	// display mode
		pDrawCan->w, pDrawCan->h, pDrawCan->colbit,			// videomode size and bits
		(ClockGetHz(CLK_SYS)+500000)/1000000,				// system clock
		pDrawCan->wb * pDrawCan->h);					// memory size
	DrawTextBg(tt, 0, pDrawCan->h-2*16, COL_GREEN, COL_BLACK);

	MemPrint(tt, 100, "vreg=%.2fV clkdiv=%d tmp=%d`C",
		VregVoltageFloat(), FlashClkDiv(), (int)(ADC_Temp()+0.5));
	DrawTextBg(tt, 0, pDrawCan->h-1*16, COL_GREEN, COL_BLACK);
}
#endif

int main()
{
	// main loop
	while (True)
	{

	// ==== Initialize videomode 640x480

		// initialize videomode 640x480
		int res = DispVMode640x480x8(0, NULL);
		CHECK_ERR();

	// ==== Draw "Hello" screen

		// clear screen with blue background
		DrawClearCol(COL_BLUE);

		// draw red frame around the screen
		DrawFrameW(0, 0, DrawWidth(), DrawHeight(), COL_RED, 5);

		// draw text message
		const char msg[] = "Hello DispHSTX!";
		DrawTextSize(msg, -1, (DrawWidth() - StrLen(msg)*8*3)/2, (DrawHeight() - 16*4)/2, COL_YELLOW, 3, 4);

#if DISPVMODEINFO	// 1=display videomode info (debug)
		// Display videomode info (debug)
		DispVModeInfo();
#endif

		// delay 5 seconds
		WaitMs(5000);

	// ==== Draw image

		// draw logo image
		DrawImg(ImgLogo, 0, 0, 0, 0, 640, 480, 640);

#if DISPVMODEINFO	// 1=display videomode info (debug)
		// Display videomode info (debug)
		DispVModeInfo();
#endif

		// delay 5 seconds
		WaitMs(5000);

	// ==== Drawing screen

#define RAND() { x = RandS16MinMax(-100, 700); \
		y = RandS16MinMax(-100, 600);	\
		w = RandU16MinMax(10, 100);	\
		h = RandU16MinMax(10, 100);	\
		col = COL8_RANDOM; }

		int k, x, y, w, h, col;
		for (k = 7000; k > 0; k--) // loop about 10 seconds
		{
			// Draw rectangle
			RAND(); DrawRect(x, y, w, h, col);

			// Draw frame
			RAND(); DrawFrameW(x, y, w, h, col, RandU8MinMax(1,5));

			// Draw line
			RAND(); DrawLineW(x, y, x+w-50, y+h-50, col, RandU8MinMax(1,5), 1);

			// Draw round
			RAND(); DrawRound(x, y, w, col, RandU8());

			// Draw circle
			RAND(); DrawCircleArc(x, y, w, col, RandU8());

			// Draw ring
			RAND(); DrawRingMask(x, y, w, h, col, RandU8());

			// Draw triangle
			RAND(); DrawTriangle(x, y, x+w-50, y+h-50, x+h-50, y+w-50, col);

			// Draw character
			RAND(); DrawCharSize(RandU8(), x, y, col, RandU8MinMax(1,2), RandU8MinMax(1,2));

			// Draw text
			RAND(); DrawText("DispHSTX", x, y, col);

			// Draw ellipse
			RAND(); DrawEllipse(x, y, w, h, col, RandU8());

			// Draw filled ellipse
			RAND(); DrawFillEllipse(x, y, w, h, col, RandU8());

#if DISPVMODEINFO	// 1=display videomode info (debug)
			// Display videomode info (debug)
			DispVModeInfo();
#endif

			// short delay
			WaitMs(1);
		}

		// terminate all current devices and free frame buffer
		DispHstxAllTerm();
		DispHstxFreeBuf();

	// ==== Multi-slot screen

		// create buffers
		u8* buf_font = (u8*)malloc(sizeof(FontBold8x16)); // font buffer (must be in RAM)
		void* buf_text = malloc(20*1); // text screen - 1 row with 20 characters
		void* buf_img = malloc(Draw16Pitch(532)*400); // image screen 532x400/16-bit
		void* buf_pat = malloc(56*54); // create pattern buffer
		u16* buf_pal = (u16*)malloc(256*2); // pattern palettes

		while ((buf_font == NULL) || (buf_text == NULL) || (buf_img == NULL) ||
			(buf_pat == NULL) || (buf_pal == NULL)) { GPIO_Flip(LED_PIN); WaitMs(100); }

		memcpy(buf_font, FontBold8x16, sizeof(FontBold8x16)); // copy text font
		memcpy(buf_text, "  MultiSlot Screen  ", 20); // set text to text screen
		memcpy(buf_pat, ImgPattern, 56*54); // copy pattern to RAM
		memcpy(buf_pal, ImgPattern_Pal, 256*2); // copy pattern palettes to RAM

		// Initialize videomode state descriptor (set timings to 640x480@60Hz)
		sDispHstxVModeState* vmode = &DispHstxVMode; // pointer to default descriptor
		DispHstxVModeInitTime(vmode, &DispHstxVModeTimeList[vmodetime_640x480_fast]);

		// add strip "0" for text screen (height 80 lines)
		res = DispHstxVModeAddStrip(vmode, 80);
		CHECK_ERR();

		// add text screen slot "0" (width 640 pixels = 20 chars of width 32 pixels)
		u16 pal_text[2] = { COL16_DKGREEN, COL16_YELLOW };
		u32 pal_vga[2*2];
		res = DispHstxVModeAddSlot(vmode, 4, 5, -1, DISPHSTX_FORMAT_MTEXT,
			buf_text, 20, pal_text, pal_vga, buf_font, 16, 0, 0);
		CHECK_ERR();

		// add strip "1" for graphics screen
		res = DispHstxVModeAddStrip(vmode, 400);
		CHECK_ERR();

		// add pattern screen slot "1-0"
		res = DispHstxVModeAddSlot(vmode, 1, 1, 54, DISPHSTX_FORMAT_PAT_8_PAL,
			buf_pat, 56, buf_pal, DispHstxDefVgaPal, NULL, 54, 0, 0);
		CHECK_ERR();

		// add graphics screen slot "1-1" - 532x400/16-bit
		res = DispHstxVModeAddSlot(vmode, 1, 1, 532, DISPHSTX_FORMAT_16,
			buf_img, Draw16Pitch(532), NULL, NULL, NULL, 0, 0, 0);
		CHECK_ERR();

		// add pattern screen slot "1-2"
		res = DispHstxVModeAddSlot(vmode, 1, 1, 54, DISPHSTX_FORMAT_PAT_8_PAL,
			buf_pat, 56, buf_pal, DispHstxDefVgaPal, NULL, 54, 0, 0);
		CHECK_ERR();

		// activate videomode
		DispHstxSelDispMode(DISPHSTX_DISPMODE_NONE, vmode);

		// link graphics screen "1-1" to 16-bit draw canvas
		DispHstxLinkCan(&vmode->strip[1].slot[1], &DrawCan16);

		// display image to 16-bit canvas
		Draw16Img(0, 0, Img532x400_16b, 0, 0, 532, 400, 1064);

		// delay 10 seconds
		WaitMs(10000);

		// terminate all current devices
		DispHstxAllTerm();

		// free buffers
		free(buf_font);
		free(buf_text);
		free(buf_img);
		free(buf_pat);
		free(buf_pal);

	// ==== Multi-format screen

#define GAP 20			// gap between slots
#define COL COL16_BLACK		// color of the gap

		// create buffers
		u8* fnt = (u8*)malloc(sizeof(FontBold8x16)); // font buffer (must be in RAM)
		void* txt = malloc(20*1); // text screen - 1 row with 20 characters
		while ((fnt==NULL)||(txt==NULL)) { GPIO_Flip(LED_PIN); WaitMs(100); }
		memcpy(fnt, FontBold8x16, sizeof(FontBold8x16)); // copy text font
		memcpy(txt, " MultiFormat Screen ", 20); // set text to text screen

		// Initialize videomode state descriptor (set timings to 640x480@60Hz)
		sDispHstxVModeState* v = &DispHstxVMode; // pointer to default descriptor
		DispHstxVModeInitTime(v, &DispHstxVModeTimeList[vmodetime_640x480_fast]);

		// add strip "0" for text screen (height 30 lines)
		res = DispHstxVModeAddStrip(v, 30);
		CHECK_ERR();

		// add text screen slot "0" (width 640 pixels = 20 chars of width 32 pixels)
		u16 palt[2] = { COL16_DKBLUE, COL16_YELLOW };
		u32 palv[2*2];
		res = DispHstxVModeAddSlot(v, 4, 2, -1, DISPHSTX_FORMAT_MTEXT,
			txt, 20, palt, palv, fnt, 16, 0, 0);
		CHECK_ERR();

		// add strip "1" for 1st graphics row
		res = DispHstxVModeAddStrip(v, 150);
		CHECK_ERR();

		// add color format 1
		res = DispHstxVModeAddSlot(v, 1, 1, 200, DISPHSTX_FORMAT_1_PAL,
			NULL, 28, NULL, NULL, NULL, 0, COL, GAP);
		CHECK_ERR();
		DispHstxLinkCan(&v->strip[1].slot[0], &DrawCan1);
		Draw1Img(0, 0, Img1b, 0, 0, 200, 150, 28);

		// add color format 2
		res = DispHstxVModeAddSlot(v, 1, 1, 200, DISPHSTX_FORMAT_2,
			NULL, 56, NULL, NULL, NULL, 0, COL, GAP);
		CHECK_ERR();
		DispHstxLinkCan(&v->strip[1].slot[1], &DrawCan2);
		Draw2Img(0, 0, Img2b, 0, 0, 200, 150, 56);

		// add color format 3
		res = DispHstxVModeAddSlot(v, 1, 1, 200, DISPHSTX_FORMAT_3_PAL,
			NULL, 88, NULL, NULL, NULL, 0, COL, GAP);
		CHECK_ERR();
		DispHstxLinkCan(&v->strip[1].slot[2], &DrawCan3);
		Draw3Img(0, 0, Img3b, 0, 0, 200, 150, 88);

		// add strip "2" for 2nd graphics row
		res = DispHstxVModeAddStrip(v, 150);
		CHECK_ERR();

		// add color format 4
		res = DispHstxVModeAddSlot(v, 1, 1, 200, DISPHSTX_FORMAT_4_PAL,
			NULL, 108, NULL, NULL, NULL, 0, COL, GAP);
		CHECK_ERR();
		DispHstxLinkCan(&v->strip[2].slot[0], &DrawCan4);
		Draw4Img(0, 0, Img4b, 0, 0, 200, 150, 108);

		// add color format 6
		res = DispHstxVModeAddSlot(v, 1, 1, 200, DISPHSTX_FORMAT_6_PAL,
			NULL, 172, NULL, NULL, NULL, 0, COL, GAP);
		CHECK_ERR();
		DispHstxLinkCan(&v->strip[2].slot[1], &DrawCan6);
		Draw6Img(0, 0, Img6b, 0, 0, 200, 150, 172);

		// add color format 8
		res = DispHstxVModeAddSlot(v, 1, 1, 200, DISPHSTX_FORMAT_8,
			NULL, 216, NULL, NULL, NULL, 0, COL, GAP);
		CHECK_ERR();
		DispHstxLinkCan(&v->strip[2].slot[2], &DrawCan8);
		Draw8Img(0, 0, Img8b, 0, 0, 200, 150, 216);

		// add strip "3" for 3rd graphics row
		res = DispHstxVModeAddStrip(v, 150);
		CHECK_ERR();

		// add color format 12
		res = DispHstxVModeAddSlot(v, 1, 1, 200, DISPHSTX_FORMAT_12,
			NULL, 324, NULL, NULL, NULL, 0, COL, GAP);
		CHECK_ERR();
		DispHstxLinkCan(&v->strip[3].slot[0], &DrawCan12);
		Draw12Img(0, 0, Img12b, 0, 0, 200, 150, 324);

		// add color format 15
		res = DispHstxVModeAddSlot(v, 1, 1, 200, DISPHSTX_FORMAT_15,
			NULL, 428, NULL, NULL, NULL, 0, COL, GAP);
		CHECK_ERR();
		DispHstxLinkCan(&v->strip[3].slot[1], &DrawCan15);
		Draw15Img(0, 0, Img15b, 0, 0, 200, 150, 428);

		// add color format 16
		res = DispHstxVModeAddSlot(v, 1, 1, 200, DISPHSTX_FORMAT_16,
			NULL, 428, NULL, NULL, NULL, 0, COL, GAP);
		CHECK_ERR();
		DispHstxLinkCan(&v->strip[3].slot[2], &DrawCan16);
		Draw16Img(0, 0, Img16b, 0, 0, 200, 150, 428);

		// activate videomode
		DispHstxSelDispMode(DISPHSTX_DISPMODE_NONE, v);

		// delay 10 seconds
		WaitMs(10000);

		// terminate all current devices
		DispHstxAllTerm();

		// free buffers
		int i, j;
		for (i = 0; i < v->stripnum; i++)
			for (j = 0; j < v->strip[i].slotnum; j++)
				free(v->strip[i].slot[j].buf);		
		free(fnt);
	}
}
