
// ****************************************************************************
//
//                                Score
//
// ****************************************************************************

#include "include.h"

int	Score = 0;	// current score, with FRAC fraction
int	MaxScore = 0;	// max. score, with FRAC fraction
int	LastScore;	// score at last life addition, with FRAC fraction

// display score
void DispScore()
{
	int i, k, d;
	int n = Score>>FRAC;
	int x = DIGITS_IMG_W*(SCORE_DIGNUM-1) + 4;
	for (i = SCORE_DIGNUM; i > 0; i--)
	{
		// get next digit
		k = n/10;
		d = n - k*10;
		n = k;

		// draw digit
		DrawBlit4Pal(DigitsImg, DigitsImg_Pal, d*DIGITS_IMG_W, 0, x, 0,
			DIGITS_IMG_W, DIGITS_IMG_H, DIGITS_IMG_TOTW, DIGITS_IMG_KEY);
		x -= DIGITS_IMG_W;
	}
}

// increase score, with FRAC fraction
void AddScore(int add)
{
	Score += add;

	// increase lives
	if ((Score - LastScore) >= SCORE_LIVE)
	{
		// mark last score
		LastScore += SCORE_LIVE;

		// increase number of lives
		Lives++;

		// display all game screen
		DispAll();

		// play sound
		PLAYSOUND(LivesSnd);

		// pause
		Delay(2500);
	}
}
