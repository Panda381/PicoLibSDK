
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// ============================================================================
//                             Main function
// ============================================================================

// main function
int main()
{
	// load top list and max. score
	MaxScore = LoadTop();

	// main loop
	while (True)
	{
		// Title screen
		if (!TitleScreen()) ResetToBootLoader();

		// game
		Game();
		StopAllSound();

		// game end
		if ((Life <= 0) || (Score > MaxScore)) GameEnd();
	}
}
