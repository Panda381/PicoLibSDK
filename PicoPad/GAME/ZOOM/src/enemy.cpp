
// ****************************************************************************
//
//                               Enemies
//
// ****************************************************************************

#include "include.h"

// time of last generated object
u32 TimeLastDisk;		// time of last generated small disk
u32 TimeLastBig;		// time of last generated big enemy
Bool BossIsGen;			// boss is generated
int BossLives;			// boss lives counter
u32 TimeLastBomb;		// time of last generated bomb

// points per enemy (no, disk, big, boss)
const int EnemyPoint[4] = { 0, 200<<FRAC, 300<<FRAC, 600<<FRAC };

// table of steps of moving
const sStep Steps[] = {
	// small disk on the left
	{ 0, ENEMY_DISK_SPEED, 200 },
	{ -4*ENEMY_DISK_SPEED, ENEMY_DISK_SPEED, 500 },
	{ 0, ENEMY_DISK_SPEED, 3500 },
	{ ENEMY_DISK_SPEED, 0, 2000 },
	{ 0, -2*ENEMY_DISK_SPEED, 800 },
	{ -2*ENEMY_DISK_SPEED, -ENEMY_DISK_SPEED, 1200 },
	{ 2*ENEMY_DISK_SPEED, -ENEMY_DISK_SPEED, 1400 },
	// small disk on the right
	{ 0, ENEMY_DISK_SPEED, 200 },
	{ 4*ENEMY_DISK_SPEED, ENEMY_DISK_SPEED, 500 },
	{ 0, ENEMY_DISK_SPEED, 3500 },
	{ -ENEMY_DISK_SPEED, 0, 2000 },
	{ 0, -2*ENEMY_DISK_SPEED, 800 },
	{ 2*ENEMY_DISK_SPEED, -ENEMY_DISK_SPEED, 1200 },
	{ -2*ENEMY_DISK_SPEED, -ENEMY_DISK_SPEED, 1400 },
	// big enemy
	{ 0, -ENEMY_BIG_SPEED, 3000 },
	{ ENEMY_BIG_SPEED, -ENEMY_BIG_SPEED, 800 },
	{ -2*ENEMY_BIG_SPEED, -ENEMY_BIG_SPEED, 1000 },
	{ 2*ENEMY_BIG_SPEED, -ENEMY_BIG_SPEED, 1400 },
	{ 0, -2*ENEMY_BIG_SPEED, 1000 },
	{ -ENEMY_BIG_SPEED, -ENEMY_BIG_SPEED, 400 },
	{ 2*ENEMY_BIG_SPEED, -ENEMY_BIG_SPEED, 400 },
	{ -ENEMY_BIG_SPEED, -ENEMY_BIG_SPEED, 800 },
	{ 2*ENEMY_BIG_SPEED, -ENEMY_BIG_SPEED, 1000 },
	{ -ENEMY_BIG_SPEED, -ENEMY_BIG_SPEED, 100 },
	{ 0, -ENEMY_BIG_SPEED, 500 },
	// boss
	{ 0, ENEMY_BOSS_SPEED, 1500 },
	{ -ENEMY_BOSS_SPEED, ENEMY_BOSS_SPEED, 800 },
	// ... boss repeat loop
	{ -ENEMY_BOSS_SPEED, -ENEMY_BOSS_SPEED, 800 },
	{ ENEMY_BOSS_SPEED, -ENEMY_BOSS_SPEED, 800 },
	{ ENEMY_BOSS_SPEED, ENEMY_BOSS_SPEED, 1600 },
	{ ENEMY_BOSS_SPEED, -ENEMY_BOSS_SPEED, 800 },
	{ -ENEMY_BOSS_SPEED, -ENEMY_BOSS_SPEED, 800 },
	{ -ENEMY_BOSS_SPEED, ENEMY_BOSS_SPEED, 1600 },
	// crash
	{ 0, 0, ENEMY_CRASH_TIME },
	{ 0, 0, ENEMY_CRASH_TIME },
	{ 0, 0, ENEMY_CRASH_TIME },
	{ 0, 0, ENEMY_CRASH_TIME },
	{ 0, 0, ENEMY_CRASH_TIME },
	{ 0, 0, ENEMY_CRASH_TIME },
	{ 0, 0, ENEMY_CRASH_TIME },
	{ 0, 0, ENEMY_CRASH_TIME },
	// bomb to left
	{ -3*ENEMY_BOMB_SPEED, -ENEMY_BOMB_SPEED/2, 2000 },
	// bomb to right
	{ 3*ENEMY_BOMB_SPEED, -ENEMY_BOMB_SPEED/2, 2000 },
	// bomb down
	{ 0, -ENEMY_BOMB_SPEED, 5000 },
};

// table of move tracks
const sMove Moves[] = {
	// 0: small disk on the left
	{ ENEMY_DISK1_STARTSTEP, ENEMY_DISK1_NUMSTEP, ENEMY_DISK1_REPSTEP },
	// 1: small disk on the right
	{ ENEMY_DISK2_STARTSTEP, ENEMY_DISK2_NUMSTEP, ENEMY_DISK2_REPSTEP },
	// 2: big enemy
	{ ENEMY_BIG_STARTSTEP, ENEMY_BIG_NUMSTEP, ENEMY_BIG_REPSTEP },
	// 3: boss
	{ ENEMY_BOSS_STARTSTEP, ENEMY_BOSS_NUMSTEP, ENEMY_BOSS_REPSTEP },
	// 4: crash
	{ ENEMY_CRASH_STARTSTEP, ENEMY_CRASH_NUMSTEP, ENEMY_CRASH_REPSTEP },
	// 5: bomb to left
	{ ENEMY_BOMBL_STARTSTEP, ENEMY_BOMBL_NUMSTEP, ENEMY_BOMBL_REPSTEP },
	// 6: bomb to right
	{ ENEMY_BOMBR_STARTSTEP, ENEMY_BOMBR_NUMSTEP, ENEMY_BOMBR_REPSTEP },
	// 7: bomb down
	{ ENEMY_BOMBD_STARTSTEP, ENEMY_BOMBD_NUMSTEP, ENEMY_BOMBD_REPSTEP },
};

// objects
sObj Obj[OBJ_MAX];		// list of objects

// initialize list of objects
void InitObj()
{
	int i;
	for (i = 0; i < OBJ_MAX; i++) Obj[i].type = ENEMY_NO;
}

// display objects
void DispObj()
{
	int i, y, n;
	sObj* o = Obj;
	for (i = OBJ_MAX; i > 0; i--)
	{
		// check if object is valid
		if (o->type != ENEMY_NO)
		{
			// get Y coordinate of the object
			y = GetPerspTab(o->z);

			// prepare size of the object (0..7)
			n = o->z >> (4+FRAC);
			if (n < 0) n = 0;
			if (n > 7) n = 7;

			// display object
			switch (o->type)
			{
			// small disk
			case ENEMY_DISK:
				DrawBlit4Pal(UfoImg, UfoImg_Pal, 0, n*UFO_IMG_H, (o->x >> FRAC) - UFO_IMG_W/2,
					y - UFO_IMG_H, UFO_IMG_W, UFO_IMG_H, UFO_IMG_W, UFO_IMG_KEY);
				break;

			// big enemy			
			case ENEMY_BIG:
				DrawBlit4Pal(Ufo2Img, Ufo2Img_Pal, n*UFO2_IMG_W, 0, (o->x >> FRAC) - UFO2_IMG_W/2,
					y - UFO2_IMG_H, UFO2_IMG_W, UFO2_IMG_H, UFO2_IMG_TOTW, UFO2_IMG_KEY);
				break;

			// boss
			case ENEMY_BOSS:
				DrawBlit4Pal(BossImg, BossImg_Pal, 0, n*BOSS_IMG_H, (o->x >> FRAC) - BOSS_IMG_W/2,
					y - BOSS_IMG_H, BOSS_IMG_W, BOSS_IMG_H, BOSS_IMG_W, BOSS_IMG_KEY);
				break;

			// crash
			case ENEMY_CRASH:
				DrawBlit4Pal(ExplosionImg, ExplosionImg_Pal, 0, o->istep*EXPLOSION_IMG_H, (o->x >> FRAC) - EXPLOSION_IMG_W/2,
					y - EXPLOSION_IMG_H, EXPLOSION_IMG_W, EXPLOSION_IMG_H, EXPLOSION_IMG_W, EXPLOSION_IMG_KEY);
				break;

			// bomb
			case ENEMY_BOMB:
				DrawBlit4Pal(BombImg, BombImg_Pal, 0, n*BOMB_IMG_H, (o->x >> FRAC) - BOMB_IMG_W/2,
					y - BOMB_IMG_H - 15, BOMB_IMG_W, BOMB_IMG_H, BOMB_IMG_W, BOMB_IMG_KEY);
				break;
			}
		}

		// next object
		o++;
	}
}

// find free object (NULL = not found)
sObj* GetFreeObj()
{
	int i;
	sObj* o = Obj;
	for (i = OBJ_MAX; i > 0; i--)
	{
		// check of object is valid
		if (o->type == ENEMY_NO) return o;
		o++;
	}
	return NULL;
}

// generate bomb
void GenBomb(int move, short x, short z)
{
	sObj* o;

	// find free object
	o = GetFreeObj();
	if (o == NULL) return;

	// setup descriptor
	o->type = ENEMY_BOMB; // type
	o->move = move; // move index
	o->istep = 0; // relative index of the step
	o->starttime = Time(); // start time of move in [us]
	o->time = Time(); // start time of the step
	o->sx = x; // start X
	o->x = x; //  current X
	o->sz = z; // start Z
	o->z = z; // current Z
}

// serve objects
void DoObj()
{
	int i, n, x, dt, z;
	sObj* o;
	const sMove* m;
	const sStep* s;

	// sublevel 3 - generate boss
	Bool boss = (SubLevel == 3) && (Goals <= 1);
	if (boss)
	{
		// generate new boss
		if (!BossIsGen)
		{
			BossIsGen = True;

			// find free object
			o = GetFreeObj();

			// start coordinate
			z = -150;
			x = (WIDTH/2) << FRAC;

			// setup descriptor
			o->type = ENEMY_BOSS; // type - boss
			o->move = ENEMY_BOSS_MOVE; // move index
			o->istep = 0; // relative index of the step
			o->starttime = Time();	// start time of move in [us]
			o->time = Time(); // start time of the step
			o->nextbomb = Time() + ENEMY_BOSS_NEXTBOMB; // time of next bomb
			o->sx = x; // start X
			o->x = x; //  current X
			o->sz = z; // start Z
			o->z = z; // current Z
		}
	}

	// generate small disk (not in sublevel 0)
	if (!boss && (SubLevel != 0) && ((Time() - TimeLastDisk) > 8000000))
	{
		// number of enemies
		n = RandU8MinMax(1, 4);

		// random X coordinate
		x = RandS16MinMax(100, WIDTH-100) << FRAC;

		for (i = 0; i < n; i++)
		{
			// find free object
			o = GetFreeObj();
			if (o == NULL) break;

			// time of last generated disk
			TimeLastDisk = Time();

			// start Z coordinate
			z = -150;

			// setup descriptor
			o->type = ENEMY_DISK; // type - small disk
			o->move = (x < ((WIDTH/2)<<FRAC)) ? ENEMY_DISK1_MOVE : ENEMY_DISK2_MOVE; // move index on the left/on the right
			o->istep = 0; // relative index of the step
			o->starttime = Time() + ENEMY_DISK_DELAY*i; // start time of move in [us]
			o->time = Time() + ENEMY_DISK_DELAY*i; // start time of the step (with delay in group)
			o->sx = x; // start X
			o->x = x; //  current X
			o->sz = z; // start Z
			o->z = z; // current Z
		}
	}

	// generate big enemy (only sublevel 2)
	if ((SubLevel == 2) && ((Time() - TimeLastBig) > 12000000))
	{
		// find free object
		o = GetFreeObj();
		if (o != NULL)
		{
			// time of last generated enemy
			TimeLastBig = Time();

			// random X coordinate
			x = RandS16MinMax(80, WIDTH-80) << FRAC;

			// start Z coordinate
			z = 150 << FRAC;

			// setup descriptor
			o->type = ENEMY_BIG; // type - big enemy
			o->move = ENEMY_BIG_MOVE; // move index
			o->istep = 0; // relative index of the step
			o->starttime = Time();	// start time of move in [us]
			o->time = Time(); // start time of the step
			o->nextbomb = Time() + ENEMY_BIG_NEXTBOMB; // time of next bomb
			o->sx = x; // start X
			o->x = x; //  current X
			o->sz = z; // start Z
			o->z = z; // current Z
		}
	}

	// moving objects
	o = Obj;
	for (i = OBJ_MAX; i > 0; i--)
	{
		// check if object is valid
		if (o->type != ENEMY_NO)
		{
			// current move
			m = &Moves[o->move]; // pointer to move
			n = m->startstep + o->istep; // current absolute step
			s = &Steps[n]; // pointer to current step

			// delta time
			dt = ((int)(Time() - o->time))>>10; // delta time of current step in [ms]
			if (dt < 0) dt = 0;

			// new step
			while (dt >= s->dt) // check length of the step
			{
				// end coordinates of this step
				x = o->sx + ((s->dx*dt)>>SPEED_SHIFT); // last X coordinate
				o->sx = x;
				o->x = x;
				z = o->sz + ((s->dz*dt)>>SPEED_SHIFT); // last Z coordinate
				o->sz = z;
				o->z = z;
				dt -= s->dt; // shift delta time

				// next step
				o->time = Time(); // new start time of the step
				o->istep++; // increase step index
				n++;
				if (o->istep >= m->numstep) // last step?
				{
					// next step to repeat
					n = m->repstep;
					o->istep = n - m->startstep;

					// end of move
					if (n < 0)
					{
						o->type = ENEMY_NO;
						break;
					}
				}

				// new step pointer
				s = &Steps[n]; // pointer to current step
			}

			// new current coordinate
			if (o->type != ENEMY_NO)
			{
				o->x = o->sx + ((s->dx*dt)>>SPEED_SHIFT);
				o->z = o->sz + ((s->dz*dt)>>SPEED_SHIFT);

				// generate bombs of big enemy
				if ((o->type == ENEMY_BIG) && (Level > 0))
				{
					if ((int)(Time() - o->nextbomb) >= 0)
					{
						GenBomb(ENEMY_BOMBD_MOVE, o->x, o->z);
						o->nextbomb = Time() + ENEMY_BIG_NEXTBOMB;
					}
				}

				// generate bombs of boss
				if (o->type == ENEMY_BOSS)
				{
					if ((int)(Time() - o->nextbomb) >= 0)
					{
						GenBomb(ENEMY_BOMBD_MOVE, o->x, o->z);
						o->nextbomb = Time() + ENEMY_BOSS_NEXTBOMB;
					}
				}
			}
		}

		// next object
		o++;
	}
}

// check hit enemy by the shot (coordinates are with FRAC; returns True if hit)
Bool EnemyHit(short x, short z)
{
	int i, t, n;
	sObj* o;
	const sMove* m;
	const sStep* s;
	short x2, z2;

	o = Obj;
	for (i = OBJ_MAX; i > 0; i--)
	{
		// check if object is valid
		t = o->type;
		if ((t == ENEMY_DISK) || (t == ENEMY_BIG) || (t == ENEMY_BOSS))
		{
			// check start protection time
			if ((int)(Time() - o->starttime) >= ENEMY_PROT_DELAY)
			{
				// get distance to the object
				x2 = o->x - x;
				z2 = o->z - z;

				// check coordinates
				if ((x2*x2 + z2*z2) <= ((ENEMY_HIT_DIST<<FRAC)*(ENEMY_HIT_DIST<<FRAC)))
				{
					// boss lives counter
					if (t == ENEMY_BOSS)
					{
						BossLives--;
						if (BossLives > 0) return True;
					}

					// prepare start size of the crash (0..7)
					n = o->z >> (4+FRAC);
					if (n < 0) n = 0;
					if (n > 7) n = 7;

					// change object to crash
					o->type = ENEMY_CRASH;
					o->move = ENEMY_CRASH_MOVE; // move index
					m = &Moves[o->move]; // pointer to move
					s = &Steps[m->startstep]; // pointer to first step
					o->istep = n; // relative index of the step
					o->starttime = Time();	// start time of move in [us]
					o->time = Time(); // start time of the step (with delay in group)
					o->sx = o->x; // start X
					o->sz = o->z; // start Z

					// play sound
					PLAYSOUND(ExplosionSnd);

					// count goal ... on sublevel 3 only boss may count last goal
					if ((SubLevel != 3) || (Goals > 1) || (t == ENEMY_BOSS))
					{
						// decrease goals
						DecGoals();

						// add score
						AddScore(EnemyPoint[t]);
					}
					return True;
				}
			}
		}

		// next object
		o++;
	}

	return False;
}
