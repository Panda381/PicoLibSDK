
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
	Loop071Snd,
	Loop072Snd,
	Loop073Snd,
	Loop074Snd,
	Loop075Snd,
	Loop076Snd,
	Loop077Snd,
	Loop078Snd,
	Loop079Snd,
	Loop080Snd,
	Loop081Snd,
	Loop082Snd,
	Loop083Snd,
	Loop084Snd,
	Loop085Snd,
	Loop086Snd,
	Loop087Snd,
	Loop088Snd,
};

// number of bytes of the loops
const int LoopLen[LOOP_NUM] = {
	count_of(Loop071Snd),
	count_of(Loop072Snd),
	count_of(Loop073Snd),
	count_of(Loop074Snd),
	count_of(Loop075Snd),
	count_of(Loop076Snd),
	count_of(Loop077Snd),
	count_of(Loop078Snd),
	count_of(Loop079Snd),
	count_of(Loop080Snd),
	count_of(Loop081Snd),
	count_of(Loop082Snd),
	count_of(Loop083Snd),
	count_of(Loop084Snd),
	count_of(Loop085Snd),
	count_of(Loop086Snd),
	count_of(Loop087Snd),
	count_of(Loop088Snd),
};

// names of loops
const char* LoopName[LOOP_NUM] = {
	"DanR-WhatDoesThatKnobDoAgain",
	"Duda-Drumsynth Trigger",
	"Fuego+Reegz-Aura",
	"Fuego-AHintOfReggae",
	"Fuego-Behaviour 9",
	"Fuego-FL303",
	"Fuego-Skyhigh",
	"gol-Random",
	"Inginer-Housy Shuffle",
	"Inginer-Jungle Fever",
	"JasonC-After Hours",
	"JasonC-Dark Corners",
	"JasonC-Glint",
	"JasonC-Groundswell",
	"JasonC-Kinetic",
	"Pilchard-D-S-L",
	"Pilchard-Goathing",
	"Reegz-Foxy Sista",
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
