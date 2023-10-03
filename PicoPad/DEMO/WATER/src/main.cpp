
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	float phase, duckx2;
	int y, y0, y2, duckx;

	// play sound
	PLAYSOUNDREP(RiverSnd);

	// display top part of the screen
	DrawImgPal(ForestImg, ForestImg_Pal, 0, 0, 0, 0, WIDTH, IMGH, WIDTH);

	// main loop
	phase = 0;
	duckx2 = 330;
	while (True)
	{
		// wait for VSync
		VgaWaitVSync();

		// animate water
		for (y = IMGH; y < HEIGHT; y++)
		{

#define WAVEAMP		0.1f	// wave amplitude
#define WAVEPERSP	20.0f	// perspective deformation
#define WAVELEN		100	// wave length

#if USE_MINIVGA
#define ANIMSPEED	0.1f	// animation speed
#else
#define ANIMSPEED	0.2f	// animation speed
#endif

			y0 = y - IMGH; // Y relative to top of water
			y2 = (int)(IMGH - 1 - (sinf(phase + WAVELEN/sqrtf(y0/WAVEPERSP + 1))*WAVEAMP + 1)*y0);
			if (y2 < 0) y2 = 0;
			if (y2 >= IMGH) y2 = IMGH-1;
			DrawImgPal(ForestImg, ForestImg_Pal, 0, y2, 0, y, WIDTH, 1, WIDTH);
		}

		// display duck
		duckx = float2int(duckx2 + 0.5f);
		DrawBlitPal(DuckImg, DuckImg_Pal, (duckx & 3)*64, 0, duckx, 180, 64, 29, 256, 0xA158);

		// increase animation phase
		phase += ANIMSPEED;

		// shift duck
		duckx2 -= 5*ANIMSPEED;
		if (duckx2 < -100) duckx2 = 330;

		// update display
		DispUpdate();

		switch (KeyGet())
		{
		// reset to boot loader
		case KEY_Y: ResetToBootLoader();

#if USE_SCREENSHOT		// use screen shots
		case KEY_X:
			ScreenShot();
			break;
#endif
		}
	}
}
