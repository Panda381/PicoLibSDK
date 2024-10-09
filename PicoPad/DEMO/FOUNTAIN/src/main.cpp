
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define DISP_FPS	0	// 1 = display FPS
#define DISP_FAST	1	// 1 = use high speed

/*
Orginal source code (BBC BASIC):

 10 MODE 0:XS=2:YS=2 
 20 VDU 23,1,0;0;0;0;
 30 VDU 29,640;512;
 40 GCOL 0,13
 50 A=640:B=A*A:C=512
 60 FOR X=0 TO A STEP XS:S=X*X:P=SQR(B-S)
 70   FOR I=-P TO P STEP 6*YS
 80     R=SQR(S+I*I)/A
 90     Q=(R-1)*SIN(24*R)
100     Y=INT(I/3+Q*C)
110     IF I=-P M=Y:N=Y
120     IF Y>M M=Y
130     IF Y<N N=Y
140     IF M=Y OR N=Y PLOT 69,NOTX,Y:PLOT 69,X,Y
150   NEXT:NEXT
160 REPEAT:UNTIL FALSE
*/

#if DISP_FAST		// 1 = use high speed
int FASTCODE NOFLASH(main)()
#else
int main()
#endif
{
	int x, y, m, n, s;
	float i, p, q, r;
	float a = 0;

#if DISP_FAST		// 1 = use high speed
	// setup high speed
#if RP2350
	FlashInit(3);
#else
	FlashInit(6);
#endif
	VregSetVoltage(VREG_VOLTAGE_1_30);
	ClockPllSysFreq(300000);
#endif

#if DISP_FPS	// 1 = display FPS
	// prepare to display FPS
	u32 t1, t2;
	t1 = Time();
#endif

#define XS 1	// step in X direction
#define YS 1	// step in Y direction
#define WIDTH2	(WIDTH/2)
#define HEIGHT2	(HEIGHT/2)
#define COL	COL_MAGENTA

	// main loop
	while (True)
	{
		// clear screen
		DrawClear();

		// loop in X direction
		for (x = 0; x < WIDTH2; x += XS)
		{
			s = x*x;
			p = sqrtf(WIDTH2*WIDTH2 - s);
			for (i = -p; i < p; i += 6*YS)
			{
				r = sqrtf(s + i*i)/WIDTH2;
				q = (r - 1)*sinf(24*r + a);
				y = float2int(i/3 + q*HEIGHT2);
				if (i == -p) { m = y; n = y; }
				if (y > m) m = y;
				if (y < n) n = y;
				if ((m == y) || (n == y))
				{
					DrawPoint(WIDTH2+x, HEIGHT2-y, COL);
					DrawPoint(WIDTH2-x, HEIGHT2-y, COL);
				}
			}
		}

#if DISP_FPS		// 1 = display FPS
		// display FPS and update display
		t2 = Time();
		DrawPrint("\r%.1f", 1000000.f/(t2-t1));
		t1 = t2;
#else // DISP_FPS
		// display update
		DispUpdate();
#endif // DISP_FPS

		// animation
		a += 0.1f;
		if (a >= (float)PI2) a -= (float)PI2;
#if USE_PICOPADVGA
		WaitMs(40);
#endif

		// keyboard
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
