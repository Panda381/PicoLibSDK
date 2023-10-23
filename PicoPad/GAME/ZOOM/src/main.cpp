
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

// game variables
int	Level;		// current game level (0..)
int	SubLevel;	// current game sub-level (0..)

// delay with corrections
void Delay(int ms)
{
	int i;

	// delay
	WaitMs(ms);

	// corrections
	u32 t = ms*1000;
	LastStarTime += t;
	TimeLastDisk += t;
	TimeLastBig += t;
	ShipNewTime += t;
	ShipCrashTime += t;
	LastShotTime += t;
	GoalTime += t;

	sObj* o = Obj;
	for (i = OBJ_MAX; i > 0; i--)
	{
		o->time += t;
		o->starttime += t;
		o->nextbomb += t;
		o++;
	}
}

// initialize new game
void NewGame()
{
	ShootSpeed = 8;	// shoot speed

#ifdef DEBUG_LIVES	// number of start lives (default 3)
	Lives = DEBUG_LIVES;	// number of lives
#else
	Lives = 3;	// number of lives
#endif

#ifdef DEBUG_LEVEL	// start level 0..4 (default 0)
	Level = DEBUG_LEVEL; // current game level (0..)
#else
	Level = 0;	// current game level (0..)
#endif

#ifdef DEBUG_SUBLEVEL	// start sublevel 0..3 (default 0)
	SubLevel = DEBUG_SUBLEVEL; // current game sub-level (0..3)
	Fuel = (FUEL_MAX*3/4) << FRAC; // fuel to maximum
#else
	SubLevel = 0;	// current game sub-level (0..3)
#endif

	Score = 0;	// current score
	LastScore = 0;	// score at last life addition
	Speed = SHIP_SPEEDDEF; // ship speed
	LastShotTime = Time() - SHOTTIME; // time of last shoot
}

// entering sector
void Enter()
{
	// clear screen
	DrawClear();

	// draw text
	DrawText("ENTERING SECTOR", (WIDTH-15*8)/2, 100, COL_WHITE);

	// display sector number
	MemPrint(DecNumBuf, DECNUMBUF_SIZE, "%d.%d", Level+1, SubLevel);
	DrawText(DecNumBuf, (WIDTH-3*8)/2, 112, COL_WHITE);

#ifdef DEBUG_SKIP_ENTER	// skip entering screen
	return;
#endif

	// play sound
	PLAYSOUND(EnteringSnd);

	// display update
	DispUpdate();

	// wait
	WaitMs(3600);

	// flush keyboard
	KeyFlush();
}

// display all game screen
void DispAll()
{
	int i, strip;

	for (strip = DISP_STRIP_NUM; strip > 0; strip--)
	{
		// next strip
		DispSetStripNext();

		// sector 3 is in the space
		if (SubLevel == 3)
		{
			// clear screen
			DrawClear();

			// draw stars
			DispStars();
		}
		else
		{
			// display horizon
			DrawImgRle(SkylineImg, SkylineImg_Pal, 0, 0, SKYLINE_IMG_W, SKYLINE_IMG_H);

			// display city
			if (Level == 1)
				DrawBlit4Pal(City1Img, City1Img_Pal, 0, 0, (WIDTH - CITY1_IMG_W)/2, HORIZON - CITY1_IMG_H,
					CITY1_IMG_W, CITY1_IMG_H, CITY1_IMG_W, CITY1_IMG_KEY);
			else if (Level >= 2)
				DrawBlit4Pal(City2Img, City2Img_Pal, 0, 0, (WIDTH - CITY2_IMG_W)/2, HORIZON - CITY2_IMG_H,
					CITY2_IMG_W, CITY2_IMG_H, CITY2_IMG_W, CITY2_IMG_KEY);

			// display ground
			DispGround();
		}

		// display number of lives
		DispLives();

		// display fuel
		DispFuel();

		// display goals
		DispGoals();

		// display score
		DispScore();

		// display objects
		DispObj();

		// display gates
		DispGate();

		// display shots
		DispShots();

		// display ship
		DispShip();

		// display update
		DispUpdate();
	}

	// set off back buffers
	DispSetStripOff();
}

// initialize new level
void InitLevel()
{
	// fuel indicator to maximum, if it is first sublevel
	if (SubLevel == 0) Fuel = FUEL_MAX << FRAC;

	// entering sector
	Enter();
	
	// initialize ship
	ShipX = SHIP_DEFX;
	ShipInCrash = 0;	// >1 ship is in crash
	ShipNewTime = Time(); // ship protection

	// initialize stars
	InitStars();

	// reset shots on start of new scene
	ResetShots();

	// reset gates on start of new scene
	ResetGates();

	// initialize list of objects
	InitObj();
	TimeLastDisk = Time() - 5000000; // time of last generated small disk
	TimeLastBig = Time();	// time of last generated big enemy
	TimeLastBomb = Time();	// time of last generated bomb
	BossIsGen = False;	// boss is generated
	BossLives = ENEMY_BOSS_LIVES;	// boss lives counter

	// initialize goals
#ifdef DEBUG_GOALS // number of goals
	Goals = DEBUG_GOALS;
#else
	Goals = GoalsInit[SubLevel];
#endif

	// display all game screen
	DispAll();
}

// one game
void Game()
{
	u32 t, dt;
	int dz;

	// initialize new game
	NewGame();

	// initialize new level
	InitLevel();

	// game loop
	t = Time();
	while (True)
	{
		// display all graphics
		DispAll();

		// generate stars (only sublevel 3)
		if (SubLevel == 3)
		{
			// shift stars
			ShiftStars();

			// generate new stars
			NewStars();
		}

		// delta time
		dt = Time() - t;
		if (dt < 0) dt = 0;
		t += dt;
		if (dt > 150000) dt = 150000; // limit to 150 ms
		if (dt < 20000)
		{
			WaitUs(20000-dt);
			dt = 20000; // min. time 20 ms
		}

		// delta Z coordinate, with FRAC fraction
		if (SubLevel == 3)
			dz = (dt*(SHIP_SPEEDDEF>>FRAC))>>(21-FRAC);
		else
			dz = (dt*(Speed>>FRAC))>>(21-FRAC);

		// update score by distance
		AddScore(dz);

		// subtract fuel
		SubFuel(dt>>13);

		// shift ground
		GrassPhase -= dz;
		while (GrassPhase < 0) GrassPhase += GROUND_DY<<FRAC;

		// serve objects
		DoObj();

		// check hit ship by enemy
		ShipEnemyCrash();

		// ship service
		DoShip(dt, dz);

		// shooting service
		Shooting(dt);

		// gate service
		Gating(dz);

		// level completed
		if ((Goals <= 0) && ((int)(Time() - GoalTime) > 1000000))
		{
			// increase sublevel
			SubLevel++;
			if (SubLevel > 3)
			{
				SubLevel = 0;
				Level++;
			}

			// initialize new level
			InitLevel();
		}

		// end game
		if ((ShipInCrash > SHIP_CRASH_WAIT+2) && (Lives == 0)) return;

		// key
		switch (KeyGet())
		{
		// quit game
		case KEY_Y:
			return;

#if USE_SCREENSHOT		// use screen shots
		case KEY_X:
			ScreenShot();
			break;
#endif
		}
	}
}

// main function
int main()
{
	// calculate perspective projection table
	CalcPerspTab();

	while (True)
	{
		// open menu (returns False to quit the game)
		if (!Open()) ResetToBootLoader();

		// play game
		Game();
	}

}
