
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define FRAMENUM	200		// number of frames
#define PERVERT		2		// number of periods of vertex number changes
#define PERLINE		4		// number of periods of line number changes
#define TURNS		2		// number of turns
#define XS		(WIDTH/2)	// screen middle X
#define YS		(HEIGHT/2)	// screen middle Y
#define COL		COL_YELLOW	// color

// global variables
float svert, sline, r, sym, ab, a, b, d, i, n, x, y, u, v, w, z;
int frame, n2;

// draw one vertex
void DrawVert(float ka)
{
	// prepare variables
	x = sinf(i);
	y = cosf(i);
	u = 0;
	v = 0;
	w = sin(i + ka);
	z = cos(i + ka);

	// draw lines
	int k = float2int(d);
	float x0, y0;
	for (; k > 0; k--)
	{
		DrawLine(XS + x*r, YS + y*r, XS + u*r, YS + v*r, COL);
		DrawLine(XS + u*r, YS + v*r, XS + w*r, YS + z*r, COL);

		// shift to next line
		x0 = x;
		y0 = y;
		x = x - (x - u)/k;
		y = y - (y - v)/k;
		u = u + (w - u)/k;
		v = v + (z - v)/k;
		w = w + (x0 - w)/k;
		z = z + (y0 - z)/k;
	}
}

int main()
{
	frame = 0; // current frame

	// main loop
	while (True)
	{
		// clear screen
		DrawClear();

		// prepare
		svert = sinf(PI2*PERVERT*frame/FRAMENUM);
		n = 1.5f*svert + 2.5f; // number of vertices
		if (n <= 1) n = 1; // minimal 1 vertex
		sline = sinf(PI2*PERLINE*frame/FRAMENUM);
		d = 10*sline + 20; // number of lines
		r = WIDTH*(0.03f*svert + 0.32f); // radius
		n2 = float2int(n);
		sym = 0.5f*(n - n2); // vertex symmetry
		n2 = BIT(n2);  // number of vertices
		if (n2 <  2) { n2 = 2; sym = 0; } // minimum
		ab = PI/n2; // AB angle
		b = ab*(1 - sym); // angle of odd vertex
		a = ab*sym; // angle of even vertex
		i = PI2*TURNS*frame/FRAMENUM; // current angle

		// draw lines
		for (; n2 > 0; n2--)
		{
			DrawVert(-a);
			DrawVert(b);
			i = i + 2*b;
			DrawVert(-b);
			DrawVert(a);
			i = i + 2*a;
		}

		// display update
		DispUpdate();

		// animation
		frame++;
		if (frame >= FRAMENUM) frame = 0;
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
