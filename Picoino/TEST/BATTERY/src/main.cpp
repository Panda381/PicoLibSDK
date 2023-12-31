
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

// color
const u8 BatCol[8] = {
	COL_BLACK,		// 0
	COL_RED,		// 1
	COL_ORANGE,		// 2
	COL_ORANGE,		// 3
	COL_YELLOW,		// 4
	COL_YELLOW,		// 5
	COL_GREEN,		// 6
	COL_GREEN,		// 7
};

int main()
{
	int i;

	pText txt;
	TextInit(&txt);	

	// set font
	pDrawFont = FontBold8x16; // font 8x8
	DrawFontHeight = 16; // font height

	int k = 0;

	while (True)
	{
		// draw battery
		float bat = GetBat();

#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
		i = (int)((bat - ConfigGetBatEmpty())/(ConfigGetBatFull()-ConfigGetBatEmpty())*8);
#else
		i = (int)((bat - BATTERY_EMPTY)/(BATTERY_FULL-BATTERY_EMPTY)*8);
#endif

		if (i < 0) i = 0;
		if (i > 7) i = 7;
		DrawImg(BatteryImg, 0, 0, 0, 0, 128, 240, 128);

		u16 col = BatCol[i];
		if (i > 0)
		{
			for (i--; i >= 0; i--) DrawRect(25, 198 - i*26, 77, 19, col);
		}

#define DISPX	128
#define DISPW	(WIDTH-128)
#define DY	25
		int y = 30;

		// title
		DrawText2("Picoino", DISPX + (DISPW-7*16)/2, y, COL_YELLOW);
		y += 50;

		// SDK version
		TextPrint(&txt, "SDK version %.2f", SDK_VER*0.01f);
		DrawTextBg(TextPtr(&txt), DISPX + (DISPW - TextLen(&txt)*8)/2, y, COL_WHITE, COL_BLACK);
		y += DY;

		// CPU
		TextPrint(&txt, "CPU version RP2040-B%d", RomGetVersion() - 1);
		DrawTextBg(TextPtr(&txt), DISPX + (DISPW - TextLen(&txt)*8)/2, y, COL_WHITE, COL_BLACK);
		y += DY;

		// temperature
		TextPrint(&txt, " temperature %.1f`C ", ADC_Temp());
		DrawTextBg(TextPtr(&txt), DISPX + (DISPW - TextLen(&txt)*8)/2, y, COL_WHITE, COL_BLACK);
		y += DY;

		// battery
		TextPrint(&txt, " battery %.2fV ", bat);
		DrawTextBg(TextPtr(&txt), DISPX + (DISPW - TextLen(&txt)*8)/2, y, COL_WHITE, COL_BLACK);
		y += DY;

		// frequency
		TextPrint(&txt, " frequency %dMHz ", (ClockGetHz(CLK_SYS)+500000)/1000000);
		DrawTextBg(TextPtr(&txt), DISPX + (DISPW - TextLen(&txt)*8)/2, y, COL_WHITE, COL_BLACK);
		y += DY;

		// redraw display
		DispUpdate();

		// indicate work done
		WaitMs(200);

		switch (KeyGet())
		{
		// reset to boot loader
		case KEY_Y: ResetToBootLoader();

#if USE_SCREENSHOT		// use screen shots
		case KEY_X:
			ScreenShot();
			break;
#endif
		}
	}
}
