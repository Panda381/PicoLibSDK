
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
	Loop031Snd,
	Loop032Snd,
	Loop033Snd,
	Loop034Snd,
	Loop035Snd,
	Loop036Snd,
	Loop037Snd,
	Loop038Snd,
	Loop039Snd,
	Loop040Snd,
	Loop041Snd,
	Loop042Snd,
	Loop043Snd,
	Loop044Snd,
	Loop045Snd,
	Loop046Snd,
	Loop047Snd,
	Loop048Snd,
	Loop049Snd,
	Loop050Snd,
};

// number of bytes of the loops
const int LoopLen[LOOP_NUM] = {
	count_of(Loop031Snd),
	count_of(Loop032Snd),
	count_of(Loop033Snd),
	count_of(Loop034Snd),
	count_of(Loop035Snd),
	count_of(Loop036Snd),
	count_of(Loop037Snd),
	count_of(Loop038Snd),
	count_of(Loop039Snd),
	count_of(Loop040Snd),
	count_of(Loop041Snd),
	count_of(Loop042Snd),
	count_of(Loop043Snd),
	count_of(Loop044Snd),
	count_of(Loop045Snd),
	count_of(Loop046Snd),
	count_of(Loop047Snd),
	count_of(Loop048Snd),
	count_of(Loop049Snd),
	count_of(Loop050Snd),
};

// names of loops
const char* LoopName[LOOP_NUM] = {
	"Pilchard-NewFruity",
	"Reegz-Loungin",
	"Reegz-Vibe",
	"Roof garden_B",
	"SpaceAce",
	"speecher",
	"spookytriangle",
	"Sympathy",
	"Tears",
	"Techmatik",
	"TheAmericanPropertyNetwork",
	"threeosc",
	"Toby-Falling",
	"Toby-Hydrosphere",
	"Toby-Industro",
	"Toby-RainDrops",
	"Toby-Reaper",
	"Vlad-She'saRican",
	"XOR-Chop my slice please",
	"XOR-Osmosis",
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
