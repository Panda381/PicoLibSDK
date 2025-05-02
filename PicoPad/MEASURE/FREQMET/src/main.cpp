
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Inputs: GPIO1, GPIO14

// 1) Fast method (460 Hz .. 100 MHz) ... low precision of high frequencies
//	- loads samples to buffer
//	- counts time between first and last rising edge
//	- counts number of pulses (= number of edges - 1)
// 2) Middle method (2.0 Hz .. 29.7 MHz) ... most precise method in full range
//	- counts time between first and last rising edge
//	- counts number of pulses (= number of edges - 1)
// 3) Slow method (0.0256 Hz = 39 seconds .. 10 MHz ) ... low precision on high frequencies
//	- counts length of one period

#include "include.h"

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define FREQ_GPIO1	26	// used GPIO input 1
#define FREQ_GPIO2	27	// used GPIO input 2 (it can be equal FREQ_GPIO1 if 2nd input is not needed)
#else // USE_PICOPADHSTX
#define FREQ_GPIO1	1	// used GPIO input 1
#define FREQ_GPIO2	14	// used GPIO input 2 (it can be equal FREQ_GPIO1 if 2nd input is not needed)
#endif // USE_PICOPADHSTX

#define FREQ_PIO1	0	// used PIO1 (medium and low frequency)
#define FREQ_PIO2	1	// used PIO2 (high frequency)

#define FREQ_SM1	0	// used SM1
#define FREQ_SM2	1	// used SM2
#define FREQ_SM3	2	// used SM3
#define FREQ_SM4	3	// used SM4

#define FREQ_OFF1	0	// PIO1 program 1 offset (medium and low frequency)
#define FREQ_OFF2	0	// PIO2 program 2 offset (high frequency)

#define FREQ_DMA1	0	// used DMA channel 1 (high frequency input 1)
#define FREQ_DMA2	1	// used DMA channel 2 (high frequency input 2)
#define FREQ_DMA3	2	// used DMA channel 3 (high frequency processing 1)
#define FREQ_DMA4	3	// used DMA channel 4 (high frequency processing 2)

#define FREQ_BUF1	0	// buffer 1
#define FREQ_BUF2	1	// buffer 2

#define GATE		500000	// measure time in [us]

#define FREQ_SYSCLK	220000000 // system clock in Hz (should be a little higher than 2*max measured frequency)

#define USESCHMITT	1	// use schmitt trigger (system default 1, slower but more stable)

#define FRAMESIZE_EXT	(FRAMESIZE + 50000) // size of extended frame buffer, in u16 (= 320*240+50000 = 126800 u16 = 253600 bytes)

#define SAMPLES		(FRAMESIZE_EXT/2/2) // size of one sample bufer in u32 (= 31700 u32 = 126800 bytes)
#define SampleBuf(n)	((u32*)FrameBuf + (n)*SAMPLES)	// sample buffer (n = 0 or 1)

// crystal
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
#define CRYSTAL		Config.crystal
#else
#define CRYSTAL		12000000
#endif

// extended frame buffer in RGB 5-6-5 pixel format
ALIGNED u16 FrameBuf[FRAMESIZE_EXT];

// accumulator of fast frequency
u32 FastTimeAccu[2];
u32 FastPulseAccu[2];

// setup and start frequency meter - fast method
void SetupStartFast(u8 sm, u8 gpio, u8 dma, u8 buf)
{
	// set wrap address
	PioSetWrap(FREQ_PIO2, sm, FREQ_OFF2 + freqmet2_offset_high_target,
		FREQ_OFF2 + freqmet2_offset_high_wrap);

	// set start address
	PioSetAddr(FREQ_PIO2, sm, FREQ_OFF2 + freqmet2_offset_high_start);

	// configure GPIO for use by PIO
	PioSetupGPIO(FREQ_PIO2, gpio, 1);

	// set pin direction to input
	PioSetPinDir(FREQ_PIO2, sm, gpio, 1, 0);

#if USESCHMITT	// use schmitt trigger
	// enable schmitt trigger
	GPIO_SchmittEnable(gpio);
#else
	// disable schmitt trigger
	GPIO_SchmittDisable(gpio);
#endif

	// map state machine's IN pins	
	PioSetupIn(FREQ_PIO2, sm, gpio);

	// join FIFO to receive only
	PioSetFifoJoin(FREQ_PIO2, sm, PIO_FIFO_JOIN_RX);

	// shift right, auto-push, push threshold = 32 bits
	PioSetInShift(FREQ_PIO2, sm, True, True, 32);

	// PIO clock divider
	PioSetClkdiv(FREQ_PIO2, sm, 1*256);

	// initialize DMA
	DMA_ConfigTrig(dma,				// channel
		PIO_RXF(FREQ_PIO2, sm),			// read from PIO data port
		SampleBuf(buf),				// destination buffer
		SAMPLES,				// number of samples to read
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ(PioGetDreq(FREQ_PIO2, sm, False)) | // data request
			DMA_CTRL_CHAIN(dma) |		// disable chaining
			//DMA_CTRL_RING_WRITE |		// wrap ring on write
			//DMA_CTRL_RING_SIZE(GENERATOR_BITS) | // ring size
			DMA_CTRL_INC_WRITE |		// increment write
			//DMA_CTRL_INC_READ |		// not increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY |	// not high priority
			DMA_CTRL_EN);			// enable DMA

	// start state machine
	PioSMEnable(FREQ_PIO2, sm);
}

// check if frequency meter is completed - fast method
Bool CheckFast(u8 dma)
{
	return !DMA_IsBusy(dma);
}

// stop frequency meter - fast method (DMA transfer is already stopped)
void StopFast(u8 sm)
{
	PioSMDisable(FREQ_PIO2, sm);
	PioFifoClear(FREQ_PIO2, sm);
}

// setup and start frequency meter - fast method count time part + count pulses part
void _SetupStartFastTimePulse(u8 sm, u8 dma, u8 buf)
{
	// FIFO will send and receive
	PioSetFifoJoin(FREQ_PIO2, sm, PIO_FIFO_JOIN_RXTX);

	// shift right, auto-push, push threshold = 32 bits
	PioSetInShift(FREQ_PIO2, sm, True, True, 32);

	// shift right, autopull, pull threshold = 32 bits
	PioSetOutShift(FREQ_PIO2, sm, True, True, 32);

	// PIO clock divider
	PioSetClkdiv(FREQ_PIO2, sm, 1*256);

	// initialize DMA
	DMA_ConfigTrig(dma,				// channel
		SampleBuf(buf),				// source buffer
		PIO_TXF(FREQ_PIO2, sm),			// write to PIO data port
		SAMPLES,				// number of samples to write
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ(PioGetDreq(FREQ_PIO2, sm, True)) | // data request
			DMA_CTRL_CHAIN(dma) |		// disable chaining
			//DMA_CTRL_RING_WRITE |		// wrap ring on write
			//DMA_CTRL_RING_SIZE(GENERATOR_BITS) | // ring size
			//DMA_CTRL_INC_WRITE |		// not increment write
			DMA_CTRL_INC_READ |		// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY |	// not high priority
			DMA_CTRL_EN);			// enable DMA

	// start state machine
	PioSMEnable(FREQ_PIO2, sm);
}

// setup and start frequency meter - fast method count time
void SetupStartFastTime(u8 sm, u8 dma, u8 buf)
{
	// set wrap address
	PioSetWrap(FREQ_PIO2, sm, FREQ_OFF2 + freqmet2_offset_time_target,
		FREQ_OFF2 + freqmet2_offset_time_wrap);

	// set start address
	PioSetAddr(FREQ_PIO2, sm, FREQ_OFF2 + freqmet2_offset_time_start);

	// rest of setup
	_SetupStartFastTimePulse(sm, dma, buf);
}

// check if frequency meter is completed - fast method count time
Bool CheckFastTime(u8 sm, u8 dma)
{
	return !DMA_IsBusy(dma) && PioTxIsEmpty(FREQ_PIO2, sm);
}

// stop frequency meter - fast method count time (DMA transfer is already stopped)
INLINE void StopFastTime(u8 sm) { StopFast(sm); }

// get result of fast method count time (0 = no pulse)
u32 ResultFastTime(u8 sm)
{
	// exec "MOV y,isr" to get Y result from time counter
	PioExecWait(FREQ_PIO2, sm, PIO_ENCODE_MOV(PIO_SRCDST_Y, PIO_OP_NONE, PIO_SRCDST_ISR));

	// exec "IN y,32" to get Y result - time counter
	PioExecWait(FREQ_PIO2, sm, PIO_ENCODE_IN(PIO_SRCDST_Y, 32));

	// get result
	return -PioReadWait(FREQ_PIO2, sm);
}

// setup and start frequency meter - fast method count pulses
void SetupStartFastPulse(u8 sm, u8 dma, u8 buf)
{
	// set wrap address
	PioSetWrap(FREQ_PIO2, sm, FREQ_OFF2 + freqmet2_offset_pulse_target,
		FREQ_OFF2 + freqmet2_offset_pulse_wrap);

	// set start address
	PioSetAddr(FREQ_PIO2, sm, FREQ_OFF2 + freqmet2_offset_pulse_start);

	// rest of setup
	_SetupStartFastTimePulse(sm, dma, buf);
}

// check if frequency meter is completed - fast method count pulses
INLINE Bool CheckFastPulse(u8 sm, u8 dma) { return CheckFastTime(sm, dma); }

// stop frequency meter - fast method count pulses (DMA transfer is already stopped)
INLINE void StopFastPulse(u8 sm) { StopFast(sm); }

// get result of fast method count pulses (0 = no pulse)
u32 ResultFastPulse(u8 sm)
{
	// exec "IN y,32" to get X result
	PioExecWait(FREQ_PIO2, sm, PIO_ENCODE_IN(PIO_SRCDST_Y, 32)); 

	// get number of edges
	u32 n = -PioReadWait(FREQ_PIO2, sm);

	// not counting first edge
	if (n != 0) n--;
	return n;
}

// process fast method
//  DMA transfers 4 ms, count time 14 ms, count pulses 10 ms, with interleaving total 14 ms (64  v measures per 1 sec)
void ProcFast()
{
	// start record samples
	SetupStartFast(FREQ_SM1, FREQ_GPIO1, FREQ_DMA1, FREQ_BUF1);
	SetupStartFast(FREQ_SM2, FREQ_GPIO2, FREQ_DMA2, FREQ_BUF2);

	// small delay to fill part of buffer (minimum 10x u32 = 320 bits = 1.5 us, to fill FIFOs)
	WaitUs(10);

	// start processing data - count time (it is slower than count pulses)
	SetupStartFastTime(FREQ_SM3, FREQ_DMA3, FREQ_BUF1);
	SetupStartFastTime(FREQ_SM4, FREQ_DMA4, FREQ_BUF2);

	// wait for end of record samples
	while (!CheckFast(FREQ_DMA1) || !CheckFast(FREQ_DMA2)) {}

	// stop recording data
	StopFast(FREQ_SM1);
	StopFast(FREQ_SM2);

	// start processing data - count pulses
	SetupStartFastPulse(FREQ_SM1, FREQ_DMA1, FREQ_BUF1);
	SetupStartFastPulse(FREQ_SM2, FREQ_DMA2, FREQ_BUF2);

	// wait for end of processing count time
	while (!CheckFastTime(FREQ_SM3, FREQ_DMA3) || !CheckFastTime(FREQ_SM4, FREQ_DMA4)) {}

	// small delay to process rest of data
	WaitUs(10);

	// stop processing count time
	StopFastTime(FREQ_SM3);
	StopFastTime(FREQ_SM4);

	// get result time
	u32 tim1 = ResultFastTime(FREQ_SM3);
	u32 tim2 = ResultFastTime(FREQ_SM4);

	// wait for end of processing count pulses
	while (!CheckFastPulse(FREQ_SM1, FREQ_DMA1) || !CheckFastPulse(FREQ_SM2, FREQ_DMA2)) {}

	// small delay to process rest of data
	WaitUs(10);

	// stop processing count pulses
	StopFastPulse(FREQ_SM1);
	StopFastPulse(FREQ_SM2);

	// get result pulses
	u32 cnt1 = ResultFastPulse(FREQ_SM1);
	u32 cnt2 = ResultFastPulse(FREQ_SM2);

	// add pulses to accumulator
	if (cnt1 > 1)
	{
		FastTimeAccu[0] += tim1;
		FastPulseAccu[0] += cnt1;
	}

	if (cnt2 > 1)
	{
		FastTimeAccu[1] += tim2;
		FastPulseAccu[1] += cnt2;
	}
}

// setup middle method
void SetupMiddle(u8 sm, u8 gpio)
{
	// set wrap address
	PioSetWrap(FREQ_PIO1, sm, FREQ_OFF1 + freqmet1_offset_medium_target,
		FREQ_OFF1 + freqmet1_offset_medium_wrap);

	// configure GPIO for use by PIO
	PioSetupGPIO(FREQ_PIO1, gpio, 1);

	// set pin direction to input
	PioSetPinDir(FREQ_PIO1, sm, gpio, 1, 0);

#if USESCHMITT	// use schmitt trigger
	// enable schmitt trigger
	GPIO_SchmittEnable(gpio);
#else
	// disable schmitt trigger
	GPIO_SchmittDisable(gpio);
#endif

	// map state machine's IN pins	
	PioSetupIn(FREQ_PIO1, sm, gpio);

	// set jump pin
	PioSetJmpPin(FREQ_PIO1, sm, gpio);

	// join FIFO to receive only
	PioSetFifoJoin(FREQ_PIO1, sm, PIO_FIFO_JOIN_RX);

	// shift right, auto-push, push threshold = 32 bits
	PioSetInShift(FREQ_PIO1, sm, True, True, 32);

	// PIO clock divider
	PioSetClkdiv(FREQ_PIO1, sm, 1*256);
}

// start middle method
void StartMiddle(u8 sm)
{
	// set start address
	PioSetAddr(FREQ_PIO1, sm, FREQ_OFF1 + freqmet1_offset_medium_start);

	// start state machine
	PioSMEnable(FREQ_PIO1, sm);
}

// stop middle method
void StopMiddle(u8 sm)
{
	PioSMDisable(FREQ_PIO1, sm);
	PioFifoClear(FREQ_PIO1, sm);
}

// get result of middle method count time (0 = time overflow or no pulse)
u32 ResultMiddleTime(u8 sm)
{
	// exec "MOV y,osr" to get Y result from time counter
	PioExecWait(FREQ_PIO1, sm, PIO_ENCODE_MOV(PIO_SRCDST_Y, PIO_OP_NONE, PIO_SRCDST_OSROUT));

	// exec "IN y,32" to get Y result - time counter
	PioExecWait(FREQ_PIO1, sm, PIO_ENCODE_IN(PIO_SRCDST_Y, 32));

	// get result - time counter
	return 0xffffffff - PioReadWait(FREQ_PIO1, sm);
}

// get result of middle method count pulses (0 = no pulse)
u32 ResultMiddlePulse(u8 sm)
{
	// exec "IN x,32" to get X result - pulse counter
	PioExecWait(FREQ_PIO1, sm, PIO_ENCODE_IN(PIO_SRCDST_X, 32));

	// get result - pulse counter
	return - PioReadWait(FREQ_PIO1, sm);
}

// setup and start slow method
void SetupStartSlow(u8 sm, u8 gpio)
{
	// set wrap address
	PioSetWrap(FREQ_PIO1, sm, FREQ_OFF1 + freqmet1_offset_low_target,
		FREQ_OFF1 + freqmet1_offset_low_wrap);

	// set start address
	PioSetAddr(FREQ_PIO1, sm, FREQ_OFF1 + freqmet1_offset_low_start);

	// configure GPIO for use by PIO
	PioSetupGPIO(FREQ_PIO1, gpio, 1);

	// set pin direction to input
	PioSetPinDir(FREQ_PIO1, sm, gpio, 1, 0);

#if USESCHMITT	// use schmitt trigger
	// enable schmitt trigger
	GPIO_SchmittEnable(gpio);
#else
	// disable schmitt trigger
	GPIO_SchmittDisable(gpio);
#endif

	// map state machine's IN pins	
	PioSetupIn(FREQ_PIO1, sm, gpio);

	// set jump pin
	PioSetJmpPin(FREQ_PIO1, sm, gpio);

	// join FIFO to receive only
	PioSetFifoJoin(FREQ_PIO1, sm, PIO_FIFO_JOIN_RX);

	// shift right, auto-push, push threshold = 32 bits
	PioSetInShift(FREQ_PIO1, sm, True, True, 32);

	// PIO clock divider
	PioSetClkdiv(FREQ_PIO1, sm, 1*256);

	// start state machine
	PioSMEnable(FREQ_PIO1, sm);
}

// get result from slow method (1 = overflow) ... only a few passes - higher frequencies could overwhelm it
void ResultSlow(u8 sm, u32* res)
{
	int i;
	for (i = 10; i > 0; i--)
	{
		if (PioRxIsEmpty(FREQ_PIO1, sm)) break;
		*res = - PioRead(FREQ_PIO1, sm);
	}
}

#define BUFN	50
char Buf[BUFN];

// display 1 result
void Disp1(double f, int y, u8 gpio)
{
	// display GPIO
	int n = MemPrint(Buf, BUFN, "GPIO%d", gpio);
	DrawTextBg(Buf, 0, y+1, COL_GRAY, COL_BLACK);

	// prepare precision of frequency
	int prec = 7;
	if (f >= 98.59e6) prec = 6;
	if (f >= 98.99e6) prec = 5;
	if (f >= 99.49e6) prec = 4;
	if (f >= 99.89e6) prec = 3;

	// prepare precision of period
	int prec2 = 7;
	if (f >= 99.29e6) prec2 = 6;

	// prepare period
	if (f < 0.001) f = 0;
	double t = (f < 0.001) ? 0 : (1/f);

	// decode frequency
	n = MemPrint(Buf, BUFN, "%.*THz", prec, f);

	// find end of number
	int n0;
	char ch = Buf[0];
	for (n0 = 0; n0 < n; n0++)
	{
		ch = Buf[n0];
		if ((ch != '.') && !((ch >= '0') && (ch <= '9'))) break;
	}

	// width in pixels
	int n1 = n0*32;
	int n2 = (n - n0)*16;
	int x = (WIDTH - n1 - n2 - 8)/2;

	// print numeric part
	Buf[n0] = 0;
	DrawTextBg4(Buf, x, y+20, COL_WHITE, COL_BLACK);

	// print unit
	Buf[n0] = ch;
	DrawTextBg2(&Buf[n0], x + n1 + 8, y+20, COL_WHITE, COL_BLACK);

	// draw period
	n = MemPrint(Buf, BUFN, "    %.*Ts    ", prec2, t);
	DrawTextBg2(Buf, (WIDTH - n*16)/2, y+80, COL_YELLOW, COL_BLACK);
}

// display result
void Disp(double f1, double f2)
{
	// clear display
	DrawClear();

	// frequency 1
	Disp1(f1, 0, FREQ_GPIO1);

	// line
	DrawRect(0, HEIGHT/2, WIDTH, 1, COL_BLUE);

	// frequency 2
	Disp1(f2, HEIGHT/2, FREQ_GPIO2);

	// update display
	DispUpdate();
}

// main function
int main()
{
	// set higher voltage to support stable higher frequencies
	VregSetVoltage(VREG_VOLTAGE_1_30);

	// setup system clock to 220 MHz
	ClockPllSysFreq(FREQ_SYSCLK/1000);

	// initialize both PIOs
	PioInit(0);
	PioInit(1);

	// load programs
	PioLoadProg(FREQ_PIO1, freqmet1_program_instructions, count_of(freqmet1_program_instructions), FREQ_OFF1);
	PioLoadProg(FREQ_PIO2, freqmet2_program_instructions, count_of(freqmet2_program_instructions), FREQ_OFF2);

	// clear accumulator of fast frequency
	FastTimeAccu[0] = 0;
	FastTimeAccu[1] = 0;
	FastPulseAccu[0] = 0;
	FastPulseAccu[1] = 0;

	// setup middle method
	SetupMiddle(FREQ_SM1, FREQ_GPIO1);
	SetupMiddle(FREQ_SM2, FREQ_GPIO2);

	// start middle method
	StartMiddle(FREQ_SM1);
	StartMiddle(FREQ_SM2);

	// setup and start slow method
	SetupStartSlow(FREQ_SM3, FREQ_GPIO1);
	SetupStartSlow(FREQ_SM4, FREQ_GPIO2);

	// start measure
	u32 tstart = Time();
	u32 t, tim1, tim2, cnt1, cnt2;
	double fa, fb, f1a, f1b, f2a, f2b, f3a, f3b;
	u32 resa = 0;
	u32 resb = 0;

	// main loop
	while (True)
	{
		// process fast method
		ProcFast();

		// read result from slow method
		ResultSlow(FREQ_SM3, &resa);
		ResultSlow(FREQ_SM4, &resb);

		// check time delta 1 second
		t = Time() - tstart;
		if (t > GATE)
		{
			tstart = Time();

		// ==== fast method

			// calculate frequency from method 1 (fast)
			f1a = 0;
			if (FastPulseAccu[0] > 10) f1a = (double)FastPulseAccu[0]/FastTimeAccu[0]*FREQ_SYSCLK/CRYSTAL*12000000;

			f1b = 0;
			if (FastPulseAccu[1] > 10) f1b = (double)FastPulseAccu[1]/FastTimeAccu[1]*FREQ_SYSCLK/CRYSTAL*12000000;

			// clear accumulator of fast frequency
			FastTimeAccu[0] = 0;
			FastTimeAccu[1] = 0;
			FastPulseAccu[0] = 0;
			FastPulseAccu[1] = 0;

		// ==== middle method

			// stop measure of method 2 (middle)
			StopMiddle(FREQ_SM1);
			StopMiddle(FREQ_SM2);

			// get result of middle method count time (0 = time overflow or no pulse)
			tim1 = ResultMiddleTime(FREQ_SM1);
			tim2 = ResultMiddleTime(FREQ_SM2);

			// get result of middle method count pulses (0 = no pulse)
			cnt1 = ResultMiddlePulse(FREQ_SM1);
			cnt2 = ResultMiddlePulse(FREQ_SM2);

			// correction - add pulses to time
			tim1 = tim1*2 + cnt1*3;
			tim2 = tim2*2 + cnt2*3;

			// calculate frequency
			f2a = 0;
			if ((tim1 != 0) && (cnt1 != 0)) f2a = (double)cnt1/tim1*FREQ_SYSCLK/CRYSTAL*12000000;

			f2b = 0;
			if ((tim2 != 0) && (cnt2 != 0)) f2b = (double)cnt2/tim2*FREQ_SYSCLK/CRYSTAL*12000000;

			// start middle method
			StartMiddle(FREQ_SM1);
			StartMiddle(FREQ_SM2);

		// ==== slow method

			f3a = 0;
			if (resa > 1) f3a = (double)FREQ_SYSCLK/(resa+1)/CRYSTAL*6000000;

			f3b = 0;
			if (resb > 1) f3b = (double)FREQ_SYSCLK/(resb+1)/CRYSTAL*6000000;

		// ==== display

			// select best method
			fa = f1a;
			if (fa <= 26e6) fa = f2a;
			if (fa <= 5) fa = f3a;

			fb = f1b;
			if (fb <= 26e6) fb = f2b;
			if (fb <= 5) fb = f3b;

			// display
			Disp(fa, fb);
		}

		// quit program
		if (KeyGet() == KEY_Y)
		{
#if USE_SCREENSHOT		// use screen shots
			Disp(fa, fb);
			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
