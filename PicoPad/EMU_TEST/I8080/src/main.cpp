
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

// used PWMs:
//   LCD backlight GPIO16 ... PWM 0
// * unused PWM 1: GPIO2 (btn RIGHT), GPIO3 (btn LEFT), GPIO18 (LCD SCK), GPIO19 (LCD MOSI)
// * unused PWM 2: GPIO4 (btn UP), GPIO5 (btn DOWN), GPIO20 (LCD RES), GPIO21 (LCD CS)
//   user LED GPIO22 ... PWM 3
//   board LED GPIO25 .. PWM 4
//   ext GPIO26, GPIO27 ... PWM 5
//   ext GPIO28 ... PWM 6
//   sound (GPIO15) ... PWM 7

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define EMU_PWM		1	// index of PWM used to synchronize emulations
#else
#define EMU_PWM		2	// index of PWM used to synchronize emulations
#endif

#define EMU_FREQ	2000000	// emulation frequency
#define EMU_CLKSYS_MIN	120	// minimal system clock in MHz
#define EMU_CLKSYS_MAX	180	// maximal system clock in MHz

// I8080 ports: 0x00..0xFF

// Emulator ports:
//
//   OUT 15: output to hardware USR LED on PicoPad panel
//		bit 0 ... set LED output (1=ON, 0=OFF), only if bit 1 is 0
//		bit 1 ... 1=flip LED output, in this case bit 0 is ignored
//
//   OUT 14: output to LEDs, bit 0 to 7 = LED 0 to 7, 1=LED is ON
//
//   OUT 13: write digit to output shift register of the 7-segment display (10-digit positions)
//		values 0 to DIG_MAX: digit 0 to 9, A to Z, space, segments, ...
//
//   OUT 12: any write - copy digits from output shift register to 7-segment display
//
//   OUT 11: position of decimal point 0..9, set other value to hide decimal point
//
//   OUT 10: write ASCII character to output text buffer (20 characters of scrolling title from right to left)
//
//   OUT 9: output to speaker
//		bit 0 ... output to speaker (if bit 1 is 0)
//		bit 1 ... flip output to speaker (bit 0 is ignored)
//
//   IN 7: input keys, 1=key is pressed
//		bit 0 ... DOWN
//		bit 1 ... RIGHT
//		bit 2 ... LEFT
//		bit 3 ... UP
//		bit 4 ... A
//		bit 5 ... B
//		bit 6 ... X
//		bit 7 ... Y

#include "include.h"

// ============================================================================
//                              Data and tables
// ============================================================================

// CPU
sI8080	i8080cpu;
u8 Memory[I8080_MEMSIZE]; // memory 64 KB

// path with samples
char SampPath[APPPATH_PATHMAX+10+1];

// LED blinking period
#define CNTTICK		2
u32 PrevTick = 0;
volatile u32 CheckTick = 0;
int CntTick = 0;

// title (scrolling from right to left)
#define TITLE_NUM	20	// title length
volatile char ALIGNED TitleBuf[(TITLE_NUM+3) & ~3]; // title buffer ... aligned to 5 u32 words

// 7-segment display
u8 ALIGNED DigBufShift[(DIG_NUM+3) & ~3]; // shift register (the display is filled from right to left)
volatile u8 ALIGNED DigBuf[(DIG_NUM+3) & ~3];	// digital buffer (values 0 to 15 = digits 0 to F) ... aligned to 3 u32 words
volatile u8 DigDP;			// decimal point position (0=right position, set out of range to hide it)

// LEDs
volatile u8 LEDs;			// LEDs state (1=LED is ON)

// Keys
volatile u8 Keys;			// Key state (1=key is pressed)

#define BUFN	40
char Buf[BUFN+1];

#if EMU_DEBUG_SYNC
u32 DebCheckSync;
#endif

// ============================================================================
//                 Callbacks from CPU emulator from core 1
// ============================================================================

// read memory
u8 FASTCODE NOFLASH(EmuGetMem)(u16 addr)
{
	return Memory[addr];
}

// write memory
void FASTCODE NOFLASH(EmuSetMem)(u16 addr, u8 data)
{
	Memory[addr] = data;
}

// read port
u8 FASTCODE NOFLASH(EmuGetPort)(u8 addr)
{
	// get keys
	if (addr == 7) return Keys;
	return 0xff;
}

// write port
void FASTCODE NOFLASH(EmuSetPort)(u8 addr, u8 data)
{
	// blinking LED
	if (addr == 15)
	{
		if ((data & B1) != 0)
		{
			if (GPIO_GetOut(LED1_PIN))
			{
				int k = CntTick + 1;
				if (k >= CNTTICK)
				{
					k = 0;
					u32 t = Time();
					CheckTick = t - PrevTick;
					PrevTick = t;
				}
				CntTick = k;
			}
			GPIO_Flip(LED1_PIN);
		}
		else
			if ((data & B0) != 0)
				GPIO_Out1(LED1_PIN);
			else
				GPIO_Out0(LED1_PIN);
	}

	// output to LEDs
	else if (addr == 14)
	{
		LEDs = data;
	}

	// output digit to the shift register
	else if (addr == 13)
	{
		u32 k, k2;
		u32* d = (u32*)DigBufShift;
		k = d[0];
		d[0] = (k << 8) | data;
		k2 = d[1];
		d[1] = (k2 << 8) | (k >> 24);
		k = d[2];
		d[2] = (k << 8) | (k2 >> 24);
	}

	// copy digits to the display
	else if (addr == 12)
	{
		u32* s = (u32*)DigBufShift;
		u32* d = (u32*)DigBuf;
		d[0] = s[0];
		d[1] = s[1];
		d[2] = s[2];
	}

	// set position of decimal point
	else if (addr == 11)
	{
		DigDP = data;
	}

	// output ASCII character to the title
	else if (addr == 10)
	{
		u32 k, k2;
		u32* d = (u32*)TitleBuf;
		k = d[4];
		d[4] = (k >> 8) | ((u32)data << 24) ;
		k2 = d[3];
		d[3] = (k2 >> 8) | (k << 24);
		k = d[2];
		d[2] = (k >> 8) | (k2 << 24);
		k2 = d[1];
		d[1] = (k2 >> 8) | (k << 24);
		k = d[0];
		d[0] = (k >> 8) | (k2 << 24);
	}

	// output to the speaker
	else if (addr == 9)
	{
#ifdef PWMSND_GPIO
		if ((data & B1) != 0)
			GPIO_Flip(PWMSND_GPIO);
		else
			if ((data & B0) != 0)
				GPIO_Out1(PWMSND_GPIO);
			else
				GPIO_Out0(PWMSND_GPIO);
#endif
	}
}

// ============================================================================
//                      Control interface on core 0
// ============================================================================

// redraw all
void DispAll()
{
	int i;
	u8 ch;

	// draw title
	for (i = 0; i < TITLE_NUM; i++)
		DrawCharBg2(TitleBuf[i], i*FONTW*2, 0, COL_BLACK, COL_GRAY);

	// draw digital display
	for (i = 0; i < DIG_NUM; i++)
	{
		ch = DigBuf[i];
		if (ch > DIG_MAX) ch = DIG_SPC;
		DrawImg4Pal(DigitsImg, DigitsImg_Pal, ch*DIG_W,
			(DigDP == i) ? DIG_H : 0, (DIG_NUM - 1 - i)*DIG_W, DIG_Y, DIG_W, DIG_H, DIG_WALL);
	}

	// draw LEDs
	u8 led = LEDs;
	for (i = LEDS_NUM-1; i >= 0; i--)
	{
		DrawImg(LedImg, i * LEDS_W, ((led & B0) == 0) ? 0 : LEDS_H, i * LEDS_DX + LEDS_X,
			LEDS_Y, LEDS_W, LEDS_H, LEDS_WALL);
		led >>= 1;
	}

#define BLINK_Y		168
#define BLINK_X0	40

	// blink time
	DrawRect(BLINK_X0+17*8, BLINK_Y, 20*FONTW, FONTH, COL_BLACK);
	MemPrint(Buf, BUFN, "%'d us", (CheckTick + CNTTICK/2)/CNTTICK);
	DrawText(Buf, BLINK_X0+17*8, BLINK_Y, COL_GREEN);	

#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization
	// CPU
	u32 t = Time();
	if ((t - DebCheckSync) >= 1000000)
	{
#define DEBSYN_Y	194
		DebCheckSync = t;
		sEmuDebClock* ec = &EmuDebClock;
		DrawRect(0, DEBSYN_Y, WIDTH, FONTH, COL_BLACK);
		int used = (int)((u64)ec->used*100/ec->total);
		if (used > 999) used = 999;
		int mx = ec->maxused*100/ec->maxtot;
		if (mx < used) mx = used;
		if (mx > 999) mx = 999;
		i = MemPrint(Buf, BUFN, "PC=%04X CPU used=%d%% CPU max=%d%%", i8080cpu.pc, used, mx);
		DrawText(Buf, (WIDTH-i*FONTW)/2, DEBSYN_Y, COL_GRAY);
		dsb();
		ec->reset = True;
	}
#endif

	// display update
	DispUpdate();
}

// draw base
void DispBase(u32 freq, u32 sysclk)
{
	int i;

	// clear screen
	DrawClear();

	// draw LEDs number
	for (i = LEDS_NUM-1; i >= 0; i--)
		DrawChar('7' - i, i * LEDS_DX + LEDS_X + 8, LEDS_Y + LEDS_H + 8, COL_WHITE);

	// blinking period
	DrawText("Blinking period:", BLINK_X0, BLINK_Y, COL_DKGREEN);

	// frequency
	i = MemPrint(Buf, BUFN, "f=%d sysclk=%d div=%#f", freq, sysclk, (double)sysclk/(freq*I8080_CLOCKMUL));
	DrawText(Buf, (WIDTH-i*FONTW)/2, 208, COL_GRAY);

	// draw help
	DrawText("Press UP and Y to exit emulation", (WIDTH - 32*8)/2, HEIGHT-FONTH, COL_WHITE);

	// display update
	DispUpdate();
}

// update keys
void UpdateKeys()
{
	u8 key = 0;
	if (KeyPressed(KEY_DOWN) || UsbKeyIsPressed(HID_KEY_ARROW_DOWN)) key |= B0;
	if (KeyPressed(KEY_RIGHT) || UsbKeyIsPressed(HID_KEY_ARROW_RIGHT)) key |= B1;
	if (KeyPressed(KEY_LEFT) || UsbKeyIsPressed(HID_KEY_ARROW_LEFT)) key |= B2;
	if (KeyPressed(KEY_UP) || UsbKeyIsPressed(HID_KEY_ARROW_UP)) key |= B3;
	if (KeyPressed(KEY_A) || UsbKeyIsPressed(HID_KEY_A)) key |= B4;
	if (KeyPressed(KEY_B) || UsbKeyIsPressed(HID_KEY_B)) key |= B5;
	if (KeyPressed(KEY_X) || UsbKeyIsPressed(HID_KEY_X)) key |= B6;
	if (KeyPressed(KEY_Y) || UsbKeyIsPressed(HID_KEY_ESCAPE) || UsbKeyIsPressed(HID_KEY_Y)) key |= B7;
	Keys = key;
}

// initialize device
void EmuDevInit()
{
	int i;

	// initialize I8080 table
	I8080_InitTab();

	// setup callback functions
	i8080cpu.readmem = EmuGetMem;
	i8080cpu.writemem = EmuSetMem;
	i8080cpu.readport = EmuGetPort;
	i8080cpu.writeport = EmuSetPort;

	// clear title
	for (i = 0; i < TITLE_NUM; i++) TitleBuf[i] = ' ';

	// clear digit display
	for (i = 0; i < DIG_NUM; i++) DigBufShift[i] = DIG_SPC;
	for (i = 0; i < DIG_NUM; i++) DigBuf[i] = DIG_SPC;
	DigDP = 255;

	// clear LEDs
	LEDs = 0;

	// clear keys
	Keys = 0;

	// LED blinking period
	CheckTick = 0;
}

// ============================================================================
//                             Main function
// ============================================================================

// main function
int main()
{
	u32 k;
	u8 ch;
	int i;
	Bool stop;

#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	DispHstxAllTerm();
	DispHstxStart(0);
#endif

#if USE_PICOPADHSTX && USE_DISPHSTX	// use PicoPadHSTX device configuration
	u32 sysclk = ClockGetHz(CLK_SYS);
#else
	// Find system clock in Hz that sets the most accurate PWM clock frequency.
	u32 sysclk = PWM_FindSysClk(EMU_CLKSYS_MIN*MHZ, EMU_CLKSYS_MAX*MHZ, EMU_FREQ*I8080_CLOCKMUL);

	// setup system clock to get precise frequency of emulation
	if (sysclk > 160*MHZ) VregSetVoltage(VREG_VOLTAGE_1_20);
	ClockPllSysFreq((sysclk+500)/1000);
#endif

	// initialize USB
	UsbHostInit();

	// get path with samples
	k = GetHomePath(SampPath, "/EMU_TEST");
	if (SampPath[k-1] != PATHCHAR)
	{
		SampPath[k] = PATHCHAR;
		k++;
	}
	memcpy(SampPath+k, "_I8080", 7);

	// initialize output to speaker
#ifdef PWMSND_GPIO 	// PWM output GPIO pin (0..29)
	GPIO_Init(PWMSND_GPIO);
	GPIO_DirOut(PWMSND_GPIO);
	GPIO_Out0(PWMSND_GPIO);
#endif

#if EMU_DEBUG_SYNC
	// debug measure time synchronization
	DebCheckSync = Time();
#endif

	// initialize file selection
	FileSelInit(SampPath, "I8080", "BIN", &FileSelColGreen);

	while (True)
	{
	// ---- select and load sample program

		// select file (or return from program)
		if (!FileSel()) ResetToBootLoader();

		// open and load file
		if (!FileOpen(&FileSelPrevFile, FileSelTempBuf))
			FileSelDispBigErr("Cannot open file");
		else
		{
			// read program
			u32 s = FileSelLastNameSize;
			if (s > I8080_MEMSIZE) s = I8080_MEMSIZE;
			FileRead(&FileSelPrevFile, Memory, s);
			FileClose(&FileSelPrevFile);

			// initialize device
			EmuDevInit();

			// run program
			u32 freq = I8080_Start(&i8080cpu, EMU_PWM, EMU_FREQ);

			// draw base
			DispBase(freq, sysclk);
			PrevTick = Time();

		// ---- emulation service

			// wait for break
			stop = False;
			while (!stop)
			{
				// redraw all
				DispAll();

				// update keys
				UpdateKeys();

				// get USB key
				k = UsbGetKey();
				ch = (u8)(k & 0xff);

				// quit
				switch (ch)
				{
				// Esc: quit
				case HID_KEY_ARROW_UP:
				case HID_KEY_ESCAPE:
				case HID_KEY_Y:
					if (UsbKeyIsPressed(HID_KEY_ARROW_UP) && (UsbKeyIsPressed(HID_KEY_ESCAPE) || UsbKeyIsPressed(HID_KEY_Y)))
					{
						I8080_Stop(EMU_PWM);
						stop = True;
					}
					break;
				}

				// keypad
				switch (KeyGet())
				{
#if USE_SCREENSHOT		// use screen shots
				case KEY_X:
					ScreenShot();
					break;
#endif

				case KEY_UP:
				case KEY_Y:
					if (KeyPressed(KEY_UP) && KeyPressed(KEY_Y))
					{
						I8080_Stop(EMU_PWM);
						stop = True;
					}
					break;
				}
			}

			// wait for no key pressed
			DrawClear();
			DispUpdate();
			while (UsbKeyIsPressed(HID_KEY_ESCAPE) || UsbKeyIsPressed(HID_KEY_Y) || KeyPressed(KEY_Y)) {}
		}
	}
}
