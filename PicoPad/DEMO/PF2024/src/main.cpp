
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// sprite
typedef struct  {
	int		x, y;	// screen coordinates
	const u8*	img;	// pointer to image data
	const u16*	pal;	// pointer to image palettes
	int		w, h;	// sprite size
	int		tw;	// sprite total width
	int		num;	// number of sprite phases
	int		anim;	// number of animation loops
	int		delmin;	// minimal delay
	int		delmax;	// minimal delay
} sSprite;

// sprites
const sSprite Sprite[] = {
	// Moon
	{ 9, 19, MoonImg, MoonImg_Pal, MOONIMG_W, MOONIMG_H, MOONIMG_TW, 2, 1, 5, 45 },

	// dog
	{ 38, 128, DogImg, DogImg_Pal, DOGIMG_W, DOGIMG_H, DOGIMG_TW, 3, 10, 10, 100 },
	
	// mouse
	{ 10, 175, MouseImg, MouseImg_Pal, MOUSEIMG_W, MOUSEIMG_H, MOUSEIMG_TW, 3, 10, 10, 100 },

	// mole
	{ 212, 134, MoleImg, MoleImg_Pal, MOLEIMG_W, MOLEIMG_H, MOLEIMG_TW, 3, 10, 10, 100 },

	// hare
	{ 262, 164, HareImg, HareImg_Pal, HAREIMG_W, HAREIMG_H, HAREIMG_TW, 3, 1, 5, 80 },

	// bird
	{ 221, 212, BirdImg, BirdImg_Pal, BIRDIMG_W, BIRDIMG_H, BIRDIMG_TW, 2, 1, 5, 45 },

	// stars
	{ 23, 79, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },
	{ 63, 72, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },
	{ 88, 88, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },
	{ 87, 23, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },
	{ 119, 56, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },
	{ 136, 12, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },
	{ 184, 25, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },
	{ 200, 72, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },
	{ 231, 91, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },
	{ 279, 75, StarImg, StarImg_Pal, STARIMG_W, STARIMG_H, STARIMG_TW, 3, 1, 20, 120 },

	// sparks
	{ 152, 71, SparkImg, SparkImg_Pal, SPARKIMG_W, SPARKIMG_H, SPARKIMG_TW, 3, 1, 10, 60 },
	{ 159, 95, SparkImg, SparkImg_Pal, SPARKIMG_W, SPARKIMG_H, SPARKIMG_TW, 3, 1, 10, 60 },
	{ 144, 119, SparkImg, SparkImg_Pal, SPARKIMG_W, SPARKIMG_H, SPARKIMG_TW, 3, 1, 10, 60 },
	{ 119, 152, SparkImg, SparkImg_Pal, SPARKIMG_W, SPARKIMG_H, SPARKIMG_TW, 3, 1, 10, 60 },
	{ 176, 141, SparkImg, SparkImg_Pal, SPARKIMG_W, SPARKIMG_H, SPARKIMG_TW, 3, 1, 10, 60 },
};

#define SPRITENUM	count_of(Sprite)

// time of next animation
u32 NextAnim[SPRITENUM];

// counter of animation loops
int AnimLoop[SPRITENUM];
int AnimStep[SPRITENUM];

#define FRAME_X		210
#define FRAME_Y		10
#define FRAME_INX	(FRAME_X + FRAMEIMG_INX)
#define FRAME_INY	(FRAME_Y + FRAMEIMG_INY + 2)

// draw PF and update (pfshift = +FRAMEIMG_INW ... -PFALLIMG_TW)
void DrawPf(int pfshift)
{
	// draw frame
	DrawBlit4Pal(FrameImg, FrameImg_Pal, 0, 0, FRAME_X, FRAME_Y, FRAMEIMG_W, FRAMEIMG_H, FRAMEIMG_TW, TRANS_COL);

	// draw PF text
	if (pfshift >= 0)
		DrawImg4Pal(PfAllImg, PfAllImg_Pal, 0, 0, FRAME_INX+pfshift, FRAME_INY, FRAMEIMG_INW-pfshift, PFALLIMG_H, PFALLIMG_TW);
	else
		if (pfshift >= FRAMEIMG_INW - PFALLIMG_W)
			DrawImg4Pal(PfAllImg, PfAllImg_Pal, -pfshift, 0, FRAME_INX, FRAME_INY, FRAMEIMG_INW, PFALLIMG_H, PFALLIMG_TW);
		else
			DrawImg4Pal(PfAllImg, PfAllImg_Pal, -pfshift, 0, FRAME_INX, FRAME_INY, PFALLIMG_W+pfshift, PFALLIMG_H, PFALLIMG_TW);

	// update display
	DispUpdate();
}

int main()
{
	int i, strip;
	const sSprite* s;
	int comx = -400;
	int comy = -110;
	int pfshift = FRAMEIMG_INW;

	// play sound
	PLAYADPCMREP(MusicSnd, MusicSnd_SampBlock);

#define ANIM_DELTA	100	// animation delta time in [ms]

	// time of first animation
	s = Sprite;
	for (i = 0; i < SPRITENUM; i++)
	{
		NextAnim[i] = (RandU16MinMax(s->delmin, s->delmax) * ANIM_DELTA)*1000 + Time();
		AnimLoop[i] = s->anim;
		AnimStep[i] = 0;
		s++;
	}
	
	// main loop
	while (True)
	{
		for (strip = DISP_STRIP_NUM; strip > 0; strip--) // VGA loop all strips; LCD has only 1 strip
		{
			// next strip
			DispSetStripNext();

			// display background
			DrawImgRle(BackImg, BackImg_Pal, 0, 0, BACKIMG_W, BACKIMG_H);

			// draw sprites
			s = Sprite;
			for (i = 0; i < SPRITENUM; i++)
			{
				DrawBlit4Pal(s->img, s->pal, AnimStep[i] * s->w, 0, s->x, s->y, s->w, s->h, s->tw, TRANS_COL);
				s++;
			}

			// draw comet
			DrawBlit4Pal(CometImg, CometImg_Pal, (comy & 1) * COMETIMG_W, 0, comx, comy, COMETIMG_W, COMETIMG_H, COMETIMG_TW, TRANS_COL);

			// draw PF and update
			DrawPf(pfshift);
		}

		// animation
		s = Sprite;
		for (i = 0; i < SPRITENUM; i++)
		{
			// animation time reacher?
			if ((s32)(Time() - NextAnim[i]) >= 0)
			{
				// increase animation step
				AnimStep[i]++;
				if (AnimStep[i] >= s->num)
				{
					// one animation loop completed
					AnimStep[i] = 1;
					AnimLoop[i]--;
					if (AnimLoop[i] <= 0)
					{
						// prepare for next animation
						NextAnim[i] = (RandU16MinMax(s->delmin, s->delmax) * ANIM_DELTA)*1000 + Time();
						AnimLoop[i] = s->anim;
						AnimStep[i] = 0;
					}
				}
			}

			s++;
		}

		// comet animation
		comx += 4;
		comy += 1;
		if (comx > 450)
		{
			comx = -400;
			comy = -110;
		}

		// frame animation
		pfshift -= 2;
		if (pfshift < -PFALLIMG_TW) pfshift = FRAMEIMG_INW;

#if USE_PICOPADVGA
		WaitMs(30);
#endif

		// draw PF and update
		DrawPf(pfshift);

		// frame animation
		pfshift -= 2;
		if (pfshift < -PFALLIMG_TW) pfshift = FRAMEIMG_INW;

		// delay
		WaitMs(ANIM_DELTA-40);

		switch (KeyGet())
		{
		// reset to boot loader
		case KEY_Y: ResetToBootLoader();

#if USE_SCREENSHOT		// use screen shots
		case KEY_X:
			ScreenShot();
			break;
#endif
		}
	}
}
