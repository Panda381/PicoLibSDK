
// ****************************************************************************
//
//                                Game
//
// ****************************************************************************

#include "../include.h"

int Level;		// current scene level - 1
int CurrentFrame;	// current game frame
int MaxFrame;		// max. game frames

// initialize new game (level = 0..; returns False to break)
Bool NewGame(int level)
{
	StopAllSound();

	// set new level
	Level = level;
	CurrentFrame = 0;	// current game frame
	MaxFrame = GAMELEN;	// max. game frames
	BloodTime = 0;		// blood timer (0 = none)

	// initialize random number generator (will be used in InitBonus)
	EnemyRandSeed = Level + 123456;

	// initialize backgrounds on new level
	InitBack();

	// prepare current actor
	InitActor();

	// initialize actor missiles on new level
	InitMissile();

	// initialize enemies
	InitEnemy();

	// initialize enemy bullets on new level
	InitBullet();

	// initialize bonus on start of next level
	InitBonus();

	// initialize points on start of level
	InitPoint();

	// display game start
	if (!GameStart()) return False;

	// play music
	MusicStart();

	return True;
}

// Game
void Game()
{
	int strip;

	// reset score
	Score = 0;

	// reset bonuses
	BonusMask = 0;

	// life (0..LIFE_MAX)
	Life = LIFE_INIT;

	// actor 0
	ActInx = 0;

	// initialize new game
#ifdef STARTLEVEL	// start level-1 (default 0 = Sector 1)
	if (!NewGame(STARTLEVEL)) return;
#else
	if (!NewGame(0)) return;
#endif

	// display header
	DispHeader();

	// display foot
	DispFoot();

	// game loop
	while (True)
	{

// --- controls

		// generate new enemy
		NewEnemy();

		// control actor
		if (!CtrlActor()) break;

// --- display

		// display
		for (strip = DISP_STRIP_NUM; strip > 0; strip--)
		{
			// next strip
			DispSetStripNext();

			// display backgrounds
			DispBack();

			// display actor
			DispActor();

			// display enemies
			EnemyDisp();

			// display explosions
			ExplosionDisp();

			// display bullets
			BulletDisp();

			// display missiles
			MissileDisp();

			// display blood
			DispBlood();

			// display bonus
			BonusDisp();

			// display points
			PointDisp();

			// display header
			DispHeader();

			// display foot
			DispFoot();

			// display update
			DispUpdate();
		}

#if USE_SCREENSHOT		// use screen shots
		// request to do screenshot
		if (ReqScreenShot)
		{
			ReqScreenShot = False;
			ScreenShot();
		}
#endif

// --- move

		// shift and move enemies, shoot bullets
		EnemyShift();

		// shift explosions
		ExplosionShift();

		// shift bullets
		BulletShift();

		// shift missiles
		MissileShift();

		// hit enemy by missile
		EnemyHit();

		// shift background animation
		ShiftBack();

		// shift actor animation
		ShiftActor();

		// shift blood
		ShiftBlood();

		// shift bonus
		BonusShift();

		// shift points
		PointShift();

		// hit actor
		HitActor();

		// collect bonus
		BonusCollect();

		// shift game frame
		CurrentFrame++;

		// end level
#if !DEB_ENDLESS	// 1=endless level
		if (CurrentFrame >= MaxFrame)
		{
			NewGame(Level+1);
		}
#endif

#if USE_PICOPADVGA
		WaitMs(45);
#endif

		// game end
		if (Life <= 0) return;
	}
}
