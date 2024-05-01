
// ****************************************************************************
//
//                       IBM PC Emulator - Sound
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

//  crystal frequency: 14.31816 MHz (this is IBM calculation; more precise should be 14.31818 MHz)
//  clock divisor: 12
//  clock: 14.31816/12 = 1.19318 MHz (used as source of sound divider)
//  sound divisor: 0..65535 (0 = 65536)

// sound counter 2 setup (write on port 43h)
//   bit 7..6: select counter
//		0 ... timer IRQ 0, INT 08h (not supported by the emulator)
//		1 ... memory refresh (not supported by the emulator)
//		2 ... speaker output
//		3 ... read back counter
// if counter selected (0..2):
//   bit 5..4: divider access mode
//		0 ... read counter using latch (read low and then high)
//		1 ... read/write only low byte of divider
//		2 ... read/write only high byte of divider
//		3 ... read/write low and then high byte of divider
//   bit 3..1: counter mode
//		0 ... (interrupt) interupt on end of counting (counter = 0), output goes H->L and stop, write divider will start new counting
//		1 ... (monostable generator) GATE input L->H starts counting (output H->L), after end of counting output back L->H
//		2,6 ... (divider, rate generator) output with pulse width given by divider value; GATE input L sets OUT to H, GATE L->H starts new counting
//		3,7 ... (divider2, square generator) as mode 2, pulse width = divider/2 (odd number: H state divider/2+1), total period given by divider
//		4 ... (software strobing) GATE input L->H start counting, after end one output pulse H->L->H
//		5 ... (hardware strobing)
//   bit 0: 0=binar counting, 1=BCD counting (not supported by the emulator)
// if read back (3):
//   bit 5..4: what to read
//		0 ... counter status, then value
//		1 ... counter value
//		2 ... counter status
//   bit 3: select counter 2
//   bit 2: select counter 1
//   bit 1: select counter 0

// normal sound counter 2 settings:
//   0b6h = 10 11 011 0 (counter 2, read/write LOW and HIGH, mode divider2, binar)

// To set frequency in Hz in divider2 mode: div = 1193182 / freq    (1193182 = 1234DEh)

// Write to port 61h; emulator uses only bit 0 and 1:
//   bit 0: GATE of counter 2 input (1=counting is enabled)
//   bit 1: 1=enable output to speaker

u8 PC_SoundOut = PC_SND_NO;	// current sound output
u8 PC_SoundOutSave = PC_SND_NO;	// current sound output - save
// GlobalSoundOff - global sound OFF
u8 PC_SoundMode = 0xb6; // sound counter 2 setup, port 43h
u16 PC_SoundDiv = 1193;	// sound counter divider, port 42h (default: 1 kHz)
u8 PC_SoundGate = 0; // gate of sound counter 2, port 61h (bit 0: counting enable, bit 1: output to speaker enable)
Bool PC_SoundHigh = False; // flip/flop - read/write high byte
u16 PC_SoundLatch = 1193; // latch

// PicoPad:
//  - output to PWMSND_GPIO port (PicoPad: GPIO 15)
//  - use default PWM sound: USE_PWMSND = number of channels (default 4)
//  - PWM slice PWMSND_SLICE (PicoPad: 7)
//  - PWM channel PWMSND_CHAN (PicoPad: chan. B)
//  - default sound rate SOUNDRATE (default: 22050 Hz)
//  - PWM top value PWMSND_TOP (default: 255, values 0..255)
//  - PWM clock PWMSND_CLOCK (= SOUNDRATE*(PWMSND_TOP+1); default 22050*256 = 5644800)
//  - 12-bit PWM divider, use PWM_Clock (= CurrentFreq[CLK_SYS]*16 / freq; default 125000000*16/5644800 = 354 (/16 = 22.144, INT=22, FRAC=2)

// setup sound output to default state (should be called on program start)
void PC_SoundDef()
{
	PC_SoundOut = PC_SND_NO; // current sound output
	PC_SoundOutSave = PC_SND_NO; // current sound output - save
	PC_SoundMode = 0xb6; // sound counter 2 setup, port 43h
	PC_SoundDiv = 1193;	// sound counter divider, port 42h (default: 1 kHz)
	PC_SoundGate = 0; // gate of sound counter 2, port 61h (bit 0: counting enable, bit 1: output to speaker enable)
	PC_SoundHigh = False; // flip/flop - read/write high byte
	PC_SoundLatch = 1193; // latch
}

// update enable/disable sound output (after changing GlobalSoundOff; PC speaker: after changing PC_SoundGate)
void PC_SoundOnUpdate()
{
	// output to speaker
	if (PC_SoundOut == PC_SND_SPK)
	{
		// enable/disable PWM slice
		if (!GlobalSoundOff && ((PC_SoundGate & 3) == 3))
			PWM_Enable(PWMSND_SLICE);
		else
			PWM_Disable(PWMSND_SLICE);
	}
}

// update PC speaker frequency (PC speaker: after changing PC_SoundMode or PC_SoundDiv)
void PC_SoundFreqUpdate()
{
	// mute
	if (GlobalSoundOff) return;

	// output to speaker
	if (PC_SoundOut == PC_SND_SPK)
	{
		// modes supported by emulator:
		//  2,6 ... (divider, rate generator) output with pulse width given by divider value; GATE input L sets OUT to H, GATE L->H starts new counting
		if ((PC_SoundMode & B1) == 0) // rate generator
		{
			// set period
			u32 n = PC_SoundDiv*2-1;
			if (PWM_GetTop(PWMSND_SLICE) != n)
			{
				PWM_Disable(PWMSND_SLICE);
				PWM_Count(PWMSND_SLICE, 0);
				PWM_Top(PWMSND_SLICE, n);

				// set compare value, on which flip HIGH->LOW
				PWM_Comp(PWMSND_SLICE, PWMSND_CHAN, PC_SoundDiv);
			}
		}

		//  3,7 ... (divider2, square generator) as mode 2, pulse width = divider/2 (odd number: H state divider/2+1), total period given by divider
		else // square generator
		{
			// set period
			u32 n = PC_SoundDiv-1;
			if (PWM_GetTop(PWMSND_SLICE) != n)
			{
				PWM_Disable(PWMSND_SLICE);
				PWM_Count(PWMSND_SLICE, 0);
				PWM_Top(PWMSND_SLICE, n);

				// set compare value, on which flip HIGH->LOW
				PWM_Comp(PWMSND_SLICE, PWMSND_CHAN, PC_SoundDiv >> 1);
			}
		}

		// enable sound output
		PC_SoundOnUpdate();
	}
}

// initialize sound output to PC_SND_* (should be called on start of sound)
void PC_SoundInit(int out)
{
	// sound is already initialize
	if (out == PC_SoundOut) return;

	// sound is OFF
	if (GlobalSoundOff)
	{
		PC_SoundOut = out;
		return;
	}

	// terminate current sound output
	PC_SoundTerm();

	// output to speaker
	if (out == PC_SND_SPK)
	{
		PC_SoundOut = PC_SND_SPK;

		// reset PWM to default state
		PWM_Reset(PWMSND_SLICE);

		// set GPIO function to PWM
		PWM_GpioInit(PWMSND_GPIO);

		// set clock divider (PWM clock frequency 1.193182 MHz):
		//   on 125 MHz: divider 1676, INT = 104, FRAC = 12, real freq = 1193317 Hz
		//   on 252 MHz: divider 3379, INT = 211, FRAC = 3, real freq = 1193252 Hz
		PWM_Clock(PWMSND_SLICE, PC_SND_CLK);

		// update PC speaker frequency, update enable/disable sound output
		PC_SoundFreqUpdate();
	}
}

// terminate sound output (should be called on program pause or termination)
void PC_SoundTerm()
{
	// terminate output to speaker
	if (PC_SoundOut == PC_SND_SPK)
	{
		// disable PWM
		PWM_Disable(PWMSND_SLICE);

		// reset PWM to default state
		PWM_Reset(PWMSND_SLICE);

		// set GPIO function to default
		GPIO_Init(PWMSND_GPIO);
	}

	// no sound
	PC_SoundOut = PC_SND_NO;
}
