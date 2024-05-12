
// ****************************************************************************
//
//                               Game menu
//
// ****************************************************************************

#define MENU_COL_KEY	COL_YELLOW	// menu color - key
#define MENU_COL_MENU	COL_WHITE	// menu color - label

// request to do screenshot
extern Bool ReqScreenShot;

// top list entry
#define TOPNAME_LEN	8	// top name length
typedef struct {
	s32	score;			// score
	char	name[TOPNAME_LEN];	// name with spaces
} sTop;

#define TOP_NUM		8	// number of top entries
extern sTop Top[TOP_NUM];	// top list

// load top list (returns max. score)
s32 LoadTop();

// save top list (returns False on error)
Bool SaveTop();

// display top score
void DispTop();

// game menu (returns False to exit)
Bool GameMenu();
