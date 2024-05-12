
// ****************************************************************************
//
//                                Game
//
// ****************************************************************************

extern int Level;		// current scene level - 1
extern int CurrentFrame;	// current game frame
extern int MaxFrame;		// max. game frames

// initialize new game (level = 0..; returns False to break)
Bool NewGame(int level);

// Game
void Game();
