
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
	Loop110Snd,
	Loop111Snd,
	Loop112Snd,
	Loop113Snd,
	Loop114Snd,
	Loop115Snd,
	Loop116Snd,
	Loop117Snd,
	Loop118Snd,
	Loop119Snd,
	Loop120Snd,
	Loop121Snd,
	Loop122Snd,
	Loop123Snd,
	Loop124Snd,
	Loop125Snd,
	Loop126Snd,
	Loop127Snd,
	Loop128Snd,
	Loop129Snd,
	Loop130Snd,
};

// number of bytes of the loops
const int LoopLen[LOOP_NUM] = {
	count_of(Loop110Snd),
	count_of(Loop111Snd),
	count_of(Loop112Snd),
	count_of(Loop113Snd),
	count_of(Loop114Snd),
	count_of(Loop115Snd),
	count_of(Loop116Snd),
	count_of(Loop117Snd),
	count_of(Loop118Snd),
	count_of(Loop119Snd),
	count_of(Loop120Snd),
	count_of(Loop121Snd),
	count_of(Loop122Snd),
	count_of(Loop123Snd),
	count_of(Loop124Snd),
	count_of(Loop125Snd),
	count_of(Loop126Snd),
	count_of(Loop127Snd),
	count_of(Loop128Snd),
	count_of(Loop129Snd),
	count_of(Loop130Snd),
};

// names of loops
const char* LoopName[LOOP_NUM] = {
	"Supa-Dupa-Fly (Sezam Remix)",
	"a-reflective-dbu",
	"Aphex Twins - Star Wars Theme (Trance)",
	"beethoven - 9th symphony (techno)",
	"beethoven - 9th symphony (techno) 2",
	"carl cox - TECHNO SETS (Live)",
	"Crystal Techno",
	"Disco Kandi 3 - DJD Hydraulic Dogs",
	"Dj Disco - Let's Dance(Extended Rmx)",
	"Dj Quicksilver - Techno Macht Spass",
	"DJ Tiesto - Forbidden Paradise",
	"DJ Tiesto - Forbidden Paradise 2",
	"robert miles - x files (techno mix)",
	"X-Perience - Land of Tomatoes",
	"Holymen - Monkey Buieness",
	"Astral Projection - Analog Voyager",
	"Astral Projection - Aurora Borealis",
	"Astral Projection - Dancing Galaxy",
	"Astral Projection - Enlightened",
	"Astral Projection - Power Gen",
	"Astral Projection - Still on Mars",
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
		PlayADPCMRep(LoopList[Loop], LoopLen[Loop], IMA_SAMPBLOCK);
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
