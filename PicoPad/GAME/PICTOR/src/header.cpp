
// ****************************************************************************
//
//                                  Header
//
// ****************************************************************************

#include "../include.h"

// current and max score
int Score = 0;
int MaxScore = 0;

// display current score
void DispScore()
{
	int len = MemPrint(DecNumBuf, DECNUMBUF_SIZE, "%07d", Score);
	DrawTextBg(DecNumBuf, 6*8+2, TIT_Y+2, (Score > MaxScore) ? COL_GREEN : COL_WHITE, HEADER_COL);
}

// display max. score
void DispMaxScore()
{
	int len = MemPrint(DecNumBuf, DECNUMBUF_SIZE, "%07d", (Score > MaxScore) ? Score : MaxScore);
	DrawTextBg(DecNumBuf, WIDTH-7*8-2, TIT_Y+2, (Score > MaxScore) ? COL_GREEN : COL_WHITE, HEADER_COL);
}

// display Picopad
void DispPico()
{
	int mask = BonusMask;

	// Picopad is complete
	if (mask == BONUS_MASK_ALL)
	{
		DrawBlit4Pal(PicopadImg, PicopadImg_Pal,	// image data and palettes
			0, 0,					// source X, Y
			(WIDTH - PICOPAD_W - (BONUS_BIGSCORE_LEN+1)*8)/2, TIT_Y + (TIT_HEIGHT - PICOPAD_H)/2, // destination X, Y
			PICOPAD_W, PICOPAD_H,			// width, height
			PICOPAD_WALL, PICOPAD_TRANS);		// source total width

		// display points
		DrawText(BONUS_BIGSCORE_TEXT,
			(WIDTH - PICOPAD_W - (BONUS_BIGSCORE_LEN+1)*8)/2 + PICOPAD_W + 8, TIT_Y + 2,
			COL_YELLOW);
	}
	else
	{
		int x = 6*8+2 + 7*8+2 + 15;
		int i = BONUS_COLFIRST;
		const sBonusTemp* b = &BonusTemp[i];
		for (; i <= BONUS_COLLAST; i++)
		{
			if ((mask & BIT(i)) != 0)
			{
				DrawBlit4Pal(b->img, b->pal,		// image data and palettes
					0, 0,					// source X, Y
					x, TIT_Y + (TIT_HEIGHT - b->h)/2, // destination X, Y
					b->w, b->h,			// width, height
					b->wall, b->trans);		// source total width
				x += b->w + 2;
			}
			b++;
		}
	}
}

// display header
void DispHeader()
{
	// background
	DrawRect(1, TIT_Y+1, WIDTH-2, TIT_HEIGHT-2, HEADER_COL);

	// frame
	DrawFrame(0, TIT_Y, WIDTH, TIT_HEIGHT, HEADER_FRAMECOL);

	// title SCORE
	DrawText("SCORE", 2, TIT_Y+2, COL_YELLOW);

	// title MAX
	DrawText("MAX", WIDTH-11*8-2, TIT_Y+2, COL_YELLOW);

	// display current score
	DispScore();

	// display max. score
	DispMaxScore();

	// display Picopad
	DispPico();
}
