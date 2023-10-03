
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define LEN_MIN	 	6	// minimal length of drop
#define LEN_MAX  	50	// maximal length of drop
#define OFF_MIN		0	// minimal offset
#define OFF_MAX		100	// maximal offset
#define SPEED_MIN	0.2f	// minimal speed
#define SPEED_MAX	0.5f	// maximal speed
#define CHAR_MIN	33	// minimal character
#define CHAR_MAX	126	// maximal character

// text screen with foreground color
char TextBuf[FTEXTSIZE];

// palette table
const u16 Pal[] = {
	COL_WHITE,
	COLOR(200,255,200),
	COLOR(127,255,127),
	COLOR(96,255,96),
	COLOR(32,255,32),
	COLOR(0,255,0),
	COLOR(0,255,0),
	COLOR(0,255,0),
	COLOR(0,240,0),
	COLOR(0,220,0),
	COLOR(0,200,0),
	COLOR(0,180,0),
	COLOR(0,160,0),
	COLOR(0,150,0),
	COLOR(0,140,0),
	COLOR(0,130,0),
	COLOR(0,120,0),
	COLOR(0,110,0),
	COLOR(0,100,0),
	COLOR(0,80,0),
	COLOR(0,70,0),
	COLOR(0,60,0),
	COLOR(0,50,0),
	COLOR(0,40,0),
	COLOR(0,30,0),
	COLOR(0,20,0),
	COLOR(0,10,0),
};
#define PALLEN count_of(Pal)	// length of palette table

// length of drops
u8 Len[TEXTWIDTH];

// offset of drops
float Off[TEXTWIDTH];

// falling speed
float Speed[TEXTWIDTH];

// generate new drop
void NewDrop(int i)
{
	Len[i] = RandU8MinMax(LEN_MIN, LEN_MAX);
	Off[i] = -RandFloatMinMax(OFF_MIN, OFF_MAX);
	Speed[i] = RandFloatMinMax(SPEED_MIN, SPEED_MAX);
}

int main()
{
	int i, x, y;
	u8* s;
	u16 c;
	char* d;

	// generate new drops
	for (i = 0; i < TEXTWIDTH; i++)
	{
		NewDrop(i);
		Off[i] *= 2;
	}

	// generate random text screen
	d = TextBuf;
	for (i = FTEXTSIZE; i > 0; i -= 3)
	{
		*d = RandU8MinMax(CHAR_MIN, CHAR_MAX);
		d += 3;
	}

	// main loop
	while (True)
	{
		// update color attributes
		d = TextBuf;
		for (y = 0; y < TEXTHEIGHT; y++)
		{
			for (x = 0; x < TEXTWIDTH; x++)
			{
				d++;
				i = (int)((Off[x] - y)/Len[x]*PALLEN + 0.5f);
				if ((i < 0) || (i >= PALLEN))
					c = COL_BLACK;
				else
					c = Pal[i];			
				*d++ = (u8)c;
				*d++ = (u8)(c>>8);
			}
		}

		// randomize some characters
		for (i = 40; i > 0; i--)
		{
			TextBuf[RandU16Max(TEXTWIDTH*TEXTHEIGHT-1)*3] = RandU8MinMax(CHAR_MIN, CHAR_MAX);
		}

		// shift drops
		for (x = 0; x < TEXTWIDTH; x++)
		{
			Off[x] += Speed[x];

			// generate new drop
			if (Off[x] - Len[x] > TEXTHEIGHT) NewDrop(x);
		}

		// display text screen (with update)
		DrawFTextBuf(TextBuf, COL_BLACK);

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
