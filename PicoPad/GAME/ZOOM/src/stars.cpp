
// ****************************************************************************
//
//                                 Stars
//
// ****************************************************************************

#include "include.h"

sStar	Stars[STARS];	// stars
u32	LastStarTime;	// time of last star

// generate one new star
void NewStar1()
{
	// time of last star
	LastStarTime += STARTIME;

	// search free star slot
	sStar* s = Stars;
	int i;
	for (i = 0; i < STARS; i++)
	{
		if (s->x == STARFREE) break;
		s++;
	}

	// no free slot
	if (i == STARS) return;

	// initialize coordinate to middle of the screen
	s->x = (WIDTH/2) << FRAC;
	s->y = (HEIGHT/2) << FRAC;

	// random speed and direction of the star
	int d = RandU16MinMax(STARMIN, STARMAX);
	float a = RandFloatMax(PI2);
	s->dx = (short)(d*sinf(a));
	s->dy = (short)(d*cosf(a));
}

// generate new stars
void NewStars()
{
	while ((s32)(Time() - LastStarTime) > 0) NewStar1();
}

// initialize stars
void InitStars()
{
	LastStarTime = Time();
	int i;
	for (i = 0; i < STARS; i++) Stars[i].x = STARFREE;
}

// shift stars
void ShiftStars()
{
	sStar* s = Stars;
	int i;
	for (i = 0; i < STARS; i++)
	{
		// shift coordinates
		s->x += s->dx;
		s->y += s->dy;

		// star is out of screen
		if (	(s->x < (-4 << FRAC)) ||
			(s->y < (-4 << FRAC)) ||
			(s->x > ((WIDTH+4) << FRAC)) ||
			(s->y > ((HEIGHT+4) << FRAC)))
		{
			// invalidate star
			s->x = STARFREE;
		}
		s++;
	}
}

// display stars
void DispStars()
{
	sStar* s = Stars;
	int i;
	int k1, k2;
	for (i = 0; i < STARS; i++)
	{
		if (s->x != STARFREE)
		{
			k1 = (s->x >> FRAC) - WIDTH/2;
			k2 = (s->y >> FRAC) - HEIGHT/2;
			k1 = k1*k1 + k2*k2; // max. 160*160 + 120*120 = 40000 (distance from the screen center)
			k1 >>= 5; // star bright
			if (k1 > 255) k1 = 255;
			DrawPoint(s->x >> FRAC, s->y >> FRAC, COLOR(k1, k1, k1));
		}
		s++;
	}
}
