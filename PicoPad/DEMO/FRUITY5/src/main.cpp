
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
	sizeof(Loop089Snd),
	sizeof(Loop090Snd),
	sizeof(Loop091Snd),
	sizeof(Loop092Snd),
	sizeof(Loop093Snd),
	sizeof(Loop094Snd),
	sizeof(Loop095Snd),
	sizeof(Loop096Snd),
	sizeof(Loop097Snd),
	sizeof(Loop098Snd),
	sizeof(Loop099Snd),
	sizeof(Loop100Snd),
	sizeof(Loop101Snd),
	sizeof(Loop102Snd),
	sizeof(Loop103Snd),
	sizeof(Loop104Snd),
	sizeof(Loop105Snd),
	sizeof(Loop106Snd),
	sizeof(Loop107Snd),
	sizeof(Loop108Snd),
	sizeof(Loop109Snd),
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
