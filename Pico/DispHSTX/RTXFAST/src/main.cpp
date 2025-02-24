
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define DISP_FPS	0		// 1=display FPS

u16 ALIGNED FrameBuf[WIDTH*HEIGHT];

int FASTCODE NOFLASH(main)()
{
#if DISPHSTX_USEPLL		// 1=use separate PLL_SYS for HSTX generator (sys_clk will use PLL_USB)
	DispHstxClockReinit(384*KHZ);
#endif

	// initialize display mode 320x240
	// - Only DVI output is supported, as VGA rendering is too slow for this purpose.
	DispVMode320x240x16(DISPHSTX_DISPMODE_DVI, FrameBuf);

	float a = 0.2f;
	float b = 0.8f;
	float c = 1.2f;
	float d = 1.9f;

#if DISP_FPS			// 1=display FPS
	u32 t = Time();
	u32 t2;
	char buf[20];
#endif

	int kk = 0;

	while(True)
	{
		// animation
		FloorPos -= 0.2f; if (FloorPos <= -2.0f) FloorPos += 2.0f;
		Spheres[0].pos.y = absf(sinf(a))*Spheres[0].rad + Spheres[0].rad + FLOORY;
		Spheres[1].pos.y = absf(sinf(b))*Spheres[1].rad*1.5f + Spheres[1].rad + FLOORY;
		Spheres[2].pos.y = absf(sinf(c))*Spheres[2].rad + Spheres[2].rad + FLOORY;
		Spheres[3].pos.y = absf(sinf(d))*Spheres[3].rad + Spheres[3].rad + FLOORY;
		a += 0.2f; if (a >= (float)PI2) a -= (float)PI2;
		b += 0.24f; if (b >= (float)PI2) b -= (float)PI2;
		c += 0.12f; if (c >= (float)PI2) c -= (float)PI2;
		d += 0.17f; if (d >= (float)PI2) d -= (float)PI2;

		// render core 0: 116 ms
		// render core 1: 119 ms
		// display DMA: 51 ms
		// render + display all: 120 ms

		// start rendering on core 1
		DispHstxCore1Exec(Render3D);

		// rendering on core 0
		Render3D();
		dsb();
		DispHstxCore1Wait();

#if DISP_FPS			// 1=display FPS
//		kk--;
//		if (kk <= 0)
		{
//			kk = 10;

		// FPS
		t2 = Time();
		MemPrint(buf, 20, "%.2f ", 1000000.0/(t2-t));
		DrawTextBg(buf, -1, 0, 0, COL_WHITE, COL_BLACK, 1, 1);
//		WaitMs(100);
		t = Time();
		}
#endif
	}
}
