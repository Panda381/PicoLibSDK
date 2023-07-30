
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Mandlebrot set

/*
Integer arithmetics:

Value range:
- ci, cr ..... -2 to +2
- zr, zi .... -2 to +2

- zr2 = zr*zr, zi2 = zi*zi .... 0 to +4
- if (zr2 + zi2 > 4) break; ..... zr2, zi2 .... 0 to +4 (zr, zi .... -2 to +2)
- zi = 2*zr*zi + ci; ..... -10 to +10
- zr = zr2 - zi2 + cr; .... -6 to +6

- zr2 = zr*zr, zi2 = zi*zi .... 0 to +60
- if (zr2 + zi2 > 4) break; ..... zr2, zi2 .... 0 to +4 (zr, zi .... -2 to +2)
- zi = 2*zr*zi + ci; ..... -10 to +10
- zr = zr2 - zi2 + cr; .... -6 to +6

Need 6 bits for integer part of number + 1 bit for sign.

We will use 32-bit integer with highest 7 bits as integer part and 25 lower bits as decimal part.
*/

#include "../include.h"

#define RENDER_2CORES	1	// 1 = use both cores to render

#define USE_INT	0	// use integer arithmetics
#define USE_FLT	1	// use float arithmetics
#define USE_DBL	2	// use double arithmetics

int Arithm = USE_INT; // used arithmetics

// current coordinates
double Size = 1;
double OffX = 0;
double OffY = 0;
int Steps = 64; // number of steps
int Shift = 0; // number of shift to convert steps to gradient
int Width = WIDTH;

// gradient
extern "C" const u8 Grad[64] = {
	0x01, 0x02, 0x06, 0x0A, 0x0E, 0x13, 0x17, 0x1B,
	0x1A, 0x16, 0x15, 0x34, 0x54, 0x78, 0x9C, 0xBC,
	0xDC, 0xFC, 0xF8, 0xF4,	0xF0, 0xEC, 0xE8, 0xE0,
	0xE1, 0xE2, 0xE7, 0xEB, 0xEF, 0xF3, 0xF7, 0xFF
};

// processed lines
Bool Ok[HEIGHT];

// Mandelbrot state
int CurY = HEIGHT; // current Y (HEIGHT = not active)
int IncY = 1; // current increment Y
fixed X0i, Y0i, SXi, SYi; // increment in fixed format
float X0f, Y0f, SXf, SYf; // increment in float format
double X0d, Y0d, SXd, SYd, Cid; // increment in double format
u8* Dst; // current buffer
int DstInc; // increment Dst
u64 StartTime; // start time of render

// Core1 state
u8* Core1Dst; // current buffer
double Core1Cid; // increment in double format

// update Y increment
void MandelUpdate()
{
	// wait for core1
	QCore1Wait();

	// current Y
	CurY = 0;

	// ci = ((double)(HEIGHT-y)/HEIGHT - 0.5f)*3*size + offy;
	// ci = y*sy + y0;
	SYd = -3*Size/HEIGHT * IncY;
	SYf = (float)SYd;
	SYi = DlbToFixed(SYd);

	// start Ci
	Cid = Y0d;

	// pointer to buffer
	Dst = FrameBuf;
	DstInc = IncY * WIDTH;
}

// start new image
void MandelStart()
{
	// wait for core1
	QCore1Wait();

	// current Y increment
	IncY = 1;
	while (IncY*2 < HEIGHT) IncY *= 2;

	// shift
	Shift = 0;
	while ((Steps >> Shift) > 64) Shift++;

	// prepare X increment
	// cr = ((double)x/WIDTH - 0.5f)*4*size + offx;
	// cr = x*sx + x0;
	SXd = 4*Size/WIDTH;
	X0d = OffX - 2*Size;
	SXf = (float)SXd;
	X0f = (float)X0d;
	SXi = DlbToFixed(SXd);
	X0i = DlbToFixed(X0d);

	// ci = ((double)(HEIGHT-y)/HEIGHT - 0.5f)*3*size + offy;
	// ci = y*sy + y0;
	Y0d = OffY + 1.5f*Size;
	Y0f = (float)Y0d;
	Y0i = DlbToFixed(Y0d);

	// clear buffer
	memset(FrameBuf, 0, WIDTH*HEIGHT);
	memset(Ok, False, HEIGHT*sizeof(Bool));

	// update Y increment
	MandelUpdate();

	// get start time
	StartTime = Time64();
}

// calculate Mandelbrot float
//  default render time (X,Y=[0,0], scale=1, steps=64, sysclk=125MHz, res=264x200): 2.28 sec
void NOFLASH(MandelFloat)(u8* dst, float ci)
{
	int x, i;
	float zr, zi, zr2, zi2, cr, sx;
	int steps = Steps;
	int w = WIDTH;
	const u8* g = Grad;
	int s = Shift;

	// prepare cr
	sx = SXf;
	cr = X0f;

	// X loop
	for (x = 0; x < w; x++)
	{
		zr = cr;
		zi = ci;

		for (i = steps; i > 0; i--)
		{
			zr2 = zr*zr;
			zi2 = zi*zi;
			if (zr2 + zi2 > 4.0f) break;
			zi = 2*zr*zi + ci;
			zr = zr2 - zi2 + cr;
		}

		if (i == 0)
			*dst = 0;
		else
			*dst = g[(steps - i) >> s];

		dst++;
		cr += sx;
	}
}

// calculate Mandelbrot double
//  default render time (X,Y=[0,0], scale=1, steps=64, sysclk=125MHz, res=264x200): 4.10 sec
void NOFLASH(MandelDouble)(u8* dst, double ci)
{
	int x, i;
	double zr, zi, zr2, zi2, cr, sx;
	int steps = Steps;
	int w = WIDTH;
	const u8* g = Grad;
	int s = Shift;

	// prepare cr
	sx = SXd;
	cr = X0f;

	// X loop
	for (x = 0; x < w; x++)
	{
		zr = cr;
		zi = ci;

		for (i = steps; i > 0; i--)
		{
			zr2 = zr*zr;
			zi2 = zi*zi;
			if (zr2 + zi2 > 4.0) break;
			zi = 2*zr*zi + ci;
			zr = zr2 - zi2 + cr;
		}

		if (i == 0)
			*dst = 0;
		else
			*dst = g[(steps - i) >> s];

		dst++;
		cr += sx;
	}
}

/*
// calculate Mandelbrot fixed integer
//  default render time (X,Y=[0,0], scale=1, steps=64, sysclk=125MHz, res=264x200): 0.32 sec
//  Note: This function is realized in assembly, but speed is almost the same.
void NOFLASH(MandelFixed)(u8* dst, fixed ci)
{
	int x, i;
	fixed zr, zi, zr2, zi2, cr, sx;
	int steps = Steps;
	int w = WIDTH;
	const u8* g = Grad;
	int s = Shift;

	// prepare cr
	sx = SXi;
	cr = X0i;

	// X loop	
	for (x = 0; x < w; x++)
	{
		zr = cr;
		zi = ci;

		for (i = steps; i > 0; i--)
		{
			zr2 = FixedSqr(zr);
			zi2 = FixedSqr(zi);
			if (FixedGr(FixedAdd(zr2, zi2), (4 << 25))) break;
			zi = FixedAdd(FixedMul2(FixedMul(zr, zi)), ci);
			zr = FixedAdd(FixedSub(zr2, zi2), cr);
		}

		if (i == 0)
			*dst = 0;
		else
			*dst = g[(steps - i) >> s];

		dst++;
		cr = FixedAdd(cr, sx);
	}
}
*/

#if RENDER_2CORES // use both cores
// render using Core1
void MandelCore1()
{
	if (Arithm == USE_INT)
		MandelFixed(Core1Dst, DlbToFixed(Core1Cid));
	else if (Arithm == USE_FLT)
		MandelFloat(Core1Dst, (float)Core1Cid);
	else
		MandelDouble(Core1Dst, Core1Cid);
}
#endif

int main()
{
	int c;

	// start new image
	MandelStart();

	while (true)
	{
		// keys
		c = KeyChar();
		if ((c != NOCHAR))
		{

			switch (c)
			{
			case CH_UP: // move up
				OffY += Size/5;
				MandelStart();
				break;

			case CH_LEFT: // move left
				OffX -= Size/5;
				MandelStart();
				break;

			case CH_RIGHT: // move right
				OffX += Size/5;
				MandelStart();
				break;

			case CH_DOWN: // move down
				OffY -= Size/5;
				MandelStart();
				break;

			case 'q':
			case 'Q': // zoom in
				Size /= 2;
				MandelStart();
				break;

			case 'a':
			case 'A': // zoom out
				Size *= 2;
				if (Size >= 1) Size = 1;
				MandelStart();
				break;

			case 'i':
			case 'I': // integer arithmetics
				Arithm = USE_INT;
				MandelStart();
				break;

			case 'f':
			case 'F': // float arithmetics
				Arithm = USE_FLT;
				MandelStart();
				break;

			case 'd':
			case 'D': // double arithmetics
				Arithm = USE_DBL;
				MandelStart();
				break;

			case 'o':
			case 'O': // decrease steps
				Steps /= 2;
				Shift--;
				if (Steps < 1) Steps = 1;
				MandelStart();
				break;

			case 'p':
			case 'P': // increase steps
				Steps *= 2;
				if (Steps < 0) Steps = 0x40000000;
				MandelStart();
				break;

			case CH_BS: // quit
				ResetToBootLoader();
				break;

#if USE_SCREENSHOT		// use screen shots
			case 'x':
			case 'X':
				ScreenShot();
				break;
#endif


			case ' ': // redraw
				MandelStart();
				break;
			}
		}

		// render line
		if (CurY < HEIGHT)
		{
			if (!Ok[CurY])
			{
				Ok[CurY] = True;
		
				// render line
#if RENDER_2CORES // use both cores
				if (!QCore1Busy())
				{
					Core1Dst = Dst;
					Core1Cid = Cid;
					QCore1Exec(MandelCore1);
				}
				else 
#endif
				if (Arithm == USE_INT)
					MandelFixed(Dst, DlbToFixed(Cid));
				else if (Arithm == USE_FLT)
					MandelFloat(Dst, (float)Cid);
				else
					MandelDouble(Dst, Cid);
			}

			// next line
			CurY += IncY;
			Cid += SYd;
			Dst += DstInc;

			if (CurY >= HEIGHT)
			{
				if (IncY > 1)
				{
					IncY /= 2;
					MandelUpdate();
				}
			}
		}
		else
		{
			// indicate work done
			GPIO_Out1(LED_PIN);
			WaitMs(125);
			GPIO_Out0(LED_PIN);
			WaitMs(125);
		}
	}
}
