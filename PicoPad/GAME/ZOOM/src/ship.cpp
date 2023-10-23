
// ****************************************************************************
//
//                               Space Ship
//
// ****************************************************************************

#include "include.h"

short	ShipX;		// ship X coordinate, with FRAC fraction 
short	Speed;		// ship speed, with FRAC fraction 
short	ShipInCrash;	// >1 ship is in crash
u32	ShipCrashTime;	// ship crash animation time
u32	ShipNewTime;	// new ship time (to protect on start)

// display ship
void DispShip()
{
	int phase, y;

	// ship in crash
	if (ShipInCrash > 0)
	{
		if (ShipInCrash < SHIP_CRASH_NUM)
		{
			y = HEIGHT-30-(Speed>>(FRAC+3));
			if (SubLevel == 3) y = HEIGHT-40;
			DrawBlit4Pal(ExplosionImg, ExplosionImg_Pal, 0, ShipInCrash*EXPLOSION_IMG_H,
				(ShipX>>FRAC)-EXPLOSION_IMG_W/2, y-15, EXPLOSION_IMG_W,
				EXPLOSION_IMG_H, EXPLOSION_IMG_W, EXPLOSION_IMG_KEY);
		}
		return;
	}

	// display shadow
	if (SubLevel != 3)
	{
		DrawBlit4Pal(ShipImg, ShipImg_Pal, 0, SHIP_SHADOW*SHIP_IMG_H, (ShipX>>FRAC)-SHIP_IMG_W/2,
			HEIGHT-20, SHIP_IMG_W, SHIP_IMG_H, SHIP_IMG_W, SHIP_IMG_KEY);
	}

	// display ship
	phase = SHIP_UP;
	if (KeyPressed(KEY_LEFT)) phase = SHIP_LEFT;
	if (KeyPressed(KEY_RIGHT)) phase = SHIP_RIGHT;
	y = HEIGHT-30-(Speed>>(FRAC+3));
	if (SubLevel == 3) y = HEIGHT-40;
	DrawBlit4Pal(ShipImg, ShipImg_Pal, 0, phase*SHIP_IMG_H, (ShipX>>FRAC)-SHIP_IMG_W/2,
		y, SHIP_IMG_W, SHIP_IMG_H, SHIP_IMG_W, SHIP_IMG_KEY);
}

// crash ship
void ShipCrash()
{
#ifdef DEBUG_NOCRASH		// no crash spaceship
	return;
#endif

	// already in crash
	if (ShipInCrash > 0) return;

	// ship protection
	if (Time() - ShipNewTime < SHIP_NEW_TIME) return;

	// ship crash flag
	ShipInCrash = 1;

	// ship crash animation time
	ShipCrashTime = Time();

	// play sound
	PLAYSOUND(ExplosionSnd);
}

// check hit ship by enemy
void ShipEnemyCrash()
{
	int i, t;
	sObj* o;
	short x2, z2;

	// ship coordinate
	short x = ShipX;
	short z = 5<<FRAC;

	o = Obj;
	for (i = OBJ_MAX; i > 0; i--)
	{
		// check if object is valid
		t = o->type;
		if ((t == ENEMY_DISK) || (t == ENEMY_BIG) || (t == ENEMY_BOSS) || (t == ENEMY_BOMB))
		{
			// check start protection time
			if ((int)(Time() - o->starttime) >= ENEMY_PROT_DELAY)
			{
				// get distance to the object
				x2 = o->x - x;
				z2 = o->z - z;

				// check coordinates
				if ((x2*x2 + z2*z2) <= (SHIP_HIT_DIST<<FRAC)*(SHIP_HIT_DIST<<FRAC))
				{
					// crash ship
					ShipCrash();
				}
			}
		}

		// next object
		o++;
	}
}

// ship service
void DoShip(u32 dt, u32 dz)
{
	// shift explosion
	if (ShipInCrash > 0)
	{
		// check animation time
		if (Time() - ShipCrashTime >= SHIP_ANIM_TIME)
		{
			ShipCrashTime = Time();
			ShipInCrash++;

			// end of explosion, new ship
			if ((ShipInCrash == SHIP_CRASH_WAIT) && (Lives > 0))
			{
				// decrease lives
				Lives--;
				if (Fuel <= 0) Fuel = (FUEL_MAX*3/4) << FRAC; // new fuel
				ShipInCrash = 0;
				ShipNewTime = Time(); // ship protection
			}
		}
		return;
	}

	// move ship left
	if (KeyPressed(KEY_LEFT))
	{
		ShipX -= dt>>7;
		if (ShipX < SHIP_MINX) ShipX = SHIP_MINX;
	}

	// move ship right
	if (KeyPressed(KEY_RIGHT))
	{
		ShipX += dt>>7;
		if (ShipX > SHIP_MAXX) ShipX = SHIP_MAXX;
	}

	// move ship up
	if (KeyPressed(KEY_UP) && (SubLevel != 3))
	{
		Speed += dt>>7;
		if (Speed > SHIP_SPEEDMAX) Speed = SHIP_SPEEDMAX;
	}

	// move ship down
	if (KeyPressed(KEY_DOWN) && (SubLevel != 3))
	{
		Speed -= dt>>7;
		if (Speed < SHIP_SPEEDMIN) Speed = SHIP_SPEEDMIN;
	}
}

