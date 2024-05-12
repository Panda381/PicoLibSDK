
// ****************************************************************************
//
//                                   Foot
//
// ****************************************************************************

// Foot (Y = 220..239)
#define FOOT_Y		(BG3_Y+BG3_HEIGHT) // background Y coordinate (= 220)
#define FOOT_HEIGHT	20		// foot height

#define BG_Y_MAX	FOOT_Y		// background maximal Y coordinate

#define LIFE_INIT	100		// init life
#define LIFE_MAX	500		// max. life

// life (0..LIFE_MAX)
extern int Life;

// display life
void DispLife();

// display load weapon
void DispReload();

// display time
void DispTime();

// display foot
void DispFoot();
