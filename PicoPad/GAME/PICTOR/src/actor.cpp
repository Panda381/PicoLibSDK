
// ****************************************************************************
//
//                                  Actors
//
// ****************************************************************************

#include "../include.h"

// actor
int ActInx = 0;		// index of current actor (0=Jill)
sActorTemp Actor;	// current actor template
int ActorPhase;		// actor animation phase
int ActorX;		// actor middle X coordinate
int ActorY;		// actor middle Y coordinate
int ActorMinX, ActorMaxX; // actor's middle min/max X coordinate
int ActorMinY, ActorMaxY; // actor's middle min/max Y coordinate
int ActorJumpSpeed;	// actor's current jump speed
int ActorReload;	// counter to reload actor's weapon
int BloodTime;		// blood timer (0 = none)
int BloodX, BloodY;	// blood coordinate

// actor templates
const sActorTemp ActorTemp[ACT_NUM] = {
	// name			missile name		pal			img		w		wall			h		trans			walk
	{ "Bara",		"Shurigen",		JillImg_Pal,		JillImg,	JILL_W,		JILL_WALL,		JILL_H,		JILL_TRANS,		True,	},
	{ "Bird",		"Seed",			BirdImg_Pal,		BirdImg,	BIRD_W,		BIRD_WALL,		BIRD_H,		BIRD_TRANS,		False,	},
	{ "Parrot",		"Berry",		ParrotImg_Pal,		ParrotImg,	PARROT_W,	PARROT_WALL,		PARROT_H,	PARROT_TRANS,		False,	},
	{ "Scarabeus",		"Sand Ball",		ScarabeusImg_Pal,	ScarabeusImg,	SCARABEUS_W,	SCARABEUS_WALL,		SCARABEUS_H,	SCARABEUS_TRANS,	True,	},
	{ "Stingray",		"Lightning",		StingrayImg_Pal,	StingrayImg,	STINGRAY_W,	STINGRAY_WALL,		STINGRAY_H,	STINGRAY_TRANS,		False,	},
	{ "Ice Bird",		"Ice Crystal",		IcebirdImg_Pal,		IcebirdImg,	ICEBIRD_W,	ICEBIRD_WALL,		ICEBIRD_H,	ICEBIRD_TRANS,		False,	},
	{ "Dragon",		"Fire Ball",		DragonImg_Pal,		DragonImg,	DRAGON_W,	DRAGON_WALL,		DRAGON_H,	DRAGON_TRANS,		False,	},
	{ "Ghost",		"Screaming",		GhostImg_Pal,		GhostImg,	GHOST_W,	GHOST_WALL,		GHOST_H,	GHOST_TRANS,		False,	},
	{ "R2-D2",		"Laser",		R2D2Img_Pal,		R2D2Img,	R2D2_W,		R2D2_WALL,		R2D2_H,		R2D2_TRANS,		True,	},
	{ "UFO",		"Phaser",		UfoImg_Pal,		UfoImg,		UFO_W,		UFO_WALL,		UFO_H,		UFO_TRANS,		False,	},
	{ "Rocket Squirrel",	"Rocket",		SquirrelImg_Pal,	SquirrelImg,	SQUIRREL_W,	SQUIRREL_WALL,		SQUIRREL_H,	SQUIRREL_TRANS,		False,	},
	{ "Gingerbread Man",	"Sugar Bomb",		GingerbreadImg_Pal,	GingerbreadImg,	GINGERBREAD_W,	GINGERBREAD_WALL,	GINGERBREAD_H,	GINGERBREAD_TRANS,	True,	},
	{ "Devil Bird",		"Hole of Nothingness",	DevilImg_Pal,		DevilImg,	DEVIL_W,	DEVIL_WALL,		DEVIL_H,	DEVIL_TRANS,		False,	},
};

// activate blood
void SetBlood(int x, int y)
{
	if (y < BG_Y_MIN + BLOOD_H/2 + 1) y = BG_Y_MIN + BLOOD_H/2 + 1;
	if (y > BG_Y_MAX - BLOOD_H/2 - 1) y = BG_Y_MAX - BLOOD_H/2 - 1;
	if (x < BLOOD_W/2 + 1) x = BLOOD_W/2 + 1;
	if (x > WIDTH - BLOOD_W/2 - 1) x = WIDTH - BLOOD_W/2 - 1;

	BloodX = x;
	BloodY = y;
	BloodTime = BLOOD_TIME;

	if (SoundMode != SOUNDMODE_OFF)
		PlaySoundChan(SOUNDCHAN_ACTORHIT, FailSnd, 2*sizeof(FailSnd), False, 1, 1.0f, SNDFORM_ADPCM, IMA_SAMPBLOCK);
}

// display blood
void DispBlood()
{
	if (BloodTime > 0)
		DrawBlit4Pal(BloodImg, BloodImg_Pal,		// image data and palettes
			0, 0,					// source X, Y
			BloodX - BLOOD_W/2, BloodY - BLOOD_H/2, // destination X, Y
			BLOOD_W, BLOOD_H,		// width, height
			BLOOD_WS, BLOOD_TRANS);		// source total width
}

// shift blood
void ShiftBlood()
{
	if (BloodTime > 0) BloodTime--;
}

// switch actor
void SetActor(int actinx)
{
	// prepare number of actors (including Jill)
#if DEB_ALLACTORS		// 1=all actors are available
	int actnum = ACT_NUM;
#else
	int actnum = Level + 2;
	if (actnum > ACT_NUM) actnum = ACT_NUM;
#endif

	// overflow index
	if (actinx < 0) actinx += actnum;
	if (actinx >= actnum) actinx -= actnum;
	if (actinx < 0) actinx = 0;
	if (actinx >= actnum) actinx = actnum-1;

	// reset animation phase
	ActorPhase = 0;

	// set maximum missile reload (to avoid cheating on actor switch)
	ActorReload = MissileTemp[actinx].reload;

	// set new actor
	ActInx = actinx;
	memcpy(&Actor, &ActorTemp[actinx], sizeof(Actor));

	// min/max X coordinate
	ActorMinX = ACT_MINX + Actor.w/2;
	ActorMaxX = ACT_MAXX - Actor.w/2;
	if (ActorX < ActorMinX) ActorX = ActorMinX;
	if (ActorX > ActorMaxX) ActorX = ActorMaxX;

	// min/max Y coordinate
	ActorMinY = ACT_MINY + Actor.h/2;
	ActorMaxY = ACT_MAXY - Actor.h/2;
	if (ActorY < ActorMinY) ActorY = ActorMinY;
	if (ActorY > ActorMaxY) ActorY = ActorMaxY;

	// not jumping
	ActorJumpSpeed = 0;
}

// initialize actor on a new game
void InitActor()
{
	// set currect actor
	SetActor(ActInx);

	// start position
	ActorX = ActorMinX;
	if (Actor.walk)
		ActorY = ActorMaxY; // Y of walking actor
	else
		ActorY = (ActorMinY + ActorMaxY)/2; // Y of flying actor
}

// display actor
void DispActor()
{
	// display shadow
	int x = ActorX - SHADOW_W/2;
	if (ActInx == 4) x -= 10; // correction X for Stingray
	DrawBlit1Shadow(ShadowImg,		// source 1-bit image
			0,			// source X
			0,			// source Y
			x,			// destination X
			ACT_SHADOWY,		// destination Y
			SHADOW_W,		// image width
			SHADOW_H,		// image height
			SHADOW_WS,		// image total width
			11);			// shadow intensity (0=black .. 15=light)

	// display actor
	DrawBlitPal(	Actor.img,		// source image data
			Actor.pal, 		// source image palettes
			ActorPhase*Actor.w,	// source X
			0,			// source Y
			ActorX - Actor.w/2,	// destination X
			ActorY - Actor.h/2,	// destination Y
			Actor.w,		// image width
			Actor.h,		// image height
			Actor.wall,		// image total width
			Actor.trans);		// transparent color
}

// shift actor animation
void ShiftActor()
{
	// actor is moving (if not jumping)
	if (!Actor.walk || (ActorY >= ActorMaxY))
	{
		ActorPhase++;
		if (ActorPhase >= ((ActInx == ACT_JILL) ? JILL_RUNNUM : 4)) ActorPhase = 0;
	}
}

// falling actor
void FallActor()
{
	if (Actor.walk)
	{
		ActorY += ActorJumpSpeed;
		if (ActorY >= ActorMaxY)
		{
			ActorY = ActorMaxY;
			ActorJumpSpeed = 0;
		}
		else
		{
			ActorJumpSpeed++;
			if (ActInx == ACT_JILL)
			{
				if (ActorJumpSpeed < 0)
					ActorPhase = JILL_JUMPUP;
				else
					ActorPhase = JILL_JUMPDN;
			}
			else
				ActorPhase = 0;
		}
	}
}

// control actor (returns False to break game)
Bool CtrlActor()
{
	// key input
	switch (KeyGet())
	{
	// A: shooting
	//case KEY_A:
	//	{
	//		NewGame(Level+1);
	//	}
	//	break;

	// B: Next actor
	case KEY_B:
		SetActor(ActInx+1);
		break;

	// X: previous actor
	case KEY_X:
		SetActor(ActInx-1);
		break;

	// Y: game menu
	case KEY_Y:
		if (!GameMenu()) return False;

		// display header
		DispHeader();

		// display foot
		DispFoot();

		// play music
		MusicStart();
		break;
	}

	// Shooting
	if (KeyPressed(KEY_A))
	{
		// generate missile
		GenMissile();
	}

	// Key left
	if (KeyPressed(KEY_LEFT))
	{
		ActorX -= ACT_SPEEDX;
		if (ActorX < ActorMinX) ActorX = ActorMinX;
	}

	// Key right
	if (KeyPressed(KEY_RIGHT))
	{
		ActorX += ACT_SPEEDX;
		if (ActorX > ActorMaxX) ActorX = ActorMaxX;
	}

	// Key up
	if (KeyPressed(KEY_UP))
	{
		// jump
		if (Actor.walk)
		{
			if (ActorY == ActorMaxY) ActorJumpSpeed = -ACT_JUMPSPEED;
		}
		else
		{
			ActorY -= ACT_SPEEDY;
			if (ActorY < ActorMinY) ActorY = ActorMinY;
		}
	}

	// Key down
	if (!Actor.walk && KeyPressed(KEY_DOWN))
	{
		ActorY += ACT_SPEEDY;
		if (ActorY > ActorMaxY) ActorY = ActorMaxY;
	}

	// jumping/falling actor
	FallActor();

	return True;
}

// hit actor
void HitActor()
{
	int i, x, y, dx, dy;

	// hit by bullets
	const sBulletTemp* temp;
	sBullet* b = Bullet;
	for (i = 0; i < BULLET_MAX; i++)
	{
		temp = b->temp;
		if (temp != NULL)
		{
			// get bullet coordinates
			x = b->x;
			y = b->y;

			// check distance
			dx = x - ActorX;
			dy = y - ActorY;
			if (dx*dx + dy*dy <= ACT_HIT_DIST*ACT_HIT_DIST)
			{
				// destroy this bullet
				b->temp = NULL;

				// decrease life
				Life -= Actor.walk ? ACT_HIT_WALK : ACT_HIT_FLY;
				if (Life < 0) Life = 0;

				// activate blood
				SetBlood(x, y);
			}
		}
		b++;
	}

	// hit by enemies
	const sEnemyTemp* te;
	sEnemy* e = Enemy;
	for (i = 0; i < ENEMY_MAX; i++)
	{
		te = e->temp;
		if (te != NULL)
		{
			// get enemy coordinates
			x = e->x;
			y = e->y;

			// check distance
			dx = x - ActorX;
			dy = y - ActorY;
			if (dx*dx + dy*dy <= ACT_HIT_DIST*ACT_HIT_DIST)
			{
				// destroy this enemy
				e->temp = NULL;

				// increase score
				Score += EnemyScore[e->inx];
				AddPoint(EnemyScore[e->inx], x, y);

				// decrease life
				Life -= Actor.walk ? ACT_HIT_WALK : ACT_HIT_FLY;
				if (Life < 0) Life = 0;

				// activate blood
				SetBlood(x, y);
			}
		}
		e++;
	}
}
