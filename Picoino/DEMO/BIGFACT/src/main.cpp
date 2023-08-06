
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Big factorial

#include "../include.h"

#define NUMBER	123456789	// input number to calculate
#define DIGITS	1150		// number of digits

// text screen
char TextBuf[DIGITS+2];

// sample to check result
const char Sample[DIGITS+2] = // 1 row = 100 characters, total 11+1/2(+1) rows
	"2.85351252191278598114390962896262500055094720259934076823834073667408136827054726611211860620557495" // 1 99 digits
	"0762446265743718176436357779674771148828849269333524648544470467821733633907957881364053751838400018" // 2
	"5997722594195180425406109899595302393673823838992414333735355859620663712534192859905252482185802070" // 3
	"2497335811806667554713059287811897140277617486680519220915356055553729100268178019142035618599832460" // 4
	"0964613551928663071104536919522791951756054640661106556707745552449230140500987241856457842860084251" // 5
	"1971716626198837639032024024590622808584152902076200755449436936107498482286935537845689982137177336" // 6
	"2267189103725107795199714469480100613489462233919493471052108719005796633323500614360339726146093508" // 7
	"9638586777723551534220548319186397628956528860803844394860384670739079524350993762473100191366403536" // 8
	"2229107231783569087661972521979151644438626738479000569308778053474596083816858632498483794863856161" // 9
	"8622478621528702656016312733075465971735012612084594657766504158948486720346556124513720727592283383" // 10
	"2216578711360016202243930473895439503722860993267668984226584476020214655770260180593586317237519256" // 11
	"744799451120205598169248189569396747518856038267017"; // 51 digits

const char OkTxt[]   = "   Result OK";
const char ErrTxt[]  = "   > ERROR <";

real4096 Num;

int main()
{
	u8 ch;

	// prompt
	DrawPrint("Calculating...");

	// enter input number to calculate
	Real4096SetUInt(&Num, NUMBER);

	// calculate factorial Log10
	Real4096FactLog(&Num);

	// get exponent
	int exp = Real4096GetUInt(&Num);

	// remove integer part
	Real4096TruncFrac(&Num);

	// convert to decimal number
	Real4096Exp10(&Num);

	// print mantissa to text buffer
	Real4096ToText(&Num, TextBuf, DIGITS+1);

	// display result
	DrawPrint("\f\27%d! = %se+%d", NUMBER, TextBuf, exp);

	// check result
	DrawPrint((memcmp(TextBuf, Sample, DIGITS+2) == 0) ? OkTxt : ErrTxt);

	// main loop
	while (True)
	{
		// blinking LED
		LedOn(LED1);
		WaitMs(250);
		ch = KeyGet();
		if (ch == KEY_Y) ResetToBootLoader();
#if USE_SCREENSHOT		// use screen shots
		if (ch == KEY_X) ScreenShot();
#endif

		LedOff(LED1);
		WaitMs(250);
		ch = KeyGet();
		if (ch == KEY_Y) ResetToBootLoader();
#if USE_SCREENSHOT		// use screen shots
		if (ch == KEY_X) ScreenShot();
#endif
	}
}
