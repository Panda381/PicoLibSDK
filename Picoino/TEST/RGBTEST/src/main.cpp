
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	int y;

	// draw rainbow gradient
	for (y = 0; y < HEIGHT; y++) GenGrad(&BackBuf[y*WIDTH], WIDTH);

	// draw text
	y = 102;
#define DX 24
	DrawRect(0, y, WIDTH, 24, COL_BLACK);
	y += 4;
	DrawText("Red", DX*1, y, COL_RED);
	DrawText("Yellow", DX*3-10, y, COL_YELLOW);
	DrawText("Green", DX*5-4, y, COL_GREEN);
	DrawText("Cyan", DX*7, y, COL_CYAN);
	DrawText("Blue", DX*9+3, y, COL_BLUE);
	DrawText("Magenta", DX*11-6, y, COL_MAGENTA);

	// gradient
	memcpy(&BackBuf[126*WIDTH], GradientImg, sizeof(GradientImg));

	DispUpdateAll();

	// reset to boot loader
	while (KeyGet() != KEY_Y) {}
#if USE_SCREENSHOT		// use screen shots
	ScreenShot();
#endif
	ResetToBootLoader();
}
