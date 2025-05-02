
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Frequency generator
// - PicoPad: output to GPIO14 or GPIO15 (Speaker)
// - PicoPadVGA: output to GPIO1 or GPIO0 (Speaker)

#include "include.h"

#define GENERATOR_PIO		0		// used PIO
#define GENERATOR_SM		0		// used state machine
#define GENERATOR_OFF		0		// PIO program offset

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define GENERATOR_GPIO1		26		// used GPIO output 1 (better to be on same PWM slice as GENERATOR_GPIO2)
#elif USE_PICOPADVGA
#define GENERATOR_GPIO1		1		// used GPIO output 1 (better to be on same PWM slice as GENERATOR_GPIO2)
#else
#define GENERATOR_GPIO1		14		// used GPIO output 1 (better to be on same PWM slice as GENERATOR_GPIO2)
#endif

#define GENERATOR_GPIO2		PWMSND_GPIO	// used GPIO output 2 (speaker) (better to be on same PWM slice as GENERATOR_GPIO1)

#define GENERATOR_DMA		0		// used DMA channel
#define GENERATOR_DIV		1		// clock divider to divide clk_sys to sample rate

#define SYSCLK_MIN	80000000		// minimal system clock in [Hz]
#define SYSCLK_MAX	200000000		// maximal system clock in [Hz]

#define SLOWGEN_BITS		13		// number of bits of wait samples array in slow mode
#define SLOWGEN_BYTES		(1<<SLOWGEN_BITS) // size in bytes of wait samples array in slow mode (= 8192)
#define SLOWGEN_WAITS		(SLOWGEN_BYTES/4) // number of entries of wait samples array in slow mode (= 2048)
#define SLOWGEN_CYCLES		(SLOWGEN_WAITS/2) // number of cycles of wait sample array in slow mode (= 1024)

#define SLOW_FRAC		(SLOWGEN_BITS-2) // number of fraction bits in slow mode (= 11)
#define SLOW_FRACNUM		(1<<SLOW_FRAC)	// number of fraction waits in slow mode (= 2048 wait delays)
#define SLOW_MASK		(SLOW_FRACNUM-1) // mask of fraction bits in slow mode (= 0x7FF)

#define FAST_FRAC		5		// number of fraction bits in fast mode
#define FAST_FRACNUM		(1<<FAST_FRAC)	// number of fraction waits in fast mode (= 32 wait delays)
#define FAST_MASK		(FAST_FRACNUM-1) // mask of fraction bits in fast mode (= 0x1F)

#define FASTGEN_WAITS		(1<<FAST_FRAC) // number of entries of wait in fast mode (= 32)
#define FASTGEN_CYCLES		(FASTGEN_WAITS/2) // number of cycles of wait in fast mode (= 16)

// frequency mode
#define MODE_SLOWSTABLE		0		// slow stable (use generator_slow and 2 waits)
#define MODE_SLOWPREC		1		// slow precise (use generator_slow and 2048 waits)
#define MODE_FASTSTABLE		2		// fast stable (use generator_short and 2 waits, max. 32 cycles per output period)
#define MODE_FASTPREC		3		// fast precise (use generator_short and 32 waits, max. 32 cycles per output period)

// crystal
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
#define CRYSTAL		Config.crystal
#else
#define CRYSTAL		12000000
#endif

// frequency set
enum {
	SET_CUSTOM,
	SET_DECIMAL,
	SET_SINEDECI,
	SET_NOTE,
	SET_SINENOTE,
	SET_TRIANGLE,
	SET_SAWTOOTH,
	SET_BITNOISE,
	SET_PULSENOISE,

	SET_NUM
};

const char* SetText[SET_NUM] = {
	"1) Custom Square      ",
	"2) Decimal Square     ",
	"3) Decimal PWM Sine   ",
	"4) Notes Square       ",
	"5) Notes PWM Sine     ",
	"6) Notes PWM Triangle ",
	"7) Notes PWM Sawtooth ",
	"8) Bit Noise Square   ",
	"9) Pulse Noise Square ",
};
u8 SetInx = SET_DECIMAL;	// current set

// available system frequencies in given range SYSCLK_MIN..SYSCLK_MAX
#define SYSCLK_NUM	500	// 370 entries

typedef struct {
	u16		fbdiv;
	u8		pd1;
	u8		pd2;
	u32		freq;	// frequency in [Hz]
	double		dfreq;
} sPLL;

#define OUTPUT_NUM	2	// number of outputs
u8 Output = 0;			// selected output
const u8 OutputGPIO[OUTPUT_NUM] = { GENERATOR_GPIO1, GENERATOR_GPIO2 };

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
const char* OutputText[OUTPUT_NUM] = { "GPIO26           ", "GPIO20 (Speaker) " };
#elif USE_PICOPADVGA
const char* OutputText[OUTPUT_NUM] = { "GPIO1            ", "GPIO0 (Speaker)  " };
#else
const char* OutputText[OUTPUT_NUM] = { "GPIO14           ", "GPIO15 (Speaker) " };
#endif

sPLL AllSysClk[SYSCLK_NUM];	// system clock
int AllSysClkNum;		// number of system clock

#define MODE_SLOWSTABLE		0		// slow stable (use generator_slow and 2 delays)
#define MODE_SLOWPREC		1		// slow precise (use generator_slow and 2048 delays)
#define MODE_FASTSTABLE		2		// fast stable (use generator_short and 2 delays, max. 32 cycles per takt)
#define MODE_FASTPREC		3		// fast precise (use generator_short and 32 delays, max. 32 cycles per takt)

const char* ModeName[4] = { "Slow_Stable", "Slow_Precise", "Fast_Stable", "Fast_Precise" };

const char* StableOn  = "Stable signal     ";
const char* StableOff = "Precise frequency ";
Bool Stable = False;		// prefer stable signal (or precise frequency otherwise)

// list of decimal frequencies
#define DECIMAL_MAX	200	// 91 entries (0.01 x9, 0.1 x9, 1 x9, 10 x9, 100 x9, 1K x9, 10K x9, 100K x9, 1M x9, 10M x9, 100M)
double DecimalList[DECIMAL_MAX];
int DecimalNum;
int DecimalSel = 45; // selected frequency 1 kHz

// note names
const char* NoteName[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

// Equal Temperament Tuning of Notes (interval 2^(1/12), base A4 with frequency 440 Hz) http://pages.mtu.edu/~suits/notefreqs.html
// cent calculators: http://www.sengpielaudio.com/calculator-centsratio.htm
// C0=16.35 Hz, C#0, D0, D#0, E0, F0, F#0, G0, G#0, A0, B0, C1=34.65 Hz ... B9=15804.26 Hz
// C0 = 440 (=A4) / (2^(57/12)) = 16,351597831287414667365624595207
#define FIRSTOCT -10 // first octave, first note, C-10 = 0.015968357257Hz
const double C0 = 16.351597831287414667365624595207/(1<<(-FIRSTOCT)); // base note C0 = 440 (=A4) / (2^(57/12))
#define NOTES (32*12+7) // number of tones (= 391); first entry is C-10 = 0.015968357257Hz, last entry is F#22 = 96991817.947Hz

// list of tone frequencies
#define TONE_MAX	500
double ToneList[TONE_MAX];
int ToneNum;
int ToneSel = 14*12 + 9; // selected tone A4

// custom frequency * 1e6
#define CUSTOM_NUM	14	// number of editable digits
#define CUSTOM_MIN	10000ULL	// custom minimal
#define CUSTOM_MAX	100000000000000ULL // custom maximal
u64 Custom = 0L; // default (Edit 14 digits, display 20 chars: 100'000'000.000000Hz)
int CustomPos = 0; // position
const int CustomX[CUSTOM_NUM] = { 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 15, 16, 17 }; // cursor position
const u64 CustomInc[CUSTOM_NUM] = { 
	10000000000000ULL,	// 10 MHz
	 1000000000000ULL,	// 1 MHz
	  100000000000ULL,	// 100 kHz
	   10000000000ULL,	// 10 kHz
	    1000000000ULL,	// 1 kHz
	     100000000ULL,	// 100 Hz
	      10000000ULL,	// 10 Hz
	       1000000ULL,	// 1 Hz
	        100000ULL,	// 100 mHz
	         10000ULL,	// 10 mHz
	          1000ULL,	// 1 mHz
	           100ULL,	// 100 uHz
	            10ULL,	// 10 uHz
	             1ULL,	// 1 uHz
};

// bit noise frequencies
#define BITNOISE_NUM	25
const u32 BitNoiseList[BITNOISE_NUM] = {
	1,		// 1 Hz
	2,		// 2 Hz
	5,		// 5 Hz
	10,		// 10 Hz
	20,		// 20 Hz
	50,		// 50 Hz
	100,		// 100 Hz
	200,		// 200 Hz
	500,		// 500 Hz
	1000,		// 1 kHz
	2000,		// 2 kHz
	5000,		// 5 kHz
	10000,		// 10 kHz
	20000,		// 20 kHz
	50000,		// 50 kHz
	100000,		// 100 kHz *default
	200000,		// 200 kHz
	500000,		// 500 kHz
	1000000,	// 1 MHz
	2000000,	// 2 MHz
	5000000,	// 5 MHz
	10000000,	// 10 MHz
	20000000,	// 20 MHz
	50000000,	// 50 MHz
	100000000,	// 100 MHz
};
int BitNoiseSel = 15; // selected

// pulse noise interval (multiply of 10 ns)
#define PULSENOISE_NUM	25
const u32 PulseNoiseList[PULSENOISE_NUM] = {
	1,		// 10 ns
	2,		// 20 ns
	5,		// 50 ns
	10,		// 100 ns
	20,		// 200 ns
	50,		// 500 ns
	100,		// 1 us
	200,		// 2 us
	500,		// 5 us
	1000,		// 10 us *default min
	2000,		// 20 us
	5000,		// 50 us
	10000,		// 100 us *default max
	20000,		// 200 us
	50000,		// 500 us
	100000,		// 1 ms
	200000,		// 2 ms
	500000,		// 5 ms
	1000000,	// 10 ms
	2000000,	// 20 ms
	5000000,	// 50 ms
	10000000,	// 100 ms
	20000000,	// 200 ms
	50000000,	// 500 ms
	100000000,	// 1 s
};
int PulseNoiseMin = 9; // selected minimal pulse
int PulseNoiseMax = 12; // selected minimal pulse

// PWM notes
//	system clock: 200 MHz
//	divider: 1
//	PWM clock frequency: 200 MHz
//	divide by TOP=2000 to sample rate: 200000kHz/2000 = 100 kHz
//	max. frequency of tone: 50 kHz
//	base wave frequency with step 1: 100000/2048 = 48.828125 Hz
//	increment of 440 Hz: 440*2048/100000 = 9.0112
//	sum divide clock of 440 Hz: 200000000/440 = 454545.4545
#define SINENOTE_FREQ	200000000 // system frequency in Hz
#define SINENOTE_FIRSTOCT FIRSTOCT // first octave, first note, C-10 = 0.015968357257Hz
#define SINENOTE_MIN	0	// minimal sine note
#define SINENOTE_MAX	(21*12+7) // maximal sine note is F#11 47.359286 kHz
#define SINENOTE_NUM	(SINENOTE_MAX - SINENOTE_MIN + 1) // number of sine notes
#define SINENOTE_TOP	2000	// max. value + 1
int SineNoteSel = 14*12 + 9;	// selected tone A4 // current selected sine note
int TriangleNoteSel = 14*12 + 9; // selected tone A4;	// current selected sine note
int SawtoothNoteSel = 14*12 + 9; // selected tone A4;	// current selected sine note

// sine decimal
#define SINEDECI_NUM 	59 // 59 entries (0.01 x9, 0.1 x9, 1 x9, 10 x9, 100 x9, 1K x9, 10K x5)
int SineDeciSel = 45;

u64 SineNoteInx = 0;	// current sample index with fraction (32 bits integer + 32 bits fraction)
u64 SineNoteInc = 1;	// sine note increment (32 bits integer + 32 bits fraction)

// frequency clock counters
u32 __attribute__((aligned(SLOWGEN_BYTES))) ClkCnt[SLOWGEN_WAITS];

u8 ClkCntOld = SET_NUM; // old generated pattern in ClkCnt

// real frequency
double FreqReq; // required frequency
double FreqReal; // real frequency
u64 ClkReal; // real clock
int ClkDiv; // real divider
u8 ModeReal; // real mode

// fill table of clock pulses (clk = max. 0xffffffff, or 0xfffffffe if frac != 0)
void FillClk(u32 clk, int frac, int num)
{
	int i;

	// set table
	for (i = 0; i < num; i++) ClkCnt[i] = clk;

	// fill fractions
	for (i = 0; i < frac; i++) ClkCnt[i*num/frac]++;
}

// generate all system frequencies in given range
void GenSysClk()
{
	u32 hz, vco;
	u16 fbdiv;
	u8 pd1, pd2;
	int i;
	int n = 0; // number of entries
	sPLL* pll;

	// fbdiv loop
	for (fbdiv = 16; fbdiv <= 320; fbdiv++)
	{
		// get current vco (crystal frequency = 12000000 Hz)
		vco = (u32)fbdiv * CRYSTAL;

		// check valid vco range (400 to 1600 MHz)
		if ((vco >= 400000000) && (vco <= 1600000000))
		{
			// pd1 loop
			for (pd1 = 7; pd1 >= 1; pd1--)
			{
				// pd2 loop
				for (pd2 = pd1; pd2 >= 1; pd2--)
				{
					// current output frequency
					hz = vco / (pd1 * pd2);

					// check valid range
					if ((hz >= SYSCLK_MIN) && (hz <= SYSCLK_MAX) && (n < SYSCLK_NUM))
					{
						// check if such frequency already exists
						pll = AllSysClk;
						for (i = n; i > 0; i--)
						{
							if (hz == pll->freq) break;
							pll++;
						}

						// new frequency
						if (i == 0)
						{
							pll->fbdiv = fbdiv;
							pll->pd1 = pd1;
							pll->pd2 = pd2;
							pll->freq = hz;
							pll->dfreq = (double)vco / (pd1 * pd2);
							n++;
						}
					}
				}
			}
		}
	}

	// number of entries
	AllSysClkNum = n;
}

// generate decimal frequencies (1 digit + zeros: 100M, 90M, 80M, 70M,...)
void GenDec()
{
	int n = 0;
	double* d = DecimalList;

	// generate 0.01 Hz
	u32 f = 1;
	u32 k = 1;
	do
	{
		*d = f*0.01;
		d++;
		n++;
		f += k;
	} while (f < 10);

	// generate 0.1 Hz
	f = 1;
	k = 1;
	do
	{
		*d = f*0.1;
		d++;
		n++;
		f += k;
	} while (f < 10);

	// generate 1 Hz
	f = 1; // 1 Hz
	k = 1; // 1 Hz
	int i = 9;
	do
	{
		*d = f;
		d++;
		n++;
		f += k;
		i--;
		if (i == 0)
		{
			k = f;
			i = 9;
		}
	} while (f <= 100000000);
	
	// set number of frequencies
	DecimalNum = n;
}

// generate tone frequencies (C-4 (1.02197Hz) .. B15 (1011474Hz))
void GenTone()
{
	int i;
	for (i = 0; i < NOTES; i++) ToneList[i] = C0 * pow(2.0, i/12.0);
	ToneNum = NOTES;
}

// stop all transfers
void StopAll()
{
	// abort DMA transfer
	DMA_Abort(GENERATOR_DMA);

	// stop state machine and clear FIFO
	PioSMDisable(GENERATOR_PIO, GENERATOR_SM);
	PioFifoClear(GENERATOR_PIO, GENERATOR_SM);

	// abort PWM 1
	PWM_Disable(PWM_GPIOTOSLICE(GENERATOR_GPIO1));
	PWM_IntDisable(PWM_GPIOTOSLICE(GENERATOR_GPIO1));
	PWM_IntClear(PWM_GPIOTOSLICE(GENERATOR_GPIO1));

	// abort PWM 2
	PWM_Disable(PWM_GPIOTOSLICE(GENERATOR_GPIO2));
	PWM_IntDisable(PWM_GPIOTOSLICE(GENERATOR_GPIO2));
	PWM_IntClear(PWM_GPIOTOSLICE(GENERATOR_GPIO2));

	// disable PWM interrupt
	NVIC_IRQDisable(IRQ_PWM_WRAP);

	// abort PWM 1
	PWM_Disable(PWM_GPIOTOSLICE(GENERATOR_GPIO1));
	PWM_IntDisable(PWM_GPIOTOSLICE(GENERATOR_GPIO1));
	PWM_IntClear(PWM_GPIOTOSLICE(GENERATOR_GPIO1));
	PWM_Reset(PWM_GPIOTOSLICE(GENERATOR_GPIO1));

	// abort PWM 2
	PWM_Disable(PWM_GPIOTOSLICE(GENERATOR_GPIO2));
	PWM_IntDisable(PWM_GPIOTOSLICE(GENERATOR_GPIO2));
	PWM_IntClear(PWM_GPIOTOSLICE(GENERATOR_GPIO2));
	PWM_Reset(PWM_GPIOTOSLICE(GENERATOR_GPIO2));

	// terminate all GPIOS
	int i;
	for (i = 0; i < OUTPUT_NUM; i++) GPIO_Init(OutputGPIO[i]);
}

// setup PIO program (clk = total period per whole loop, mode = frequency mode MODE_*)
void SetupPIO(u64 clk, u8 mode)
{
	int i, frac;

	// stop all transfers
	StopAll();

	// destroy old pattern
	ClkCntOld = SetInx;

	// restart PIO state machine to flush delay from OSR register
	// Without a reset, X will load old delay from OSR and may remain in delay for a long time
	PioSMRestart(GENERATOR_PIO, GENERATOR_SM);

	// configure GPIO for use by PIO
	PioSetupGPIO(GENERATOR_PIO, OutputGPIO[Output], 1);

	// set pin direction to output
	PioSetPinDir(GENERATOR_PIO, GENERATOR_SM, OutputGPIO[Output], 1, 1);

	// fast slew rate control
	GPIO_Fast(OutputGPIO[Output]);

	// set output strength to 12 mA (pin = 0..29)
	GPIO_Drive12mA(OutputGPIO[Output]);

	// set sideset
	PioSetupSideset(GENERATOR_PIO, GENERATOR_SM, OutputGPIO[Output], 1, False, False);

	// map state machine's OUT and MOV pins	
	PioSetupOut(GENERATOR_PIO, GENERATOR_SM, OutputGPIO[Output], 1);

	// join FIFO to send only
	PioSetFifoJoin(GENERATOR_PIO, GENERATOR_SM, PIO_FIFO_JOIN_TX);

	// PIO clock divider
	PioSetClkdiv(GENERATOR_PIO, GENERATOR_SM, GENERATOR_DIV*256);

	// shift right, autopull, pull threshold = 32 bits
	PioSetOutShift(GENERATOR_PIO, GENERATOR_SM, True, True, 32);

	// save setup
	ModeReal = mode;

	// fast precise mode
	if (mode == MODE_FASTPREC)
	{
		// configure main program instructions
		uint16_t ins[32]; // temporary buffer of program instructions
		memcpy(ins, &generator_long_program_instructions, count_of(generator_long_program_instructions)*sizeof(uint16_t)); // copy program into buffer

		// fill table of clock pulses
		if (clk > FAST_FRACNUM*16) clk = FAST_FRACNUM*16;
		if (clk < FAST_FRACNUM) clk = FAST_FRACNUM;
		ClkReal = clk;
		FillClk((u32)((clk >> FAST_FRAC) - 1), (int)(clk & FAST_MASK), FAST_FRACNUM);

		// update waits
		for (i = 0; i < count_of(generator_long_program_instructions); i++) ins[i] |= ClkCnt[i] << 8;

		// load PIO program
		PioLoadProg(GENERATOR_PIO, ins, count_of(generator_long_program_instructions), GENERATOR_OFF);

		// set wrap address
		PioSetWrap(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_long_wrap_target,
			GENERATOR_OFF + generator_long_wrap);

		// set start address
		PioSetAddr(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_long_wrap_target);
	}

	// fast stable mode
	else if (mode == MODE_FASTSTABLE)
	{
		// configure main program instructions
		uint16_t ins[32]; // temporary buffer of program instructions
		memcpy(ins, &generator_short_program_instructions, count_of(generator_short_program_instructions)*sizeof(uint16_t)); // copy program into buffer

		// update waits
		if (clk > 2*16) clk = 2*16;
		if (clk < 2) clk = 2;
		ClkReal = clk;
		ins[0] |= (u16)(clk/2-1) << 8;
		ins[1] |= (u16)((clk+1)/2-1) << 8;

		// load PIO program
		PioLoadProg(GENERATOR_PIO, ins, count_of(generator_short_program_instructions), GENERATOR_OFF);

		// set wrap address
		PioSetWrap(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_short_wrap_target,
			GENERATOR_OFF + generator_short_wrap);

		// set start address
		PioSetAddr(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_short_wrap_target);
	}

	// slow precise mode
	else if (mode == MODE_SLOWPREC)
	{
		// fill table of clock pulses
		if (clk > (u64)SLOW_FRACNUM*0xffffffff) clk = (u64)SLOW_FRACNUM*0xffffffff;
		if (clk < SLOW_FRACNUM*2) clk = SLOW_FRACNUM*2;
		ClkReal = clk;
		FillClk((u32)((clk >> SLOW_FRAC) - 2), (int)(clk & SLOW_MASK), SLOW_FRACNUM);

		// load PIO program
		PioLoadProg(GENERATOR_PIO, generator_slow_program_instructions,
			count_of(generator_slow_program_instructions), GENERATOR_OFF);

		// set wrap address
		PioSetWrap(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_slow_wrap_target,
			GENERATOR_OFF + generator_slow_wrap);

		// set start address
		PioSetAddr(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_slow_wrap_target);

		// initialize DMA of generator
		DMA_ConfigTrig(GENERATOR_DMA,			// channel
			ClkCnt,					// source buffer
			PIO_TXF(GENERATOR_PIO, GENERATOR_SM),	// write to PIO data port
			(u32)-1,				// number of samples to write
				// DMA_CTRL_SNIFF |		// not sniff
				// DMA_CTRL_BSWAP |		// not byte swap
				// DMA_CTRL_QUIET |		// not quiet
				DMA_CTRL_TREQ(PioGetDreq(GENERATOR_PIO, GENERATOR_SM, True)) | // data request
				DMA_CTRL_CHAIN(GENERATOR_DMA) |	// disable chaining
				//DMA_CTRL_RING_WRITE |		// wrap ring on write
				DMA_CTRL_RING_SIZE(SLOWGEN_BITS) | // ring size
				//DMA_CTRL_INC_WRITE |		// not increment write
				DMA_CTRL_INC_READ |		// increment read
				DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
				// DMA_CTRL_HIGH_PRIORITY |	// not high priority
				DMA_CTRL_EN);			// enable DMA
	}

	// slow stable mode
	else // MODE_SLOWSTABLE
	{
		// set counters
		if (clk > (u64)2*0xffffffff) clk = (u64)2*0xffffffff;
		if (clk < 2*2) clk = 2*2;
		ClkReal = clk;
		ClkCnt[0] = (u32)(clk/2-2);
		ClkCnt[1] = (u32)((clk+1)/2-2);

		// load PIO program
		PioLoadProg(GENERATOR_PIO, generator_slow_program_instructions,
			count_of(generator_slow_program_instructions), GENERATOR_OFF);

		// set wrap address
		PioSetWrap(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_slow_wrap_target,
			GENERATOR_OFF + generator_slow_wrap);

		// set start address
		PioSetAddr(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_slow_wrap_target);

		// initialize DMA of generator
		DMA_ConfigTrig(GENERATOR_DMA,			// channel
			ClkCnt,					// source buffer
			PIO_TXF(GENERATOR_PIO, GENERATOR_SM),	// write to PIO data port
			(u32)-1,				// number of samples to write
				// DMA_CTRL_SNIFF |		// not sniff
				// DMA_CTRL_BSWAP |		// not byte swap
				// DMA_CTRL_QUIET |		// not quiet
				DMA_CTRL_TREQ(PioGetDreq(GENERATOR_PIO, GENERATOR_SM, True)) | // data request
				DMA_CTRL_CHAIN(GENERATOR_DMA) |	// disable chaining
				//DMA_CTRL_RING_WRITE |		// wrap ring on write
				DMA_CTRL_RING_SIZE(3) | 	// ring size
				//DMA_CTRL_INC_WRITE |		// not increment write
				DMA_CTRL_INC_READ |		// increment read
				DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
				// DMA_CTRL_HIGH_PRIORITY |	// not high priority
				DMA_CTRL_EN);			// enable DMA
	}

	// start state machine
	PioSMEnable(GENERATOR_PIO, GENERATOR_SM);
}

// setup frequency
void Setup(double freq)
{
	u64 clk, clkbest;
	int i, mode, div;
	int ibest, modebest, divbest;
	double dist, distbest, dfreqbest, f, fbest;

	FreqReq = freq; // required frequency

	ibest = 0;
	clkbest = 2;
	modebest = MODE_SLOWSTABLE;
	distbest = 1000000000;
	dfreqbest = 1000000000;
	fbest = 1000000000;

	// loop frequencies
	sPLL* pll = AllSysClk;
	for (i = 0; i < AllSysClkNum; i++)
	{
		// get clock
		if (Stable)
		{
			// stable mode
			div = 1;
			mode = MODE_SLOWSTABLE; // slow stable mode
			clk = (u64)(pll->dfreq/freq + 0.5);
			if (clk > (u64)2*0xffffffff) clk = (u64)2*0xffffffff;
			if (clk <= 32) mode = MODE_FASTSTABLE;
			if (clk < 2) clk = 2;
		}
		else
		{
			// precise mode
			div = SLOWGEN_CYCLES;
			mode = MODE_SLOWPREC;
			clk = (u64)(SLOWGEN_CYCLES*pll->dfreq/freq + 0.5);
			if (clk > (u64)SLOW_FRACNUM*0xffffffff) clk = (u64)SLOW_FRACNUM*0xffffffff;
			if (clk < SLOWGEN_CYCLES*4) // slow generator can be more precise than fast generator
			{
				div = FASTGEN_CYCLES;
				mode = MODE_FASTPREC;
				clk = (u64)(FASTGEN_CYCLES*pll->dfreq/freq + 0.5);
				if (clk < FASTGEN_CYCLES*2) clk = FASTGEN_CYCLES*2;
			}

			// change to stable mode
			if ((clk & (div-1)) == 0)
			{
				div = 1;
				mode = MODE_SLOWSTABLE; // slow stable mode
				clk = (u64)(pll->dfreq/freq + 0.5);
				if (clk > (u64)2*0xffffffff) clk = (u64)2*0xffffffff;
				if (clk <= 32) mode = MODE_FASTSTABLE;
				if (clk < 2) clk = 2;
			}
		}

		// get difference
		f = div*pll->dfreq/clk;
		dist = f - freq;
		if (dist < 0) dist = -dist;

		// better
		if ((clk >= 2*div) && // clock must be valid, >= 2
			((dist < distbest) || // and better distance to required frequency
				((dist == distbest) && // or distance is equal
					((mode == MODE_SLOWSTABLE) || (mode == MODE_FASTSTABLE)) && // and only stable mode can overwrite
					((pll->dfreq < dfreqbest) || // and either frequency is lower
						((modebest == MODE_SLOWPREC) || (modebest == MODE_FASTPREC)) // or old mode is not stable
					)
				)
			)
		   )
		{
			ibest = i;
			clkbest = clk;
			modebest = mode;
			distbest = dist;
			dfreqbest = pll->dfreq;
			divbest = div;
			fbest = f;
		}

		pll++;
	}

	// stop all transfers
	StopAll();

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
	WaitVSync();		// interrupt from the image generator must not occur during a sys_clock change
#endif

	// setup system clock
	pll = &AllSysClk[ibest];
	ClockPllSysSetup(pll->fbdiv, pll->pd1, pll->pd2);

#if USE_PICOPADVGA
	// retune VGA
	VgaRetune(ClockGetHz(CLK_PLL_SYS));
#endif

	// setup PIO
	SetupPIO(clkbest, modebest);

	// real frequency
	FreqReal = fbest;
	ClkDiv = divbest;
}

// setup bit noise
void SetupBitNoise(u32 freq)
{
	FreqReq = freq;
	FreqReal = (double)freq/12000000*CRYSTAL;
	ClkReal = 100000000/freq;
	ClkDiv = 1;

	// stop all transfers
	StopAll();

	// generate random data
	int i;
	for (i = 0; i < SLOWGEN_WAITS; i++) ClkCnt[i] = RandU32();

	// destroy old pattern
	ClkCntOld = SetInx;

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
	WaitVSync();		// interrupt from the image generator must not occur during a sys_clock change
#endif

	// setup system clock to 100 MHz
	ClockPllSysFreq(100000);

#if USE_PICOPADVGA
	// retune VGA
	VgaRetune(ClockGetHz(CLK_PLL_SYS));
#endif

	// restart PIO state machine to flush delay from OSR register
	// Without a reset, X will load old delay from OSR and may remain in delay for a long time
	PioSMRestart(GENERATOR_PIO, GENERATOR_SM);

	// configure GPIO for use by PIO
	PioSetupGPIO(GENERATOR_PIO, OutputGPIO[Output], 1);

	// set pin direction to output
	PioSetPinDir(GENERATOR_PIO, GENERATOR_SM, OutputGPIO[Output], 1, 1);

	// fast slew rate control
	GPIO_Fast(OutputGPIO[Output]);

	// set output strength to 12 mA (pin = 0..29)
	GPIO_Drive12mA(OutputGPIO[Output]);

	// set sideset
	PioSetupSideset(GENERATOR_PIO, GENERATOR_SM, 0, 0, False, False);

	// map state machine's OUT and MOV pins	
	PioSetupOut(GENERATOR_PIO, GENERATOR_SM, OutputGPIO[Output], 1);

	// join FIFO to send only
	PioSetFifoJoin(GENERATOR_PIO, GENERATOR_SM, PIO_FIFO_JOIN_TX);

	// shift right, autopull, pull threshold = 32 bits
	PioSetOutShift(GENERATOR_PIO, GENERATOR_SM, True, True, 32);

	// fast or slow?
	if (freq <= 10000) // 1 Hz..10 kHz, slow mode
	{
		// PIO clock divider
		PioSetClkdiv(GENERATOR_PIO, GENERATOR_SM, (10000/freq)*256);

		// save setup
		ModeReal = MODE_SLOWSTABLE;

		// load PIO program
		PioLoadProg(GENERATOR_PIO, generator_slowbit_program_instructions, count_of(generator_slowbit_program_instructions), GENERATOR_OFF);

		// set wrap address
		PioSetWrap(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_slowbit_wrap_target,
			GENERATOR_OFF + generator_slowbit_wrap);

		// set start address
		PioSetAddr(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_slowbit_wrap_target);
	}

	// 100 kHz..100 MHz, fast mode
	else
	{
		// PIO clock divider
		PioSetClkdiv(GENERATOR_PIO, GENERATOR_SM, (10000/(freq/10000))*256);

		// save setup
		ModeReal = MODE_FASTSTABLE;

		// load PIO program
		PioLoadProg(GENERATOR_PIO, generator_fastbit_program_instructions, count_of(generator_fastbit_program_instructions), GENERATOR_OFF);

		// set wrap address
		PioSetWrap(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_fastbit_wrap_target,
			GENERATOR_OFF + generator_fastbit_wrap);

		// set start address
		PioSetAddr(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_fastbit_wrap_target);
	}

	// initialize DMA of generator
	DMA_ConfigTrig(GENERATOR_DMA,			// channel
		ClkCnt,					// source buffer
		PIO_TXF(GENERATOR_PIO, GENERATOR_SM),	// write to PIO data port
		(u32)-1,				// number of samples to write
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ(PioGetDreq(GENERATOR_PIO, GENERATOR_SM, True)) | // data request
			DMA_CTRL_CHAIN(GENERATOR_DMA) |	// disable chaining
			//DMA_CTRL_RING_WRITE |		// wrap ring on write
			DMA_CTRL_RING_SIZE(SLOWGEN_BITS) | // ring size
			//DMA_CTRL_INC_WRITE |		// not increment write
			DMA_CTRL_INC_READ |		// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY |	// not high priority
			DMA_CTRL_EN);			// enable DMA

	// start state machine
	PioSMEnable(GENERATOR_PIO, GENERATOR_SM);
}

// generate pulse noise
void GenPulseNoise()
{
	// get intervals
	u32 min = PulseNoiseList[PulseNoiseMin];
	u32 max = PulseNoiseList[PulseNoiseMax];
	if (min > max)
	{
		max = PulseNoiseList[PulseNoiseMin];
		min = PulseNoiseList[PulseNoiseMax];
	}

	// generate intervals
	min = min*2-2;
	max = max*2-2;
	int i;
	for (i = 0; i < SLOWGEN_WAITS; i++) ClkCnt[i] = RandU32MinMax(min, max);

	// destroy old pattern
	ClkCntOld = SetInx;
}

// setup pulse noise
void SetupPulseNoise()
{
	// stop all transfers
	StopAll();

	// generate pulse noise
	GenPulseNoise();

	// frequency and dividers
	u32 min = PulseNoiseList[PulseNoiseMin];
	u32 max = PulseNoiseList[PulseNoiseMax];
	if (min > max) min = PulseNoiseList[PulseNoiseMax];
	ClkDiv = 1;
	ClkReal = min*4;
	FreqReq = 50000000/min;
	FreqReal = (double)FreqReq/12000000*CRYSTAL;
	ModeReal = MODE_SLOWSTABLE;

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
	WaitVSync();		// interrupt from the image generator must not occur during a sys_clock change
#endif

	// setup system clock to 200 MHz
	ClockPllSysFreq(200000);

#if USE_PICOPADVGA
	// retune VGA
	VgaRetune(ClockGetHz(CLK_PLL_SYS));
#endif

	// restart PIO state machine to flush delay from OSR register
	// Without a reset, X will load old delay from OSR and may remain in delay for a long time
	PioSMRestart(GENERATOR_PIO, GENERATOR_SM);

	// configure GPIO for use by PIO
	PioSetupGPIO(GENERATOR_PIO, OutputGPIO[Output], 1);

	// set pin direction to output
	PioSetPinDir(GENERATOR_PIO, GENERATOR_SM, OutputGPIO[Output], 1, 1);

	// fast slew rate control
	GPIO_Fast(OutputGPIO[Output]);

	// set output strength to 12 mA (pin = 0..29)
	GPIO_Drive12mA(OutputGPIO[Output]);

	// set sideset
	PioSetupSideset(GENERATOR_PIO, GENERATOR_SM, OutputGPIO[Output], 1, False, False);

	// map state machine's OUT and MOV pins	
	PioSetupOut(GENERATOR_PIO, GENERATOR_SM, OutputGPIO[Output], 1);

	// join FIFO to send only
	PioSetFifoJoin(GENERATOR_PIO, GENERATOR_SM, PIO_FIFO_JOIN_TX);

	// PIO clock divider
	PioSetClkdiv(GENERATOR_PIO, GENERATOR_SM, GENERATOR_DIV*256);

	// shift right, autopull, pull threshold = 32 bits
	PioSetOutShift(GENERATOR_PIO, GENERATOR_SM, True, True, 32);

	// load PIO program
	PioLoadProg(GENERATOR_PIO, generator_slow_program_instructions,
		count_of(generator_slow_program_instructions), GENERATOR_OFF);

	// set wrap address
	PioSetWrap(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_slow_wrap_target,
		GENERATOR_OFF + generator_slow_wrap);

	// set start address
	PioSetAddr(GENERATOR_PIO, GENERATOR_SM, GENERATOR_OFF + generator_slow_wrap_target);

	// initialize DMA of generator
	DMA_ConfigTrig(GENERATOR_DMA,			// channel
		ClkCnt,					// source buffer
		PIO_TXF(GENERATOR_PIO, GENERATOR_SM),	// write to PIO data port
		(u32)-1,				// number of samples to write
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ(PioGetDreq(GENERATOR_PIO, GENERATOR_SM, True)) | // data request
			DMA_CTRL_CHAIN(GENERATOR_DMA) |	// disable chaining
			//DMA_CTRL_RING_WRITE |		// wrap ring on write
			DMA_CTRL_RING_SIZE(SLOWGEN_BITS) | // ring size
			//DMA_CTRL_INC_WRITE |		// not increment write
			DMA_CTRL_INC_READ |		// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY |	// not high priority
			DMA_CTRL_EN);			// enable DMA

	// start state machine
	PioSMEnable(GENERATOR_PIO, GENERATOR_SM);
}

// PWM note handler
void PWMNoteIrq()
{
	// prepare GPIO
	u8 gpio = OutputGPIO[Output];
	u8 slice = PWM_GPIOTOSLICE(gpio);

	// clear interrupt request
	PWM_IntClear(slice);

	// send sample
	u64 inx = SineNoteInx;	// sample index
	*PWM_CC(slice) = ClkCnt[(u32)(inx >> 32)]; // write compare value to PWM

	// shift sample index
	inx += SineNoteInc;
	if (inx >= ((u64)SLOWGEN_WAITS << 32)) inx -= ((u64)SLOWGEN_WAITS << 32);
	SineNoteInx = inx;
}

// setup PWM sine note
void SetupSineNote(double freq)
{
	int i;

	// stop all transfers
	StopAll();

	FreqReq = freq; // required frequency
	FreqReal = freq; // real frequency
	ModeReal = MODE_SLOWPREC;
	ClkDiv = 1<<20;
	ClkReal = double2uint64(SINENOTE_FREQ/freq*(1<<20)+0.5);

	// generate pattern
	if (SetInx != ClkCntOld) // check if pattern is already generated
	{
		ClkCntOld = SetInx;

		if ((SetInx == SET_SINENOTE) || (SetInx == SET_SINEDECI))
		{
			for (i = 0; i < SLOWGEN_WAITS; i++) ClkCnt[i] =
				double2uint(sin(i*PI2/SLOWGEN_WAITS)*(SINENOTE_TOP/2-1)+SINENOTE_TOP/2+0.5);
		}
		else if (SetInx == SET_TRIANGLE)
		{
			for (i = 0; i < SLOWGEN_WAITS/2; i++) ClkCnt[i] =
				i*(SINENOTE_TOP-1)/(SLOWGEN_WAITS/2);

			for (; i < SLOWGEN_WAITS; i++) ClkCnt[i] =
				(SLOWGEN_WAITS-i)*(SINENOTE_TOP-1)/(SLOWGEN_WAITS/2);
		}
		else // SET_SAWTOOTH
		{
			for (i = 0; i < SLOWGEN_WAITS; i++) ClkCnt[i] =
				i*(SINENOTE_TOP-1)/SLOWGEN_WAITS;
		}

		// duplicate comp value for both channels
		for (i = 0; i < SLOWGEN_WAITS; i++) ClkCnt[i] |= (ClkCnt[i] << 16);
	}

	// set GPIO function to PWM
	PWM_GpioInit(OutputGPIO[Output]);

	// fast slew rate control
	GPIO_Fast(OutputGPIO[Output]);

	// set output strength to 12 mA (pin = 0..29)
	GPIO_Drive12mA(OutputGPIO[Output]);

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
	WaitVSync();		// interrupt from the image generator must not occur during a sys_clock change
#endif

	// setup system clock to 200 MHz
	ClockPllSysFreq(SINENOTE_FREQ/1000);

#if USE_PICOPADVGA
	// retune VGA
	VgaRetune(ClockGetHz(CLK_PLL_SYS));
#endif

	// prepare increment
	SineNoteInx = 0;
	SineNoteInc = double2uint64(freq/SINENOTE_FREQ*SINENOTE_TOP*SLOWGEN_WAITS*(1ULL<<32) + 0.5);

	// prepare GPIO and slice
	u8 gpio = OutputGPIO[Output];
	u8 slice = PWM_GPIOTOSLICE(gpio);

	// set clock divider to 1
	PWM_ClkDiv(slice, 16);

	// set period
	PWM_Top(slice, SINENOTE_TOP-1);

	// set IRQ handler
	SetHandler(IRQ_PWM_WRAP, PWMNoteIrq);
	NVIC_IRQEnable(IRQ_PWM_WRAP);

	// interrupt enable
	PWM_IntEnable(slice);

	// enable PWM
	PWM_Enable(slice);
}

// set new frequency
void SetFreq()
{
	// set selected frequency
	if (SetInx == SET_SINEDECI)
		SetupSineNote(DecimalList[SineDeciSel]);
	else if (SetInx == SET_SINENOTE)
		SetupSineNote(ToneList[SineNoteSel]);
	else if (SetInx == SET_TRIANGLE)
		SetupSineNote(ToneList[TriangleNoteSel]);
	else if (SetInx == SET_SAWTOOTH)
		SetupSineNote(ToneList[SawtoothNoteSel]);
	else if (SetInx == SET_PULSENOISE)
		SetupPulseNoise();
	else if (SetInx == SET_BITNOISE)
		SetupBitNoise(BitNoiseList[BitNoiseSel]);
	else if (SetInx == SET_CUSTOM)
	{
		u64 c = Custom;
		if (c < CUSTOM_MIN) c = CUSTOM_MIN;
		if (c > CUSTOM_MAX) c = CUSTOM_MAX;
		Setup((double)(c * 0.000001));
	}
	else if (SetInx == SET_NOTE)
		Setup(ToneList[ToneSel]);
	else
		Setup(DecimalList[DecimalSel]);
}

#define BUFN	50
char Buf[BUFN];

// display menu and set frequency
void DispMenu()
{
	// mode
	DrawTextBg("KEY_A/KEY_B mode: ", 0, 0, COL_WHITE, COL_BLACK);
	DrawTextBg(SetText[SetInx], 18*8, 0, COL_YELLOW, COL_BLACK);
	
	// prefer
//	DrawTextBg("KEY_B prefer: ", 0, 16, COL_WHITE, COL_BLACK);
//	DrawTextBg(Stable ? StableOn : StableOff, 14*8, 16, COL_YELLOW, COL_BLACK);

	// output
	DrawTextBg("KEY_X output: ", 0, 20, COL_WHITE, COL_BLACK);
	DrawTextBg(OutputText[Output], 14*8, 20, COL_YELLOW, COL_BLACK);

#define REQY 55
	// help
	if (SetInx == SET_CUSTOM)
		DrawTextBg("LFT/RGH shift, UP/DN set, UP+DN clear: ", 0, REQY, COL_WHITE, COL_BLACK);
	else if (SetInx == SET_PULSENOISE)
		DrawTextBg("UP/DOWN minimum, LEFT/RIGHT maximum:   ", 0, REQY, COL_WHITE, COL_BLACK);
	else
		DrawTextBg("UP/DOWN select:                        ", 0, REQY, COL_WHITE, COL_BLACK);

	// display selected frequency
	if (SetInx == SET_CUSTOM)
	{
		MemPrint(Buf, BUFN, "%018.6'fHz  ", (double)(Custom*0.000001));

		// grouping correction
		if (Buf[0] == '0')
		{
			Buf[0] = ' ';
			if ((Buf[1] == '0') && (Buf[2] == '0'))
			{
				Buf[3] = '\'';

				if ((Buf[4] == '0') && (Buf[5] == '0') && (Buf[6] == '0'))
				{
					Buf[7] = '\'';
				}
			}
		}
	}
	else if (SetInx == SET_NOTE)
		MemPrint(Buf, BUFN, "F=%.8THz %s%d        ", ToneList[ToneSel], NoteName[ToneSel % 12], ToneSel/12+FIRSTOCT);
	else if (SetInx == SET_SINENOTE)
		MemPrint(Buf, BUFN, "F=%.8THz %s%d        ", ToneList[SineNoteSel], NoteName[SineNoteSel % 12], SineNoteSel/12+SINENOTE_FIRSTOCT);
	else if (SetInx == SET_TRIANGLE)
		MemPrint(Buf, BUFN, "F=%.8THz %s%d        ", ToneList[TriangleNoteSel], NoteName[TriangleNoteSel % 12], TriangleNoteSel/12+SINENOTE_FIRSTOCT);
	else if (SetInx == SET_SAWTOOTH)
		MemPrint(Buf, BUFN, "F=%.8THz %s%d        ", ToneList[SawtoothNoteSel], NoteName[SawtoothNoteSel % 12], SawtoothNoteSel/12+SINENOTE_FIRSTOCT);
	else if (SetInx == SET_BITNOISE)
		MemPrint(Buf, BUFN, "Bit Noise %.1THz             ", (double)BitNoiseList[BitNoiseSel]);
	else if (SetInx == SET_PULSENOISE)
		MemPrint(Buf, BUFN, "Pulse %.1Ts - %.1Ts          ", (double)PulseNoiseList[PulseNoiseMin]*1e-8, (double)PulseNoiseList[PulseNoiseMax]*1e-8);
	else if (SetInx == SET_SINEDECI)
		MemPrint(Buf, BUFN, "F=%.8THz          ", DecimalList[SineDeciSel]);
	else
		MemPrint(Buf, BUFN, "F=%.8THz          ", DecimalList[DecimalSel]);
	DrawTextBg2(Buf, 0, REQY+14, COL_YELLOW, COL_BLACK);

	if (SetInx == SET_PULSENOISE)
		MemPrint(Buf, BUFN, "F=%.8THz         ", FreqReq);
	else
		MemPrint(Buf, BUFN, "T=%.8Ts          ", 1.0/FreqReq);
	DrawTextBgW(Buf, 0, REQY+44, COL_YELLOW, COL_BLACK);

	// draw custom cursor
	if (SetInx == SET_CUSTOM)
	{
		DrawRect(0, REQY+42, WIDTH, 2, COL_BLACK);
		DrawRect(CustomX[CustomPos]*16, REQY+42, 16, 2, COL_RED);
	}

#define REALY 134
	// display real frequency
	DrawTextBg("Real frequency:", 0, REALY, COL_WHITE, COL_BLACK);
	MemPrint(Buf, BUFN, "F=%.8THz          ", FreqReal);
	DrawTextBg2(Buf, 0, REALY+14, COL_GREEN, COL_BLACK);

	MemPrint(Buf, BUFN, "T=%.8Ts          ", 1.0/FreqReal);
	DrawTextBgW(Buf, 0, REALY+44, COL_GREEN, COL_BLACK);

	// display info
	MemPrint(Buf, BUFN, "crystal=%'uHz div=%.5#f           ", CRYSTAL, (double)ClkReal/ClkDiv);
	DrawTextBg(Buf, 0, 240-2*15, COL_GRAY, COL_BLACK);

	MemPrint(Buf, BUFN, "sys_clk=%'uHz mode=%s       ", ClockGetHz(CLK_SYS), ModeName[ModeReal]);
	DrawTextBg(Buf, 0, 240-15, COL_GRAY, COL_BLACK);

	// display update
	DispUpdate();
}

// main function
int main()
{
	int i;
	char ch;

	// set higher voltage to support stable higher frequencies
	VregSetVoltage(VREG_VOLTAGE_1_30);

	// generate all system frequencies in given range
	GenSysClk();
	//printf("AllSysClkNum = %d\n", AllSysClkNum); while(True) {}

	// generate decimal frequencies (1 digit + zeros: 100M, 90M, 80M, 70M,...)
	GenDec();
	//printf("DecimalNum = %d\n", DecimalNum); while(True) {}

	// generate tone frequencies
	GenTone();
	//printf("ToneNum = %d\n", ToneNum); while(True) {}

	// reset PWM to default state
	PWM_Reset(PWM_GPIOTOSLICE(GENERATOR_GPIO1));
	PWM_Reset(PWM_GPIOTOSLICE(GENERATOR_GPIO2));

	// initialize PIO
	PioInit(GENERATOR_PIO);

	// set new frequency
	SetFreq();

	// display menu
	DispMenu();

	// main loop
	while (True)
	{
		// regenerate random data
		if (SetInx == SET_BITNOISE)
		{
			for (i = 0; i < SLOWGEN_WAITS; i++) ClkCnt[i] = RandU32();
		}

		// regenerate pulse noise
		if (SetInx == SET_PULSENOISE) GenPulseNoise();

		// refresh DMA transfer
		if (DMA_IsBusy(GENERATOR_DMA) && ((u32)DMA_GetCount(GENERATOR_DMA) < 0x40000000))
		{
			DMA_Abort(GENERATOR_DMA);
			DMA_SetCountTrig(GENERATOR_DMA, (u32)-1);
		}

		// key
		ch = KeyGet();
		switch (ch)
		{
		// previous set
		case KEY_A:
			if (SetInx == 0)
				SetInx = SET_NUM-1;
			else
				SetInx--;
			SetFreq();
			DispMenu();
			break;

		// next set
		case KEY_B:
			SetInx++;
			if (SetInx == SET_NUM) SetInx = 0;
			SetFreq();
			DispMenu();
			break;

/*
		// select prefer
		case KEY_B:
			Stable = !Stable;
			SetFreq();
			DispMenu();
			break;
*/
		// select output
		case KEY_X:
			Output++;
			if (Output >= OUTPUT_NUM) Output = 0;
			SetFreq();
			DispMenu();
			break;	

		// left
		case KEY_LEFT:
			if (SetInx == SET_PULSENOISE)
			{
				if (PulseNoiseMax > 0)
				{
					PulseNoiseMax--;
					SetFreq();
					DispMenu();
				}
			}
			else if (SetInx == SET_CUSTOM)
			{
				if (CustomPos > 0)
				{
					CustomPos--;
					DispMenu();
				}
			}
			break;

		// right
		case KEY_RIGHT:
			if (SetInx == SET_PULSENOISE)
			{
				if (PulseNoiseMax < PULSENOISE_NUM-1)
				{
					PulseNoiseMax++;
					SetFreq();
					DispMenu();
				}
			}
			else if (SetInx == SET_CUSTOM)
			{
				if (CustomPos < CUSTOM_NUM-1)
				{
					CustomPos++;
					DispMenu();
				}
			}
			break;

		// up
		case KEY_UP:
			switch (SetInx)
			{
			case SET_SINEDECI:
				if (SineDeciSel < SINEDECI_NUM-1)
				{
					SineDeciSel++;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_SINENOTE:
				if (SineNoteSel < SINENOTE_NUM-1)
				{
					SineNoteSel++;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_TRIANGLE:
				if (TriangleNoteSel < SINENOTE_NUM-1)
				{
					TriangleNoteSel++;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_SAWTOOTH:
				if (SawtoothNoteSel < SINENOTE_NUM-1)
				{
					SawtoothNoteSel++;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_PULSENOISE:
				if (PulseNoiseMin < PULSENOISE_NUM-1)
				{
					PulseNoiseMin++;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_BITNOISE:
				if (BitNoiseSel < BITNOISE_NUM-1)
				{
					BitNoiseSel++;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_CUSTOM:
				if (KeyPressed(KEY_DOWN))
					Custom = 0;
				else
				{
					if (Custom < CUSTOM_MAX) Custom += CustomInc[CustomPos];
				}

				SetFreq();
				DispMenu();
				break;

			case SET_NOTE:
				if (ToneSel < ToneNum-1)
				{
					ToneSel++;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_DECIMAL:
				if (DecimalSel < DecimalNum-1)
				{
					DecimalSel++;
					SetFreq();
					DispMenu();
				}
				break;
			}
			break;

		// down
		case KEY_DOWN:
			switch (SetInx)
			{
			case SET_SINEDECI:
				if (SineDeciSel > 0)
				{
					SineDeciSel--;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_SINENOTE:
				if (SineNoteSel > 0)
				{
					SineNoteSel--;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_TRIANGLE:
				if (TriangleNoteSel > 0)
				{
					TriangleNoteSel--;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_SAWTOOTH:
				if (SawtoothNoteSel > 0)
				{
					SawtoothNoteSel--;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_PULSENOISE:
				if (PulseNoiseMin > 0)
				{
					PulseNoiseMin--;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_BITNOISE:
				if (BitNoiseSel > 0)
				{
					BitNoiseSel--;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_CUSTOM:
				if (KeyPressed(KEY_UP))
					Custom = 0;
				else
				{
					if (Custom >= CustomInc[CustomPos]) Custom -= CustomInc[CustomPos];
				}
				SetFreq();
				DispMenu();
				break;

			case SET_NOTE:
				if (ToneSel > 0)
				{
					ToneSel--;
					SetFreq();
					DispMenu();
				}
				break;

			case SET_DECIMAL:
				if (DecimalSel > 0)
				{
					DecimalSel--;
					SetFreq();
					DispMenu();
				}
				break;
			}
			break;

		// quit program
		case KEY_Y:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
