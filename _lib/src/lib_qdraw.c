
// ****************************************************************************
//
//                           Draw to QVGA 8-bit buffer
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include "../../global.h"	// globals

#if USE_QDRAW		// use QVGA drawing (lib_qdraw.c, lib_qdraw.h)

#include "../../_font/_include.h"
#include "../../_sdk/inc/sdk_float.h"
#include "../../_sdk/inc/sdk_divider.h"
#include "../../_display/qvga/qvga.h"
#include "../inc/lib_qdraw.h"
#include "../inc/lib_stream.h"
#include "../inc/lib_text.h"
#include "../inc/lib_print.h"

// pointer to draw frame buffer
u8* pDrawBuf = FrameBuf; // current draw buffer
const u8* pDrawFont = FONT; // font 8x8
int DrawFontHeight = FONTH; // font height
int DrawFontWidth = FONTW; // font width (5 to 8)
u8 DrawPrintPosNum = TEXTWIDTH; // number of text positions per row (= WIDTH/DrawFontWidth)
u8 DrawPrintRowNum = TEXTHEIGHT; // number of text rows (= HEIGHT/DrawFontHeight)
u8 DrawPrintPos = 0;  // console print character position
u8 DrawPrintRow = 0;  // console print character row
u8 DrawPrintInv = 0; // offset added to character (128 = print inverted characters, 0 = normal character)
u8 DrawPrintSize = 0; // font size: 0=normal, 1=double-height, 2=double-width, 3=double-size
u8 DrawPrintCol = COL_PRINT_DEF; // console print color

// update size of text screen after changing font size
void SelFontUpdate()
{
	DrawPrintPosNum = WIDTH/DrawFontWidth;
	DrawPrintRowNum = HEIGHT/DrawFontHeight;
}

// select font 8x8
void SelFont8x8()
{
	pDrawFont = FontBold8x8;
	DrawFontHeight = 8;
	DrawFontWidth = 8;
	SelFontUpdate(); // update size of text screen
}

// select font 8x14
void SelFont8x14()
{
	pDrawFont = FontBold8x14;
	DrawFontHeight = 14;
	DrawFontWidth = 8;
	SelFontUpdate(); // update size of text screen
}

// select font 8x16
void SelFont8x16()
{
	pDrawFont = FontBold8x16;
	DrawFontHeight = 16;
	DrawFontWidth = 8;
	SelFontUpdate(); // update size of text screen
}

// select font 6x8
void SelFont6x8()
{
	pDrawFont = FontCond6x8;
	DrawFontHeight = 8;
	DrawFontWidth = 6;
	SelFontUpdate(); // update size of text screen
}

// select font 5x8
void SelFont5x8()
{
	pDrawFont = FontTiny5x8;
	DrawFontHeight = 8;
	DrawFontWidth = 5;
	SelFontUpdate(); // update size of text screen
}

// Draw rectangle
void DrawRect(int x, int y, int w, int h, u8 col)
{
	// limit x
	if (x < 0)
	{
		w += x;
		x = 0;
	}

	// limit w
	if (x + w > WIDTH) w = WIDTH - x;
	if (w <= 0) return;

	// limit y
	if (y < 0)
	{
		h += y;
		y = 0;
	}

	// limit h
	if (y + h > HEIGHT) h = HEIGHT - y;
	if (h <= 0) return;

	// update dirty rectangle
	DispDirtyRect(x, y, w, h);

	// draw
	u8* d = &pDrawBuf[x + y*WIDTH];
	int wb = WIDTH - w;
	int i;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--) *d++ = col;
		d += wb;
	}
}

// Draw frame
void DrawFrame(int x, int y, int w, int h, u8 col)
{
	if ((w <= 0) || (h <= 0)) return;
	DrawRect(x, y, w-1, 1, col);
	DrawRect(x+w-1, y, 1, h-1, col);
	DrawRect(x+1, y+h-1, w-1, 1, col);
	DrawRect(x, y+1, 1, h-1, col);
}

// clear canvas with color
void DrawClearCol(u8 col)
{
	int i;
	memset(pDrawBuf, col, FRAMESIZE);
	DispDirtyAll();
}

// clear canvas with black color
void DrawClear()
{
	DrawClearCol(COL_BLACK);
}

// Draw point
void DrawPoint(int x, int y, u8 col)
{
	// check coordinates
	if (((u32)x >= (u32)WIDTH) || ((u32)y >= (u32)HEIGHT)) return;

	// draw pixel
	pDrawBuf[x + y*WIDTH] = col;

	// update dirty area by rectangle (must be in valid limits)
	DispDirtyPoint(x, y);
}

// Draw line
void DrawLine(int x1, int y1, int x2, int y2, u8 col)
{
	// difference of coordinates
	int dx = x2 - x1;
	int dy = y2 - y1;

	// increment X
	int sx = 1;
	if (dx < 0)
	{
		sx = -1;
		dx = -dx;
	}

	// increment Y
	int sy = 1;
	int ddy = WIDTH;
	if (dy < 0)
	{
		sy = -1;
		ddy = -ddy;
		dy = -dy;
	}

	// destination address
	u8* d = &pDrawBuf[x1 + y1*WIDTH];

	// steeply in X direction, X is prefered as base
	if (dx > dy)
	{
		int m = 2*dy;
		int p = m - dx;
		dx = 2*dx;
		x2 += sx;
		for (; x1 != x2; x1 += sx)
		{
			if (((u32)x1 < (u32)WIDTH) && ((u32)y1 < (u32)HEIGHT))
			{
				*d = col;
				DispDirtyPoint(x1, y1);
			}

			if (p > 0)
			{
				d += ddy;
				y1 += sy;;
				p -= dx;
			}
			p += m;
			d += sx;
		}
	}

	// steeply in Y direction, Y is prefered as base
	else
	{
		int m = 2*dx;
		int p = m - dy;
		dy = 2*dy;
		y2 += sy;
		for (; y1 != y2; y1 += sy)
		{
			if (((u32)x1 < (u32)WIDTH) && ((u32)y1 < (u32)HEIGHT))
			{
				*d = col;
				DispDirtyPoint(x1, y1);
			}

			if (p > 0)
			{
				d += sx;
				x1 += sx;
				p -= dy;
			}
			p += m;
			d += ddy;
		}
	}
}

// Draw filled circle
void DrawFillCircle(int x0, int y0, int r, u8 col)
{
	int x, y;
	if (r <= 0) return;
	int r2 = r*(r-1);
	r--;

	// full circle
	for (y = -r; y <= r; y++)
	{
		for (x = -r; x <= r; x++)
		{
			if ((x*x + y*y) <= r2) DrawPoint(x+x0, y+y0, col);
		}
	}
}

// Draw circle
void DrawCircle(int x0, int y0, int r, u8 col)
{
	int x, y;
	if (r <= 0) return;
	r--;

	x = 0;
	y = r;
	int p = 1 - r;

	while (x <= y)
	{
		DrawPoint(x0+y, y0-x, col);
		DrawPoint(x0+x, y0-y, col);
		DrawPoint(x0-x, y0-y, col);
		DrawPoint(x0-y, y0-x, col);
		DrawPoint(x0-y, y0+x, col);
		DrawPoint(x0-x, y0+y, col);
		DrawPoint(x0+x, y0+y, col);
		DrawPoint(x0+y, y0+x, col);

		x++;
		if (p > 0)
		{
			y--;
			p -= 2*y;
		}
		p += 2*x + 1;
	}
}

// Draw character (transparent background)
void DrawChar(char ch, int x, int y, u8 col)
{
	int x0 = x;
	int i, j;
	u8* d;

	// prepare pointer to font sample
	const u8* s = &pDrawFont[ch];

	// check if drawing is safe to use fast drawing
	if ((x >= 0) && (y >= 0) && (x + DrawFontWidth <= WIDTH) && (y + DrawFontHeight <= HEIGHT))
	{
		// update dirty rectangle
		DispDirtyRect(x, y, DrawFontWidth, DrawFontHeight);

		// destination address
		d = &pDrawBuf[y*WIDTH + x];

		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			for (j = DrawFontWidth; j > 0; j--)
			{
				// draw pixel
				if ((ch & 0x80) != 0) *d = col;
				d++;
				ch <<= 1;
			}
			d += WIDTH - DrawFontWidth;
		}
	}

	// use slow safe drawing
	else
	{
		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			x = x0;
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				if ((ch & 0x80) != 0) DrawPoint(x, y, col);
				x++;
				ch <<= 1;
			}
			y++;
		}
	}
}

// Draw character double height (transparent background)
void DrawCharH(char ch, int x, int y, u8 col)
{
	int x0 = x;
	int i, j;
	u8* d;

	// prepare pointer to font sample
	const u8* s = &pDrawFont[ch];

	// check if drawing is safe to use fast drawing
	if ((x >= 0) && (y >= 0) && (x + DrawFontWidth <= WIDTH) && (y + 2*DrawFontHeight <= HEIGHT))
	{
		// update dirty rectangle
		DispDirtyRect(x, y, DrawFontWidth, 2*DrawFontHeight);

		// destination address
		d = &pDrawBuf[y*WIDTH + x];

		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			for (j = DrawFontWidth; j > 0; j--)
			{
				// draw pixel
				if ((ch & 0x80) != 0)
				{
					*d = col;
					d[WIDTH] = col;
				}
				d++;
				ch <<= 1;
			}
			d += 2*WIDTH - DrawFontWidth;
		}
	}

	// use slow safe drawing
	else
	{
		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			x = x0;
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				if ((ch & 0x80) != 0)
				{
					DrawPoint(x, y, col);
					DrawPoint(x, y+1, col);
				}
				x++;
				ch <<= 1;
			}
			y += 2;
		}
	}
}

// Draw character double width (transparent background)
void DrawCharW(char ch, int x, int y, u8 col)
{
	int x0 = x;
	int i, j;
	u8* d;

	// prepare pointer to font sample
	const u8* s = &pDrawFont[ch];

	// check if drawing is safe to use fast drawing
	if ((x >= 0) && (y >= 0) && (x + 2*DrawFontWidth <= WIDTH) && (y + DrawFontHeight <= HEIGHT))
	{
		// update dirty rectangle
		DispDirtyRect(x, y, 2*DrawFontWidth, DrawFontHeight);

		// destination address
		d = &pDrawBuf[y*WIDTH + x];

		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			for (j = DrawFontWidth; j > 0; j--)
			{
				// draw pixel
				if ((ch & 0x80) != 0)
				{
					*d = col;
					d[1] = col;
				}
				d += 2;
				ch <<= 1;
			}
			d += WIDTH - 2*DrawFontWidth;
		}
	}

	// use slow safe drawing
	else
	{
		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			x = x0;
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				if ((ch & 0x80) != 0)
				{
					DrawPoint(x, y, col);
					DrawPoint(x+1, y, col);
				}
				x += 2;
				ch <<= 1;
			}
			y++;
		}
	}
}

// Draw character double sized (transparent background)
void DrawChar2(char ch, int x, int y, u8 col)
{
	int x0 = x;
	int i, j;
	u8* d;

	// prepare pointer to font sample
	const u8* s = &pDrawFont[ch];

	// check if drawing is safe to use fast drawing
	if ((x >= 0) && (y >= 0) && (x + 2*DrawFontWidth <= WIDTH) && (y + 2*DrawFontHeight <= HEIGHT))
	{
		// update dirty rectangle
		DispDirtyRect(x, y, 2*DrawFontWidth, 2*DrawFontHeight);

		// destination address
		d = &pDrawBuf[y*WIDTH + x];

		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			for (j = DrawFontWidth; j > 0; j--)
			{
				// draw pixel
				if ((ch & 0x80) != 0)
				{
					*d = col;
					d[1] = col;
					d[WIDTH] = col;
					d[WIDTH+1] = col;
				}
				d += 2;
				ch <<= 1;
			}
			d += 2*WIDTH - 2*DrawFontWidth;
		}
	}

	// use slow safe drawing
	else
	{
		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			x = x0;
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				if ((ch & 0x80) != 0)
				{
					DrawPoint(x, y, col);
					DrawPoint(x+1, y, col);
					DrawPoint(x, y+1, col);
					DrawPoint(x+1, y+1, col);
				}
				x += 2;
				ch <<= 1;
			}
			y += 2;
		}
	}
}

// Draw character with background
void DrawCharBg(char ch, int x, int y, u8 col, u8 bgcol)
{
	int x0 = x;
	int i, j;
	u8 c;
	u8* d;

	// prepare pointer to font sample
	const u8* s = &pDrawFont[ch];

	// check if drawing is safe to use fast drawing
	if ((x >= 0) && (y >= 0) && (x + DrawFontWidth <= WIDTH) && (y + DrawFontHeight <= HEIGHT))
	{
		// update dirty rectangle
		DispDirtyRect(x, y, DrawFontWidth, DrawFontHeight);

		// destination address
		d = &pDrawBuf[y*WIDTH + x];

		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			for (j = DrawFontWidth; j > 0; j--)
			{
				// draw pixel
				*d++ = ((ch & 0x80) != 0) ? col : bgcol;
				ch <<= 1;
			}
			d += WIDTH - DrawFontWidth;
		}
	}

	// use slow safe drawing
	else
	{
		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			x = x0;
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				c = ((ch & 0x80) != 0) ? col : bgcol;

				// draw pixel
				DrawPoint(x, y, c);
				x++;
				ch <<= 1;
			}
			y++;
		}
	}
}

// Draw character double height with background
void DrawCharBgH(char ch, int x, int y, u8 col, u8 bgcol)
{
	int x0 = x;
	int i, j;
	u8 c;
	u8* d;

	// prepare pointer to font sample
	const u8* s = &pDrawFont[ch];

	// check if drawing is safe to use fast drawing
	if ((x >= 0) && (y >= 0) && (x + DrawFontWidth <= WIDTH) && (y + 2*DrawFontHeight <= HEIGHT))
	{
		// update dirty rectangle
		DispDirtyRect(x, y, DrawFontWidth, 2*DrawFontHeight);

		// destination address
		d = &pDrawBuf[y*WIDTH + x];

		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				c = ((ch & 0x80) != 0) ? col : bgcol;

				// draw pixel
				*d = c;
				d[WIDTH] = c;

				d++;
				ch <<= 1;
			}
			d += 2*WIDTH - DrawFontWidth;
		}
	}

	// use slow safe drawing
	else
	{
		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			x = x0;
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				c = ((ch & 0x80) != 0) ? col : bgcol;

				// draw pixel
				DrawPoint(x, y, c);
				DrawPoint(x, y+1, c);

				x++;
				ch <<= 1;
			}
			y += 2;
		}
	}
}

// Draw character double width with background
void DrawCharBgW(char ch, int x, int y, u8 col, u8 bgcol)
{
	int x0 = x;
	int i, j;
	u8 c;
	u8* d;

	// prepare pointer to font sample
	const u8* s = &pDrawFont[ch];

	// check if drawing is safe to use fast drawing
	if ((x >= 0) && (y >= 0) && (x + 2*DrawFontWidth <= WIDTH) && (y + DrawFontHeight <= HEIGHT))
	{
		// update dirty rectangle
		DispDirtyRect(x, y, 2*DrawFontWidth, DrawFontHeight);

		// destination address
		d = &pDrawBuf[y*WIDTH + x];

		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				c = ((ch & 0x80) != 0) ? col : bgcol;

				// draw pixel
				*d++ = c;
				*d++ = c;
				ch <<= 1;
			}
			d += WIDTH - 2*DrawFontWidth;
		}
	}

	// use slow safe drawing
	else
	{
		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			x = x0;
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				c = ((ch & 0x80) != 0) ? col : bgcol;

				// draw pixel
				DrawPoint(x, y, c);
				DrawPoint(x+1, y, c);

				x += 2;
				ch <<= 1;
			}
			y++;
		}
	}
}

// Draw character double sized with background
void DrawCharBg2(char ch, int x, int y, u8 col, u8 bgcol)
{
	int x0 = x;
	int i, j;
	u8 c;
	u8* d;

	// prepare pointer to font sample
	const u8* s = &pDrawFont[ch];

	// check if drawing is safe to use fast drawing
	if ((x >= 0) && (y >= 0) && (x + 2*DrawFontWidth <= WIDTH) && (y + 2*DrawFontHeight <= HEIGHT))
	{
		// update dirty rectangle
		DispDirtyRect(x, y, 2*DrawFontWidth, 2*DrawFontHeight);

		// destination address
		d = &pDrawBuf[y*WIDTH + x];

		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				c = ((ch & 0x80) != 0) ? col : bgcol;

				// draw pixel
				*d = c;
				d[1] = c;
				d[WIDTH] = c;
				d[WIDTH+1] = c;

				d += 2;
				ch <<= 1;
			}
			d += 2*WIDTH - 2*DrawFontWidth;
		}
	}

	// use slow safe drawing
	else
	{
		// loop through lines of one character
		for (i = DrawFontHeight; i > 0; i--)
		{
			// get one font sample
			ch = *s;
			s += 256;

			// loop through pixels of one character line
			x = x0;
			for (j = DrawFontWidth; j > 0; j--)
			{
				// pixel is set
				c = ((ch & 0x80) != 0) ? col : bgcol;

				// draw pixel
				DrawPoint(x, y, c);
				DrawPoint(x+1, y, c);
				DrawPoint(x, y+1, c);
				DrawPoint(x+1, y+1, c);

				x += 2;
				ch <<= 1;
			}
			y += 2;
		}
	}
}

// Draw text (transparent background)
void DrawText(const char* text, int x, int y, u8 col)
{
	u8 ch;

	// loop through characters of text
	for (;;)
	{
		// get next character of the text
		ch = (u8)*text++;
		if (ch == 0) break;

		// draw character
		DrawChar(ch, x, y, col);

		// shift to next character position
		x += DrawFontWidth;
	}
}

// Draw text double width (transparent background)
void DrawTextW(const char* text, int x, int y, u8 col)
{
	u8 ch;

	// loop through characters of text
	for (;;)
	{
		// get next character of the text
		ch = (u8)*text++;
		if (ch == 0) break;

		// draw character
		DrawCharW(ch, x, y, col);

		// shift to next character position
		x += DrawFontWidth*2;
	}
}

// Draw text double height (transparent background)
void DrawTextH(const char* text, int x, int y, u8 col)
{
	u8 ch;

	// loop through characters of text
	for (;;)
	{
		// get next character of the text
		ch = (u8)*text++;
		if (ch == 0) break;

		// draw character
		DrawCharH(ch, x, y, col);

		// shift to next character position
		x += DrawFontWidth;
	}
}

// Draw text double sized (transparent background)
void DrawText2(const char* text, int x, int y, u8 col)
{
	u8 ch;

	// loop through characters of text
	for (;;)
	{
		// get next character of the text
		ch = (u8)*text++;
		if (ch == 0) break;

		// draw character
		DrawChar2(ch, x, y, col);

		// shift to next character position
		x += DrawFontWidth*2;
	}
}

// Draw text with background
void DrawTextBg(const char* text, int x, int y, u8 col, u8 bgcol)
{
	u8 ch;

	// loop through characters of text
	for (;;)
	{
		// get next character of the text
		ch = (u8)*text++;
		if (ch == 0) break;

		// draw character
		DrawCharBg(ch, x, y, col, bgcol);

		// shift to next character position
		x += DrawFontWidth;
	}
}

// Draw text double width with background
void DrawTextBgW(const char* text, int x, int y, u8 col, u8 bgcol)
{
	u8 ch;

	// loop through characters of text
	for (;;)
	{
		// get next character of the text
		ch = (u8)*text++;
		if (ch == 0) break;

		// draw character
		DrawCharBgW(ch, x, y, col, bgcol);

		// shift to next character position
		x += DrawFontWidth*2;
	}
}

// Draw text double height with background
void DrawTextBgH(const char* text, int x, int y, u8 col, u8 bgcol)
{
	u8 ch;

	// loop through characters of text
	for (;;)
	{
		// get next character of the text
		ch = (u8)*text++;
		if (ch == 0) break;

		// draw character
		DrawCharBgH(ch, x, y, col, bgcol);

		// shift to next character position
		x += DrawFontWidth;
	}
}

// Draw text double sized with background
void DrawTextBg2(const char* text, int x, int y, u8 col, u8 bgcol)
{
	u8 ch;

	// loop through characters of text
	for (;;)
	{
		// get next character of the text
		ch = (u8)*text++;
		if (ch == 0) break;

		// draw character
		DrawCharBg2(ch, x, y, col, bgcol);

		// shift to next character position
		x += DrawFontWidth*2;
	}
}

// Draw text buffer
void DrawTextBuf(const char* textbuf, u8 col, u8 bgcol)
{
	// prepre
	u8 ch, c;
	int num, i, j;
	const u8* s;
	int y0, x0;
	u8* d;

	// loop through characters of text
	for (y0 = 0; y0 <= HEIGHT - DrawFontHeight; y0 += DrawFontHeight)
	{
		d = &pDrawBuf[y0*WIDTH];

		for (x0 = 0; x0 <= WIDTH - DrawFontWidth; x0 += DrawFontWidth)
		{
			// get next character of the text
			ch = (u8)*textbuf++;

			// prepare pointer to font sample
			s = &pDrawFont[ch];

			// loop through lines of one character
			for (i = DrawFontHeight; i > 0; i--)
			{
				// get one font sample
				ch = *s;
				s += 256;

				// loop through pixels of one character line
				for (j = DrawFontWidth; j > 0; j--)
				{
					// draw pixel
					*d++ = ((ch & 0x80) != 0) ? col : bgcol;
					ch <<= 1;
				}
				d += WIDTH - DrawFontWidth;
			}
			d += DrawFontWidth - DrawFontHeight*WIDTH;
		}
	}

	DispDirtyAll();
}

// Draw text buffer with foreground color
void DrawFTextBuf(const char* textbuf, u8 bgcol)
{
	// prepre
	u8 ch, c, col;
	int num, i, j;
	const u8* s;
	int y0, x0;
	u8* d;

	// loop through characters of text
	for (y0 = 0; y0 <= HEIGHT - DrawFontHeight; y0 += DrawFontHeight)
	{
		d = &pDrawBuf[y0*WIDTH];

		for (x0 = 0; x0 <= WIDTH - DrawFontWidth; x0 += DrawFontWidth)
		{
			// get next character of the text
			ch = (u8)*textbuf++;

			// get foreground color
			col = (u8)*textbuf++;

			// prepare pointer to font sample
			s = &pDrawFont[ch];

			// loop through lines of one character
			for (i = DrawFontHeight; i > 0; i--)
			{
				// get one font sample
				ch = *s;
				s += 256;

				// loop through pixels of one character line
				for (j = DrawFontWidth; j > 0; j--)
				{
					// draw pixel
					*d++ = ((ch & 0x80) != 0) ? col : bgcol;
					ch <<= 1;
				}
				d += WIDTH - DrawFontWidth;
			}
			d += DrawFontWidth - DrawFontHeight*WIDTH;
		}
	}

	DispDirtyAll();
}

// Draw image
void DrawImg(const u8* src, int xd, int yd, int w, int h, int ws)
{
	// limit coordinate X
	if (xd < 0)
	{
		w += xd;
		src += -xd;
		xd = 0;
	}

	// limit w
	if (xd + w > WIDTH) w = WIDTH - xd;
	if (w <= 0) return;

	// limit coordinate Y
	if (yd < 0)
	{
		h += yd;
		src += -yd*ws;
		yd = 0;
	}

	// limit h
	if (yd + h > HEIGHT) h = HEIGHT - yd;
	if (h <= 0) return;

	// update dirty rectangle
	DispDirtyRect(xd, yd, w, h);

	// draw image
	u8* d = &pDrawBuf[xd + yd*WIDTH];
	for (; h > 0; h--)
	{
		memcpy(d, src, w);
		d += WIDTH;
		src += ws;
	}
}

// Draw image with transparency (col = transparency key color)
void DrawBlit(const u8* src, int xd, int yd, int w, int h, int ws, u8 col)
{
	// limit coordinate X
	if (xd < 0)
	{
		w += xd;
		src += -xd;
		xd = 0;
	}

	// limit w
	if (xd + w > WIDTH) w = WIDTH - xd;
	if (w <= 0) return;

	// limit coordinate Y
	if (yd < 0)
	{
		h += yd;
		src += -yd*ws;
		yd = 0;
	}

	// limit h
	if (yd + h > HEIGHT) h = HEIGHT - yd;
	if (h <= 0) return;

	// update dirty rectangle
	DispDirtyRect(xd, yd, w, h);

	// draw image
	u8* d = &pDrawBuf[xd + yd*WIDTH];
	int i;
	u8 c;
	int wbd = WIDTH - w;
	int wbs = ws - w;
	for (; h > 0; h--)
	{
		for (i = w; i > 0; i--)
		{
			c = *src++;
			if (c != col) *d = c;
			d++;
		}
		d += wbd;
		src += wbs;
	}
}

#if USE_MAT2D

// draw 8-bit image with 2D transformation matrix
//  src ... source image
//  ws ... source image width
//  hs ... source image height
//  x ... destination coordinate X
//  y ... destination coordinate Y
//  w ... destination width
//  h ... destination height
//  m ... transformation matrix (should be prepared using PrepDrawImg function)
//  mode ... draw mode QDRAWIMG_*
//  color ... key or border color (QDRAWIMG_PERSP mode: horizon offset)
// Note to wrap and perspective mode: Width and height of source image should be power of 2, or it will render slower.
void DrawImgMat(const u8* src, int ws, int hs, int x, int y, int w, int h, const sMat2D* m, u8 mode, u8 color)
{
	// limit x
	int x0 = -w/2; // start X coordinate
	if (x < 0)
	{
		w += x;
		x0 -= x;
		x = 0;
	}
	if (x + w > WIDTH) w = WIDTH - x;
	if (w <= 0) return;

	// limit y
	int h0 = h;
	int y0 = (mode == DRAWIMG_PERSP) ? (-h) : (-h/2); // start Y coordinate
	if (y < 0)
	{
		h += y;
		y0 -= y;
		y = 0;
	}
	if (y + h > HEIGHT) h = HEIGHT - y;
	if (h <= 0) return;

	// update dirty rectangle
	DispDirtyRect(x, y, w, h);

	// load transformation matrix and convert to integer fractional number
	int m11 = TOFRACT(m->m11);
	int m12 = TOFRACT(m->m12);
	int m13 = TOFRACT(m->m13);
	int m21 = TOFRACT(m->m21);
	int m22 = TOFRACT(m->m22);
	int m23 = TOFRACT(m->m23);

	// check invalid zero matrix
	Bool zero = (m11 | m12 | m13 | m21 | m22 | m23) == 0;

	// prepare variables
	int xy0m, yy0m; // temporary Y members
	u8* d = &pDrawBuf[WIDTH*y + x]; // destination image
	int wbd = WIDTH - w; // destination width bytes
	int i, x2, y2;

	// wrap image
	if (mode == DRAWIMG_WRAP)
	{
		// image dimension is power of 2
		if (IsPow2(ws) && IsPow2(hs))
		{
			// coordinate mask
			int xmask = ws - 1;
			int ymask = hs - 1;

			for (; h > 0; h--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				for (i = w; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) & xmask;
					y2 = (yy0m>>FRACT) & ymask;
					*d++ = src[x2 + y2*ws];
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}

				y0++;
				d += wbd;
			}
		}

		// image dimension is not power of 2
		else
		{
			for (; h > 0; h--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				for (i = w; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) % ws;
					if (x2 < 0) x2 += ws;
					y2 = (yy0m>>FRACT) % hs;
					if (y2 < 0) y2 += hs;
					*d++ = src[x2 + y2*ws];
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}

				y0++;
				d += wbd;
			}
		}
	}

	// no border
	else if (mode == DRAWIMG_NOBORDER)
	{
		// if matrix is valid
		if (!zero)
		{
			// source image dimension
			u32 ww = ws;
			u32 hh = hs;

			for (; h > 0; h--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;

				for (i = w; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (((u32)x2 < ww) && ((u32)y2 < hh)) *d = src[x2 + y2*ws];
					d++;
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d += wbd;
			}
		}
	}

	// clamp image
	else if (mode == DRAWIMG_CLAMP)
	{
		// invalid matrix
		if (zero)
		{
			u8 col = *src;
			for (; h > 0; h--)
			{
				for (i = w; i > 0; i--)
				{
					*d++ = col;
				}
				d += wbd;
			}
		}
		else
		{
			// source image dimension
			u32 ww = ws - 1;
			u32 hh = hs - 1;

			for (; h > 0; h--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				for (i = w; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (x2 < 0) x2 = 0;
					if (x2 > ww) x2 = ww;
					if (y2 < 0) y2 = 0;
					if (y2 > hh) y2 = hh;
					*d++ = src[x2 + y2*ws];
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d += wbd;
			}
		}
	}

	// color border
	else if (mode == DRAWIMG_COLOR)
	{
		// invalid matrix
		if (zero)
		{
			for (; h > 0; h--)
			{
				for (i = w; i > 0; i--)
				{
					*d++ = color;
				}
				d += wbd;
			}
		}
		else
		{
			// source image dimension
			u32 ww = ws;
			u32 hh = hs;

			for (; h > 0; h--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				for (i = w; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (((u32)x2 < ww) && ((u32)y2 < hh))
						*d = src[x2 + y2*ws];
					else
						*d = color;
					d++;
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d += wbd;
			}
		}
	}

	// transparency
	else if (mode == DRAWIMG_TRANSP)
	{
		// if matrix is valid
		if (!zero)
		{
			u32 ww = ws;
			u32 hh = hs;
			u8 c;

			for (; h > 0; h--)
			{
				xy0m = x0*m11 + y0*m12 + m13;
				yy0m = x0*m21 + y0*m22 + m23;
		
				for (i = w; i > 0; i--)
				{
					x2 = xy0m>>FRACT;
					y2 = yy0m>>FRACT;
					if (((u32)x2 < ww) && ((u32)y2 < hh))
					{
						c = src[x2 + y2*ws];
						if (c != color) *d = c;
					}
					d++;
					xy0m += m11; // x0*m11
					yy0m += m21; // x0*m21
				}
				y0++;
				d += wbd;
			}
		}
	}

	// perspective
	else if (mode == DRAWIMG_PERSP)
	{
		// image dimension is power of 2
		if (IsPow2(ws) && IsPow2(hs))
		{
			// coordinate mask
			int xmask = ws - 1;
			int ymask = hs - 1;

			for (; h > 0; h--)
			{
				int dist = FRACTMUL*h0/(h0 + y0 + color + 1);
				int m11b = (m11*dist)>>FRACT;
				int m21b = (m21*dist)>>FRACT;
				int m12b = (m12*dist)>>FRACT;
				int m22b = (m22*dist)>>FRACT;

				xy0m = x0*m11b + y0*m12b + m13;
				yy0m = x0*m21b + y0*m22b + m23;
		
				for (i = w; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) & xmask;
					xy0m += m11b; // x0*m11

					y2 = (yy0m>>FRACT) & ymask;
					yy0m += m21b; // x0*m21

					*d++ = src[x2 + y2*ws];
				}
				y0++;
				d += wbd;
			}
		}

		// image dimension is not power of 2
		else
		{
			for (; h > 0; h--)
			{
				int dist = FRACTMUL*h0/(h0 + y0 + color + 1);
				int m11b = (m11*dist)>>FRACT;
				int m21b = (m21*dist)>>FRACT;
				int m12b = (m12*dist)>>FRACT;
				int m22b = (m22*dist)>>FRACT;

				xy0m = x0*m11b + y0*m12b + m13;
				yy0m = x0*m21b + y0*m22b + m23;
		
				for (i = w; i > 0; i--)
				{
					x2 = (xy0m>>FRACT) % ws;
					if (x2 < 0) x2 += ws;
					xy0m += m11b; // x0*m11

					y2 = (yy0m>>FRACT) % hs;
					if (y2 < 0) y2 += hs;
					yy0m += m21b; // x0*m21

					*d++ = src[x2 + y2*ws];
				}
				y0++;
				d += wbd;
			}
		}
	}
}

// draw tile map using perspective projection
//  src ... source image with column of 8-bit square tiles (width = tile size, must be power of 2)
//  map ... byte map of tile indices
//  mapwbits ... number of bits of map width (number of tiles; width must be power of 2)
//  maphbits ... number of bits of map height (number of tiles; height must be power of 2)
//  tilebits ... number of bits of tile size in pixels (e.g. 5 = tile 32x32 pixel)
//  x ... destination coordinate X
//  y ... destination coordinate Y
//  w ... destination width
//  h ... destination height
//  mat ... transformation matrix (should be prepared using PrepDrawImg function)
//  horizon ... horizon offset (0=do not use perspective projection)
void DrawTileMap(const u8* src, const u8* map, int mapwbits, int maphbits,
	int tilebits, int x, int y, int w, int h, const sMat2D* mat, u8 horizon)
{
	// limit x
	int x0 = -w/2; // start X coordinate
	if (x < 0)
	{
		w += x;
		x0 -= x;
		x = 0;
	}
	if (x + w > WIDTH) w = WIDTH - x;
	if (w <= 0) return;

	// limit y
	int h0 = h;
	int y0 = (horizon == 0) ? (-h/2) : (-h); // start Y coordinate
	if (y < 0)
	{
		h += y;
		y0 -= y;
		y = 0;
	}
	if (y + h > HEIGHT) h = HEIGHT - y;
	if (h <= 0) return;

	// update dirty rectangle
	DispDirtyRect(x, y, w, h);

	// load transformation matrix and convert to integer fractional number
	int m11 = TOFRACT(mat->m11);
	int m12 = TOFRACT(mat->m12);
	int m13 = TOFRACT(mat->m13);
	int m21 = TOFRACT(mat->m21);
	int m22 = TOFRACT(mat->m22);
	int m23 = TOFRACT(mat->m23);
	int m11b = m11;
	int m21b = m21;
	int m12b = m12;
	int m22b = m22;

	// prepare variables
	int xy0m, yy0m; // temporary Y members
	u8* d = &pDrawBuf[WIDTH*y + x]; // destination image
	int wbd = WIDTH - w; // destination width in pixels
	int i, x2, y2;

	int tilesize = 1 << tilebits; // tile size in pixels
	int tilemask = tilesize - 1; // tile mask in pixels
	int tileinx; // tile index
	int mapw = 1<<mapwbits; // map width in tiles
	int maph = 1<<maphbits; // map height in tiles
	int mapmaskx = (mapw * tilesize) - 1; // mask of map width in pixels
	int mapmasky = (maph * tilesize) - 1; // mask of map height in pixels

	for (; h > 0; h--)
	{
	
		if (horizon != 0)
		{
			int dist = FRACTMUL*h0/(h0 + y0 + horizon);
			m11b = (m11*dist)>>FRACT;
			m12b = (m12*dist)>>FRACT;
			m21b = (m21*dist)>>FRACT;
			m22b = (m22*dist)>>FRACT;
		}

		xy0m = x0*m11b + y0*m12b + m13;
		yy0m = x0*m21b + y0*m22b + m23;

		for (i = w; i > 0; i--)
		{
			// pixel X coordinate on the map
			x2 = (xy0m>>FRACT) & mapmaskx;
			xy0m += m11b;

			// pixel Y coordinate on the map
			y2 = (yy0m>>FRACT) & mapmasky;
			yy0m += m21b;

			// tile index
			tileinx = map[(x2 >> tilebits) + (y2 >> tilebits)*mapw];

			// tile pixel
			*d++ = src[(tileinx<<(2*tilebits)) + (x2 & tilemask) + ((y2 & tilemask)<<tilebits)];
		}
		y0++;
		d += wbd;
	}
}

#endif // USE_MAT2D

// draw image line interpolated
//  src = source image
//  xd,yd = destination coordinates
//  wd = destination width
//  ws = source width
//  wbs = source line pitch in bytes
void DrawImgLine(const u8* src, int xd, int yd, int wd, int ws, int wbs)
{
	// some base checks
	if ((wd <= 0) || (ws <= 0) || (yd < 0) || (yd >= HEIGHT)) return;

	// pixel increment
	int dinc = FRACTMUL*ws/wd;

	// update dirty rectangle
	DispDirtyRect(xd, yd, wd, 1);

	// prepare buffers
	u8* d = &pDrawBuf[xd + yd*WIDTH]; // destination address
	int i, j;

	int dadd = 0;
	for (i = 0; i < wd; i++)
	{
		*d++ = *src;
		dadd += dinc;
		src += dadd >> FRACT;
		dadd &= (FRACTMUL-1);
	}
}

// rainbow gradient table
//	GP0 ... B0 ... VGA B0 blue
//	GP1 ... B1 ... VGA B1
//	GP2 ... B2 ... VGA G0 green
//	GP3 ... B3 ... VGA G1
//	GP4 ... B4 ... VGA G2
//	GP5 ... B5 ... VGA R0 red
//	GP6 ... B6 ... VGA R1
//	GP7 ... B7 ... VGA R2
const u8 RainbowGrad[] = {
	B7,			// dark red
	B5+B6+B7,		// red
	B4+B6+B7,		// orange
	B2+B3+B4+B5+B6+B7,	// yellow
	B2+B3+B4+B7,		// yellow-green
	B2+B3+B4,		// green
	B0+B3+B4,		// green-cyan
	B0+B1+B2+B3+B4,		// cyan
	B0+B1+B4,		// cyan-blue
	B0+B1,			// blue
	B0+B1+B7,		// blue-magenta
	B0+B1+B5+B6+B7,		// magenta
	B1+B7,			// dark magenta
};

#define RAINBOW_NUM count_of(RainbowGrad)

// generate gradient
void GenGrad(u8* dst, int w)
{
	// sample - rainbow pattern
	int i;
	for (i = 0; i < w; i++)
	{
		*dst++ = RainbowGrad[i*RAINBOW_NUM/w];
	}
}

// scroll screen one row up
void DrawScroll()
{
	// size of one row (in number of pixels)
	int rowsize = WIDTH*DrawFontHeight;

	// move 1 row up
	memmove(pDrawBuf, &pDrawBuf[rowsize], (FRAMESIZE - rowsize)*sizeof(u8));

	// clear last row
	u8* d = &pDrawBuf[FRAMESIZE - rowsize];
	for (; rowsize > 0; rowsize--) *d++ = COL_BLACK;

	// update screen
	DispDirtyAll();
}

// console print character (without display update)
//   Control characters:
//     0x01 '\1' ^A ... set not-inverted text
//     0x02 '\2' ^B ... set inverted text (shift character code by 0x80)
//     0x03 '\3' ^C ... use normal-sized font (default)
//     0x04 '\4' ^D ... use double-height font
//     0x05 '\5' ^E ... use double-width font
//     0x06 '\6' ^F ... use double-sized font
//     0x07 '\a' ^G ... (bell) move cursor 1 position right (no print; uses width of normal-sized font)
//     0x08 '\b' ^H ... (back space) move cursor 1 position left (no print; uses width of normal-sized font)
//     0x09 '\t' ^I ... (tabulator) move cursor to next 8-character position, print spaces (uses width of normal-sized font)
//     0x0A '\n' ^J ... (new line) move cursor to start of next row
//     0x0B '\v' ^K ... (vertical tabulator) move cursor to start of previous row
//     0x0C '\f' ^L ... (form feed) clear screen, reset cursor position and set default color
//     0x0D '\r' ^M ... (carriage return) move cursor to start of current row
//     0x10 '\20' ^P ... set gray text color (COL_GRAY, default)
//     0x11 '\21' ^Q ... set blue text color (COL_AZURE)
//     0x12 '\22' ^R ... set green text color (COL_GREEN)
//     0x13 '\23' ^S ... set cyan text color (COL_CYAN)
//     0x14 '\24' ^T ... set red text color (COL_RED)
//     0x15 '\25' ^U ... set magenta text color (COL_MAGENTA)
//     0x16 '\26' ^V ... set yellow text color (COL_YELLOW)
//     0x17 '\27' ^W ... set white text color (COL_WHITE)
void DrawPrintCharRaw(char ch)
{
	switch ((u8)ch)
	{
	// printable characters
	default:
		if ((u8)ch < 32) break; // not printable character

		// double-width or double-sized font
		if (DrawPrintSize >= 2)
		{
			if (DrawPrintSize == 3)
			{
				if (DrawPrintRow >= DrawPrintRowNum-1)
				{
					DrawScroll(); // scroll screen one row up
					DrawPrintRow = DrawPrintRowNum-2;
				}

				// double-sized font
				DrawCharBg2((char)(ch + DrawPrintInv), (int)DrawPrintPos*DrawFontWidth,
					(int)DrawPrintRow*DrawFontHeight, DrawPrintCol, COL_BLACK);
			}
			else
				// double-width font
				DrawCharBgW((char)(ch + DrawPrintInv), (int)DrawPrintPos*DrawFontWidth,
					(int)DrawPrintRow*DrawFontHeight, DrawPrintCol, COL_BLACK);

			// move cursor 1 position right
			DrawPrintPos++;
		}
		else
		{
			if (DrawPrintSize == 1)
			{
				if (DrawPrintRow >= DrawPrintRowNum-1)
				{
					DrawScroll(); // scroll screen one row up
					DrawPrintRow = DrawPrintRowNum-2;
				}

				// double-height font
				DrawCharBgH((char)(ch + DrawPrintInv), (int)DrawPrintPos*DrawFontWidth,
					(int)DrawPrintRow*DrawFontHeight, DrawPrintCol, COL_BLACK);
			}
			else
				// normal font size
				DrawCharBg((char)(ch + DrawPrintInv), (int)DrawPrintPos*DrawFontWidth,
					(int)DrawPrintRow*DrawFontHeight, DrawPrintCol, COL_BLACK);
		}

  // ! continue case 0x07 (move right)

	// '\a' ... (bell) move cursor 1 position right (no print)
	case 0x07:
		DrawPrintPos++;
		if (DrawPrintPos < DrawPrintPosNum) break;

  // ! continue case 0x0A (new line)

	// '\n' ... (new line) move cursor to start of next row
	case 0x0A:
		DrawPrintPos = 0;
		DrawPrintRow++; // increase row
		if (DrawPrintRow >= DrawPrintRowNum)
		{
			DrawScroll(); // scroll screen one row up
			DrawPrintRow = DrawPrintRowNum-1;
		}

		// double height
		if ((DrawPrintSize & 1) != 0)
		{
			DrawPrintRow++; // increase row
			if (DrawPrintRow >= DrawPrintRowNum)
			{
				DrawScroll(); // scroll screen one row up
				DrawPrintRow = DrawPrintRowNum-1;
			}
		}
		break;

	// 'x01' ... set not-inverted text
	case 0x01:
		DrawPrintInv = 0;
		break;

	// 'x02' ... set inverted text (shift character code by 0x80)
	case 0x02:
		DrawPrintInv = 128;
		break;

	// 'x03' ... use normal-sized font (default)
	case 0x03:
		DrawPrintSize = 0; // fonf size: 0=normal, 1=double-height, 2=double-width, 3=double-size
		break;

	// 'x04' ... use double-height font
	case 0x04:
		DrawPrintSize = 1; // fonf size: 0=normal, 1=double-height, 2=double-width, 3=double-size
		break;

	// 'x05' ... use double-width font
	case 0x05:
		DrawPrintSize = 2; // fonf size: 0=normal, 1=double-height, 2=double-width, 3=double-size
		break;

	// 'x06' ... use double-sized font
	case 0x06:
		DrawPrintSize = 3; // fonf size: 0=normal, 1=double-height, 2=double-width, 3=double-size
		break;

	// '\b' ... (back space) move cursor 1 position left (no print)
	case 0x08:
		if (DrawPrintPos > 0) DrawPrintPos--;
		break;

	// '\t' ... (tabulator) move cursor to next 8-character position, print spaces
	case 0x09:
		{
			u8 oldsize = DrawPrintSize; // save current font size
			DrawPrintSize = oldsize & 1; // change double-width or double-size to normal or double-height
			do DrawPrintCharRaw(' '); while ((DrawPrintPos & 7) != 0); // print spaces
			DrawPrintSize = oldsize; // restore font size
		}
		break;		

	// '\v' ... (vertical tabulator) move cursor to start of previous row
	case 0x0B:
		if (DrawPrintRow > 0) DrawPrintRow--; // decrease row
		DrawPrintPos = 0;
		break;

	// '\f' ... (form feed) clear screen, reset cursor position and set default color
	case 0x0C:
		DrawClear(); // clear screen
		DrawPrintPos = 0;  // console print character position
		DrawPrintRow = 0;  // console print character row
		DrawPrintInv = 0; // offset added to character (128 = print inverted characters)
		DrawPrintSize = 0; // fonf size: 0=normal, 1=double-width, 2=double-size
		DrawPrintCol = COL_PRINT_DEF; // console print color
		break;

	// '\r' ... (carriage return) move cursor to start of current row
	case 0x0D:
		DrawPrintPos = 0;
		break;

	// 'x10' ... set gray text color (COL_GRAY, normal color)
	case 0x10:
		DrawPrintCol = COL_PRINT_DEF;
		break;

	// 'x11' ... set blue text color (COL_AZURE)
	case 0x11:
		DrawPrintCol = COL_AZURE;
		break;

	// 'x12' ... set green text color (COL_GREEN)
	case 0x12:
		DrawPrintCol = COL_GREEN;
		break;

	// 'x13' ... set cyan text color (COL_CYAN)
	case 0x13:
		DrawPrintCol = COL_CYAN;
		break;

	// 'x14' ... set red text color (COL_RED)
	case 0x14:
		DrawPrintCol = COL_RED;
		break;

	// 'x15' ... set magenta text color (COL_MAGENTA)
	case 0x15:
		DrawPrintCol = COL_MAGENTA;
		break;

	// 'x16' ... set yellow text color (COL_YELLOW)
	case 0x16:
		DrawPrintCol = COL_YELLOW;
		break;

	// 'x17' ... set white text color (COL_WHITE)
	case 0x17:
		DrawPrintCol = COL_WHITE;
		break;
	}
}

// console print character (with display update; Control characters - see DrawPrintCharRaw)
void DrawPrintChar(char ch)
{
	DrawPrintCharRaw(ch);
	DispUpdate();
}

// console print text (Control characters - see DrawPrintCharRaw)
//  If text contains digit after hex numeric code of control character,
//  split text to more parts: use "\4" "1" instead of "\41".
void DrawPrintText(const char* txt)
{
	char ch;
	while ((ch = *txt++) != 0) DrawPrintCharRaw(ch);
	DispUpdate();
}

// callback - write data to drawing console
u32 StreamWriteDrawPrint(sStream* str, const void* buf, u32 num)
{
	u32 n = num;
	const char* txt = (const char*)buf;
	for (; n > 0; n--) DrawPrintCharRaw(*txt++);
	return num;
}

// formatted print string to drawing console, with argument list (returns number of characters, without terminating 0)
u32 DrawPrintArg(const char* fmt, va_list args)
{
	// write and read stream
	sStream wstr, rstr;

	// initialize stream to read from
	StreamReadBufInit(&rstr, fmt, StrLen(fmt));

	// initialize stream to write to
	Stream0Init(&wstr); // initialize nul stream
	wstr.write = StreamWriteDrawPrint; // write callback
	
	// print string
	u32 num = StreamPrintArg(&wstr, &rstr, args);

	// display update
	if (num > 0) DispUpdate();
	return num;
}

// formatted print string to drawing console, with variadic arguments (returns number of characters, without terminating 0)
NOINLINE u32 DrawPrint(const char* fmt, ...)
{
	u32 n;
	va_list args;
	va_start(args, fmt);
	n = DrawPrintArg(fmt, args);
	va_end(args);
	return n;
}


/*
DrawPrint test sample:

	SelFont8x16(); // select font 8x16
	DrawPrint(
		"\f"					// clear screen (resolution: 40 characters x 15 rows)

		"- invisible row scrolls up -\n"	// 0: this row will be scrolled-up

		"Hello world?\b!\n"			// 1: first visible print, overprint "?" -> "!"

		"First! row\r" "Second\n"		// 2: overprint "First! row" -> "Second row"

		"\3normal \4height \5width \6sized\n"	// 3: double-height row with font of all 4 sizes
		"\3font   \a\a\a\a\a\a\a\5font\3\n"	// 4: second half of previous double-height row

		"non-invert \2invert\1 non-invert\n"	// 5: inverted text

		"0123456701234567012345670123456701234567" // 6: print ruler 40 characters and move to next row (row is 40 columns width)
		"\t123\t1234567\t12345678\tnext row\n"	// 7, 8: move to tab positions and move to next row after all

		"\20 normal \21 blue \22 green \23 cyan \n" // 9: print colors
		"\24 red \25 magenta \26 yellow \27 white \n" // 10:

		"\2"
		"\20 normal \21 blue \22 green \23 cyan \n" // 11: print colors inverted
		"\24 red \25 magenta \26 yellow \27 white \n" // 12:

		"\1\20"					// back to default state
		"%d 0x%08x ALL OK\n"			// 13: print numbers 12345678, 0x12345678

		"\nscrolling up",			// 14: this command scrolls screen up

		12345678, 0x12345678
	);
*/

#endif // USE_QDRAW		// use QVGA drawing (lib_qdraw.c, lib_qdraw.h)
