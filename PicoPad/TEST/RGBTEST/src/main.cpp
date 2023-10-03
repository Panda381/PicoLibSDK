
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

int main()
{
	int y = 0;

	pDrawFont = FontBold8x16; // font 8x8
	DrawFontHeight = 16; // font height

/*
	// draw pin gradients
#if USE_PICOPADVGA
#define DDY	8
	u16 m = 1;
	int x;
	for (x = 0; x < 5; x++) { DrawRect(x*64, y, 64, DDY, m); m <<= 1; } // blue components
	y += DDY;
	for (x = 0; x < 6; x++) { DrawRect(x*53, y, 53, DDY, m); m <<= 1; } // green components
	y += DDY;
	for (x = 0; x < 5; x++) { DrawRect(x*64, y, 64, DDY, m); m <<= 1; } // red components
	y += DDY;
	for (x = 0; x < 32; x++) { DrawRect(x*10, y, 10, DDY, (u16)x); } // blue gradient
	y += DDY;
	for (x = 0; x < 64; x++) { DrawRect(x*5, y, 5, DDY, (u16)(x<<5)); } // green gradient
	y += DDY;
	for (x = 0; x < 32; x++) { DrawRect(x*10, y, 10, DDY, (u16)(x<<11)); } // red gradient
	y += DDY;
#endif
*/
	// draw rainbow gradient
	for (; y < HEIGHT; y++) GenGrad(&FrameBuf[y*WIDTH], WIDTH);

	// draw text
	y = 106;
#define DX 24
	DrawRect(0, y, WIDTH, 20, COL_BLACK);
	y += 2;
	DrawText("Red", DX*1-14, y, COL_RED);
	DrawText("Yellow", DX*3-20, y, COL_YELLOW);
	DrawText("Green", DX*5-10, y, COL_GREEN);
	DrawText("Cyan", DX*7, y, COL_CYAN);
	DrawText("Blue", DX*9+3, y, COL_BLUE);
	DrawText("Magenta", DX*11-2, y, COL_MAGENTA);

	// gradient
//	memcpy(&FrameBuf[126*WIDTH], GradientImg, sizeof(GradientImg));

	// display update
	DispUpdateAll();

	// reset to boot loader
	while (KeyGet() != KEY_Y) { }
#if USE_SCREENSHOT		// use screen shots
	ScreenShot();
#endif
	ResetToBootLoader();
}
