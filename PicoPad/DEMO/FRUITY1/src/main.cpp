
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
	sizeof(Loop001Snd),
	sizeof(Loop002Snd),
	sizeof(Loop003Snd),
	sizeof(Loop004Snd),
	sizeof(Loop005Snd),
	sizeof(Loop006Snd),
	sizeof(Loop007Snd),
	sizeof(Loop008Snd),
	sizeof(Loop009Snd),
	sizeof(Loop010Snd),
	sizeof(Loop011Snd),
	sizeof(Loop012Snd),
	sizeof(Loop013Snd),
	sizeof(Loop014Snd),
	sizeof(Loop015Snd),
	sizeof(Loop016Snd),
	sizeof(Loop017Snd),
	sizeof(Loop018Snd),
	sizeof(Loop019Snd),
	sizeof(Loop020Snd),
	sizeof(Loop021Snd),
	sizeof(Loop022Snd),
	sizeof(Loop023Snd),
	sizeof(Loop024Snd),
	sizeof(Loop025Snd),
	sizeof(Loop026Snd),
	sizeof(Loop027Snd),
	sizeof(Loop028Snd),
	sizeof(Loop029Snd),
	sizeof(Loop030Snd),
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
#if !USE_PICOPADNES		// use PicoPadNES device configuration
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
