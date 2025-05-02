
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Input: GPIO1 (PicopadHSTX GPIO26), max. frequency range 4 Hz .. 7 MHz

#include "include.h"

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define PULSE_GPIO	26	// used GPIO input
#else
#define PULSE_GPIO	1	// used GPIO input
#endif

#define PULSE_PIO	0	// used PIO
#define PULSE_SM	0	// used SM
#define PULSE_OFF	0	// PIO program offset
#define PULSE_DMA	0	// used DMA channel
#define PULSE_SYSCLK	125000000 // system clock in Hz
#define USESCHMITT	1	// use schmitt trigger (system default 1, slower but more stable)

#define SAMPLES_BITS	15	// number of bits of sample buffer
#define SAMPLES_BYTES	(1<<SAMPLES_BITS) // size in bytes of sample buffer (= 32K)
#define SAMPLES		(SAMPLES_BYTES/4) // size in u32 of sample buffer (= 8K)

#define HISTNUM		WIDTH	// histogram width

// sample buffer
u32 __attribute__((aligned(SAMPLES_BYTES))) SampleBuf[SAMPLES];

// histograms
u32 HistHigh[HISTNUM];	// histogram of HIGH pulses
u32 HistLow[HISTNUM];	// histogram of LOW pulses

// start new measure
void Start()
{
	// abort DMA
	DMA_Abort(PULSE_DMA);

	// disable SM
	PioSMDisable(PULSE_PIO, PULSE_SM);
	PioFifoClear(PULSE_PIO, PULSE_SM);

	// clear histograms
	memset(HistHigh, 0, HISTNUM*sizeof(u32));
	memset(HistLow, 0, HISTNUM*sizeof(u32));

	// set wrap address
	PioSetWrap(PULSE_PIO, PULSE_SM, PULSE_OFF + pulsgram_wrap_target, PULSE_OFF + pulsgram_wrap);

	// set start address
	PioSetAddr(PULSE_PIO, PULSE_SM, PULSE_OFF + pulsgram_offset_start);

	// configure GPIO for use by PIO
	PioSetupGPIO(PULSE_PIO, PULSE_GPIO, 1);

	// set pin direction to input
	PioSetPinDir(PULSE_PIO, PULSE_SM, PULSE_GPIO, 1, 0);

#if USESCHMITT	// use schmitt trigger
	// enable schmitt trigger
	GPIO_SchmittEnable(PULSE_GPIO);
#else
	// disable schmitt trigger
	GPIO_SchmittDisable(PULSE_GPIO);
#endif

	// map state machine's IN pins	
	PioSetupIn(PULSE_PIO, PULSE_SM, PULSE_GPIO);

	// setup GPIO for jump
	PioSetJmpPin(PULSE_PIO, PULSE_SM, PULSE_GPIO);

	// join FIFO to receive only
	PioSetFifoJoin(PULSE_PIO, PULSE_SM, PIO_FIFO_JOIN_RX);

	// shift right, auto-push, push threshold = 32 bits
	PioSetInShift(PULSE_PIO, PULSE_SM, True, True, 32);

	// PIO clock divider
	PioSetClkdiv(PULSE_PIO, PULSE_SM, 1*256);

	// initialize DMA
	DMA_ConfigTrig(PULSE_DMA,			// channel
		PIO_RXF(PULSE_PIO, PULSE_SM),		// read from PIO data port
		SampleBuf,				// destination buffer
		(u32)-1,				// number of samples to read
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ(PioGetDreq(PULSE_PIO, PULSE_SM, False)) | // data request
			DMA_CTRL_CHAIN(PULSE_DMA) |	// disable chaining
			DMA_CTRL_RING_WRITE |		// wrap ring on write
			DMA_CTRL_RING_SIZE(SAMPLES_BITS) | // ring size
			DMA_CTRL_INC_WRITE |		// increment write
			//DMA_CTRL_INC_READ |		// not increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY |	// not high priority
			DMA_CTRL_EN);			// enable DMA

	// start state machine
	PioSMEnable(PULSE_PIO, PULSE_SM);
}

#define Y1	100	// base Y of HIGH pulses
#define Y2	220	// base Y of LOW pulses

// draw one pulsegram
void Draw1(u32* hist, int y, COLTYPE col)
{
	int i, h;
	u32 samp;

	// draw pulsegram
	for (i = 0; i < HISTNUM; i++)
	{
		samp = *hist++;
		h = Order(samp);
		if (h >= 3) // minimum 4
			h = h*4 + ((samp >> (h-3)) & 3) - 8; // x = 4..123
		else
			h = samp; // 0..3

		h++;
		if (h > 100) h = 100;
		DrawRect(i, y - h, 1, h, col);
	}

	// draw bottom line
	DrawRect(0, y, WIDTH, 1, COL_WHITE);
	y++;

#define SCALEH	6

	// draw main scale
	DrawRect(6, y, 1, SCALEH, COL_WHITE);	// 0.1us
	DrawRect(46, y, 1, SCALEH, COL_WHITE);	// 1us
	DrawRect(99, y, 1, SCALEH, COL_WHITE);	// 10us
	DrawRect(152, y, 1, SCALEH, COL_WHITE);	// 100us
	DrawRect(205, y, 1, SCALEH, COL_WHITE);	// 1ms
	DrawRect(259, y, 1, SCALEH, COL_WHITE);	// 10ms
	DrawRect(310, y, 1, SCALEH, COL_WHITE);	// 100ms

#define SCALEH2	2

	// draw side scale
	DrawRect(31, y, 1, SCALEH2, COL_WHITE);	// 0.5us
	DrawRect(82, y, 1, SCALEH2, COL_WHITE);	// 5us
	DrawRect(135, y, 1, SCALEH2, COL_WHITE); // 50us
	DrawRect(189, y, 1, SCALEH2, COL_WHITE); // 500us
	DrawRect(242, y, 1, SCALEH2, COL_WHITE); // 5ms
	DrawRect(295, y, 1, SCALEH2, COL_WHITE); // 50ms

	// draw description
	DrawText(".1us", -2, y+8, COL_WHITE);
	DrawText("1us   10us  100us   1ms   10ms 100ms", 35, y+8, COL_WHITE);
}

// main function
int main()
{
	int x, i, h, off, inx;
	u32 samp, t;

	// initialize PIO
	PioInit(PULSE_PIO);

	// load programs
	PioLoadProg(PULSE_PIO, pulsgram_program_instructions, count_of(pulsgram_program_instructions), PULSE_OFF);

	// start new measure
	Start();

	// sample index
	inx = 0;

	// time
	t = Time();

	// main loop
	while (True)
	{
		// refresh DMA transfer
		if (!DMA_IsBusy(PULSE_DMA) || ((u32)DMA_GetCount(PULSE_DMA) < 0x40000000))
		{
			DMA_Abort(PULSE_DMA);
			DMA_SetCountTrig(PULSE_DMA, (u32)-1);
		}

		// get DMA index
		off = ((u32)DMA_GetWrite(PULSE_DMA) - (u32)SampleBuf) / sizeof(u32);

		// restart DMA transfer
		if (off < inx) inx = 0;

		// DMA not reached, process samples
		i = 2000;
		while ((inx < off) && (i-- > 0))
		{
			// read sample
			samp = -SampleBuf[inx];

			// convert to logarithm (0->0, 1->1, 2..3->2, 4..7->3, 8..15->4, 16..31->5, ...)
			x = Order(samp);
			if (x >= 5) // minimum 16
				x = x*16 + ((samp >> (x-5)) & 0x0f) - 64; // x = 16..463
			else
				x = samp; // 0..15

			// increment counter
			if ((x < HISTNUM) && (x > 4))
			{
				if ((inx & 1) == 0)
				{
					// HIGH pulse
					HistHigh[x]++;
				}
				else
				{
					// LOW pulse
					HistLow[x]++;
				}
			}
			inx++;
		}

		// display
		if ((u32)(Time() - t) >= 500000)
		{
			t = Time();

			// clear display
			DrawClear();

			// titles
			DrawText("HIGH", WIDTH-4*8, 0, COL_AZURE);
			DrawText("LOW", WIDTH-3*8, Y2-Y1, COL_RED);

			// draw HIGH pulses
			Draw1(HistHigh, Y1, COL_AZURE);

			// draw LOW pulses
			Draw1(HistLow, Y2, COL_RED);

			// display update
			DispUpdate();
		}

		// quit program
		switch (KeyGet())
		{
		case KEY_A:
			Start();
			break;

		case KEY_Y:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
