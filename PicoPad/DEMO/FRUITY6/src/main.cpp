
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
	sizeof(Loop110Snd),
	sizeof(Loop111Snd),
	sizeof(Loop112Snd),
	sizeof(Loop113Snd),
	sizeof(Loop114Snd),
	sizeof(Loop115Snd),
	sizeof(Loop116Snd),
	sizeof(Loop117Snd),
	sizeof(Loop118Snd),
	sizeof(Loop119Snd),
	sizeof(Loop120Snd),
	sizeof(Loop121Snd),
	sizeof(Loop122Snd),
	sizeof(Loop123Snd),
	sizeof(Loop124Snd),
	sizeof(Loop125Snd),
	sizeof(Loop126Snd),
	sizeof(Loop127Snd),
	sizeof(Loop128Snd),
	sizeof(Loop129Snd),
	sizeof(Loop130Snd),
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
