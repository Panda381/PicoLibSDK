
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#define DISP_FRAMEINFO	0	// 1=display MP3 frame info

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
	sizeof(Loop071Snd),
	sizeof(Loop072Snd),
	sizeof(Loop073Snd),
	sizeof(Loop074Snd),
	sizeof(Loop075Snd),
	sizeof(Loop076Snd),
	sizeof(Loop077Snd),
	sizeof(Loop078Snd),
	sizeof(Loop079Snd),
	sizeof(Loop080Snd),
	sizeof(Loop081Snd),
	sizeof(Loop082Snd),
	sizeof(Loop083Snd),
	sizeof(Loop084Snd),
	sizeof(Loop085Snd),
	sizeof(Loop086Snd),
	sizeof(Loop087Snd),
	sizeof(Loop088Snd),
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

// MP3 player
sMP3Player MP3Player;
u8 ALIGNED MP3PlayerOutBuf[MP3PLAYER_OUTSIZE];

// start new loop
void NewLoop()
{
	// clear screen
	DrawClearCol(COL_WHITE);

	// draw logo
	DrawImg(LogoImg, 0, 0, (WIDTH - LOGO_W)/2, (HEIGHT - LOGO_H)/2, LOGO_W, LOGO_H, LOGO_W_FULL);

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

	// start/stop playing loop
	sMP3Player* mp3 = &MP3Player;
	if (Mute)
		MP3Stop(mp3);
	else
	{
		int r = MP3PlayerInit(mp3, NULL, LoopList[Loop], LoopLen[Loop], MP3PlayerOutBuf, MP3PLAYER_OUTSIZE, -1);
		if (r == ERR_MP3_NONE)
		{
			MP3Play(mp3, 0, True);

#if DISP_FRAMEINFO	// display MP3 frame info
			DrawRect(0, 0, WIDTH, 3*16, COL_BLUE);
			char bf[70];
			MemPrint(bf, 70, "FrameTime=%dus Frames=%d", mp3->frametime, mp3->frames);
			DrawText(bf, 0, 0, COL_YELLOW);
			MemPrint(bf, 70, "FrameIn=%dB FrameOut=%dB Size=%dB", mp3->framesizeavg, mp3->sampnumavg*2, LoopLen[Loop]);
			DrawText(bf, 0, 16, COL_YELLOW);
			MemPrint(bf, 70, "Bitrate=%dkbps SampRate=%dHz", mp3->bitrateavg/1000, mp3->info.samprate);
			DrawText(bf, 0, 32, COL_YELLOW);
			DispUpdate();
#endif
		}
		else
			Mute = True;
	}
}

int main()
{
#if !USE_PICOPADHSTX		// use PicoPadHSTX device configuration
	ClockPllSysFreqVolt(200000);
#endif

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

		sMP3Player* mp3 = &MP3Player;
		if (!Mute && MP3Playing(mp3))
		{
			MP3Poll(mp3);

			int pos = mp3->pos * WIDTH / mp3->frames;
			DrawRect(0, HEIGHT-4, pos, 4, COL_GREEN);
			DrawRect(pos, HEIGHT-4, WIDTH - pos, 4, COL_BLACK);
			DispUpdate();
		}
	}
}
