
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

// open selection
int OpenSelInx = 0;

// version text
const char* VerText = "v1.2";

// open text
const char* const OpenText[] = {
	"None of the black or red ants remember",
	"who and when started their age-old war",
	"for the anthill \"At the Two Spruces\".",
	"In order to put an end to the eternal",
	"battles, they decided to build castles,",
	"with the help of which they would gain",
	"dominance over the whole territory.",
	"The aim of the game is to build a castle with a height of 100",
	"or destroy the opponent's castle. Each card consumes certain",
	"amount of resources (number at the top right). The rate of",
	"resource generation depends on the amount of workers.",
};

#if USE_PICOINOMINI
const char* SoundText = "X:Sound ";
#else
const char* SoundText = "Ctrl:Sound ";
#endif
const char* SoundTextSel[3]  = { "OFF ", "SOME", "ALL " };

// display open screen
void DispOpen()
{
	// clear screen
	DrawClear();

	// display ants
	DrawImg(OpenImg, 0, 0, 0, 0, 120, 120, IMGWB(240));
	DrawImg(OpenImg+120, 0, 0, WIDTH-120, 53, 120, 120, IMGWB(240));

	// display title
	DrawImg(TitleImg, 0, 0, 120+20, 0, 140, 53, IMGWB(140));
	DrawImg(Title2Img, 0, 0, 120+10, 53, 60, 50, IMGWB(60));

	// display open text
	SelFont5x8();
	int i;
	for (i = 0; i < count_of(OpenText); i++)
	{
		DrawText(OpenText[i], 0, 110+i*8, COL_WHITE);
	}

	// display version
	SelFont8x16();
	DrawText(VerText, 280, 40, COL_WHITE);

	// sound
	DrawText(SoundText, 0, 0, COL_GRAY);
#if USE_PICOINOMINI
	DrawText(SoundTextSel[SoundMode], 8*8, 0, COL_GRAY);
#else
	DrawText(SoundTextSel[SoundMode], 11*8, 0, COL_GRAY);
#endif
}

// display open selection
void DispOpenSel()
{
	DrawImg(OpenSelImg, 0, 0, 0, HEIGHT-40, WIDTH, 40, WIDTH);
	if (OpenSelInx < 4)
		DrawBlit(OpenSel2Img, 0, 0, OpenSelInx*60, HEIGHT-40, 60, 40, IMGWB(60), COL_BLACK);
	else
		DrawBlit(OpenSel3Img, 0, 0, (OpenSelInx-4)*40+4*60, HEIGHT-40, 40, 40, IMGWB(40), COL_BLACK);
}

// main function
int main()
{
	char ch;
	// u32 t;

	// display open screen
	DispOpen();
	DispOpenSel();

	// main loop
	// t = Time(); // auto-demo time
	while (True)
	{
		// input from keyboard
		ch = KeyGet();
		switch (ch)
		{
		// cursor left
		case KEY_LEFT:
			OpenSelInx--;
			if (OpenSelInx < 0) OpenSelInx = 5;
			DispOpenSel();
			KeyFlush();
			// t = Time(); // reset auto-demo time
			break;

		// cursor right
		case KEY_RIGHT:
			OpenSelInx++;
			if (OpenSelInx > 5) OpenSelInx = 0;
			DispOpenSel();
			KeyFlush();
			// t = Time(); // reset auto-demo time
			break;

		// run game
		case KEY_DOWN:
		case KEY_A:
			switch (OpenSelInx)
			{
			case 0: Game(PLAYER_HUMAN, PLAYER_HUMAN, False); break;
			case 1: Game(PLAYER_HUMAN, PLAYER_COMP, False); break;
			case 2: Game(PLAYER_COMP, PLAYER_HUMAN, False); break;
			case 3: Game(PLAYER_COMP, PLAYER_COMP, False); break;
			case 4: Game(PLAYER_HUMAN, PLAYER_USB, False); UsbPortTerm(); break;
			case 5: Game(PLAYER_USB, PLAYER_HUMAN, False); UsbPortTerm(); RandShift(); break;
			}
			StopSound();
			DispOpen();
			DispOpenSel();
			KeyFlush();
			// t = Time(); // reset auto-demo time
			break;
	
		// no key - start auto-demo
		case NOKEY:
		/*
			if ((u32)(Time() - t) >= 30000000ul) // auto demo after 30 seconds
			{
				Game(PLAYER_COMP, PLAYER_COMP, True);
				DispOpen();
				DispOpenSel();
				KeyFlush();
				t = Time(); // reset auto-demo time
			}
		*/
			break;

		// reset to boot loader
		case KEY_Y:
			ResetToBootLoader();
			break;

		// sound mode
		case KEY_X:
			SoundMode--;
			if (SoundMode < 0) SoundMode = 2;
			DispOpen();
			DispOpenSel();
			break;

		// unknown key
		default:
			KeyFlush();
			// t = Time(); // reset auto-demo time
			break;
		}

		// display update
		DispUpdate();
	}
}
