
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define DELAY		4	// delay in [ms]
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
const u8 Pal[] = {
	COL_WHITE,
	COL_WHITE,
	COL_LTGREEN,
	COL_LTGREEN,
	COL_LTGREEN,
	COL_LTGREEN,
	COL_LTGREEN,
	COL_LTGREEN,
	COL_LTGREEN,
	COL_GREEN,
	COL_GREEN,
	COL_GREEN,
	COL_GREEN,
	COL_GREEN,
	COL_GREEN,
	COL_GREEN,
	COL_GREEN,
	COL_GREEN,
	COL_DKGRAY,
	COL_DKGRAY,
	COL_DKGRAY,
	COL_DKGRAY,
	COL_DKGRAY,
	COL_DKGRAY,
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
	char* d;

	// generate new drops
	for (i = 0; i < TEXTWIDTH; i++)
	{
		NewDrop(i);
		Off[i] *= 2;
	}

	// generate random text screen
	d = TextBuf;
	for (i = TEXTSIZE; i > 0; i -= 2)
	{
		*d = RandU8MinMax(CHAR_MIN, CHAR_MAX);
		d += 2;
	}

	// main loop
	while (True)
	{
		// update color attributes
		d = &TextBuf[1];
		for (y = 0; y < TEXTHEIGHT; y++)
		{
			for (x = 0; x < TEXTWIDTH; x++)
			{
				i = (int)((Off[x] - y)/Len[x]*PALLEN + 0.5f);
				if ((i < 0) || (i >= PALLEN))
					*d = COL_BLACK;
				else
					*d = Pal[i];			
				d += 2;
			}
		}

		// randomize some characters
		for (i = 40; i > 0; i--)
		{
			TextBuf[RandU16Max(TEXTWIDTH*TEXTHEIGHT-1)*2] = RandU8MinMax(CHAR_MIN, CHAR_MAX);
		}

		// shift drops
		for (x = 0; x < TEXTWIDTH; x++)
		{
			Off[x] += Speed[x];

			// generate new drop
			if (Off[x] - Len[x] > TEXTHEIGHT) NewDrop(x);
		}

		// display text screen
		DrawFTextBuf(TextBuf, COL_BLACK);
//		DispUpdate();

		// speed
//		WaitMs(DELAY);

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
