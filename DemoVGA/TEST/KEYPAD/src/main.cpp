
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
	DrawText("Hold C for 2 seconds to quit", (WIDTH-28*FONTW)/2, HEIGHT-FONTH, COL_YELLOW);

	while (True)
	{
		// wait for VSync
		VgaWaitVSync();

		// C
		DispKey(WIDTH/4*1, HEIGHT/2, "C", KeyPressed(BTN_C));

		// B
		DispKey(WIDTH/4*2, HEIGHT/2, "B", KeyPressed(BTN_B));

		// A
		DispKey(WIDTH/4*3, HEIGHT/2, "A", KeyPressed(BTN_A));

		// update display
		DispUpdate();

		// update Y hold counter
		if (!KeyPressed(BTN_C))
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
	}
}
