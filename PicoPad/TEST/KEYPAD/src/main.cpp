
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

#define BTNW	50	// button width
#define BTNH	34	// button height
#define FRAME	2	// frame width

void DispKey(int x, int y, const char* txt, Bool hold)
{
	x -= BTNW/2;
	y -= BTNH/2;

	// prepare color
	u16 fgcol = COL_BLACK;
	u16 bgcol = COL_WHITE;
	if (hold)
	{
		fgcol = COL_YELLOW;
		bgcol = COL_BLUE;
	}

	// draw background
	DrawRect(x, y, BTNW, BTNH, bgcol);

	// draw frame
	DrawFrame(x+FRAME, y+FRAME, BTNW-2*FRAME, BTNH-2*FRAME, fgcol);

	// draw text
	int len = StrLen(txt);
	DrawText(txt, x + (BTNW-len*FONTW)/2, y + (BTNH-FONTH)/2, fgcol);
}

int main()
{
	// clear screen
	DrawClearCol(COL_DKGREEN);

	// Y hold counter
	u32 t = Time();
	DrawText("Hold Y for 2 seconds to quit", (WIDTH-28*FONTW)/2, HEIGHT-FONTH, COL_YELLOW);

	while (True)
	{
		// left
		DispKey(WIDTH/8*1, HEIGHT/4*2, "Left", KeyPressed(KEY_LEFT));

		// up
		DispKey(WIDTH/8*2, HEIGHT/4*1, "Up", KeyPressed(KEY_UP));

		// right
		DispKey(WIDTH/8*3, HEIGHT/4*2, "Right", KeyPressed(KEY_RIGHT));

		// down
		DispKey(WIDTH/8*2, HEIGHT/4*3, "Down", KeyPressed(KEY_DOWN));

		// A
		DispKey(WIDTH/8*5, HEIGHT/8*6, "A", KeyPressed(KEY_A));

		// B
		DispKey(WIDTH/8*7, HEIGHT/8*5, "B", KeyPressed(KEY_B));

		// X
		DispKey(WIDTH/8*5, HEIGHT/8*3, "X", KeyPressed(KEY_X));

		// Y
		DispKey(WIDTH/8*7, HEIGHT/8*2, "Y", KeyPressed(KEY_Y));

		// update display
		DispUpdate();

		// update Y hold counter
		if (!KeyPressed(KEY_Y))
			t = Time();
		else
		{
			if ((s32)(Time() - t) >= 2000000)
			{
				DrawClear();
				DispUpdate();
				ResetToBootLoader();
			}
		}

#if USE_SCREENSHOT		// use screen shots
		if (KeyGet() == KEY_X) ScreenShot();
#endif
	}
}
