
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
	Loop001Snd,
	Loop002Snd,
	Loop003Snd,
	Loop004Snd,
	Loop005Snd,
	Loop006Snd,
	Loop007Snd,
	Loop008Snd,
	Loop009Snd,
	Loop010Snd,
	Loop011Snd,
	Loop012Snd,
	Loop013Snd,
	Loop014Snd,
	Loop015Snd,
	Loop016Snd,
	Loop017Snd,
	Loop018Snd,
	Loop019Snd,
	Loop020Snd,
	Loop021Snd,
	Loop022Snd,
	Loop023Snd,
	Loop024Snd,
	Loop025Snd,
	Loop026Snd,
	Loop027Snd,
	Loop028Snd,
	Loop029Snd,
	Loop030Snd,
};

// number of bytes of the loops
const int LoopLen[LOOP_NUM] = {
	count_of(Loop001Snd),
	count_of(Loop002Snd),
	count_of(Loop003Snd),
	count_of(Loop004Snd),
	count_of(Loop005Snd),
	count_of(Loop006Snd),
	count_of(Loop007Snd),
	count_of(Loop008Snd),
	count_of(Loop009Snd),
	count_of(Loop010Snd),
	count_of(Loop011Snd),
	count_of(Loop012Snd),
	count_of(Loop013Snd),
	count_of(Loop014Snd),
	count_of(Loop015Snd),
	count_of(Loop016Snd),
	count_of(Loop017Snd),
	count_of(Loop018Snd),
	count_of(Loop019Snd),
	count_of(Loop020Snd),
	count_of(Loop021Snd),
	count_of(Loop022Snd),
	count_of(Loop023Snd),
	count_of(Loop024Snd),
	count_of(Loop025Snd),
	count_of(Loop026Snd),
	count_of(Loop027Snd),
	count_of(Loop028Snd),
	count_of(Loop029Snd),
	count_of(Loop030Snd),
};

// names of loops
const char* LoopName[LOOP_NUM] = {
	"3xOSC fun",
	"C_HC",
	"C_kick",
	"Dance_X_B",
	"dnbautoscratch",
	"Dreamzofluxury",
	"DrumCircle",
	"ElvisInADress",
	"FruityExample-alpharisc",
	"Fuego-BigKilling",
	"Fuego-FenderBender",
	"Fuego-GraySunshine",
	"H441-GOA",
	"Halfwaytonowhere",
	"ISuckAtTitles",
	"JasonC-Ebb_and_Flow",
	"JasonC-Faded@4am",
	"JasonC-Glimmer",
	"JasonC-Highpass",
	"JasonC-Stasis",
	"JasonC-Tranced@4am",
	"Land of rock",
	"Light already (remixed)",
	"MentalFusion",
	"Myopic-DepartureLounge",
	"Native soil",
	"Oscar22",
	"Oscarparty",
	"Oscillations",
	"Pilchard-KickingBeat",
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
