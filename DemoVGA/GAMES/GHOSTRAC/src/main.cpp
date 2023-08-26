
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// game state
float Speed = 0; // current speed
float CarX = 8.5*TILESIZE; // car X coordinate
float CarY = 15.5*TILESIZE; // car Y coordinate
float CarDir = 0; // car direction
float CarTurn = 0; 
int CurTile = 0; // current tile under car

u64 StartTime; // start time
int ThisTime = 0; // this time
int LastTime = 0; // last time
int BestTime = 0; // best time
Bool NewBest = False; // new best time
int CheckNext = GRASS; // next check point (START = search start, GRASS = start new game)
Bool CheckOK[CHECKNUM]; // check points processed
const u16 CheckCol[CHECKNUM] = { COL_CYAN, COL_YELLOW, COL_RED }; // check point color

// current history
u32 RecordStep; // next record step after 1 second
int RecordInx; // record index
float RecordX[RECORDMAX]; // record of X coordinates
float RecordY[RECORDMAX]; // record of Y coordinates
float RecordDir[RECORDMAX]; // record of direction

// best history
int RecordBestNum; // number of entries
float RecordBestX[RECORDMAX]; // record of X coordinates
float RecordBestY[RECORDMAX]; // record of Y coordinates
float RecordBestDir[RECORDMAX]; // record of direction

// tile map
//  grass 0 ... GRASS
// |  start 1 ... START
// |  check A (blue) 2 ... CHECKA
// |  check B (yellow) 3 ... CHECKB
// |  check C (red) 4 ... CHECKC
// |  straight 5
// -- straight 6
//
//  -- 7  8
// |   9  10

// --  11  12
//   | 13  14
   
// |   15  16
//  -- 17  18

//   | 19  20
// --  21  22

const u8 TileMap[MAPSIZE] = {
//	0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 1
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 3
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 4
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 5
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 6
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 7
	0, 0, 0, 0, 0, 0, 0, 0, 7, 8, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,11,12, 0, 0, 0, 0, 0, 0, 0, 0,	// 8
	0, 0, 0, 0, 0, 0, 0, 0, 9,10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,13,14, 0, 0, 0, 0, 0, 0, 0, 0,	// 9
	0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0,	// 10
	0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 7, 8, 6, 6, 6,11,12, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0,	// 11
	0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 9,10, 0, 0, 0,13,14, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0,	// 12
	0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 5, 0, 0, 0, 0, 0,15,16, 0, 0, 0,19,20, 0, 0, 0, 0, 0, 0, 0, 0,	// 13
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0,17,18, 6, 6, 6,21,22, 0, 0, 0, 0, 0, 0, 0, 0,	// 14
	0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 15
	0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0,15,16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 16
	0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0,17,18, 6, 6, 6, 6, 6, 6, 6, 6,11,12, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 17
	0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,13,14, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 18
	0, 0, 0, 0, 0, 0, 0, 0,15,16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 19
	0, 0, 0, 0, 0, 0, 0, 0,17,18, 6, 6, 6, 6, 6, 6, 6,11,12, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 20
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,13,14, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 21
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,16, 0,19,20, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 22
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,17,18, 6,21,22, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 23
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 24
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 25
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 26
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 27
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 28
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 29
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 30
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 31
};	

// list of car images
const u8* const CarImgList[CARIMGNUM] = {
	Car0Img,
	Car1Img,
	Car2Img,
	Car3Img,
	Car4Img,
	Car5Img,
	Car6Img,
	Car7Img,
	Car8Img,
	Car9Img,
	Car10Img,
	Car11Img,
	Car12Img,
	Car13Img,
	Car14Img,
	Car15Img,
	Car16Img,
};

const u16* const CarImgPalList[CARIMGNUM] = {
	Car0Img_Pal,
	Car1Img_Pal,
	Car2Img_Pal,
	Car3Img_Pal,
	Car4Img_Pal,
	Car5Img_Pal,
	Car6Img_Pal,
	Car7Img_Pal,
	Car8Img_Pal,
	Car9Img_Pal,
	Car10Img_Pal,
	Car11Img_Pal,
	Car12Img_Pal,
	Car13Img_Pal,
	Car14Img_Pal,
	Car15Img_Pal,
	Car16Img_Pal,
};

// list of ghost images
const u8* const GhostImgList[8] = {
	Ghost0Img,
	Ghost1Img,
	Ghost2Img,
	Ghost3Img,
	Ghost4Img,
	Ghost5Img,
	Ghost6Img,
	Ghost7Img,
};

const u16* const GhostImgPalList[8] = {
	Ghost0Img_Pal,
	Ghost1Img_Pal,
	Ghost2Img_Pal,
	Ghost3Img_Pal,
	Ghost4Img_Pal,
	Ghost5Img_Pal,
	Ghost6Img_Pal,
	Ghost7Img_Pal,
};

// projection matrix - terrain
sMat2D Mat;
//int MatInt[6];

// projection matrix - ghost
sMat2D Mat2;
//int Mat3Int[6];

const char TimeTxt[] = "TIME:";
const char LastTxt[] = "LAST:";
const char BestTxt[] = "BEST:";

char NumBuf[20];

// display time
void InfoTime(int x, int t, u16 col)
{
	if (t > 99*60 + 59) t = 99*60 + 59;
	int min = t/60;
	int sec = t - min*60;
	int n = DecNum(NumBuf, min, 0);
	NumBuf[n] = ':';
	n++;
	if (sec < 10)
	{
		NumBuf[n] = '0';
		n++;
	}
	DecNum(&NumBuf[n], sec, 0);
	DrawText(NumBuf, x, INFOY, col);
}

// display info row
void InfoRow()
{
#define COLTIT	COL_YELLOW
#define COLVAL COL_WHITE
#define COLBEST COL_GREEN

	int i;
	int x = 0;

	// clear
	DrawRect(0, INFOY, WIDTH, INFOH, COL_BLACK);

	// display this time
	DrawText(TimeTxt, x, INFOY, COLTIT);
	x += 5*8;
	InfoTime(x, ThisTime, COLVAL);
	x += 7*8;
	
	// display last time
	DrawText(LastTxt, x, INFOY, COLTIT);
	x += 5*8;
	InfoTime(x, LastTime, NewBest ? COLBEST : COLVAL);
	x += 7*8;

	// display best time
	DrawText(BestTxt, x, INFOY, COLTIT);
	x += 5*8;
	InfoTime(x, BestTime, NewBest ? COLBEST : COLVAL);
	x += 7*8;

	// display check points
	for (i = 0; i < CHECKNUM; i++)
	{
		DrawText(CheckOK[i] ? "\x03" : "\x02", x, INFOY, CheckCol[i]);
		x += 8;
	}
}

// main function
int main()
{
	int t0, t2, i, dif, xi, yi, offx, turn, strip;
	u8 tile;
	float dt;

	float x1, x2, x;
	float y1, y2, y;
	float d, a, a0;
	float k, s;
	int wd, hd;
	const u8* img;
	const u16* pal;
	int x0, y0;

	// initialize USB
	UsbHostInit();

	// connectint USB keyboard
	printf("\fconnecting USB keyboard...");
	while (!UsbKeyIsMounted()) { }

	// time mark
	t0 = Time();

	// main loop
	while (True)
	{
		// delta time (at seconds)
		t2 = (int)Time();
		dt = (t2 - t0)/1000000.f;
		t0 = t2;

		// accelerate
		if (UsbKeyIsPressed(HID_KEY_ARROW_UP)) Speed += ACCEL*dt;

		// slowdown
		if (UsbKeyIsPressed(HID_KEY_ARROW_DOWN)) Speed -= ACCEL*dt*2;

		switch (UsbGetChar())
		{
		// reset to boot loader
		case CH_ESC: ResetToBootLoader();
		}

		// auto-slow down
		Speed -= ACCELSLOW*dt;

		// grass slow down
		if (TileMap[CurTile] == GRASS) Speed -= GRASSSLOW*dt*Speed;

		// limit speed
		if (Speed > SPEEDMAX) Speed = SPEEDMAX;
		if (Speed < 0) Speed = 0;

		// turning
		if (Speed > 0)
		{
			// left
			if (UsbKeyIsPressed(HID_KEY_ARROW_LEFT)) CarTurn -= TURNACCEL*dt;

			// right
			if (UsbKeyIsPressed(HID_KEY_ARROW_RIGHT)) CarTurn += TURNACCEL*dt;

			// auto-turn
			if (CarTurn > 0)
			{
				CarTurn -= AUTOTURN*dt; //*(1+Speed/SPEEDMAX/5);
				if (CarTurn < 0) CarTurn = 0;
			}

			if (CarTurn < 0)
			{
				CarTurn += AUTOTURN*dt; //*(1+Speed/SPEEDMAX/5);
				if (CarTurn > 0) CarTurn = 0;
			}
		}

		// limit turn
		if (CarTurn < -TURNMAX) CarTurn = -TURNMAX;
		if (CarTurn > TURNMAX) CarTurn = TURNMAX;

		// turn car
		CarDir += CarTurn*dt*Speed;
		if (CarDir < 0) CarDir += PI2;
		if (CarDir >= PI2) CarDir -= PI2;

		// update car sound
		if (Speed == 0)
			StopSound();
		else
		{
			if (!PlayingSound())
			{
				PLAYSOUNDREP(EngineSnd);
			}
			SpeedSound(Speed/(SPEEDCOEF*10)/5);
		}

		// update car position
		float s, c;
		sincosf(CarDir, &s, &c);
		CarY += - Speed * dt * c;
		CarX += Speed * dt * s;

		// get current tile
		xi = (int)((CarX+1024*TILESIZE)/TILESIZE) & (MAPW-1);
		yi = (int)((CarY+1024*TILESIZE)/TILESIZE) & (MAPH-1);
		CurTile = xi + yi*MAPW;

		// DEBUG: check tile coordinates (visible as blinking tile under car) ... necessary to remove "const" in TileMap
		//TileMap[CurTile] ^= 1;

	// ==== calculations

		// move skyline
		offx = CarDir*2*SKYLINEW/PI;
		while (offx >= SKYLINEW) offx -= SKYLINEW;

		// update terrain
		Mat2D_PrepDrawImg(&Mat, TILESIZE, TILESIZE, CarX, CarY, WIDTH, WIDTH, 0, 0, CarDir, 0, TILESIZE);

		// record step delta
		dif = (int)(Time() - RecordStep);

		// display ghost
		if ((RecordBestNum > 0) && (RecordInx+1 < RecordBestNum))
		{
			i = RecordInx;

			// X coordinate
			x1 = RecordBestX[i];
			x2 = RecordBestX[i+1];
			x = (x2 - x1)*dif/1000000 + x1;
			x = x - CarX;

			// Y coordinate
			y1 = RecordBestY[i];
			y2 = RecordBestY[i+1];
			y = (y2 - y1)*dif/1000000 + y1;
			y = y - CarY;

			// direction
			d = RecordBestDir[i];

			// angle of vector to ghost
			a = atan2f(y, x);
			a0 = a;

			// distane of vector to ghost
			k = sqrtf(y*y + x*x);

			// view angle
			a = a - CarDir + PI/2;
			while (a < -PI) a += PI2;
			while (a > PI) a -= PI2;

			// check if view angle is out of view frustum (frustrum is +- 45 deg)
			if ((a >= -PI/3) && (a <= PI/3))
			{
				// ghost size
				s = 200/(k+50);
				wd = (int)(GHOSTW*s);
				hd = (int)(GHOSTH*s);
				if ((wd < 30) || (wd > 160)) wd = 0;

				// ghost direction
				d -= a0 - PI/2;
				d += PI/8; // rounding
				while (d < -PI) d += PI2;
				while (d > PI) d -= PI2;

				// ghost image
				i = (int)((d + PI)*8/PI2);
				if (i < 0) i = 0;
				if (i > 7) i = 7;
				img = GhostImgList[i];
				pal = GhostImgPalList[i];

				// ghost position on the screen
				x0 = (int)((tan(a) + 1)*WIDTH/2) - wd/2;
				y0 = INFOH + SKYLINEH - 20 + hd/2;
			}
		}

		// car
		turn = (int)((CarTurn + TURNMAX)/(2*TURNMAX)*CARIMGNUM - 0.5);
		if (turn < 0) turn = 0;
		if (turn >= CARIMGNUM) turn = CARIMGNUM-1;

	// ==== display

		for (strip = DISP_STRIP_NUM; strip > 0; strip--)
		{
			// next strip
			DispSetStripNext();

			// info row
			InfoRow();

			// update skyline
			DrawImgPal(SkylineImg, SkylineImg_Pal, 0, 0, -offx, SKYLINEY, SKYLINEW, SKYLINEH, SKYLINEW);
			DrawImgPal(SkylineImg, SkylineImg_Pal, 0, 0, WIDTH-offx, SKYLINEY, SKYLINEW, SKYLINEH, SKYLINEW);

			// update terrain
			DrawTile4PalMap(TilesPalImg, TilesPalImg_Pal, TileMap, MAPWBITS, MAPHBITS,
				TILEBITS, 0, MAPY, WIDTH, HEIGHT-HORIZONH, &Mat, HORIZ);

			// display ghost
			if ((RecordBestNum > 0) && (RecordInx+1 < RecordBestNum))
			{
				// check if view angle is out of view frustum (frustrum is +- 45 deg)
				if ((a >= -PI/3) && (a <= PI/3))
				{
					// check if coordinates are valid
					if ((wd > 0) && (hd > 0) && 
						(x0 >= -wd) && (x0 < WIDTH) && 
						(y0 >= -hd) && (y0 < HEIGHT))
					{
						// display ghost image
						Mat2D_PrepDrawImg(&Mat2, GHOSTW, GHOSTH, GHOSTW/2, GHOSTH/2, wd, hd, 0, 0, 0, 0, 0);
						DrawImg4PalMat(img, pal, GHOSTW, GHOSTH, x0, y0, wd, hd, &Mat2, DRAWIMG_TRANSP, CARTRANS);
					}
				}
			}

			// display car
			DrawBlit4Pal(CarImgList[turn], CarImgPalList[turn], 0, 0, CARIMGX, CARIMGY, CARIMGW, CARIMGH, CARIMGW, CARTRANS);

			// update display
			DispUpdate();
		}

		// update time counter
		if (CheckNext != GRASS) // game started
		{
			ThisTime = (int)((u32)(Time64() - StartTime)/1000000);
		}

		// game goal
		tile = TileMap[CurTile]; // get current tile
		if ((tile == CheckNext) || ((tile == START) && (CheckNext == GRASS)))
		{
			switch (tile)
			{
			// start tile
			case START:

				// start new lap, except first lap
				if (CheckNext != GRASS)
				{
					// process last lap
					LastTime = ThisTime;

					// new best time
					NewBest = False;
					if ((LastTime < BestTime) || (BestTime == 0))
					{
						BestTime = LastTime;
						NewBest = True;

						// copy history
						RecordBestNum = RecordInx;
						memcpy(RecordBestX, RecordX, RecordInx*sizeof(float));
						memcpy(RecordBestY, RecordY, RecordInx*sizeof(float));
						memcpy(RecordBestDir, RecordDir, RecordInx*sizeof(float));

					}
				}

				StartTime = Time64(); // start measure current game time
				CheckNext = CHECKA; // next checkpoint
				CheckOK[0] = False; // checkpoint 0 not reached
				CheckOK[1] = False; // checkpoint 1 not reached
				CheckOK[2] = False; // checkpoint 2 not reached
				RecordInx = 0; // index of next record step
				RecordStep = Time(); // time to next record step
				break;

			// check point A
			case CHECKA:
				CheckOK[0] = True;
				CheckNext = CHECKB;
				break;

			// check point B
			case CHECKB:
				CheckOK[1] = True;
				CheckNext = CHECKC;
				break;

			// check pojnt C
			case CHECKC:
				CheckOK[2] = True;
				CheckNext = START;
				break;
			}
		}

		// record game (every 1 second)
		if ((CheckNext != GRASS) && (dif >= 1000000))
		{
			RecordStep += 1000000;
			i = RecordInx;
			if (i < RECORDMAX)
			{
				RecordX[i] = CarX;
				RecordY[i] = CarY;
				RecordDir[i] = CarDir;
				RecordInx = i + 1;
			}
		}
	}
}
