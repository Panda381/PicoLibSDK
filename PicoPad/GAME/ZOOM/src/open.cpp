
// ****************************************************************************
//
//                                Opening
//
// ****************************************************************************

#include "include.h"

// open screen (returns False to quit the game)
Bool Open()
{
	int strip;

	// stop sounds
	StopAllSound();	// stop all sounds

	// new max. score
	Bool newmax = False;
	if (Score > MaxScore)
	{
		MaxScore = Score;
		newmax = True;
	}

	// initialize stars
	InitStars();

	// open loop
	while (True)
	{
		for (strip = DISP_STRIP_NUM; strip > 0; strip--)
		{
			// next strip
			DispSetStripNext();

			// clear screen
			DrawClear();

			// draw stars
			DispStars();

			// display score (if any)
			if (MaxScore > 0)
			{
				// display score
				DrawText("SCORE:", 3*8, 0, COL_WHITE);
				DecNum(DecNumBuf, Score>>FRAC, 0);
				DrawText(DecNumBuf, (3+6)*8, 0, newmax ? COL_GREEN : COL_WHITE);

				// display max. score
				DrawText("HIGH:", 26*8, 0, COL_WHITE);
				DecNum(DecNumBuf, MaxScore>>FRAC, 0);
				DrawText(DecNumBuf, (26+5)*8, 0, newmax ? COL_GREEN : COL_WHITE);
			}

			// display title
			DrawBlitPal(TitleImg, TitleImg_Pal, 0, 0, (WIDTH - TITLE_IMG_W)/2, 10,
				TITLE_IMG_W, TITLE_IMG_H, TITLE_IMG_W, TITLE_IMG_KEY);

			// display copyright
			DrawText("ORIGINAL COPYRIGHT 1984", (WIDTH - 23*8)/2, HEIGHT-38, COL_WHITE);
			DrawText("SEGA ENTERPRISES, INC.", (WIDTH - 22*8)/2, HEIGHT-30, COL_WHITE);
			DrawText("Version for Raspberry Pico", (WIDTH - 26*8)/2, HEIGHT-16, COL_YELLOW);
			DrawText("(c) 2023 Miroslav Nemecek", (WIDTH - 25*8)/2, HEIGHT-8, COL_YELLOW);

			// display update
			DispUpdate();
		}

		// set off back buffers
		DispSetStripOff();

#ifdef DEBUG_SKIP_OPEN		// skip open screen
		return True;
#endif

		// key
		switch (KeyGet())
		{

		// quit game
		case KEY_Y:
			return False;

		// start game
		case KEY_A:
			return True;
		}

		// shift stars
		ShiftStars();

		// generate new stars
		NewStars();
	}
}

