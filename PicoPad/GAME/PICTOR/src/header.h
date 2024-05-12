
// ****************************************************************************
//
//                                  Header
//
// ****************************************************************************

// Header (Y = 0..19)
#define TIT_Y		0	// header Y coordinate
#define TIT_HEIGHT	20	// header height

#define HEADER_COL	COL_DKBLUE	// header background color
#define HEADER_FRAMECOL	COL_LTBLUE 	// header frame color

// current and max score
extern int Score;
extern int MaxScore;

// display current score
void DispScore();

// display max. score
void DispMaxScore();

// display Picopad
void DispPico();

// display header
void DispHeader();
