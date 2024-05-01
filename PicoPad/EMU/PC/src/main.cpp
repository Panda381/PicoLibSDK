
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

// CPU frequency = NTSC subcarrier frequency * 4 / 3 = 3.579545 MHz * 4 / 3 = 4.772727 MHz
// Real emulated frequency = 252/52.8125 = 4.7715976 MHz

#define EMU_PWM		2	// index of PWM used to synchronize emulations

//#define EMU_FREQ	4771598	// PC-XT: real emulation frequency (nominal frequency is 4770000 or more precise 4772727), used divisor 52.8125
//#define EMU_FREQ	12000000 // PC-AT: real emulation frequency
//#define EMU_FREQ	25200000 // Turbo: max. emulation frequency, many functions do not run in real time

//#define EMU_CLKSYS_MIN	252	// minimal system clock in MHz (frequency 252 MHz is required by HDMI display output)
//#define EMU_CLKSYS_MAX	252	// maximal system clock in MHz

#include "../include.h"

/* Configuration file *.CFG:
; comment
name=value ; configuration parameter (DEC number or HEX number with 0x or $ prefix); do not use spaces around "="

Freq ... emulated frequency in [kHz] (4772 = XT, default 12000 = AT, 25200 = max)
MinClk ... minimal system clock in [kHz] (default 160000)
MaxClk ... maximal system clock in [kHz] (default 220000)

; PC key scan codes - see #define on end of file sdk_usb_hid.h, add 0x80 for keys with prefix 0xE0
KeyUp ... scan key for UP keypad (default 0xC8 = B7 + PC_KEYSCAN_UP)
KeyLeft ... scan key for LEFT keypad (default 0xCB = B7 + PC_KEYSCAN_LEFT)
KeyRight ... scan key for RIGHT keypad (default 0xCD = B7 + PC_KEYSCAN_RIGHT)
KeyDown ... scan key for DOWN keypad (default 0xD0 = B7 + PC_KEYSCAN_DOWN)
KeyX ... scan key for X keypad (default 0x46 = PC_KEYSCAN_SCROLL = zoom function)
KeyY ... scan key for Y keypad (default 0xC5 = B7 + PC_KEYSCAN_PAUSE = menu function)
KeyA ... scan key for A keypad (default 0x1C = PC_KEYSCAN_ENTER)
KeyB ... scan key for B keypad (default 0x01 = PC_KEYSCAN_ESC)

Card ... video card (0=MDA, 1=CGA, 2=PCjr, 3=Tandy, 4=EGA64, 5=EGA128, 6=MCGA, 7=VGA, default 7 VGA)

VideoMode ... initial videomode 0..19, default 1

VSyncWait ... vertical sync wait in [ms], default 2

*/

// ============================================================================
//                              Data and tables
// ============================================================================

ALIGNED FRAMETYPE FrameBuf[PC_RAM_SIZE/sizeof(FRAMETYPE)];

// CPU
sI8086	i8086cpu;

// path with samples
char SampPath[APPPATH_PATHMAX+10+1];

#define BUFN	40
char Buf[BUFN+1];

#if EMU_DEBUG_SYNC
u32 DebCheckSync;
#endif

// program configuration
u32 CfgFreq = 12000000;		// emulated frequency in Hz
u32 CfgMinClk = 160000000;	// minimal system clock in Hz
u32 CfgMaxClk = 220000000;	// maximal system clock in Hz
u8 CfgVmode = 1;		// initial videomode

// ============================================================================
//                             Main function
// ============================================================================

int WriteInjectInx = 0;

// write injection key
void WriteInject(u8 key, u32 tim)
{
	int inx = WriteInjectInx;
	PC_InjectKey[inx] = key;
	PC_InjectTime[inx] = tim;
	PC_InjectMask |= BIT(inx);
	WriteInjectInx = inx + 1;
}

// load configuration
void LoadCfg()
{
	// prepare filename
	memcpy(PC_RAM_BASE, FileSelLastName, FileSelLastNameLen);
	memcpy(&PC_RAM_BASE[FileSelLastNameLen], ".CFG", 5);

	// load config file
	PC_LoadCfg((const char*)PC_RAM_BASE);

	// get configuration
	int n = PC_GetCfg("Freq", 12000);
	if (n < 100) n = 100;
	if (n > 100000) n = 100000;
	CfgFreq = n*1000;

	n = PC_GetCfg("MinClk", 160000);
	if (n < 10000) n = 10000;
	if (n > 460000) n = 460000;
	CfgMinClk = n*1000;

	n = PC_GetCfg("MaxClk", 220000);
	if (n < CfgMinClk) n = CfgMinClk;
	if (n > 460000) n = 460000;
	CfgMaxClk = n*1000;

	n = PC_GetCfg("Card", PC_CARD_VGA);
	if (n < 0) n = 0;
	if (n >= PC_CARD_NUM) n = PC_CARD_NUM-1;
	PC_Vmode.card = (u8)n;

	n = PC_GetCfg("VideoMode", PC_VMODE_1);
	if (n < 0) n = 0;
	if (n >= PC_VMODE_NUM) n = PC_VMODE_NUM-1;
	CfgVmode = (u8)n;

	PC_VSyncWait = PC_GetCfg("VSyncWait", 2);

	// Key injection
	PC_InjectMask = 0;
	WriteInjectInx = 0;
	int inx = 0;
	u8 key;
	u32 time;
	int i;
	char b[20];
	for (i = 0; i < PC_INJECT_NUM; i++)
	{
		// get injext key
		MemPrint(b, 20, "InjectKey%d", i+1);
		key = PC_GetCfg(b, 0);
		if (key != 0)
		{
			// get inject time
			MemPrint(b, 20, "InjectTime%d", i+1);
			time = PC_GetCfg(b, (i+1)*100) * 1000;

			if ((key & B7) != 0)
			{
				// write extended key
				WriteInject(PC_KEYSCAN_EXT, time); // extended
				time += 5000;
				key &= 0x7f;
				WriteInject(key, time); // press
				time += 150000;
				WriteInject(PC_KEYSCAN_EXT, time); // extended
				time += 5000;
				WriteInject(key | B7, time); // release
			}
			else
			{
				// write normal key
				WriteInject(key, time); // press
				time += 150000;
				WriteInject(key | B7, time); // release
			}
		}
	}

	// Key mapping
	PC_KeypadScan[KEY_UP] = PC_GetCfg("KeyUp", B7 + PC_KEYSCAN_UP); // 0xC8
	PC_KeypadScanA[KEY_UP] = PC_GetCfg("KeyUpA", 0);
	PC_KeypadScanB[KEY_UP] = PC_GetCfg("KeyUpB", 0);
	PC_KeypadScanX[KEY_UP] = PC_GetCfg("KeyUpX", 0);
	PC_KeypadScanY[KEY_UP] = PC_GetCfg("KeyUpY", 0);

	PC_KeypadScan[KEY_LEFT] = PC_GetCfg("KeyLeft", B7 + PC_KEYSCAN_LEFT); // 0xCB
	PC_KeypadScanA[KEY_LEFT] = PC_GetCfg("KeyLeftA", 0);
	PC_KeypadScanB[KEY_LEFT] = PC_GetCfg("KeyLeftB", 0);
	PC_KeypadScanX[KEY_LEFT] = PC_GetCfg("KeyLeftX", 0);
	PC_KeypadScanY[KEY_LEFT] = PC_GetCfg("KeyLeftY", 0);

	PC_KeypadScan[KEY_RIGHT] = PC_GetCfg("KeyRight", B7 + PC_KEYSCAN_RIGHT); // 0xCD
	PC_KeypadScanA[KEY_RIGHT] = PC_GetCfg("KeyRightA", 0);
	PC_KeypadScanB[KEY_RIGHT] = PC_GetCfg("KeyRightB", 0);
	PC_KeypadScanX[KEY_RIGHT] = PC_GetCfg("KeyRightX", 0);
	PC_KeypadScanY[KEY_RIGHT] = PC_GetCfg("KeyRightY", 0);

	PC_KeypadScan[KEY_DOWN] = PC_GetCfg("KeyDown", B7 + PC_KEYSCAN_DOWN); // 0xDD
	PC_KeypadScanA[KEY_DOWN] = PC_GetCfg("KeyDownA", 0);
	PC_KeypadScanB[KEY_DOWN] = PC_GetCfg("KeyDownB", 0);
	PC_KeypadScanX[KEY_DOWN] = PC_GetCfg("KeyDownX", 0);
	PC_KeypadScanY[KEY_DOWN] = PC_GetCfg("KeyDownY", 0);

	PC_KeypadScanU[KEY_A] = PC_KeypadScanA[KEY_UP];
	PC_KeypadScanL[KEY_A] = PC_KeypadScanA[KEY_LEFT];
	PC_KeypadScanR[KEY_A] = PC_KeypadScanA[KEY_RIGHT];
	PC_KeypadScanD[KEY_A] = PC_KeypadScanA[KEY_DOWN];

	PC_KeypadScanU[KEY_B] = PC_KeypadScanB[KEY_UP];
	PC_KeypadScanL[KEY_B] = PC_KeypadScanB[KEY_LEFT];
	PC_KeypadScanR[KEY_B] = PC_KeypadScanB[KEY_RIGHT];
	PC_KeypadScanD[KEY_B] = PC_KeypadScanB[KEY_DOWN];

	PC_KeypadScanU[KEY_X] = PC_KeypadScanX[KEY_UP];
	PC_KeypadScanL[KEY_X] = PC_KeypadScanX[KEY_LEFT];
	PC_KeypadScanR[KEY_X] = PC_KeypadScanX[KEY_RIGHT];
	PC_KeypadScanD[KEY_X] = PC_KeypadScanX[KEY_DOWN];

	PC_KeypadScanU[KEY_Y] = PC_KeypadScanY[KEY_UP];
	PC_KeypadScanL[KEY_Y] = PC_KeypadScanY[KEY_LEFT];
	PC_KeypadScanR[KEY_Y] = PC_KeypadScanY[KEY_RIGHT];
	PC_KeypadScanD[KEY_Y] = PC_KeypadScanY[KEY_DOWN];

	PC_KeypadScan[KEY_A] = PC_GetCfg("KeyA", PC_KEYSCAN_ENTER); // 0x1C
	PC_KeypadScanB[KEY_A] = PC_GetCfg("KeyAB", 0);
	PC_KeypadScanX[KEY_A] = PC_GetCfg("KeyAX", 0);
	PC_KeypadScanY[KEY_A] = PC_GetCfg("KeyAY", 0);

	PC_KeypadScan[KEY_B] = PC_GetCfg("KeyB", PC_KEYSCAN_ESC); // 0x01
	PC_KeypadScanA[KEY_B] = PC_KeypadScanB[KEY_A];
	PC_KeypadScanX[KEY_B] = PC_GetCfg("KeyBX", 0);
	PC_KeypadScanY[KEY_B] = PC_GetCfg("KeyBY", 0);

	PC_KeypadScan[KEY_X] = PC_GetCfg("KeyX", PC_KEYSCAN_SCROLL); // 0x46
	PC_KeypadScanA[KEY_X] = PC_KeypadScanX[KEY_A];
	PC_KeypadScanB[KEY_X] = PC_KeypadScanX[KEY_B];
	PC_KeypadScanY[KEY_X] = PC_GetCfg("KeyXY", 0);

	PC_KeypadScan[KEY_Y] = PC_GetCfg("KeyY", B7 + PC_KEYSCAN_PAUSE); // 0xC5
	PC_KeypadScanA[KEY_Y] = PC_KeypadScanY[KEY_A];
	PC_KeypadScanB[KEY_Y] = PC_KeypadScanY[KEY_B];
	PC_KeypadScanX[KEY_Y] = PC_KeypadScanY[KEY_X];
}

// main function
int main()
{
	u32 k;
	u8 ch;
	int i;
	Bool stop;

	// initialize USB
#if USE_USB_HOST_HID
	UsbHostInit();
#endif

	// mount disk if not mounted
	//DiskAutoMount();

	// get path with samples
	k = GetHomePath(SampPath, "/EMU");
	if (SampPath[k-1] != PATHCHAR)
	{
		SampPath[k] = PATHCHAR;
		k++;
	}
	memcpy(SampPath+k, "_PC", 4);

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
	FileSelInit(SampPath, "IBM PC", "BIN", &FileSelColGreen);

	while (True)
	{
	// ---- select and load sample program


reset:

		// select file (or return from program)
		if (!FileSel()) ResetToBootLoader();

		// load configuration
		LoadCfg();

		// Find system clock in Hz that sets the most accurate PWM clock frequency.
		u32 sysclk = PWM_FindSysClk(CfgMinClk, CfgMaxClk, CfgFreq*I8086_CLOCKMUL);

		// setup system clock to get precise frequency of emulation
		if (sysclk > 230*MHZ)
			VregSetVoltage(VREG_VOLTAGE_1_25);
		else if (sysclk > 200*MHZ)
			VregSetVoltage(VREG_VOLTAGE_1_20);
		else if (sysclk > 140*MHZ)
			VregSetVoltage(VREG_VOLTAGE_1_15);
		else
			VregSetVoltage(VREG_VOLTAGE_1_10);

		ClockPllSysFreq((sysclk+500)/1000);

		// open and load file
		if (!FileOpen(&FileSelPrevFile, FileSelTempBuf))
			FileSelDispBigErr("Cannot open file");
		else
		{
			// load program to Flash memory
			//  file ... open file
			//  size ... file size in bytes
			if (!PC_LoadProg(&FileSelPrevFile, FileSelLastNameSize))
				FileSelDispBigErr("Cannot load program");
			else
			{
				FileClose(&FileSelPrevFile);

				PC_SetVmode(CfgVmode);

				// start emulation
				u32 freq = PC_Start(&i8086cpu, EMU_PWM, CfgFreq);

				// wait for break
				while (True)
				{
					// not running
					if (!PC_IsRunning())
					{
						// error handler, stop emulation
						if (!PC_ErrorMenu()) break;
					}

					// redraw screen
					PC_LCDRedraw();

					// Zoom
					if (PC_KeyZoomReq)
					{
						PC_KeyZoomReq = False;
						PC_LCDShiftZoom();
					}

					// Menu
					if (PC_KeyMenuReq)
					{
						PC_KeyMenuReq = False;
						PC_Menu();
					}
				}

				// reset system clock
				ClockPllSysFreq(PLL_KHZ);
				VregSetVoltage(VREG_VOLTAGE_1_10);

				// wait for no key pressed
				DrawClear();
				DispUpdate();
#if USE_USB_HOST_HID
				while (UsbKeyIsPressed(HID_KEY_ESCAPE) || UsbKeyIsPressed(HID_KEY_Y) || KeyPressed(KEY_Y)) {}
#else
				while (KeyPressed(KEY_Y)) {}
#endif

				// close DOS files
				PC_CloseFiles();
			}
		}
	}
}
