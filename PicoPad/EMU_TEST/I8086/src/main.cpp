
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

// CPU frequency = NTSC subcarrier frequency * 4 / 3 = 3.579545 MHz * 4 / 3 = 4.772727 MHz
// Real emulated frequency = 252/52.8125 = 4.7715976 MHz

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define EMU_PWM		1	// index of PWM used to synchronize emulations
#else
#define EMU_PWM		2	// index of PWM used to synchronize emulations
#endif

#define EMU_FREQ	4771598	// real emulation frequency (nominal frequency is 4770000 or mode precise 4772727), used divisor 52.8125
#define EMU_CLKSYS_MIN	252	// minimal system clock in MHz (frequency 252 MHz is required by HDMI display output)
#define EMU_CLKSYS_MAX	252	// maximal system clock in MHz

// Memory (size min. 0x11000 = 69632 bytes = 68 KB):
// - 1 KB vector table (0x0000 - 0x03FF)
// - 512 bytes BIOS data (0x0400 - 0x05FF), with emulator INT traps at its end
// - start of user program at 0x0600
// 	256 bytes PSP Program Segment Prefix of user COM program (0x0060:0x0000 - 0x0060:0x00FF)
//	start of user COM program 0x0060:0x0100 (CS=DS=SS=ES=0x0060, IP=0x0100, SP=0xFFFE), size of COM max. 0xFE00 = 65024 bytes)

// Emulator ports:
//
//   OUT 0xEF: output to hardware USR LED on PicoPad panel
//		bit 0 ... set LED output (1=ON, 0=OFF), only if bit 1 is 0
//		bit 1 ... 1=flip LED output, in this case bit 0 is ignored
//
//   OUT 0xEE: output to LEDs, bit 0 to 7 = LED 0 to 7, 1=LED is ON
//
//   OUT 0xED: write digit to output shift register of the 7-segment display (10-digit positions)
//		values 0 to DIG_MAX: digit 0 to 9, A to Z, space, segments, ...
//
//   OUT 0xEC: any write - copy digits from output shift register to 7-segment display
//
//   OUT 0xEB: position of decimal point 0..9, set other value to hide decimal point
//
//   OUT 0xEA: write ASCII character to output text buffer (20 characters of scrolling title from right to left)
//
//   OUT 0xE9: output to speaker
//		bit 0 ... output to speaker (if bit 1 is 0)
//		bit 1 ... flip output to speaker (bit 0 is ignored)
//
//   IN 0xE7: input keys, 1=key is pressed
//		bit 0 ... DOWN
//		bit 1 ... RIGHT
//		bit 2 ... LEFT
//		bit 3 ... UP
//		bit 4 ... A
//		bit 5 ... B
//		bit 6 ... X
//		bit 7 ... Y

#include "include.h"

#define I8086_MEMSIZE	0x11000		// 68 KB
#define I8086_LOADADDR	0x00700		// program COM load address

// ============================================================================
//                              Data and tables
// ============================================================================

// CPU
sI8086	i8086cpu;
u8 Memory[I8086_MEMSIZE]; // memory 64 KB (first 1 KB is vector table)

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
u8 FASTCODE NOFLASH(EmuGetMem)(u32 addr)
{
	if (addr < I8086_MEMSIZE) return Memory[addr];
	return 0xff;
}

// write memory
void FASTCODE NOFLASH(EmuSetMem)(u32 addr, u8 data)
{
	if (addr < I8086_MEMSIZE) Memory[addr] = data;
}

// read port
u8 FASTCODE NOFLASH(EmuGetPort)(u16 addr)
{
	// get keys
	if (addr == 0xE7) return Keys;
	return 0xff;
}

u16 FASTCODE NOFLASH(EmuGetPort16)(u16 addr)
{
	// get keys
	if (addr == 0xE7) return Keys;
	return 0xffff;
}

// write port
void FASTCODE NOFLASH(EmuSetPort)(u16 addr, u8 data)
{
	// blinking LED
	if (addr == 0xEF)
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
	else if (addr == 0xEE)
	{
		LEDs = data;
	}

	// output digit to the shift register
	else if (addr == 0xED)
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
	else if (addr == 0xEC)
	{
		u32* s = (u32*)DigBufShift;
		u32* d = (u32*)DigBuf;
		d[0] = s[0];
		d[1] = s[1];
		d[2] = s[2];
	}

	// set position of decimal point
	else if (addr == 0xEB)
	{
		DigDP = data;
	}

	// output ASCII character to the title
	else if (addr == 0xEA)
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
	else if (addr == 0xE9)
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

// write port
void FASTCODE NOFLASH(EmuSetPort16)(u16 addr, u16 data)
{
	EmuSetPort(addr, (u8)data);
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
		i = MemPrint(Buf, BUFN, "IP=%04X CPU used=%d%% CPU max=%d%%", i8086cpu.ip, used, mx);
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
	i = MemPrint(Buf, BUFN, "f=%d sysclk=%d div=%#.4f", freq, sysclk, (double)sysclk/(freq*I8086_CLOCKMUL));
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

	// initialize I8086 table
	I8086_InitTab();

	// setup callback functions
	i8086cpu.readmem = EmuGetMem;
	i8086cpu.writemem = EmuSetMem;
	i8086cpu.readport8 = EmuGetPort;
	i8086cpu.writeport8 = EmuSetPort;
	i8086cpu.readport16 = EmuGetPort16;
	i8086cpu.writeport16 = EmuSetPort16;

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
	u32 sysclk = PWM_FindSysClk(EMU_CLKSYS_MIN*MHZ, EMU_CLKSYS_MAX*MHZ, EMU_FREQ*I8086_CLOCKMUL);

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
	memcpy(SampPath+k, "_I8086", 7);

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
	FileSelInit(SampPath, "I8086", "BIN", &FileSelColGreen);

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
			if (s > (I8086_MEMSIZE-I8086_LOADADDR)) s = I8086_MEMSIZE-I8086_LOADADDR;
			FileRead(&FileSelPrevFile, &Memory[I8086_LOADADDR], s);
			FileClose(&FileSelPrevFile);

			// initialize device
			EmuDevInit();

			// run user program from address 0x0060:0x0100
			I8086_Reset(&i8086cpu);
			i8086cpu.cs = 0x0060;
			i8086cpu.ds = 0x0060;
			i8086cpu.ss = 0x0060;
			i8086cpu.es = 0x0060;
			i8086cpu.ip = 0x0100;
			i8086cpu.sp = 0xfffe;
			i8086cpu.flags |= I8086_IF_BIT; // enable interrupts
			u32 freq = I8086_Cont(&i8086cpu, EMU_PWM, EMU_FREQ);

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
						I8086_Stop(EMU_PWM);
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
						I8086_Stop(EMU_PWM);
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
