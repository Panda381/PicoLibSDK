
// ****************************************************************************
//
//                               Title screen
//
// ****************************************************************************

#include "../include.h"

// title screen (returns False to exit)
Bool TitleScreen()
{
	int strip;
#define TOPBUFN 30
	char buf[TOPBUFN];

	// flush keys
	KeyFlush();

	// loop
	while (True)
	{
		// draw title screen
		for (strip = DISP_STRIP_NUM; strip > 0; strip--)
		{
			DispSetStripNext();
			DrawImgRle(TitleImg, TitleImg_Pal, 0, 0, WIDTH, HEIGHT);
			MemPrint(buf, TOPBUFN, "max %06d", MaxScore);
			DrawText(buf, 4, 240-13, COL_DKGREEN);
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

		// keyboard
		switch (KeyGet())
		{
		case KEY_A:
			return True;

		case KEY_Y:
			if (!GameMenu()) return False;
			break;
		}
	}
}
