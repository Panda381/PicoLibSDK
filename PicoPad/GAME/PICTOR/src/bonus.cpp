
// ****************************************************************************
//
//                                 Bonus
//
// ****************************************************************************

#include "../include.h"

// bonus templates
const sBonusTemp BonusTemp[BONUS_NUM] = {
	// pal			img		w		wall		h		trans
	{ HeartImg_Pal,		HeartImg,	HEART_W,	HEART_WALL,	HEART_H,	HEART_TRANS,	},
	{ Picopad1Img_Pal,	Picopad1Img,	PICOPAD1_W,	PICOPAD1_WALL,	PICOPAD1_H,	PICOPAD1_TRANS,	},
	{ Picopad2Img_Pal,	Picopad2Img,	PICOPAD2_W,	PICOPAD2_WALL,	PICOPAD2_H,	PICOPAD2_TRANS,	},
	{ Picopad3Img_Pal,	Picopad3Img,	PICOPAD3_W,	PICOPAD3_WALL,	PICOPAD3_H,	PICOPAD3_TRANS,	},
	{ Picopad4Img_Pal,	Picopad4Img,	PICOPAD4_W,	PICOPAD4_WALL,	PICOPAD4_H,	PICOPAD4_TRANS,	},
	{ PicopadImg_Pal,	PicopadImg,	PICOPAD_W,	PICOPAD_WALL,	PICOPAD_H,	PICOPAD_TRANS,	},
};

// bonuses in levels
int BonusLevel[BG_NUM] = {
	BONUS_TOP,		// 1 Meadow
	BONUS_CPU,		// 2 Jungle
	BONUS_HEART,		// 3 Sandy Beach
	BONUS_DISP,		// 4 Underwater
	BONUS_BAT,		// 5 Ice Land
	BONUS_HEART,		// 6 Fiery Land
	BONUS_TOP,		// 7 Haunted Hill
	BONUS_CPU,		// 8 Spacecraft
	BONUS_HEART,		// 9 Galaxy
	BONUS_DISP,		// 10 Alien Planet
	BONUS_BAT,		// 11 Candy Land
	BONUS_HEART,		// 12 Surreal Land
};

// current bonus
int BonusMask;			// mask of collected Picopad components (BONUS_MASK_*)
int BonusInx;			// index of current bonus (-1 if bonus is not active)
const sBonusTemp* Bonus;	// current bonus template
int BonusX;			// coordinate X of current bonus
int BonusY;			// coordinate Y of current bonus
int BonusPhase;			// bonus animation phase (0..2)

// initialize bonus on start of next level (requires initialized EnemyRandSeed)
void InitBonus()
{
	BonusInx = BonusLevel[BackInx];	// bonus index
	Bonus = &BonusTemp[BonusInx];	// bonus template
	BonusX = MaxFrame/2*BG_SPEED3 + WIDTH;	// X coordinate
	BonusY = EnemyRandMinMax(BG_Y_MIN + 100, BG_Y_MAX - 40); // random Y coordiate
	BonusPhase = 0;			// bonus animation phase
}

// shift bonus
void BonusShift()
{
	// shift animation phase
	BonusPhase++;
	if (BonusPhase >= BONUS_PHASES) BonusPhase = 0;

	// shift bonus X coordinate
	BonusX -= BG_SPEED3;
}

// display bonus
void BonusDisp()
{
	int x = BonusX;
	if ((x >= -30) && (BonusInx >= 0) && (x < WIDTH + 30))
	{
		int inx = BonusInx;
		int y = BonusY;
		const sBonusTemp* b = Bonus;
		
		// draw cloud
		DrawBlit4Pal(BonuscloudImg, BonuscloudImg_Pal,	// image data and palettes
			BonusPhase * BONUS_W, 0,		// source X, Y
			x - BONUS_W/2, y - BONUS_H/2, 		// destination X, Y
			BONUS_W, BONUS_H,			// width, height
			BONUS_WALL, BONUS_TRANS);		// source total width

		// draw bonus
		DrawBlit4Pal(b->img, b->pal,			// image data and palettes
			0, 0,					// source X, Y
			x - b->w/2, y - b->h/2,			// destination X, Y
			b->w, b->h,				// width, height
			b->wall, b->trans);			// source total width
	}
}

// collect bonus
void BonusCollect()
{
	int strip;
	int x = BonusX;
	if ((x >= -30) && (BonusInx >= 0) && (x < WIDTH + 30))
	{
		// check bonus collision
		if (	(ActorX + Actor.w/2 >= x) &&
			(ActorX - Actor.w/2 < x) &&
			(ActorY + Actor.h/2 >= BonusY) &&
			(ActorY - Actor.h/2 < BonusY))
		{
			// add score
			if (BonusInx == BONUS_HEART)
				Life += 100;
			else
			{
				Score += BONUS_SCORE;
				AddPoint(BONUS_SCORE, BonusX, BonusY);
			}

			// sound
			if (SoundMode != SOUNDMODE_OFF)
				PlaySoundChan(SOUNDCHAN_ACTORHIT, ZingSnd, 2*count_of(ZingSnd), False, 1, 1, SNDFORM_ADPCM, IMA_SAMPBLOCK);

			if ((BonusInx >= BONUS_COLFIRST) && (BonusInx <= BONUS_COLLAST))
			{
				BonusMask |= BIT(BonusInx);
				if (BonusMask == BONUS_MASK_ALL)
				{
					// Picopad is complete
					Score += BONUS_BIGSCORE;
					for (strip = DISP_STRIP_NUM; strip > 0; strip--)
					{
						DispSetStripNext();
						DispHeader();
						DispUpdate();
					}

					if (SoundMode != SOUNDMODE_OFF)
						PlaySoundChan(SOUNDCHAN_ACTORHIT, BigbonusSnd, 2*count_of(BigbonusSnd), False, 1, 1, SNDFORM_ADPCM, IMA_SAMPBLOCK);
					WaitMs(1000);

					BonusMask = 0;
					for (strip = DISP_STRIP_NUM; strip > 0; strip--)
					{
						DispSetStripNext();
						DispHeader();
						DispUpdate();
					}
				}
			}

			// delete bonus
			BonusInx = -1;
		}
	}
}
