
// ****************************************************************************
//
//                               Shooting
//
// ****************************************************************************

#include "include.h"

int	ShootSpeed;	// shoot speed
sShot	Shots[SHOTS];	// shots
u32	LastShotTime;	// time of last shoot

// reset shots on start of new scene
void ResetShots()
{
	int i;
	for (i = 0; i < SHOTS; i++) Shots[i].x = SHOTFREE;
}

// display shots
void DispShots()
{
	int i, y, n;
	sShot* s = Shots;
	for (i = SHOTS; i > 0; i--)
	{
		// is this shoot used?
		if (s->x != SHOTFREE)
		{
			// get Y coordinate of the shoot
			y = GetPerspTab(s->z);

			// prepare size of the shoot (0..3)
			n = s->z >> (4+FRAC);
			if (n > 3) n = 3;

			// display shoot
			DrawBlit4Pal(ShotImg, ShotImg_Pal, 0, n*SHOT_IMG_H, (s->x >> FRAC) - SHOT_IMG_W/2,
				y - SHOT_IMG_H, SHOT_IMG_W, SHOT_IMG_H, SHOT_IMG_W, SHOT_IMG_KEY);
		}
		s++;
	}
}

// shooting service (dt = delta time in [us])
void Shooting(int dt)
{
	int i;
	sShot* s;

	// generate new shot
	if (KeyPressed(KEY_A) && (ShipInCrash == 0))
	{
		// check time elapsed from last shot
		int i = Time() - LastShotTime;
		if ((i < -1000000) || (i >= SHOTTIME))
		{
			// find free descriptor
			s = Shots;
			for (i = SHOTS; i > 0; i--)
			{
				if (s->x == SHOTFREE) break;
				s++;
			}

			// free entry has been found
			if (i > 0)
			{
				s->x = ShipX; // X coordinate
				s->z = (12<<FRAC); // start Z coordinate
				LastShotTime = Time(); // last shot time
				PlaySoundChan(SNDCHAN_SHOT, LaserSnd, sizeof(LaserSnd), False, 1, 1);
				return;
			}
		}
	}

	// shot movement
	s = Shots;
	for (i = SHOTS; i > 0; i--)
	{
		if (s->x != SHOTFREE)
		{
			// shift shot
			s->z += dt>>8;

			// end of movement
			if (s->z >= SHOTDIST) s->x = SHOTFREE;

			// check hit enemy
			if (EnemyHit(s->x, s->z)) s->x = SHOTFREE;
		}
		s++;
	}
}

