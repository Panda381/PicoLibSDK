
// ****************************************************************************
//
//                               Enemies
//
// ****************************************************************************

#include "../include.h"

// move templates
const sMoveSeg Move1[] = { MOVEL(0), };		// straight left
const sMoveSeg Move1U[] = { MOVEL_SU(0), };	// straight left, slow up
const sMoveSeg Move1D[] = { MOVEL_SD(0), };	// straight left, slow down

const sMoveSeg Move2[] = { MOVEL(20), MOVELD(30), MOVEL(5), MOVELU(10), MOVEL(0), };	// left down
const sMoveSeg Move3[] = { MOVEL(20), MOVELU(30), MOVEL(5), MOVELD(10), MOVEL(0), };	// left up

const sMoveSeg Move4[] = { MOVEL(10), MOVELU(10), MOVEL(5), MOVELD(20), MOVEL(5), MOVELU(20), MOVEL(5), MOVELD(10), MOVEL(0), };	// saw
const sMoveSeg Move5[] = { MOVEL(10), MOVELD(10), MOVEL(5), MOVELU(20), MOVEL(5), MOVELD(20), MOVEL(5), MOVELU(10), MOVEL(0), };	// saw 2
const sMoveSeg Move6[] = { MOVEL(40), MOVEU(20), MOVEL(0), };	// up
const sMoveSeg Move7[] = { MOVEL(40), MOVED(20), MOVEL(0), };	// down
const sMoveSeg* Moves[] = { Move1, Move1U, Move1D,   Move2, Move3,   Move4, Move5, Move6, Move7 };

// enemy score
const s8 EnemyScore[3] = { 10, 20, 50 };

// current enemy template
sEnemyTemp EnemyNow[ENEMY_LEVNUM];

// enemies
sEnemy Enemy[ENEMY_MAX];

// explosions
sExplosion Explosion[EXPLOSIONS_MAX];

// enemy templates
const sEnemyTemp EnemyTemp[ENEMY_NUM] = {
	// name			pal			img		w		wall		h		trans			animmax		animshift	animmask
// 1:
	{ "Fly",		FlyImg_Pal,		FlyImg,		FLY_W,		FLY_WALL,	FLY_H,		FLY_TRANS,		1,		0,		1,	},
	{ "Wasp",		WaspImg_Pal,		WaspImg,	WASP_W,		WASP_WALL,	WASP_H,		WASP_TRANS,		1,		0,		1,	},
	{ "Hornet",		HornetImg_Pal,		HornetImg,	HORNET_W,	HORNET_WALL,	HORNET_H,	HORNET_TRANS,		1,		0,		1,	},
// 2:
	{ "Dragonfly",		DragonflyImg_Pal,	DragonflyImg,	DRAGONFLY_W,	DRAGONFLY_WALL,	DRAGONFLY_H,	DRAGONFLY_TRANS,	1,		0,		1,	},
	{ "Moth",		MothImg_Pal,		MothImg,	MOTH_W,		MOTH_WALL,	MOTH_H,		MOTH_TRANS,		1,		0,		1,	},
	{ "Kestrel",		KestrelImg_Pal,		KestrelImg,	KESTREL_W,	KESTREL_WALL,	KESTREL_H,	KESTREL_TRANS,		3,		1,		1,	},
// 3:
	{ "Maggot",		MaggotImg_Pal,		MaggotImg,	MAGGOT_W,	MAGGOT_WALL,	MAGGOT_H,	MAGGOT_TRANS,		0,		0,		0,	},
	{ "Eagle",		EagleImg_Pal,		EagleImg,	EAGLE_W,	EAGLE_WALL,	EAGLE_H,	EAGLE_TRANS,		3,		1,		1,	},
	{ "Antman",		AntmanImg_Pal,		AntmanImg,	ANTMAN_W,	ANTMAN_WALL,	ANTMAN_H,	ANTMAN_TRANS,		3,		1,		1,	},
// 4:
	{ "Fish",		FishImg_Pal,		FishImg,	FISH_W,		FISH_WALL,	FISH_H,		FISH_TRANS,		3,		1,		1,	},
	{ "Anglerfish",		AnglerfishImg_Pal,	AnglerfishImg,	ANGLERFISH_W,	ANGLERFISH_WALL,ANGLERFISH_H,	ANGLERFISH_TRANS,	3,		1,		1,	},
	{ "Shark",		SharkImg_Pal,		SharkImg,	SHARK_W,	SHARK_WALL,	SHARK_H,	SHARK_TRANS,		3,		1,		1,	},
// 5:
	{ "Penguin",		PenguinImg_Pal,		PenguinImg,	PENGUIN_W,	PENGUIN_WALL,	PENGUIN_H,	PENGUIN_TRANS,		1,		0,		1,	},
	{ "Polar Bear",		PolarbearImg_Pal,	PolarbearImg,	POLARBEAR_W,	POLARBEAR_WALL,	POLARBEAR_H,	POLARBEAR_TRANS,	3,		1,		1,	},
	{ "Snowflake",		SnowflakeImg_Pal,	SnowflakeImg,	SNOWFLAKE_W,	SNOWFLAKE_WALL,	SNOWFLAKE_H,	SNOWFLAKE_TRANS,	0,		0,		0,	},
// 6:
	{ "Phoenix",		PhoenixImg_Pal,		PhoenixImg,	PHOENIX_W,	PHOENIX_WALL,	PHOENIX_H,	PHOENIX_TRANS,		3,		1,		1,	},
	{ "Fire Ball",		FireballImg_Pal,	FireballImg,	FIREBALL_W,	FIREBALL_WALL,	FIREBALL_H,	FIREBALL_TRANS,		3,		1,		1,	},
	{ "Evil",		EvilImg_Pal,		EvilImg,	EVIL_W,		EVIL_WALL,	EVIL_H,		EVIL_TRANS,		3,		1,		1,	},
// 7:
	{ "Pumpkin",		PumpkinImg_Pal,		PumpkinImg,	PUMPKIN_W,	PUMPKIN_WALL,	PUMPKIN_H,	PUMPKIN_TRANS,		3,		1,		1,	},
	{ "Zombie",		ZombieImg_Pal,		ZombieImg,	ZOMBIE_W,	ZOMBIE_WALL,	ZOMBIE_H,	ZOMBIE_TRANS,		0,		0,		0,	},
	{ "Death",		DeathImg_Pal,		DeathImg,	DEATH_W,	DEATH_WALL,	DEATH_H,	DEATH_TRANS,		3,		1,		1,	},
// 8:
	{ "Camera",		CameraImg_Pal,		CameraImg,	CAMERA_W,	CAMERA_WALL,	CAMERA_H,	CAMERA_TRANS,		0,		0,		0,	},
	{ "Explorer",		ExplorerImg_Pal,	ExplorerImg,	EXPLORER_W,	EXPLORER_WALL,	EXPLORER_H,	EXPLORER_TRANS,		3,		1,		1,	},
	{ "War Dron",		DronImg_Pal,		DronImg,	DRON_W,		DRON_WALL,	DRON_H,		DRON_TRANS,		0,		0,		0,	},
// 9:
	{ "Starship Enterprise", EnterpriseImg_Pal,	EnterpriseImg,	ENTERPRISE_W,	ENTERPRISE_WALL, ENTERPRISE_H,	ENTERPRISE_TRANS,	0,		0,		0,	},
	{ "Klingon Warship",	KlingonImg_Pal,		KlingonImg,	KLINGON_W,	KLINGON_WALL,	KLINGON_H,	KLINGON_TRANS,		0,		0,		0,	},
	{ "Death Star",		DeathstarImg_Pal,	DeathstarImg,	DEATHSTAR_W,	DEATHSTAR_WALL,	DEATHSTAR_H,	DEATHSTAR_TRANS,	0,		0,		0,	},
// 10:
	{ "Alien",		AlienImg_Pal,		AlienImg,	ALIEN_W,	ALIEN_WALL,	ALIEN_H,	ALIEN_TRANS,		1,		0,		1,	},
	{ "Xenomorph",		XenomorphImg_Pal,	XenomorphImg,	XENOMORPH_W,	XENOMORPH_WALL,	XENOMORPH_H,	XENOMORPH_TRANS,	1,		0,		1,	},
	{ "Meteor",		MeteorImg_Pal,		MeteorImg,	METEOR_W,	METEOR_WALL,	METEOR_H,	METEOR_TRANS,		3,		1,		1,	},
// 11:
	{ "Candy Cane",		CandycaneImg_Pal,	CandycaneImg,	CANDYCANE_W,	CANDYCANE_WALL,	CANDYCANE_H,	CANDYCANE_TRANS,	3,		1,		1,	},
	{ "Lollipop",		LollipopImg_Pal,	LollipopImg,	LOLLIPOP_W,	LOLLIPOP_WALL,	LOLLIPOP_H,	LOLLIPOP_TRANS,		3,		1,		1,	},
	{ "Fat Man",		FatmanImg_Pal,		FatmanImg,	FATMAN_W,	FATMAN_WALL,	FATMAN_H,	FATMAN_TRANS,		3,		1,		1,	},
// 12:
	{ "Cloud",		CloudImg_Pal,		CloudImg,	CLOUD_W,	CLOUD_WALL,	CLOUD_H,	CLOUD_TRANS,		0,		0,		0,	},
	{ "Star",		StarImg_Pal,		StarImg,	STAR_W,		STAR_WALL,	STAR_H,		STAR_TRANS,		0,		0,		0,	},
	{ "Moon",		MoonImg_Pal,		MoonImg,	MOON_W,		MOON_WALL,	MOON_H,		MOON_TRANS,		0,		0,		0,	},
};

// initialize enemies on new level
void InitEnemy()
{
	// prepare enemy templates
	int i = BackInx * ENEMY_LEVNUM;
	int j;
	for (j = 0; j < ENEMY_LEVNUM; j++)
	{
		memcpy(&EnemyNow[j], &EnemyTemp[i], sizeof(sEnemyTemp));
		i++;
	}

	// clear enemies
	for (i = 0; i < ENEMY_MAX; i++) Enemy[i].temp = NULL;

	// clear explosions
	for (i = 0; i < EXPLOSIONS_MAX; i++) Explosion[i].anim = EXPLOSION_PHASES;
}

// add new enemy (temp = enemy template index 0..2)
void AddEnemy(const sEnemyTemp* temp, int x, int y, const sMoveSeg* move, int inx)
{
	int i;
	sEnemy* e = Enemy;
	for (i = 0; i < ENEMY_MAX; i++)
	{
		if (e->temp == NULL)
		{
			e->temp = temp;
			e->x = x;
			e->y = y;
			e->anim = 0;
			e->move = move;
			e->step = 0;
			e->inx = inx;
			break;
		}
		e++;
	}
}

// add explosion
void AddExplosion(int x, int y)
{
	int i;
	sExplosion* e = Explosion;
	for (i = 0; i < EXPLOSIONS_MAX; i++)
	{
		if (e->anim >= EXPLOSION_PHASES) // if entry is not used
		{
			e->anim = 0;
			e->x = x;
			e->y = y;
			break;
		}
		e++;
	}
}

// generate new enemy
void NewEnemy()
{
	// prepare current randomness
	int gen = f2i(((ENEMY_GEN_END - ENEMY_GEN_BEG)*CurrentFrame/MaxFrame + ENEMY_GEN_BEG) * powf(ENEMY_GEN_LEVEL, Level));

	// check randomness
	if (EnemyRand() <= (u16)gen)
	{
		// randomness of enemy 3 (range 0..65535, starting at 1/2 of the level)
		gen = 0;
		if (CurrentFrame*2 >= MaxFrame) gen = (CurrentFrame*2 - MaxFrame)*65536/MaxFrame;

		// generate enemy 3
		int inx;
		if (EnemyRand() < gen)
			inx = 2;
		else
		{
			// randomness of enemy 2 (range 0..65535)
			gen = CurrentFrame*50000/MaxFrame;

			// generate enemy 2, or 1 otherwise
			if (EnemyRand() < gen)
				inx = 1;
			else
				inx = 0;
		}

		// enemy template
		const sEnemyTemp* e = &EnemyNow[inx];

		// enemy coordinate Y
		int ymin = BG_Y_MIN + e->h/2 + ENEMY_BORDERY;
		int ymax = BG_Y_MAX - e->h/2 - ENEMY_BORDERY;
		int y = EnemyRandMinMax(ymin, ymax);

		// move
		const sMoveSeg* move;
		int n;
		if (inx == 0) // enemy 1: move straight left
			n = 2;
		else if (inx == 1) // enemy 2: move diagonally
			n = 4;
		else // enemy 3: random movement
			n = count_of(Moves)-1;

		move = Moves[EnemyRandMax(n)];

		// add enemy to the list
		AddEnemy(e, WIDTH + e->w/2 + 2, y, move, inx);
	}
}

// shift enemies, shoot bullets
void EnemyShift()
{
	int i, a;
	const sEnemyTemp* temp;
	sEnemy* e = Enemy;
	for (i = 0; i < ENEMY_MAX; i++)
	{
		temp = e->temp;
		if (temp != NULL)
		{
			// shift enemy animation
			a = e->anim + 1;
			if (a > temp->animmax) a = 0;
			e->anim = a;

			// move enemy
			const sMoveSeg* move = e->move;
			e->x += move->dx;
			e->y += move->dy;

			// limit Y coordinate
			int ymin = BG_Y_MIN + temp->h/2 + ENEMY_BORDERY;
			int ymax = BG_Y_MAX - temp->h/2 - ENEMY_BORDERY;

			if (e->y < ymin)
			{
				e->y = ymin;
				e->step = move->steps; // next move segment
				if (move == Move1U) e->move = Move1D; // flip direction
			}

			if (e->y > ymax)
			{
				e->y = ymax;
				e->step = move->steps; // next move segment
				if (move == Move1D) e->move = Move1U; // flip direction
			}

			// shift move step
			if (move->steps != 0)
			{
				e->step++;

				// next move segment
				if (e->step >= move->steps)
				{
					e->step = 0;
					e->move = move + 1;
				}
			}

			// delete enemy
			if (e->x + temp->w/2 <= -2)
				e->temp = NULL;
			else
			{
				// generate bullet
				if (RandU16() < ENEMY_GEN_BUL)
				{
					AddBullet(&BulletTemp[e->inx], e->x - temp->w/2, e->y);
				}
			}
		}
		e++;
	}
}

// shift explosions
void ExplosionShift()
{
	int i;
	sExplosion* e = Explosion;
	for (i = 0; i < EXPLOSIONS_MAX; i++)
	{
		if (e->anim < EXPLOSION_PHASES)
		{
			e->anim++;
		}
		e++;
	}
}

// display enemies
void EnemyDisp()
{
	int i;
	const sEnemyTemp* temp;
	sEnemy* e = Enemy;
	for (i = 0; i < ENEMY_MAX; i++)
	{
		temp = e->temp;
		if (temp != NULL)
		{
			DrawBlitPal(temp->img, temp->pal,		// image data and palettes
				((e->anim >> temp->animshift) & temp->animmask)*temp->w, 0,	// source X, Y
				e->x - temp->w/2, e->y - temp->h/2, // destination X, Y
				temp->w, temp->h,		// width, height
				temp->wall, temp->trans);	// source total width
		}
		e++;
	}
}

// display explosions
void ExplosionDisp()
{
	int i;
	sExplosion* e = Explosion;
	for (i = 0; i < EXPLOSIONS_MAX; i++)
	{
		if (e->anim < EXPLOSION_PHASES)
		{
			DrawBlit4Pal(ExplosionImg, ExplosionImg_Pal,	// image data and palettes
				e->anim * EXLPOSION_W, 0,	// source X, Y
				e->x - EXLPOSION_W/2, e->y - EXLPOSION_H/2, // destination X, Y
				EXLPOSION_W, EXLPOSION_H,	// width, height
				EXLPOSION_WALL, EXLPOSION_TRANS);	// source total width
		}
		e++;
	}
}

// hit enemy by missile
void EnemyHit()
{
	int i, j, k, xm, ym, xe, ye, dx, dy, dx2, dy2;
	const sMissileTemp* tm;
	const sEnemyTemp* te;
	sEnemy* e;
	sMissile* m;
	sBullet* b;

	// loop missiles
	m = Missile;
	for (i = 0; i < MISSILE_MAX; i++)
	{
		tm = m->temp;
		if (tm != NULL)
		{
			// get missile coordinates
			xm = m->x;
			ym = m->y;

			// loop enemies
			e = Enemy;
			for (j = 0; j < ENEMY_MAX; j++)
			{
				te = e->temp;
				if ((te != NULL) && (e->x - te->w/2 < WIDTH)) // cannot shot target out of screen
				{
					// get enemy coordinates
					xe = e->x;
					ye = e->y;

					// check distance
					dx = xe - xm;
					dy = ye - ym;
					if (dx*dx + dy*dy <= ENEMY_HIT_DIST*ENEMY_HIT_DIST)
					{
						// destroy this enemy
						e->temp = NULL;

						// increase score
						Score += EnemyScore[e->inx];
						AddPoint(EnemyScore[e->inx], xe, ye);

						// serve black hole
						if (m->actinx == ACT_DEVILBIRD)
						{
							// sound of suck
							if (SoundMode != SOUNDMODE_OFF) PlaySoundChan(SOUNDCHAN_ENEMYHIT,
								Blackhole2Snd, count_of(Blackhole2Snd), False, 1, 1, SNDFORM_ADPCM, IMA_SAMPBLOCK);
						}
						else
						{
							// create explosion
							AddExplosion(xe, ye);

							// sugar bomb
							if (m->actinx == ACT_GINGER)
							{
								// loop enemies
								for (k = 0; k < ENEMY_MAX; k++)
								{
									if (Enemy[k].temp != NULL)
									{
										dx2 = Enemy[k].x - xe;
										dy2 = Enemy[k].y - ye;
										if (dx2*dx2 + dy2*dy2 <= SUGAR_HIT_RADIUS*SUGAR_HIT_RADIUS)
										{
											// destroy this enemy
											Enemy[k].temp = NULL;

											// increase score
											Score += EnemyScore[Enemy[k].inx];
											AddPoint(EnemyScore[Enemy[k].inx], Enemy[k].x, Enemy[k].y);

											// create explosion
											AddExplosion(Enemy[k].x, Enemy[k].y);
										}
									}
								}
							}

							// sound of explosion
							if (SoundMode != SOUNDMODE_OFF) PlaySoundChan(SOUNDCHAN_ENEMYHIT,
								EnemyHitSnd, count_of(EnemyHitSnd), False, 1, 1, SNDFORM_ADPCM, IMA_SAMPBLOCK);
						}

						// destroy missile
						m->hits--;
						if (m->hits <= 0)
						{
							m->temp = NULL;
							break;
						}
					}
				}

				// next enemy
				e++;
			}

			// black hole - catch bullets
			if (m->actinx == ACT_DEVILBIRD)
			{
				// loop bullets
				b = Bullet;
				for (j = 0; j < BULLET_MAX; j++)
				{
					// check if bullet is valid
					if (b->temp != NULL)
					{
						// get bullet coordinates
						xe = b->x;
						ye = b->y;

						// check distance
						dx = xe - xm;
						dy = ye - ym;
						if (dx*dx + dy*dy <= ENEMY_HIT_DIST*ENEMY_HIT_DIST)
						{
							// destroy this bullet
							b->temp = NULL;

							// sound of suck
							if (SoundMode != SOUNDMODE_OFF) PlaySoundChan(SOUNDCHAN_ENEMYHIT,
								Blackhole2Snd, count_of(Blackhole2Snd), False, 1, 1, SNDFORM_ADPCM, IMA_SAMPBLOCK);
						}
					}

					// next bullet
					b++;
				}
			}
		}
		
		// next missile
		m++;
	}
}
