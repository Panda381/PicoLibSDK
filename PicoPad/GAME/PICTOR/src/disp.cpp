
// ****************************************************************************
//
//                             Display Commons
//
// ****************************************************************************

#include "../include.h"

// blend 2 colors together
u16 Col16Blend2(u16 col1, u16 col2)
{
	u16 col = (((col1 & 0xf81f) + (col2 & 0xf81f)) >> 1) & 0xf81f; // blend red and blue components
	col |= (((col1 & 0x07e0) + (col2 & 0x07e0)) >> 1) & 0x07e0; // blend green components
	return col;
}

// blend 4 colors together
u16 Col16Blend4(u16 col1, u16 col2, u16 col3, u16 col4)
{
	u16 col = (((col1 & 0xf81f) + (col2 & 0xf81f) + (col3 & 0xf81f) + (col4 & 0xf81f)) >> 2) & 0xf81f; // blend red and blue components
	col |= (((col1 & 0x07e0) + (col2 & 0x07e0) + (col3 & 0x07e0) + (col4 & 0x07e0)) >> 2) & 0x07e0; // blend green components
	return col;
}

// Draw 8-bit paletted image with half X and Y resolution
//  src ... image data
//  pal ... palettes
//  xs ... source X*2 (can be odd)
//  ys ... source Y*2 (cannot be odd)
//  xd ... destination X
//  yd ... destination Y
//  w .... width*2
//  h .... height*2
//  ws ... source total width*2 (in pixels)
void DrawImgPalDbl(const u8* src, const u16* pal, int xs, int ys, int xd, int yd, int w, int h, int ws)
{
	// limit coordinate X
	if (xs < 0)
	{
		w += xs;
		xd += -xs;
		xs = 0;
	}

	if (xd < 0)
	{
		w += xd;
		xs += -xd;
		xd = 0;
	}

	// limit w
	if (xd + w > WIDTH) w = WIDTH - xd;
	if (xs + w > ws) w = ws - xs;
	if (w <= 0) return;

	// limit coordinate Y
	if (ys < 0)
	{
		h += ys;
		yd += -ys;
		ys = 0;
	}

	if (yd < DispMinY)
	{
		h -= DispMinY-yd;
		ys += DispMinY-yd;
		yd = DispMinY;
	}

	// limit h
	if (yd + h > DispMaxY) h = DispMaxY - yd;
	if (h <= 0) return;

	// update dirty rectangle
	DispDirtyRect(xd, yd, w, h);

	// draw image
	ws /= 2;
	h /= 2;
	u16* d = &pDrawBuf[xd + (yd-DispMinY)*WIDTH];
	const u8* s0 = &src[xs/2 + (ys/2)*ws];
	int i;
	int wd = WIDTH - w;
	const u8* s;
	u16 col, collast, col2, col2last;
	for (; h > 0; h--)
	{
		// even line (not interpolated)
		s = s0;
		i = w;

		if ((xs & 1) != 0)
		{
			collast = pal[*s++]; // odd color is not interpolated
			*d++ = collast;
			i--;
		}
		else
			collast = pal[(xs > 0) ? s[-1] : s[ws-1]];

		for (; i > 1; i -= 2)
		{
			col = pal[*s++];
			*d++ = Col16Blend2(collast, col); // even color is interpolated
			*d++ = col;  // odd color is not interpolated
			collast = col;
		}

		if (i > 0) *d++ = Col16Blend2(collast, pal[*s]); // even color is interpolated

		d += wd;

		s = s0;
		i = w;

		// last line is not interpolated
		if (h == 1)
		{
			if ((xs & 1) != 0)
			{
				collast = pal[*s++]; // odd color is not interpolated
				*d++ = collast;
				i--;
			}
			else
				collast = pal[(xs > 0) ? s[-1] : s[ws-1]];

			for (; i > 1; i -= 2)
			{
				col = pal[*s++];
				*d++ = Col16Blend2(collast, col); // even color is interpolated
				*d++ = col;  // odd color is not interpolated
				collast = col;
			}

			if (i > 0) *d++ = Col16Blend2(collast, pal[*s]); // even color is interpolated
		}
		else
		{
			// odd line (interpolated)
			if ((xs & 1) != 0)
			{
				collast = pal[*s];
				col2last = pal[s[ws]];
				*d++ = Col16Blend2(collast, col2last);
				s++;
				i--;
			}
			else
			{
				collast = pal[(xs > 0) ? s[-1] : s[ws-1]];
				col2last = pal[(xs > 0) ? s[ws-1] : s[2*ws-1]];
			}

			for (; i > 1; i -= 2)
			{
				col = pal[*s];
				col2 = pal[s[ws]];
				*d++ = Col16Blend4(collast, col, col2last, col2);
				*d++ = Col16Blend2(col, col2);
				s++;
				collast = col;
				col2last = col2;
			}

			if (i > 0)
			{
				col = pal[*s];
				col2 = pal[s[ws]];
				*d++ = Col16Blend4(collast, col, col2last, col2);
			}

			d += wd;
			s0 += ws;
		}

	}
}

// shadow screen
void DispShadow()
{
	int y = DispMinY;
	int h = DispMaxY - y;

	// update dirty rectangle
	DispDirtyRect(0, y, WIDTH, h);

	// shadow image
	u16* d = pDrawBuf;
	int i = h * WIDTH;
	int k;
	for (; i > 0; i--)
	{
		k = *d;
		k = ((k & 0x1f) + ((k >> 5) & 0x3f) + (k >> 11)) >> 3;
		*d = (u16)(k | (k << 6) | (k << 11));
		d++;
	}
}
