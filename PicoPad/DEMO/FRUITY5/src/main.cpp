
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
	Loop089Snd,
	Loop090Snd,
	Loop091Snd,
	Loop092Snd,
	Loop093Snd,
	Loop094Snd,
	Loop095Snd,
	Loop096Snd,
	Loop097Snd,
	Loop098Snd,
	Loop099Snd,
	Loop100Snd,
	Loop101Snd,
	Loop102Snd,
	Loop103Snd,
	Loop104Snd,
	Loop105Snd,
	Loop106Snd,
	Loop107Snd,
	Loop108Snd,
	Loop109Snd,
};

// number of bytes of the loops
const int LoopLen[LOOP_NUM] = {
	count_of(Loop089Snd),
	count_of(Loop090Snd),
	count_of(Loop091Snd),
	count_of(Loop092Snd),
	count_of(Loop093Snd),
	count_of(Loop094Snd),
	count_of(Loop095Snd),
	count_of(Loop096Snd),
	count_of(Loop097Snd),
	count_of(Loop098Snd),
	count_of(Loop099Snd),
	count_of(Loop100Snd),
	count_of(Loop101Snd),
	count_of(Loop102Snd),
	count_of(Loop103Snd),
	count_of(Loop104Snd),
	count_of(Loop105Snd),
	count_of(Loop106Snd),
	count_of(Loop107Snd),
	count_of(Loop108Snd),
	count_of(Loop109Snd),
};

// names of loops
const char* LoopName[LOOP_NUM] = {
	"Reegz-IFI",
	"Reegz-Messin Around",
	"Reegz-The Way (Instrumental)",
	"Toby-Carrier",
	"Toby-Striker",
	"XOR-Eliminator",
	"AmbushInTheBush",
	"BeatMachine",
	"StringLoop",
	"NewStuff Rap",
	"Korben Dallas",
	"Aknot! Wot",
	"Holymen - The Last Universe",
	"Holymen - Area 51 (Shiva Shidapu)",
	"Holymen - Seven'th Heaven",
	"Holymen - Sicko Leako",
	"Holymen - Time for Peace",
	"Holymen - Romeo 4 Juliete",
	"prodigy-out_of_space_(techno_remix)",
	"D.O.N.S feat. Technotronic - Pump Up",
	"punk city - mission-(sample.b6)",
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
