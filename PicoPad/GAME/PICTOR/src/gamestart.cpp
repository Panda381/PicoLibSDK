
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// display game start
void DispGameStart(Bool prompt)
{
	// start position, shift animation
	ActorX = ActorMinX;
	if (Actor.walk)
	{
		// Jill
		if (ActInx == ACT_JILL)
		{
			// Y of walking actor
			ActorY = ActorMaxY + 6;

			// set phase of standing actor animation
			ActorPhase = JILL_STAND;
		}

		// other walking actors
		else
		{
			// Y of walking actor
			ActorY = ActorMaxY;

			// set phase of standing actor animation
			ActorPhase = 0;
		}
	}
	else
	{
		ActorY = (ActorMinY + ActorMaxY)/2; // Y of flying actor

		// shift flying actor animation
		ShiftActor();
	}

	int strip;

	// display
	for (strip = DISP_STRIP_NUM; strip > 0; strip--)
	{
		// next strip
		DispSetStripNext();

		// display shadow backgrounds
		DrawImgPalDbl(Back.img1, Back.pal1, 	// image data and palettes
			0, 0,				// source X, Y
			0, BG_Y,			// destination X, Y
			WIDTH, BG_HEIGHT,		// width, height
			BG_WIDTH);			// source total width

		DrawBlitPal(Back.img2, Back.pal2, 	// image data and palettes
			0, 0,				// source X, Y
			0, BG2_Y+Back.dy,		// destination X, Y
			WIDTH, BG2_HEIGHT-Back.dy,	// width, height
			BG2_WIDTH, BG2_TRANST);		// source total width

		DrawBlitPal(Back.img3, Back.pal3, 	// image data and palettes
			0, 0,				// source X, Y
			0, BG3_Y,			// destination X, Y
			WIDTH, BG3_HEIGHT,		// width, height
			BG3_WIDTH, BG3_TRANST);		// source total width

		// display header
		DispHeader();

		// display foot
		DispFoot();

		// shadow screen
		DispShadow();

		// display actor
		DispActor();

#define OPEN_X	190
		int y = (Level < BG_NUM) ? 20 : 60;

		// Title
		int len = DecNum(DecNumBuf, Level + 1, 0);
		len += 16; // add text "Entering Sector "
		int x = OPEN_X - len*8/2;
		DrawText("Entering Sector", x, y, COL_WHITE);
		x += 16*8;
		DrawText(DecNumBuf, x, y, COL_WHITE);
		y += 16;

		len = StrLen(Back.name);
		DrawText2(Back.name, OPEN_X - len*16/2, y, COL_AZURE);

		// new helper
		if (Level < BG_NUM)
		{
			y += 32; // 20 + 16 + 32 = 68
			const sActorTemp* a = &ActorTemp[BackInx+1];
			// y_help = 190, height_of_area = 190 - 68 = 122, height_of_actor = 16 + 16 + a->h + 16
			y += (122 - (16 + 16 + a->h + 16))/2;

			// Helper
			DrawText("New Helper", OPEN_X - 10*8/2, y, COL_WHITE);
			y += 16;
			len = StrLen(a->name);
			DrawTextW(a->name, OPEN_X - len*16/2, y, COL_GREEN);
			y += 16;

			// draw helper
			DrawBlitPal(	a->img,			// source image data
					a->pal, 		// source image palettes
					0,			// source X
					0,			// source Y
					OPEN_X - a->w/2,	// destination X
					y,			// destination Y
					a->w,			// image width
					a->h,			// image height
					a->wall,		// image total width
					a->trans);		// transparent color
			y += a->h;

			// Attack
			len = StrLen(a->missile) + 7;
			x = OPEN_X  - len*8/2;
			DrawText("Attack:", x, y, COL_RED);
			DrawText(a->missile, x+7*8, y, COL_RED);
		}

		// help
		if (prompt)
		{
			DrawText("Press B/X to transform", OPEN_X - 22*8/2, 190, COL_YELLOW);
			DrawText("Press A to start game", OPEN_X - 21*8/2, 205, COL_YELLOW);
		}

		// display update
		DispUpdate();
	}
}

// open game start (return False to break)
Bool GameStart()
{
	// display game start
	DispGameStart(False);

	// wait 1 second
	WaitMs(1000);

	// flush keys
	KeyFlush();

	while (True)
	{
		// display game start
		DispGameStart(True);

		// key control
		switch (KeyGet())
		{
		// start game
		case KEY_A:
			while (KeyPressed(KEY_A)) {}
			return True;

		// next actor
		case KEY_B:
			SetActor(ActInx+1);
			ActorReload = 0;
			break;

		// previous actor
		case KEY_X:
			SetActor(ActInx-1);
			ActorReload = 0;
			break;

		// menu
		case KEY_Y:
			if (!GameMenu()) return False;
			break;
		}

#if USE_PICOPADVGA || USE_PICOPADHSTX
		WaitMs(45);
#endif
	}
}
