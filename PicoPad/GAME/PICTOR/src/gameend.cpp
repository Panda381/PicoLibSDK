
// ****************************************************************************
//
//                               Game end
//
// ****************************************************************************

#include "../include.h"

char PlayerName[TOPNAME_LEN+1] = "        ";
int PlayerNameInx = 0;

// display game end
void DispGameEnd(Bool prompt)
{
#define TOPBUFN 30
	char buf[TOPBUFN];
	int strip;
	for (strip = DISP_STRIP_NUM; strip > 0; strip--)
	{
		DispSetStripNext();

		// clear screen
		DrawClear();

		// display title
		DrawText2("Game Over", (WIDTH - 9*16)/2, 0, COL_RED);

		// display top list
		DispTop();

		// high score
		if (Score > MaxScore)
		{
			int y = 35;
			MemPrint(buf, TOPBUFN, "New MaxScore %07d", Score);
			DrawTextW(buf, (WIDTH - 20*16)/2, y, COL_MAGENTA);
			if (prompt)
			{
				y += 15;
				DrawText("Enter your name and press A to continue", (WIDTH - 39*8)/2, y, COL_YELLOW);
				y += 15;
				DrawText2(PlayerName, (WIDTH - 8*16)/2, y, COL_AZURE);
				if (((Time() >> 18) & B0) == 0) DrawText2("_", (WIDTH - 8*16)/2 + PlayerNameInx*16, y + 4, COL_AZURE);
			}
		}
		else
		{
			MemPrint(buf, TOPBUFN, "Your Score %07d", Score);
			DrawTextW(buf, (WIDTH - 18*16)/2, 35, COL_MAGENTA);
			if (prompt) DrawText("Press any key...", (WIDTH - 16*8)/2, 70, COL_YELLOW);
		}

		// display update
		DispUpdate();
	}
}

// game end
void GameEnd()
{
	u8 key;
	char ch;
	int strip;

	// get name of best player
	memcpy(PlayerName, Top[0].name, TOPNAME_LEN);

	// display game end
	DispGameEnd(False);

	// stop sounds
	StopAllSound();

	// wait 1 second
	WaitMs(1000);

	// flush keys
	KeyFlush();

	while (True)
	{
		// display game end
		DispGameEnd(True);

		// get key
		key = KeyGet();

		// high score
		if (Score > MaxScore)
		{
			switch (key)
			{
			case KEY_LEFT:
				if (PlayerNameInx > 0) PlayerNameInx--;
				break;

			case KEY_RIGHT:
				if (PlayerNameInx < TOPNAME_LEN-1) PlayerNameInx++;
				break;

			case KEY_UP:
				ch = PlayerName[PlayerNameInx] + 1;
				if (ch == 33)
					ch = '0';
				else if (ch == '9'+1)
					ch = 'A';
				else if (ch == 'Z'+1)
					ch = ' ';
				PlayerName[PlayerNameInx] = ch;
				break;

			case KEY_DOWN:
				ch = PlayerName[PlayerNameInx] - 1;
				if (ch == 31)
					ch = 'Z';
				else if (ch == '0'-1)
					ch = ' ';
				else if (ch == 'A'-1)
					ch = '9';
				PlayerName[PlayerNameInx] = ch;
				break;

			case KEY_A:
				// move top table
				memmove(&Top[1], &Top[0], (TOP_NUM-1)*sizeof(sTop));

				// insert new score
				Top[0].score = Score;
				MaxScore = Score;
				memcpy(Top[0].name, PlayerName, TOPNAME_LEN);

				// save table
				while (!SaveTop())
				{
					// error
					for (strip = DISP_STRIP_NUM; strip > 0; strip--)
					{
						DispSetStripNext();
						DrawClear();
						DrawText2("Error writing score", 0, 50, COL_RED);
						DrawText2("to SD card. Press A", 0, 80, COL_RED);
						DrawText2("to repeat or Y to", 0, 110, COL_RED);
						DrawText2("ignore the error.", 0, 140, COL_RED);
						DispUpdate();
					}

					while (True)
					{
						key = KeyGet();
						if (key == KEY_Y) return;
						if (key == KEY_A) break;
					}
				}
				return;

			case KEY_Y:
				return;
			}		
		}

		// no high score
		else
		{
			if (key != NOKEY) return;
		}
	}
}
