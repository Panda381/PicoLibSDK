
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int Loop = 0; // current loop
Bool Mute = False; // mute music

// Loop list
const u8* LoopList[LOOP_NUM] = {
	Loop051Snd,
	Loop052Snd,
	Loop053Snd,
	Loop054Snd,
	Loop055Snd,
	Loop056Snd,
	Loop057Snd,
	Loop058Snd,
	Loop059Snd,
	Loop060Snd,
	Loop061Snd,
	Loop062Snd,
	Loop063Snd,
	Loop064Snd,
	Loop065Snd,
	Loop066Snd,
	Loop067Snd,
	Loop068Snd,
	Loop069Snd,
	Loop070Snd,
};

// number of bytes of the loops
const int LoopLen[LOOP_NUM] = {
	count_of(Loop051Snd),
	count_of(Loop052Snd),
	count_of(Loop053Snd),
	count_of(Loop054Snd),
	count_of(Loop055Snd),
	count_of(Loop056Snd),
	count_of(Loop057Snd),
	count_of(Loop058Snd),
	count_of(Loop059Snd),
	count_of(Loop060Snd),
	count_of(Loop061Snd),
	count_of(Loop062Snd),
	count_of(Loop063Snd),
	count_of(Loop064Snd),
	count_of(Loop065Snd),
	count_of(Loop066Snd),
	count_of(Loop067Snd),
	count_of(Loop068Snd),
	count_of(Loop069Snd),
	count_of(Loop070Snd),
};

// names of loops
const char* LoopName[LOOP_NUM] = {
	"XOR-Reggophany",
	"asche",
	"drwho",
	"Fuego96-NashvilleWest",
	"404lament",
	"hERETIC-Tubular",
	"Try Again (remixed)",
	"Try Again (remixed) 2",
	"JasonC-Fubar",
	"JasonC-Greyscale",
	"MakeMeFeelLove",
	"Ori'sBminors",
	"Vlad-FeveriaMagma4-InstrumentalRemix",
	"FruityTechTrance",
	"JasonC-Shear",
	"Arpeggiator",
	"NewStuff",
	"Daft Punk - Revolution 909",
	"CWCrisman-Real Mellow",
	"DanR-Taken",
};

// start new loop
void NewLoop()
{
	// clear screen
	DrawClearCol(COL_WHITE);

	// draw logo
	DrawImg4Pal(LogoImg, LogoImg_Pal, 0, 0, (WIDTH - LOGO_W)/2, (HEIGHT - LOGO_H)/2, LOGO_W, LOGO_H, LOGO_W);

	// prepare loop number
	int len0 = DecNum(DecNumBuf, Loop+LOOP_FIRST, 0);
	DecNumBuf[len0++] = ':';
	DecNumBuf[len0] = 0;

	// draw loop name
	int len = len0 + StrLen(LoopName[Loop]);
	if (len > 20)
	{
		int x = (WIDTH - len*8)/2;
		DrawTextH(DecNumBuf, x, HEIGHT-32, COL_RED);
		x += len0*8;
		DrawTextH(LoopName[Loop], x, HEIGHT-32, COL_RED);
	}
	else
	{
		int x = (WIDTH - len*16)/2;
		DrawText2(DecNumBuf, x, HEIGHT-32, COL_RED);
		x += len0*16;
		DrawText2(LoopName[Loop], x, HEIGHT-32, COL_RED);
	}

	// display update
	DispUpdate();

	// start playing loop
	if (Mute)
		StopSound();
	else
		PlayADPCMRep(LoopList[Loop], LoopLen[Loop]*2, IMA_SAMPBLOCK);
}

int main()
{
	Loop = 0;
	NewLoop();

	while(True)
	{
		switch (KeyGet())
		{
		// mute
		case KEY_X:
			Mute = !Mute;
			NewLoop();
			break;

		// exit
		case KEY_Y:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			ResetToBootLoader();
			break;

		// previous
		case KEY_LEFT:
		case KEY_A:
			Loop--;
			if (Loop < 0) Loop = LOOP_NUM-1;
			NewLoop();
			break;

		// next
		case KEY_RIGHT:
		case KEY_B:
			Loop++;
			if (Loop >= LOOP_NUM) Loop = 0;
			NewLoop();
			break;
		}
	}
}
