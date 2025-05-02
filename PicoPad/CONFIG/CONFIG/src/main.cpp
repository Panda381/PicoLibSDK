
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

#define MENU_NUM	6	// number of menu entries
int MenuSel = 0;		// selected menu entry

const char* MenuTxt[MENU_NUM] = { // menu entries
	" Volume        ",
	" Backlight     ",
	" ADC reference ",
	" Battery       ",
	" Temperature   ",
	" Crystal       ",
};

// display progress bar
void Progress(int i, int n, int y, COLTYPE col)
{
#define PROGRESS_X 32
#define PROGRESS_W 256
#define PROGRESS_H 16

	int w = i*PROGRESS_W/n;
	if (w > PROGRESS_W) w = PROGRESS_W;

	DrawFrame(PROGRESS_X-2, y-2, PROGRESS_W+4, PROGRESS_H+4, COL_WHITE);
	DrawRect(PROGRESS_X, y, PROGRESS_W, PROGRESS_H, COL_GRAY);
	DrawRect(PROGRESS_X, y, w, PROGRESS_H, col);
	DispUpdate();
}

// display page title
void PgTitle()
{
	// clear screen
	DrawClear();

	// prepare text and its length (without trailing spaces)
	const char* t = MenuTxt[MenuSel];
	int len = StrLen(t);
	while ((len > 0) && (t[len-1] == ' ')) len--;

	// prepare X
	int x = (WIDTH - len*16 - 40)/2;

	// display icon
	DrawImg(IconImg, 0, MenuSel*40, x, 0, 32, 40, 32);

	// display title
	DrawText2(t, x + 40, 4, COL_WHITE);
}

// display volume
void PgVolumeDisp()
{
	COLTYPE col = COL_GREEN;
	u8 vol = ConfigGetVolume();
	if (vol > CONFIG_VOLUME_FULL) col = COL_YELLOW;
	if (vol > (CONFIG_VOLUME_FULL+CONFIG_VOLUME_MAX)/2) col = COL_RED;
	Progress(vol, CONFIG_VOLUME_MAX, 116, col);
}

// page Volume
void PgVolume()
{
	// draw title
	PgTitle();

	// scale
	DrawText("0%    50%   100%  150%  200%  250%", 26, 100, COL_WHITE);

	// display progress bar
	PgVolumeDisp();

	// help
	DrawText("LEFT/RIGHT=set, X=sample, A=OK, Y=cancel", 0, 224, COL_WHITE);

	// display update
	DispUpdate();

	while (True)
	{
		switch (KeyGet())
		{
		// decrease volume
		case KEY_LEFT:
			ConfigDecVolume();
			PgVolumeDisp();	// display progress bar
			break;

		// increase volume
		case KEY_RIGHT:
			ConfigIncVolume();
			PgVolumeDisp();	// display progress bar
			break;

		// play test sound
		case KEY_X:
			if (PlayingSound())
				StopSound();
			else
				PlaySoundChan(0, TestSnd, sizeof(TestSnd), True, SNDSPEED_32K, 1.0f, SNDFORM_ADPCM_S, 1017);
			break;

		// save
		case KEY_A:
			StopSound(); // stop sample
			ConfigSave(); // save configuration
			return;

		// return
		case KEY_Y:
			StopSound(); // stop sample
			ConfigLoad(); // restore settings
			return;
		}
	}
}

// display backlight
void PgBacklightDisp()
{
	Progress(ConfigGetBacklight(), CONFIG_BACKLIGHT_MAX, 116, COL_GREEN);
}

// page Backlight
void PgBacklight()
{
	// draw title
	PgTitle();

	// scale
	DrawText("0   1   2   3   4   5   6   7   8", 26, 100, COL_WHITE);

	// display progress bar
	PgBacklightDisp();

	// help
	DrawText("LEFT/RIGHT=set, A=OK, Y=cancel", (WIDTH-30*8)/2, 224, COL_WHITE);

	// display update
	DispUpdate();

	while (True)
	{
		switch (KeyGet())
		{
		// decrease bright
		case KEY_LEFT:
			if (Config.backlight > CONFIG_BACKLIGHT_MIN)
			{
				ConfigDecBacklight();
				PgBacklightDisp();	// display progress bar
			}
			break;

		// increase bright
		case KEY_RIGHT:
			ConfigIncBacklight();
			PgBacklightDisp();	// display progress bar
			break;

		// save
		case KEY_A:
			ConfigSave(); // save configuration
			return;

		// return
		case KEY_Y:
			ConfigLoad(); // restore settings
#if USE_ST7789		// use ST7789 TFT display (st7789.c, st7789.h)
			// display backlight control config update
			DispBacklightUpdate();
#endif
			return;
		}
	}
}

// display ADC
void PgADCDisp()
{
	int mv;

#define ADCBUFN	40
	char buf[ADCBUFN];

#define ADCY 44
#define ADCDY 25

	// GPIO26 voltage
	ADC_Mux(ADC_MUX_GPIO26);
	MemPrint(buf, ADCBUFN, "GPIO26: %'dmV  ", ADC_SingleUint());
	DrawTextBg2(buf, 0, ADCY, COL_YELLOW, COL_BLACK);

	// GPIO27 voltage
	ADC_Mux(ADC_MUX_GPIO27);
	MemPrint(buf, ADCBUFN, "GPIO27: %'dmV  ", ADC_SingleUint());
	DrawTextBg2(buf, 0, ADCY+1*ADCDY, COL_YELLOW, COL_BLACK);

	// GPIO28 voltage
	ADC_Mux(ADC_MUX_GPIO28);
	MemPrint(buf, ADCBUFN, "GPIO28: %'dmV  ", ADC_SingleUint());
	DrawTextBg2(buf, 0, ADCY+2*ADCDY, COL_YELLOW, COL_BLACK);

	// GPIO29 voltage
	ADC_Mux(ADC_MUX_GPIO29);
	MemPrint(buf, ADCBUFN, "GPIO29: %'dmV  ", ADC_SingleUint());
	DrawTextBg2(buf, 0, ADCY+3*ADCDY, COL_YELLOW, COL_BLACK);

	// Battery voltage (GPIO29)
	MemPrint(buf, ADCBUFN, "Battery: %'dmV  ", GetBatInt());
	DrawTextBg2(buf, 0, ADCY+4*ADCDY, COL_YELLOW, COL_BLACK);

	// ADC reference
	MemPrint(buf, ADCBUFN, "ADC ref. %'dmV  ", ConfigGetAdcRefInt());
	DrawTextBg2(buf, 0, ADCY+5*ADCDY+10, COL_GREEN, COL_BLACK);

	// display update
	DispUpdate();
}

// page ADC
void PgADC()
{
	int n = 0;

	// initialize pins
	ADC_PinInit(26);
	ADC_PinInit(27);
	ADC_PinInit(28);

	// draw title
	PgTitle();

	// help
	DrawText("LEFT/RIGHT=set, A=OK, Y=cancel", (WIDTH-30*8)/2, 224, COL_WHITE);

	// display ADC
	PgADCDisp();

	while (True)
	{
		// measure update
		n++;
		if (n > 100)
		{
			n = 0;

			// display voltage
			PgADCDisp();
		}
		WaitMs(10);

		switch (KeyGet())
		{
		// LEFT decrease
		case KEY_LEFT:
			ConfigDecAdcRef();
			PgADCDisp();
			break;

		// RIGHT decrease
		case KEY_RIGHT:
			ConfigIncAdcRef();
			PgADCDisp();
			break;

		// save
		case KEY_A:
			ConfigSave(); // save configuration
			ADC_PinTerm(26);
			ADC_PinTerm(27);
			ADC_PinTerm(28);
			return;

		// return
		case KEY_Y:
			ConfigLoad(); // restore settings
			ADC_PinTerm(26);
			ADC_PinTerm(27);
			ADC_PinTerm(28);
			return;
		}
	}
}

// display battery
void PgBatteryDisp()
{
#define BATBUFN	40
	char buf[BATBUFN];

	// Battery voltage
	MemPrint(buf, BATBUFN, "Battery voltage: %dmV  ", GetBatInt());
	DrawTextBg(buf, (WIDTH-24*8)/2, 36, COL_YELLOW, COL_BLACK);

	// full battery
#define BATY 64
#define BATDY 54
	DrawTextBg("LEFT/RIGHT:", 0, BATY, COL_WHITE, COL_BLACK);
	MemPrint(buf, BATBUFN, "Full at %'dmV  ", ConfigGetBatFullInt());
	DrawTextBg2(buf, 0, BATY+14, COL_GREEN, COL_BLACK);

	// empty battery
	DrawTextBg("UP/DOWN:", 0, BATY+1*BATDY, COL_WHITE, COL_BLACK);
	MemPrint(buf, BATBUFN, "Empty at %'dmV  ", ConfigGetBatEmptyInt());
	DrawTextBg2(buf, 0, BATY+1*BATDY+14, COL_GREEN, COL_BLACK);

	// diode drop
	DrawTextBg("X/B:", 0, BATY+2*BATDY, COL_WHITE, COL_BLACK);
	MemPrint(buf, BATBUFN, "Diode drop %'dmV  ", ConfigGetBatDiodeInt());
	DrawTextBg2(buf, 0, BATY+2*BATDY+14, COL_GREEN, COL_BLACK);

	// display update
	DispUpdate();
}

// page Battery
void PgBattery()
{
	int n = 0;

	// draw title
	PgTitle();

	// help
	DrawText("A=OK, Y=cancel", (WIDTH-14*8)/2, 224, COL_WHITE);

	// display battery
	PgBatteryDisp();

	while (True)
	{
		// battery update
		n++;
		if (n > 100)
		{
			n = 0;

			// display battery
			PgBatteryDisp();
		}
		WaitMs(10);

		switch (KeyGet())
		{
		// Left - full decrease
		case KEY_LEFT:
			ConfigDecBatFull();
			PgBatteryDisp();
			break;

		// Right - full increase
		case KEY_RIGHT:
			ConfigIncBatFull();
			PgBatteryDisp();
			break;

		// Up - empty increase
		case KEY_UP:
			ConfigIncBatEmpty();
			PgBatteryDisp();
			break;

		// Down - empty decrease
		case KEY_DOWN:
			ConfigDecBatEmpty();
			PgBatteryDisp();
			break;

		// X - diode increase
		case KEY_X:
			ConfigIncBatDiode();
			PgBatteryDisp();
			break;

		// B - diode decrease
		case KEY_B:
			ConfigDecBatDiode();
			PgBatteryDisp();
			break;

		// save
		case KEY_A:
			ConfigSave(); // save configuration
			return;

		// return
		case KEY_Y:
			ConfigLoad(); // restore settings
			return;
		}
	}
}

// display Temperature
void PgTempDisp()
{
#define TEMPBUFN 40
	char buf[TEMPBUFN];

	// Temperature
	MemPrint(buf, BATBUFN, "Temperature %.2f`C  ", ADC_Temp());
	DrawTextBg2(buf, 0, 55, COL_YELLOW, COL_BLACK);

	// Temperature base
#define TEMPY 100
#define TEMPDY 60
	DrawTextBg("LEFT/RIGHT (referenced to 27`C):", 0, TEMPY, COL_WHITE, COL_BLACK);
	MemPrint(buf, BATBUFN, "Base %.2fmV  ", ConfigGetTempBase()*1000);
	DrawTextBg2(buf, 0, TEMPY+14, COL_GREEN, COL_BLACK);

	// Temperature slope
	DrawTextBg("UP/DOWN:", 0, TEMPY+1*TEMPDY, COL_WHITE, COL_BLACK);
	MemPrint(buf, BATBUFN, "Slope %.3fmV/`C  ", ConfigGetTempSlope()*1000);
	DrawTextBg2(buf, 0, TEMPY+1*TEMPDY+14, COL_GREEN, COL_BLACK);

	// display update
	DispUpdate();
}

// page Temperature
void PgTemp()
{
	int n = 0;

	// draw title
	PgTitle();

	// help
	DrawText("A=OK, Y=cancel", (WIDTH-14*8)/2, 224, COL_WHITE);

	// display Temperature
	PgTempDisp();

	while (True)
	{
		// temperature update
		n++;
		if (n > 100)
		{
			n = 0;

			// display temperature
			PgTempDisp();
		}
		WaitMs(10);

		switch (KeyGet())
		{
		// Left - base decrease
		case KEY_LEFT:
			ConfigDecTempBase();
			PgTempDisp();
			break;

		// Right - base increase
		case KEY_RIGHT:
			ConfigIncTempBase();
			PgTempDisp();
			break;

		// Up - slope increase
		case KEY_UP:
			ConfigIncTempSlope();
			PgTempDisp();
			break;

		// Down - slope decrease
		case KEY_DOWN:
			ConfigDecTempSlope();
			PgTempDisp();
			break;

		// save
		case KEY_A:
			ConfigSave(); // save configuration
			return;

		// return
		case KEY_Y:
			ConfigLoad(); // restore settings
			return;
		}
	}
}

// display Crystal
void PgCrystalDisp()
{
#define QBUFN 40
	char buf[QBUFN];

	// Crystal
#define QY 45
#define QDY 50
	DrawTextBg("LEFT/RIGHT for manual setup:", 0, QY, COL_WHITE, COL_BLACK);
	MemPrint(buf, QBUFN, "Crystal %'dHz ", ConfigGetCrystal());
	DrawTextBg2(buf, 0, QY+12, COL_GREEN, COL_BLACK);

	// help
#define QY2 98
#define QDY2 14
	DrawTextBg("To calibrate crystal:", 0, QY2+0*QDY2-4, COL_WHITE, COL_BLACK);
	DrawTextBg("1) Wait for seconds of current time=00", 0, QY2+1*QDY2, COL_WHITE, COL_BLACK);
	DrawTextBg("2) Press B to start calibration", 0, QY2+2*QDY2, COL_WHITE, COL_BLACK);
	DrawTextBg("3) Wait a few hours for the displayed", 0, QY2+3*QDY2, COL_WHITE, COL_BLACK);
	DrawTextBg("   seconds to differ by a few seconds", 0, QY2+4*QDY2, COL_WHITE, COL_BLACK);
	DrawTextBg("   from the current time (max. 28 sec)", 0, QY2+5*QDY2, COL_WHITE, COL_BLACK);
	DrawTextBg("4) Press B again when the seconds of", 0, QY2+6*QDY2, COL_WHITE, COL_BLACK);
	DrawTextBg("   the current time will become :00", 0, QY2+7*QDY2, COL_WHITE, COL_BLACK);

	// help
	DrawText("A=OK, B=calibration, Y=cancel", (WIDTH-29*8)/2, 224, COL_WHITE);

	// display update
	DispUpdate();
}

// calculate frequency from indicated time
// Datasheet: 12MHz crystal with 30ppm tolerance (= 11999640 to 12000360, variation +- 360 Hz)
u32 GetCalib(u64 it)
{
	// milliseconds are enough (to avoid calculation overflow)
	it /= 1000;

	// get microseconds modulo minute
	u32 ms = it % 60000;

	// get real time
	u64 rt;
	if (ms >= 30000) // >= 30 sec, indicated time is later
		rt = it + (60000 - ms);
	else // < 30 sec, indicated time is earlier
		rt = it - ms;
	
	// get real frequency
	if (rt == 0) return 0;
	u32 f = (u32)(12000000 * it / rt);
	if ((f < CONFIG_CRYSTAL_MIN) || (f > CONFIG_CRYSTAL_MAX)) f = 0;
	return f;
}

// calibration
void Calibration()
{
	// get start time
	u64 start = Time64();
	u64 dt;
	int hour, min, sec, us;
	u32 f;
	Bool dispoff = False;
	char ch;

#define TBUFN 40
	char buf[TBUFN];

CALIB_REDRAW:

	// draw title
	PgTitle();
	DrawTextBg2("calibration", (WIDTH-11*16)/2, 40, COL_WHITE, COL_BLACK);

	// help
	DrawText("B=stop on :00, X=display OFF, Y=cancel", 8, 224, COL_WHITE);

	// display update
	DispUpdate();

#if USE_ST7789		// use ST7789 TFT display (st7789.c, st7789.h)
			// display backlight control config update
	DispBacklightUpdate();
#endif

	while (True)
	{
		// load time
		dt = Time64() - start;
		sec = (int)(dt/1000000);
		us = (int)(dt - sec*1000000);
		min = sec/60;
		sec -= min*60;
		hour = min/60;
		min -= hour*60;

		// LED
		if (us < 100000) LedOn(LED1); else LedOff(LED1);

		if (!dispoff)
		{
			// display time
			MemPrint(buf, TBUFN, "%02d:%02d:%02d ", hour, min, sec);
			DrawTextBg2(buf, (WIDTH-8*16)/2, 90, COL_YELLOW, COL_BLACK);

			// calculate frequency
			DrawText("If you press B at this moment,", (WIDTH-30*8)/2, 135, COL_WHITE);
			DrawText("crystal frequency would be:", (WIDTH-26*8)/2, 135+16, COL_WHITE);
			f = GetCalib(dt);
			if (f != 0)
				MemPrint(buf, TBUFN, "%'uHz", f);
			else
				memcpy(buf, "----------Hz", 13);
			DrawTextBg2(buf, (WIDTH-12*16)/2, 135+32, COL_AZURE, COL_BLACK);

			DispUpdate();
		}

		ch = KeyGet();

		if ((ch != NOKEY) && dispoff)
		{
			dispoff = False;
			goto CALIB_REDRAW;
		}

		switch (ch)
		{
		// display OFF
		case KEY_X:
			DrawClear();
			DispUpdate();
#if USE_ST7789		// use ST7789 TFT display (st7789.c, st7789.h)
			// display backlight control config update
			DispBacklight(0);
#endif
			dispoff = True;
			break;

		// calibration
		case KEY_B:
			f = GetCalib(dt);
			if (f != 0)
			{
				ConfigSetCrystal(f);
				LedOff(LED1);
				return;
			}
			break;

		// return
		case KEY_Y:
			LedOff(LED1);
			return;
		}
	}
}

// page Crystal
void PgCrystal()
{
	// draw title
	PgTitle();

	// display Crystal
	PgCrystalDisp();

	while (True)
	{
		switch (KeyGet())
		{
		// left
		case KEY_LEFT:
			ConfigDecCrystal();
			PgCrystalDisp();
			break;

		// right
		case KEY_RIGHT:
			ConfigIncCrystal();
			PgCrystalDisp();
			break;

		// save
		case KEY_A:
			ConfigSave(); // save configuration
			return;

		// calibration
		case KEY_B:
			Calibration();
			PgTitle();
			PgCrystalDisp();
			break;

		// return
		case KEY_Y:
			ConfigLoad(); // restore settings
			return;
		}
	}
}

// list of functions
void (*PgFunc[MENU_NUM])() = {
	PgVolume,
	PgBacklight,
	PgADC,
	PgBattery,
	PgTemp,
	PgCrystal,
};

// display menu
void DispMenu()
{
	int i;
	COLTYPE fg, bg;	

	// draw icons
	DrawImg(IconImg, 0, 0, 8, 0, 32, 240, 32);

	// display entries
	for (i = 0; i < MENU_NUM; i++)
	{
		// prepare color
		fg = COL_WHITE;
		bg = COL_BLACK;
		if (i == MenuSel)
		{
			fg = COL_BLACK;
			bg = COL_WHITE;
		}

		// draw row
		DrawTextBg2(MenuTxt[i], 48, i*40+4, fg, bg);
	}

	// display update
	DispUpdate();
}

// Config RAW
void CfgRaw()
{
CFG_RESET:
	// load config
	int n = ConfigLoad();

#if USE_ST7789		// use ST7789 TFT display (st7789.c, st7789.h)
	// display backlight control config update
	DispBacklightUpdate();
#endif

	// print
	Print("\f\6\27     Config RAW\3\n\n");
	Print("\22Config.crc:\26\5 0x%04X\3\n", ConfigGetCRC(&Config));
	Print("\22Config.volume: \26\5%d%%\3\n", Config.volume);
	Print("\22Config.backlight: \26\5%d\3\n", Config.backlight);
	Print("\22Config.bat_full: \26\5%'dmV\3\n", Config.bat_full);
	Print("\22Config.bat_empty: \26\5%'dmV\3\n", Config.bat_empty);
	Print("\22Config.bat_diode: \26\5%'dmV\3\n", Config.bat_diode);
	Print("\22Config.adc_ref: \26\5%'dmV\3\n", Config.adc_ref);
	Print("\22Config.temp_base: \26\5%.2fmV\3\n", Config.temp_base*1000);
	Print("\22Config.temp_slope: \26\5%.3fmV\3\n", Config.temp_slope*1000);
	Print("\22Config.crystal: \26\5%dHz\3\n", Config.crystal);

	Print("\22Config entries: \26\5%d/%d\3\n", n+1, CONFIG_NUM);

	Print("\n\27      B=Reset to default, Y=Return");

	while (True)
	{
		switch (KeyGet())
		{
		// reset memory
		case KEY_B:
			ConfigClear();
			goto CFG_RESET;

		// return
		case KEY_Y: return;
		}
	}
}

int main()
{
	// display menu
	DispMenu();

	while (True)
	{
		switch (KeyGet())
		{
		// key down, next menu entry
		case KEY_DOWN:
			MenuSel++;
			if (MenuSel >= MENU_NUM) MenuSel = 0;
			DispMenu();
			break;

		// key up, previous menu entry
		case KEY_UP:
			MenuSel--;
			if (MenuSel < 0) MenuSel = MENU_NUM - 1;
			DispMenu();
			break;

		// select page
		case KEY_A:
			PgFunc[MenuSel]();
			DrawClear();
			DispMenu();
			break;

		// config RAW
		case KEY_X:
			CfgRaw();
			DrawClear();
			DispMenu();
			break;

		// reset to boot loader
		case KEY_Y:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
