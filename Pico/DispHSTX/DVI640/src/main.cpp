
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

int main()
{
	int x, y, len, i;
	char buf[50];

	// initialize videomode (must be 3-bit videomode)
	int res = VMODEFNC(0, NULL);
	while (res != DISPHSTX_ERR_OK) { GPIO_Flip(LED_PIN); WaitMs(100); }

	// clear screen - white color
	Draw3ClearCol(COL3_GRAY);

	// draw vertical lines
#define X0 (WIDTH/2 % 32)
#define DX 32
	for (x = 0; x <= WIDTH + X0; x += DX)
	{
		Draw3VLine(x-1 + X0, 0, HEIGHT, COL3_BLUE);
		Draw3VLine(x + X0, 0, HEIGHT, COL3_BLUE);
	}
	
	// draw horizontal lines
#define Y0 (HEIGHT/2 % 32)
#define DY 32
	for (y = 0; y <= HEIGHT + Y0; y += DY)
	{
		Draw3HLine(0, y-1 + Y0, WIDTH, COL3_BLUE);
		Draw3HLine(0, y + Y0, WIDTH, COL3_BLUE);
	}

	// draw big circle
#if WIDTH > HEIGHT
#define D HEIGHT
#else
#define D WIDTH
#endif
	Draw3CircleInv(WIDTH/2, HEIGHT/2, D, DRAWCAN_CIRCLE_ALL);

	// draw small circles
#define D2 (3*32)
	Draw3CircleInv(D2/2, D2/2, D2, DRAWCAN_CIRCLE_ALL);
	Draw3CircleInv(WIDTH-D2/2, D2/2, D2, DRAWCAN_CIRCLE_ALL);
	Draw3CircleInv(D2/2, HEIGHT-D2/2, D2, DRAWCAN_CIRCLE_ALL);
	Draw3CircleInv(WIDTH-D2/2, HEIGHT-D2/2, D2, DRAWCAN_CIRCLE_ALL);

	// draw edge triangles
#define TT 16
	Draw3Triangle(WIDTH/2, 0, WIDTH/2-TT, TT, WIDTH/2+TT, TT, COL3_BLACK);
	Draw3Triangle(WIDTH/2, HEIGHT-1, WIDTH/2-TT, HEIGHT-TT-1, WIDTH/2+TT, HEIGHT-TT-1, COL3_BLACK);
	Draw3Triangle(0, HEIGHT/2, TT, HEIGHT/2-TT, TT, HEIGHT/2+TT, COL3_BLACK);
	Draw3Triangle(WIDTH-1, HEIGHT/2, WIDTH-1-TT, HEIGHT/2-TT, WIDTH-1-TT, HEIGHT/2+TT, COL3_BLACK);

	// draw color bars
#define CCX (2*32)
#if HEIGHT > 400
#define CCY (3*32)
#else
#define CCY (2*32)
#endif
#define CY (HEIGHT/2 - CCY - 32)
#define CX (WIDTH/2 - 3*CCX)
	Draw3Rect(CX+0*CCX, CY, CCX, CCY, COL3_BLUE);
	Draw3Rect(CX+1*CCX, CY, CCX, CCY, COL3_GREEN);
	Draw3Rect(CX+2*CCX, CY, CCX, CCY, COL3_CYAN);
	Draw3Rect(CX+3*CCX, CY, CCX, CCY, COL3_RED);
	Draw3Rect(CX+4*CCX, CY, CCX, CCY, COL3_MAGENTA);
	Draw3Rect(CX+5*CCX, CY, CCX, CCY, COL3_YELLOW);

	// draw vertical lines
#define CY2 (HEIGHT/2 + 32)
	Draw3Rect(CX, CY2, CCX*6, CCY, COL3_BLACK);
	for (i = 0; i < CCX; i += 2) Draw3Rect(CX+0*CCX+i, CY2, 1, CCY, COL3_WHITE);
	for (i = 0; i < CCX; i += 4) Draw3Rect(CX+1*CCX+i, CY2, 2, CCY, COL3_WHITE);
	for (i = 0; i < CCX; i += 8) Draw3Rect(CX+2*CCX+i, CY2, 4, CCY, COL3_WHITE);
	for (i = 0; i < CCX; i += 16) Draw3Rect(CX+3*CCX+i, CY2, 8, CCY, COL3_WHITE);
	for (i = 0; i < CCX; i += 32) Draw3Rect(CX+4*CCX+i, CY2, 16, CCY, COL3_WHITE);
	for (i = 0; i < CCX; i += 64) Draw3Rect(CX+5*CCX+i, CY2, 32, CCY, COL3_WHITE);

	// draw horizontal lines
#define HW (4*32)
#define HY (HEIGHT/2 + 32 + CCY + 32)
#define HH 32
#define HX (WIDTH/2 - 2*32)
	Draw3Rect(HX, HY, HW, HH, COL3_BLACK);
	for (i = 0; i < HH; i += 2) Draw3HLine(HX, HY+i, HW, COL3_WHITE);

	// test pattern (font size 8x8)
	const char* txt = "Test Pattern";
	len = strlen(txt);
	Draw3Rect(WIDTH/2 - len*8/2 - 2, HEIGHT/2 - 8, len*8 + 4, 16, COL3_BLACK);
	Draw3Text(txt, len, WIDTH/2 - len*8/2, HEIGHT/2 - 8/2, COL3_WHITE, 1, 1);

	// small test patterns (font size 8x8)
	Draw3Rect(32, 32+8, 32, 16, COL3_BLACK);
	Draw3Text(txt, 4, 32, 32+12,COL3_WHITE, 1, 1);

	Draw3Rect(WIDTH-2*32, 32+8, 32, 16, COL3_BLACK);
	Draw3Text(txt, 4, WIDTH-2*32, 32+12, COL3_WHITE, 1, 1);

	Draw3Rect(32, HEIGHT-2*32+8, 32, 16, COL3_BLACK);
	Draw3Text(txt, 4, 32, HEIGHT-2*32+12, COL3_WHITE, 1, 1);

	Draw3Rect(WIDTH-2*32, HEIGHT-2*32+8, 32, 16, COL3_BLACK);
	Draw3Text(txt, 4, WIDTH-2*32, HEIGHT-2*32+12, COL3_WHITE, 1, 1);

	// draw videomode descriptor (font size 16x16)
#if DISPHSTX_USE_DVI	// 1=use DVI (HDMI) support (DVI requires about 15 KB of RAM)
	len = MemPrint(buf, 50, "%dx%d DVI", WIDTH, HEIGHT);
#else
	len = MemPrint(buf, 50, "%dx%d VGA", WIDTH, HEIGHT);
#endif
	Draw3Rect(WIDTH/2 - 4*32, HEIGHT/2 - CCY - 3*32, 8*32, 32, COL3_BLACK);
	Draw3Text(buf, len, WIDTH/2 - len*16/2, HEIGHT/2 - CCY - 3*32 + 9, COL3_WHITE, 2, 2);

	while (True) {}
}
