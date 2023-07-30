
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

#define USERANDSHADOW	1	// 1=use random shadow, 0=use rastered shadow

// precalculated U coordinate
#define UNUM 1024
#define UNUM2 (UNUM/2)
int UTab[UNUM];

// main function
int main()
{
	int i, x, y, xr, u, v, off;
	u8* d;
	const u8* s1;
	const u8* s2;
	const u8* s;

	// clear draw box
	memset(FrameBuf, COL_BLACK, sizeof(FrameBuf));

	// prepare U table
	for (i = 0; i < UNUM; i++)
	{
		UTab[i] = (int)((asinf((float)(i-UNUM2)/UNUM2)/PI2+0.25f)*EARTHW);
	}

	// main loop
	off = 0;
	while (true)
	{
		// Y loop
		for (y = -GLOBER; y < +GLOBER; y++)
		{
			// X offset
			xr = (int)(sqrtf((float)GLOBER*GLOBER - y*y) + 0.5f);
			if (xr == 0) xr = 1; // to avoid divide by zero

			// destination address
			d = &FrameBuf[GLOBEX-xr + (y+GLOBEY)*WIDTH];

			// texture V coordinate
			v = (int)((asinf((float)y/GLOBER)/PI+0.5f)*EARTHH);

			// source address
			s1 = &Earth1Img[v*EARTHW];
			s2 = &Earth2Img[v*EARTHW];

			// X loop
			for (x = -xr; x < xr; x++)
			{
				// texture U coordinate
				u = UTab[x*UNUM2/xr+UNUM2];

#define SHADOWBEG	(xr*7/16) // shadow beging
#define SHADOW1		(xr*8/16) // shadow boundary 1
#define SHADOW2		(xr*9/16) // shadow boundary 2
#define SHADOWEND	(xr*10/16) // shadow end

#if USERANDSHADOW // use random shadow

				// select bitmap
				if (x < SHADOWBEG) // always day
					s = s1;	// day
				else if (x >= SHADOWEND) // always night
					s = s2;	// night
				else
				{
					if (x <= (int)RandU16MinMax(SHADOWBEG, SHADOWEND))
						s = s1;	// day
					else
						s = s2;	// night
				}

#else // use random shadow

				if (x < SHADOWBEG) // always day
					s = s1;	// day
				else if (x >= SHADOWEND) // always night
					s = s2;	// night
				else if (x < SHADOW1) // light shadow
				{
					if (((x & 1) + ((y & 1)<<1)) == 0)
						s = s2;	// night
					else
						s = s1;	// day
				}
				else if (x < SHADOW2) // shadow
				{
					if (((x + y) & 1) == 0)
						s = s2;	// night
					else
						s = s1;	// day
				}
				else // dark shadow
				{
					if (((x & 1) + ((y & 1)<<1)) != 3)
						s = s2;	// night
					else
						s = s1;	// day
				}

#endif // use random shadow

				// draw pixel
				*d++ = s[(u + off + EARTHW/2) & (EARTHW-1)];
			}
		}

		// shift angle
		off -= 2;
		while (off < EARTHW) off += EARTHW;

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
