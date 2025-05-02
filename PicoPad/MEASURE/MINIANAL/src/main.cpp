
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Simple logic analyzer and noise generator.

//  Analyzer input: PicoPad GPIO0, PicoPadVGA GPIO12, PicopadHSTX GPIO26
//  Analyzer sample rate: 125 MHz
//  Displays: 9600 samples
//  Total sampling time: 76.8 us

//  Noise output: GPIO1, PicopadHSTX GPIO27
//  Noise sample rate: 15.75 MHz

#include "include.h"

// ====== analyzer input
// Time to load whole sample buffer: 300*32/125 = 76.8 us

#define ANALYZER_PIO		0	// used PIO (common to analyzer and generator)
#define ANALYZER_SM		0	// used sample state machine
#define ANALYZER_OFF		0	// PIO input sample program offset
#define ANALYZER_DMA		0	// used DMA channel
#define ANALYZER_DIV		1	// clock divider to divide CLK_SYS=125 MHz to sample rate
#define ANALYZER_SAMPLES	300	// number of 32-bit samples

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define ANALYZER_GPIO		26	// used GPIO input
#elif USE_PICOPADVGA
#define ANALYZER_GPIO		12	// used GPIO input
#else
#define ANALYZER_GPIO		0	// used GPIO input
#endif

u32 AnalyzerSamples[ANALYZER_SAMPLES];		// input samples from analyzer

// colors
COLTYPE Colors[] = {
	COL_AZURE,
	COL_GREEN,
	COL_CYAN,
	COL_RED,
	COL_MAGENTA,
	COL_YELLOW,
	COL_WHITE,
	COL_ORANGE,
};	

// capture samples (wait to complete)
void AnalyzerCapt()
{
	// stop state machine and clear FIFOs
	PioSMDisable(ANALYZER_PIO, ANALYZER_SM);
	PioFifoClear(ANALYZER_PIO, ANALYZER_SM);

	// start DMA
	DMA_SetWriteTrig(ANALYZER_DMA, AnalyzerSamples);

	// start state machine
	PioSMEnable(ANALYZER_PIO, ANALYZER_SM);
}

// display samples
//  1 row = 8 lines
//  1 row = 320 samples
//  total 240 lines -> 30 rows = 9600 bits = 300 u32 samples
void AnalyzerDisp()
{
	int x, y;
	u8 colinx = 0;
	COLTYPE col, col2;
	u32* src = AnalyzerSamples;
	u32 s;
	int i = 0;

	for (y = 0; y <= HEIGHT-8; y += 8)
	{
		col = Colors[colinx];
		for (x = 0; x < WIDTH; x++)
		{
			DrawPoint(x, y+4, col);
			col2 = col;
			if (i == 0) s = *src++;
			i = (i+1) & 0x1f;
			if ((s & 1) == 0) col2 = COL_BLACK;
			s >>= 1;
			DrawPoint(x, y+0, col2);
			DrawPoint(x, y+1, col2);
			DrawPoint(x, y+2, col2);
			DrawPoint(x, y+3, col2);
		}
		colinx++;
		if (colinx >= count_of(Colors)) colinx = 0;
	}

	// update display
	DispUpdate();
}

// ====== random test samples output
// Time to send whole sample buffer: 8192*32/125*8 = 16.777 ms

#define GENERATOR_PIO		ANALYZER_PIO	// used PIO (common to analyzer and generator)
#define GENERATOR_SM		1		// used state machine
#define GENERATOR_OFF		1		// PIO output random program offset

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define GENERATOR_GPIO		27		// used GPIO output
#else
#define GENERATOR_GPIO		1		// used GPIO output
#endif

#define GENERATOR_DMA		1		// used DMA channel
#define GENERATOR_DIV		8		// clock divider to divide clk_sys=126 MHz to sample rate
#define GENERATOR_BITS		15		// number of bits of random sample buffer size in bytes (max. 15 allowed)
#define GENERATOR_BYTES		(1<<GENERATOR_BITS) // size of random sample buffer in bytes (=32768)
#define GENERATOR_SAMPLES	(GENERATOR_BYTES/4) // number of output random samples in u32 (=8192)
#define GENERATOR_LOOP		30		// number of repeats sending whole buffer (= time 503 ms)

// buffer must be aligned to its size (64 KB)
u32 __attribute__((aligned(GENERATOR_SAMPLES))) GeneratorSamples[GENERATOR_SAMPLES];	// output random test samples (=65536 bytes)

// send samples
void GeneratorSend()
{
	// stop state machine and clear FIFO
	PioSMDisable(GENERATOR_PIO, GENERATOR_SM);
	PioFifoClear(GENERATOR_PIO, GENERATOR_SM);

	// start state machine
	PioSMEnable(GENERATOR_PIO, GENERATOR_SM);

	// start DMA
	DMA_SetReadTrig(GENERATOR_DMA, GeneratorSamples);
}

// main function
int main()
{
	int i;

	// initialize PIO (common to analyzer and generator)
	PioInit(ANALYZER_PIO);

// ==== initialize analyzer

	// load PIO program
	PioLoadProg(ANALYZER_PIO, analyzer_program_instructions,
		count_of(analyzer_program_instructions), ANALYZER_OFF);

	// set wrap address
	PioSetWrap(ANALYZER_PIO, ANALYZER_SM, ANALYZER_OFF + analyzer_wrap_target,
		ANALYZER_OFF + analyzer_wrap);

	// set start address
	PioSetAddr(ANALYZER_PIO, ANALYZER_SM, ANALYZER_OFF);

	// configure GPIO for use by PIO
	PioSetupGPIO(ANALYZER_PIO, ANALYZER_GPIO, 1);

	// set pin direction to input
	PioSetPinDir(ANALYZER_PIO, ANALYZER_SM, ANALYZER_GPIO, 1, 0);

	// map state machine's IN pins	
	PioSetupIn(ANALYZER_PIO, ANALYZER_SM, ANALYZER_GPIO);

	// join FIFO to receive only
	PioSetFifoJoin(ANALYZER_PIO, ANALYZER_SM, PIO_FIFO_JOIN_RX);

	// PIO clock divider
	PioSetClkdiv(ANALYZER_PIO, ANALYZER_SM, ANALYZER_DIV*256);

	// shift right, auto-push, push threshold = 32 bits
	PioSetInShift(ANALYZER_PIO, ANALYZER_SM, True, True, 32);

	// initialize DMA of analyzer
	DMA_Config(ANALYZER_DMA,			// channel
		PIO_RXF(ANALYZER_PIO, ANALYZER_SM),	// read from PIO data port
		AnalyzerSamples,			// destination buffer
		ANALYZER_SAMPLES,			// number of samples to read
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ(PioGetDreq(ANALYZER_PIO, ANALYZER_SM, False)) | // data request
			DMA_CTRL_CHAIN(ANALYZER_DMA) |	// disable chaining
			//DMA_CTRL_RING_WRITE |		// wrap ring on write
			//DMA_CTRL_RING_SIZE(GENERATOR_BITS) | // ring size
			DMA_CTRL_INC_WRITE |		// increment write
			//DMA_CTRL_INC_READ |		// not increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY |	// not high priority
			DMA_CTRL_EN);			// enable DMA

// ==== initialize generator

	// load PIO program
	PioLoadProg(GENERATOR_PIO, generator_program_instructions,
		count_of(generator_program_instructions), GENERATOR_OFF);

	// set wrap address
	PioSetWrap(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_wrap_target,
		GENERATOR_OFF + generator_wrap);

	// set start address
	PioSetAddr(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF);

	// configure GPIO for use by PIO
	PioSetupGPIO(GENERATOR_PIO, GENERATOR_GPIO, 1);

	// set pin direction to output
	PioSetPinDir(GENERATOR_PIO, GENERATOR_SM, GENERATOR_GPIO, 1, 1);

	// map state machine's OUT and MOV pins	
	PioSetupOut(GENERATOR_PIO, GENERATOR_SM, GENERATOR_GPIO, 1);

	// join FIFO to send only
	PioSetFifoJoin(GENERATOR_PIO, GENERATOR_SM, PIO_FIFO_JOIN_TX);

	// PIO clock divider
	PioSetClkdiv(GENERATOR_PIO, GENERATOR_SM, GENERATOR_DIV*256);

	// shift right, autopull, pull threshold = 32 bits
	PioSetOutShift(GENERATOR_PIO, GENERATOR_SM, True, True, 32);

	// initialize DMA of generator
	DMA_Config(GENERATOR_DMA,			// channel
		GeneratorSamples,			// source buffer
		PIO_TXF(GENERATOR_PIO, GENERATOR_SM),	// write to PIO data port
		GENERATOR_SAMPLES*GENERATOR_LOOP,	// number of samples to write
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ(PioGetDreq(GENERATOR_PIO, GENERATOR_SM, True)) | // data request
			DMA_CTRL_CHAIN(GENERATOR_DMA) |	// disable chaining
			//DMA_CTRL_RING_WRITE |		// wrap ring on write
			DMA_CTRL_RING_SIZE(GENERATOR_BITS) | // ring size
			//DMA_CTRL_INC_WRITE |		// not increment write
			DMA_CTRL_INC_READ |		// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY |	// not high priority
			DMA_CTRL_EN);			// enable DMA

	// main loop
	while (True)
	{
		// fill random sample buffer with output samples
		//  (why just now - to minimize the delay until the new transmission)
		for (i = 0; i < GENERATOR_SAMPLES; i++) GeneratorSamples[i] = RandU32();

		// send samples
		GeneratorSend();

		// capture samples
		AnalyzerCapt();

		// wait for transfers (generator takes longer, 503 ms)
		DMA_Wait(ANALYZER_DMA);
		DMA_Wait(GENERATOR_DMA);

		// display samples
		AnalyzerDisp();

		// quit program
		if (KeyGet() == KEY_Y)
		{
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
