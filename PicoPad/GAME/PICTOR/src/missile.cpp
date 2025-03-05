
// ****************************************************************************
//
//                               Actor Missiles
//
// ****************************************************************************

#include "../include.h"

// actor missile templates
const sMissileTemp MissileTemp[ACT_NUM] = {
	// pal			img		sound		soundlen,		w		wall		h		trans			phasenum	reload	hits		bounce
	{ ShurikenImg_Pal,	ShurikenImg,	SwingSnd,	count_of(SwingSnd),	SHURIKEN_W,	SHURIKEN_WALL,	SHURIKEN_H,	SHURIKEN_TRANS,		4,		30,	10,		True,	},
	{ SeedImg_Pal,		SeedImg,	MinigunSnd,	count_of(MinigunSnd),	SEED_W,		SEED_WALL,	SEED_H,		SEED_TRANS,		1,		6,	10,		True,	},
	{ BerryImg_Pal,		BerryImg,	GunSnd,		count_of(GunSnd),	BERRY_W,	BERRY_WALL,	BERRY_H,	BERRY_TRANS,		1,		13,	10,		True,	},
	{ SandballImg_Pal,	SandballImg,	NoiseSnd,	count_of(NoiseSnd),	SANDBALL_W,	SANDBALL_WALL,	SANDBALL_H,	SANDBALL_TRANS,		2,		8,	10,		False,	},
	{ FlashImg_Pal,		FlashImg,	ElectricitySnd,	count_of(ElectricitySnd), FLASH_W,	FLASH_WALL,	FLASH_H,	FLASH_TRANS,		2,		16,	1,		False,	},
	{ CrystalImg_Pal,	CrystalImg,	GlassSnd,	count_of(GlassSnd), 	CRYSTAL_W,	CRYSTAL_WALL,	CRYSTAL_H,	CRYSTAL_TRANS,		1,		20,	2,		True,	},
	{ FireImg_Pal,		FireImg,	FlameSnd,	count_of(FlameSnd), 	FIRE_W,		FIRE_WALL,	FIRE_H,		FIRE_TRANS,		4,		22,	2,		False,	},
	{ ScreamImg_Pal,	ScreamImg,	ScreamingSnd,	count_of(ScreamingSnd), SCREAM_W,	SCREAM_WALL,	SCREAM_H,	SCREAM_TRANS,		1,		25,	3,		True,	},
	{ LaserImg_Pal,		LaserImg,	LaserSnd,	count_of(LaserSnd), 	LASER_W,	LASER_WALL,	LASER_H,	LASER_TRANS,		1,		4,	1,		False,	},
	{ PhaserImg_Pal,	PhaserImg,	PhaserSnd,	count_of(PhaserSnd), 	PHASER_W,	PHASER_WALL,	PHASER_H,	PHASER_TRANS,		1,		4,	1,		False,	},
	{ RocketImg_Pal,	RocketImg,	RocketSnd,	count_of(RocketSnd), 	ROCKET_W,	ROCKET_WALL,	ROCKET_H,	ROCKET_TRANS,		2,		4,	1,		True,	},
	{ SugarbombImg_Pal,	SugarbombImg,	ThrowSnd,	count_of(ThrowSnd), 	SUGAR_W,	SUGAR_WALL,	SUGAR_H,	SUGAR_TRANS,		4,		22,	1,		True,	},
	{ SpiralImg_Pal,	SpiralImg,	BlackholeSnd,	count_of(BlackholeSnd), SPIRAL_W,	SPIRAL_WALL,	SPIRAL_H,	SPIRAL_TRANS,		6,		20,	10,		True,	},
};

// actor missile list
sMissile Missile[MISSILE_MAX];

// initialize actor missiles on new level
void InitMissile()
{
	// clear bullets
	int i;
	for (i = 0; i < MISSILE_MAX; i++) Missile[i].temp = NULL;
	ActorReload = 0;
}

// add new missile
void AddMissile(const sMissileTemp* temp, int actinx, int x, int y, int dx, int dy, sEnemy* target)
{
	int i;
	sMissile* m = Missile;
	for (i = 0; i < MISSILE_MAX; i++)
	{
		if (m->temp == NULL)
		{
			m->temp = temp;
			m->x = x;
			m->y = y;
			m->dx = dx;
			m->dy = dy;
			m->actinx = actinx;
			m->phase = 0;
			m->lifetime = (actinx == ACT_STINGRAY) ? MISSILE_LIFETIME_FLASH : MISSILE_LIFETIME_SCREAM;
			m->hits = temp->hits;
			m->target = target;
			break;
		}
		m++;
	}
}

// shift missiles
void MissileShift()
{
	int i;
	const sMissileTemp* temp;
	sMissile* m = Missile;
	sEnemy* e;
	for (i = 0; i < MISSILE_MAX; i++)
	{
		temp = m->temp;
		if (temp != NULL)
		{
			// auto-target
			e = m->target;
			if (e != NULL)
			{
				// enemy was destroyed ... or random lost focus, except Squirrel (which has more accurate targeting system)
				if ((e->temp == NULL) || ((RandU16() < 20000) && (m->actinx != ACT_SQUIRREL) && (m->actinx != ACT_GINGER)))
					m->target = NULL;
				else
				{
					// prepare direction
					int dx = e->x - m->x;
					if (dx > 0)
					{
						int dy = e->y - m->y;
						int speed = ((m->actinx == ACT_SQUIRREL) || (m->actinx == ACT_GINGER)) ? MISSILE_SPEEDX_MID : MISSILE_SPEEDX_MAX;

						// direction straight up
						if (-dy >= dx)
						{
							m->dy = -speed;
							m->dx = dx * speed / -dy;
						}
						else
						{
							// straight down
							if (dy >= dx)
							{
								m->dy = speed;
								m->dx = dx * speed / dy;
							}
							else
							{
								m->dx = speed;
								m->dy = dy * speed / dx;
							}
						}
					}
				}
			}			

			// shift missile
			m->x += m->dx;
			m->y += m->dy;

			// missile 03 Scarabeus - falling down
			if (m->actinx == ACT_SCARABEUS)
			{
				m->dy++;
			}

			// missile deflection from the edges
			int k = BG_Y_MIN + temp->h/2 + 1;
			if (m->y < k)
			{
				if (!temp->bounce || (m->dx == 0)) m->temp = NULL; // destroy on sky
				m->dy = - m->dy;
				m->y = k;
			}

			k = BG_Y_MAX - temp->h/2 - 1;
			if (m->y > k)
			{
				if (!temp->bounce || (m->dx == 0)) m->temp = NULL; // destroy on earth
				m->dy = - m->dy;
				m->y = k;
			}

			// lifetime
			if ((m->actinx == ACT_STINGRAY) || (m->actinx == ACT_GHOST))
			{
				m->lifetime--;
				if (m->lifetime < 0) m->temp = NULL;
			}

			// shift animation phase
			m->phase++;
			if (m->phase >= temp->phasenum) m->phase = 0;

			// delete missile
			if ((m->x - temp->w/2 >= WIDTH) || (m->x < 0)) m->temp = NULL;
		}
		m++;
	}
	if (ActorReload > 0) ActorReload--;
}

// display missiles
void MissileDisp()
{
	int i;
	const sMissileTemp* temp;
	sMissile* m = Missile;
	for (i = 0; i < MISSILE_MAX; i++)
	{
		temp = m->temp;
		if (temp != NULL)
		{
			DrawBlit4Pal(temp->img, temp->pal,		// image data and palettes
				m->phase * temp->w, 0,			// source X, Y
				m->x - temp->w/2, m->y - temp->h/2, // destination X, Y
				temp->w, temp->h,		// width, height
				temp->wall, temp->trans);	// source total width
		}
		m++;
	}
}

// generate missile
void GenMissile()
{
	// check reload timer
	if (ActorReload > 0) return;
	int actinx = ActInx;
	ActorReload = MissileTemp[actinx].reload;

	// prepare start position
	int x = ActorX + Actor.w/2;
	int y = ActorY;

	// missile template
	const sMissileTemp* temp = &MissileTemp[actinx];

	// add missiles
	switch (actinx)
	{
	// 00 Jill
	case 0:
		AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, -MISSILE_SPEEDY_MID, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, 0, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, +MISSILE_SPEEDY_MID, NULL);
		break;

	// 01 Bird
	case 1:
		AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, 0, NULL);
		break;

	// 02 Parrot
	case 2:
		AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, -MISSILE_SPEEDY_MIN, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, +MISSILE_SPEEDY_MIN, NULL);
		break;

	// 03 Scarabeus
	case 3:
		AddMissile(temp, actinx, x - Actor.w/4, y - Actor.h/2,
			RandS16MinMax(MISSILE_SPEEDX_MIN, MISSILE_SPEEDX_MID),
			RandS16MinMax(-MISSILE_SPEEDY_MAX, -MISSILE_SPEEDY_MIN), NULL);
		break;

	// 04 Stingray
	case 4:
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH0, 0, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH1, -MISSILE_SPEED_FLASH3, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH2, -MISSILE_SPEED_FLASH2, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH3, -MISSILE_SPEED_FLASH1, NULL);
		AddMissile(temp, actinx, x, y, 0, -MISSILE_SPEED_FLASH0, NULL);
		AddMissile(temp, actinx, x, y, -MISSILE_SPEED_FLASH3, -MISSILE_SPEED_FLASH1, NULL);
		AddMissile(temp, actinx, x, y, -MISSILE_SPEED_FLASH2, -MISSILE_SPEED_FLASH2, NULL);
		AddMissile(temp, actinx, x, y, -MISSILE_SPEED_FLASH1, -MISSILE_SPEED_FLASH3, NULL);
		AddMissile(temp, actinx, x, y, -MISSILE_SPEED_FLASH0, 0, NULL);
		AddMissile(temp, actinx, x, y, -MISSILE_SPEED_FLASH1, MISSILE_SPEED_FLASH3, NULL);
		AddMissile(temp, actinx, x, y, -MISSILE_SPEED_FLASH2, MISSILE_SPEED_FLASH2, NULL);
		AddMissile(temp, actinx, x, y, -MISSILE_SPEED_FLASH3, MISSILE_SPEED_FLASH1, NULL);
		AddMissile(temp, actinx, x, y, 0, MISSILE_SPEED_FLASH0, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH3, MISSILE_SPEED_FLASH1, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH2, MISSILE_SPEED_FLASH2, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH1, MISSILE_SPEED_FLASH3, NULL);
		break;

	// 05 Ice Bird
	case 5:
		AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, -MISSILE_SPEEDY_MIN, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, 0, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, +MISSILE_SPEEDY_MIN, NULL);
		break;

	// 06 Dragon
	case 6:
	// 07 Ghost
	case 7:
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH2, -MISSILE_SPEED_FLASH2, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH1, -MISSILE_SPEED_FLASH3, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH0, 0, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH1, +MISSILE_SPEED_FLASH3, NULL);
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_FLASH2, +MISSILE_SPEED_FLASH2, NULL);
		break;

	// 08 R2-D2
	case 8:
		y -= Actor.h/2;
	// 09 UFO
	case 9:
		{
			int j;
			sEnemy* target = NULL;
			sEnemy* enemy;

			// try to find some enemy as target
			for (j = ENEMY_MAX; j > 0; j--)
			{
				enemy = &Enemy[RandU16Max(ENEMY_MAX-1)];
				if ((enemy->temp != NULL) && (enemy->x > x) && (enemy->x - enemy->temp->w/2 < WIDTH))
				{
					target = enemy;
					break;
				}
			}

			AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, 0, target);
		}
		break;

	// 10 Squirrel
	case 10:
	// 11 Gingerbread Man
	case 11:
		{
			int j;
			sEnemy* target = NULL;
			sEnemy* enemy;

			// try to find some enemy as target
			for (j = ENEMY_MAX*3; j > 0; j--)
			{
				enemy = &Enemy[RandU16Max(ENEMY_MAX-1)];
				if ((enemy->temp != NULL) && (enemy->x > x) && (enemy->x - enemy->temp->w/2 < WIDTH))
				{
					target = enemy;
					break;
				}
			}

			// not found? Try find in order
			if (target == NULL)
			{
				enemy = Enemy;
				for (j = ENEMY_MAX; j > 0; j--)
				{
					if ((enemy->temp != NULL) && (enemy->x > x) && (enemy->x - enemy->temp->w/2 < WIDTH))
					{
						target = enemy;
						break;
					}
					enemy++;
				}
			}

			AddMissile(temp, actinx, x, y, MISSILE_SPEEDX_MID, 0, target);
		}
		break;

	// 12 Devil Bird
	case 12:
		AddMissile(temp, actinx, x, y, MISSILE_SPEED_BLACKHOLE, 0, NULL);
		break;
	}

	// sound
	if (SoundMode != SOUNDMODE_OFF)
		PlaySoundChan(SOUNDCHAN_MISSILE, temp->sound, temp->soundlen, False, 1, 1, SNDFORM_ADPCM, IMA_SAMPBLOCK);
}
