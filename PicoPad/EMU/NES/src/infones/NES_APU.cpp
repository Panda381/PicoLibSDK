
// ****************************************************************************
//
//                             NES APU sound library
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

// >>> Keep tables and functions in RAM (without "const") for faster access.

#include "../../include.h"

// ----------------------------------------------------------------------------
//                            Length counter
// ----------------------------------------------------------------------------
// Used in pulse, triangle and noise generators.

/*
typedef struct {
	u8	cnt;		// 0x00 length counter (0 = mute channel)
	u8	en;		// 0x01: <>0 enabled
	u8	halt;		// 0x02: <>0 halt counting
	u8	res;		// 0x03: ... reserved (align)
} sApuLen;
*/

// active time length
u8 NES_ApuLenTab[32] = {
	10,	// 0x00 sixteenth note, note with base length 10 (4/4 at 90 bpm)
	254,	// 0x01 linear length 254
	20,	// 0x02 eighth note, note with base length 10 (4/4 at 90 bpm)
	2,	// 0x03 linear length 2
	40,	// 0x04 quarter note, note with base length 10 (4/4 at 90 bpm)
	4,	// 0x05 linear length 4
	80,	// 0x06 half note, note with base length 10 (4/4 at 90 bpm)
	6,	// 0x07 linear length 6
	160,	// 0x08 whole note, note with base length 10 (4/4 at 90 bpm)
	8,	// 0x09 linear length 8
	60,	// 0x0A dotted quarter note, note with base length 10 (40 times 1/2, 4/4 at 90 bpm)
	10,	// 0x0B linear length 10
	14,	// 0x0C eighth note triplet, note with base length 10 (40 times 1/3, 4/4 at 90 bpm)
	12,	// 0x0D linear length 12
	26,	// 0x0E quarter note triplet, note with base length 10 (80 times 1/3, 4/4 at 90 bpm)
	14,	// 0x0F linear length 14
        12,	// 0x10 sixteenth note, note with base length 12 (4/4 at 75 bpm)
	16,	// 0x11 linear length 16
	24,	// 0x12 eighth note, note with base length 12 (4/4 at 75 bpm)
	18,	// 0x13 linear length 18
	48,	// 0x14 quarter note, note with base length 12 (4/4 at 75 bpm)
	20,	// 0x15 linear length 20
	96,	// 0x16 half note, note with base length 12 (4/4 at 75 bpm)
	22,	// 0x17 linear length 22
	192,	// 0x18 whole note, note with base length 12 (4/4 at 75 bpm)
	24,	// 0x19 linear length 24
	72,	// 0x1A dotted quarter note, note with base length 12 (48 times 1/2, 4/4 at 75 bpm)
	26,	// 0x1B linear length 26
	16,	// 0x1C eighth note triplet, note with base length 12 (48 times 1/3, 4/4 at 75 bpm)
	28,	// 0x1D linear length 28
	32,	// 0x1E quarter note triplet, note with base length 12 (96 times 1/3, 4/4 at 75 bpm)
	30,	// 0x1F linear length 30
};

// clock length counter (clocked by half frame)
void FASTCODE NOFLASH(NES_ApuLenClock)(sApuLen* c)
{
	// if not zero and if not halt
	if ((c->cnt != 0) && (c->halt == 0)) c->cnt--;
}

// check if output is enabled: length counter must not be zero
INLINE Bool NES_ApuLenOut(sApuLen* c) { return c->cnt != 0; }

// set enabled (<>0 enabled)
void FASTCODE NOFLASH(NES_ApuLenSetEn)(sApuLen* c, u8 en)
{
	// save enabled
	c->en = en;

	// if disabled, reset counter to 0
	if (en == 0) c->cnt = 0;
}

// set halt counting (<>0 halt counting)
INLINE void NES_ApuLenSetHalt(sApuLen* c, u8 halt) { c->halt = halt; }

// set length counter (len = 0..31)
void FASTCODE NOFLASH(NES_ApuLenSetCnt)(sApuLen* c, u8 len)
{
	// must be enabled
	if (c->en != 0) c->cnt = NES_ApuLenTab[len];
}

// ----------------------------------------------------------------------------
//                             Sweep unit
// ----------------------------------------------------------------------------
// Used in pulse generators.

/*
typedef struct {
	u8	cnt;		// 0x00: counter
	u8	period;		// 0x01: divider's period 0..7
	u8	shift;		// 0x02: shift count 0..7 (0=sweep unit is disabled)
	u8	comp;		// 0x03: <>0 ones' complement mode (pulse generator 1), =0 negate mode (pulse generator 2)
	u8	en;		// 0x04: <>0 enabled
	u8	reload;		// 0x05: <>0 reload request
	u8	neg;		// 0x06: <>0 decrease period, =0 increase period
	u8	res;		// 0x07: ... reserved (align)
	u16*	target;		// 0x08: target channel 11-bit period ('timer')
} sApuSweep;
*/

// get next target period
u16 FASTCODE NOFLASH(NES_ApuSweepTarget)(sApuSweep* w)
{
	// get target period
	u16 timer = *(w->target);

	// timer 0 will always produce 0, but negate with ones' complement would produce -1
	if (timer == 0) return 0;

	// prepare change amount
	u16 delta = timer >> w->shift;

	// negate increment
	if (w->neg != 0)
	{
		if (w->comp == 0)
			// negate mode (pulse generator 2)
			delta = -delta;
		else
		{
			// ones' complement mode (pulse generator 1)
			delta = ~delta; // or delta = - delta - 1;

			// clamp to zero (it will underflow if shift = 0)
			if (w->shift == 0) delta = -timer;
		}
	}

	// update period
	return timer + delta;
}

// check if output is enabled: target period must be in valid range
Bool FASTCODE NOFLASH(NES_ApuSweepOut)(sApuSweep* w)
{
	// get shifted target period
	u16 timer = *(w->target);		// get current target period (must be >= 8)
	u16 timer2 = NES_ApuSweepTarget(w);	// get next target period (must be <= 0x7FF)

	// check target period range
	return (timer >= 8) && (timer2 <= 0x7ff);
}

// clock sweep counter (clocked by half frame)
void FASTCODE NOFLASH(NES_ApuSweepClock)(sApuSweep* w)
{
	// get shifted target period
	u16 timer = *(w->target);		// get current target period (must be >= 8)
	u16 timer2 = NES_ApuSweepTarget(w);	// get next target period (must be <= 0x7FF)

	// update channel period if allowed
	if ((w->cnt == 0) && (w->en != 0) && (w->shift != 0) && (timer >= 8) && (timer2 <= 0x7ff))
	{
		*(w->target) = timer2;
	}

	// reload if needed
	if ((w->cnt == 0) || (w->reload != 0))
	{
		// reload counter
		w->cnt = w->period;

		// reset reload flag
		w->reload = 0;
	}
	else
		// decrement counter
		w->cnt--;
}

// set mode (<>0 one complement mode - pulse generator 1, =0 negate mode - pulse generator 2)
INLINE void NES_ApuSweepSetComp(sApuSweep* w, u8 comp) { w->comp = comp; }

// set divider's period 0..7
INLINE void NES_ApuSweepSetPeriod(sApuSweep* w, u8 per) { w->period = per; }

// set shift count 0..7 (0=sweep unit is disabled)
INLINE void NES_ApuSweepSetShift(sApuSweep* w, u8 shift) { w->shift = shift; }

// set enabled (<>0 enabled)
INLINE void NES_ApuSweepSetEn(sApuSweep* w, u8 en) { w->en = en; }

// set negate (<>0 decrease period, =0 increase period)
INLINE void NES_ApuSweepSetNeg(sApuSweep* w, u8 neg) { w->neg = neg; }

// set reload request
INLINE void NES_ApuSweepReload(sApuSweep* w) { w->reload = 1; }

// ----------------------------------------------------------------------------
//                           Envelope unit
// ----------------------------------------------------------------------------
// Used in pulse and noise generators.

/*
typedef struct {
	u8	cnt;		// 0x00: counter
	u8	decay;		// 0x01: decay level counter 15..0
	u8	pervol;		// 0x02: divider period or constant volume 0..15
	u8	start;		// 0x03: <>0 start flag
	u8	loop;		// 0x04: <>0 loop mode
	u8	mode;		// 0x05: <>0 constant volume mode, =0 envelope volume mode
	u8	res[2];		// 0x06: ... reserved (align)
} sApuEnv;
*/

// clock envelope counter (clocked by quarter frame)
void FASTCODE NOFLASH(NES_ApuEnvClock)(sApuEnv* e)
{
	// if started
	if (e->start != 0)
	{
		// reset start flag
		e->start = 0;

		// reset decay counter
		e->decay = 15;

		// reload counter
		e->cnt = e->pervol;
		return;
	}

	// counting counter if not zero
	if (e->cnt != 0)
	{
		// decrement counter
		e->cnt--;
		return;
	}

	// counter is zero - reload counter
	e->cnt = e->pervol;

	// count decay level counter
	if (e->decay != 0)
	{
		e->decay--;
		return;
	}

	// if loop, reload decay counter
	if (e->loop != 0)
	{
		// reload decay counter
		e->decay = 15;
	}
}

// start envelope
INLINE void NES_ApuEnvStart(sApuEnv* e) { e->start = 1; }

// get volume (0..15)
INLINE u8 NES_ApuEnvGetVol(sApuEnv* e) { return (e->mode == 0) ? e->decay : e->pervol; }

// set divider period or constant volume 0..15
INLINE void NES_ApuEnvSetPerVol(sApuEnv* e, u8 pervol) { e->pervol = pervol; }

// set loop mode (<>0 loop mode)
INLINE void NES_ApuEnvSetLoop(sApuEnv* e, u8 loop) { e->loop = loop; }

// set volume mode (<>0 constant volume mode, =0 envelope volume mode)
INLINE void NES_ApuEnvSetMode(sApuEnv* e, u8 mode) { e->mode = mode; }

// ----------------------------------------------------------------------------
//                           Pulse generator
// ----------------------------------------------------------------------------

/*
typedef struct {
	u8		waveform;	// 0x00: duty waveform
	u8		step;		// 0x01: sequencer current step 0..7 (increasing)
	u8		res[2];		// 0x02: ... reserved (align)
	u16		timer;		// 0x04: timer period 0..2047 (11 bits)
	u16		cnt;		// 0x06: counter (loaded with timer*2+1)
	sApuLen		len;		// 0x08: (4) length counter
	sApuSweep	sweep;		// 0x0C: (12) sweep unit
	sApuEnv		env;		// 0x18: (8) envelope unit
} sApuPulse;
*/

// duty wave forms
u8 NES_ApuPulseWave[4] = {
        0b00000010,     // 12.5%
        0b00000110,     // 25%
        0b00011110,     // 50%
        0b11111001      // 75%
};

// initialize pulse generator
//   comp ... <>0 ones' complement mode (pulse generator 1), =0 negate mode (pulse generator 2)
void FASTCODE NOFLASH(NES_ApuPulseInit)(sApuPulse* p, u8 comp)
{
	// set sweep complement mode
	p->sweep.comp = comp;

	// set sweep target period
	p->sweep.target = &p->timer;

	// set default waveform
	p->waveform = NES_ApuPulseWave[0];
}

// clock pulse generator counter by CPU clocks (2 CPU clocks = 1 APU clock)
void FASTCODE NOFLASH(NES_ApuPulseClock)(sApuPulse* p, u16 clk)
{
	u16 clk2;

	// process all clocks
	while (clk > 0)
	{
		// if counter reaches 0, reload counter
		if (p->cnt == 0)
		{
			// reload counter
			//   One step of sequencer is "timer+1" APU cycles or "2*(timer+1)" CPU cycles
			//   "<< 1" because timer is in APU cycles
			//   "+ 1" because 1 additional step on 0
			//   Example for t = 2:
			//     3 APU cycles: 2 - 1 - 0
			//     6 CPU cycles: 5 - 4 - 3 - 2 - 1 - 0
			p->cnt = (p->timer << 1) + 1;

			// increment sequencer current step
			p->step = (p->step + 1) & 7;	// 8 steps of the sequencer

			// decrement clocks
			clk--;
		}
		else
		{
			// decrement counter and clocks
			clk2 = clk;		// use remaining clocks
			if (clk2 > p->cnt) clk2 = p->cnt; // limit clocks to the counter value
			p->cnt -= clk2;		// decrement counter
			clk -= clk2;		// decrement clocks
		}
	}
}

// clock pulse generator counter by quarter frame
INLINE void NES_ApuPulseQuartFrame(sApuPulse* p)
{
	// clock envelope counter
	NES_ApuEnvClock(&p->env);
}

// clock pulse generator counter by half frame
INLINE void NES_ApuPulseHalfFrame(sApuPulse* p)
{
	// clock length counter
	NES_ApuLenClock(&p->len);

	// clock sweep unit
	NES_ApuSweepClock(&p->sweep);
}

// get output level (output level is 0..15)
u8 FASTCODE NOFLASH(NES_ApuPulseOut)(sApuPulse* p)
{
	return (NES_ApuLenOut(&p->len) &&			// length counter must not be zero
		NES_ApuSweepOut(&p->sweep) &&			// timer period must be in valid range
		(((p->waveform >> p->step) & 1) != 0))		// wave form bit is '1'
			? NES_ApuEnvGetVol(&p->env) : 0;	// return volume or 0
}

// set registers of pulse generator (reg = 0..3)
void FASTCODE NOFLASH(NES_ApuPulseSetReg)(sApuPulse* p, u8 reg, u8 data)
{
	u8 k;

	switch (reg)
	{
	// 0x4000/0x4004 control pulse generator
	case 0:
		// set envelope divider period or constant volume 0..15
		NES_ApuEnvSetPerVol(&p->env, data & 0x0f);

		// set envelope constant volume mode (<>0 constant volume mode, =0 envelope volume mode)
		NES_ApuEnvSetMode(&p->env, data & B4);

		// set envelope loop mode (<>0 loop mode)
		k = data & B5; // loop mode
		NES_ApuEnvSetLoop(&p->env, k);

		// halt length counter if envelope loop mode (<>0 halt counting)
		NES_ApuLenSetHalt(&p->len, k);

		// prepare duty waveform
		p->waveform = NES_ApuPulseWave[data >> 6];
		break;

	// 0x4001/0x4005 control sweep unit
	case 1:
		// set sweep shift count 0..7 (0=sweep unit is disabled)
		NES_ApuSweepSetShift(&p->sweep, data & 0x07);

		// set sweep negate flag (<>0 decrease period, =0 increase period)
		NES_ApuSweepSetNeg(&p->sweep, data & B3);

		// set sweep divider's period 0..7
		NES_ApuSweepSetPeriod(&p->sweep, (data >> 4) & 0x07);

		// set sweep enabled (<>0 enabled)
		NES_ApuSweepSetEn(&p->sweep, data >> 7);

		// set sweep reload request
		NES_ApuSweepReload(&p->sweep);
		break;

	// 0x4002/0x4006 timer low
	case 2:
		// set timer low
		p->timer = (p->timer & 0x700) | data;
		break;

	// 0x4003/0x4007 timer high
	// case 3:
	default:
		// set timer high
		p->timer = ((u16)(data & 0x07) << 8) | (p->timer & 0x00ff);

		// set length counter (len = 0..31)
		NES_ApuLenSetCnt(&p->len, data >> 3);
		
		// start envelope
		NES_ApuEnvStart(&p->env);

		// reset sequencer current step
		p->step = 0;
		break;
	}
}

// check length counter if output is enabled: length counter must not be zero
INLINE Bool NES_ApuPulseLenOut(sApuPulse* p) { return NES_ApuLenOut(&p->len); }

// set length counter enabled (<>0 enabled)
INLINE void NES_ApuPulseLenSetEn(sApuPulse* p, u8 en) { NES_ApuLenSetEn(&p->len, en); }

// ----------------------------------------------------------------------------
//                           Triangle generator
// ----------------------------------------------------------------------------

/*
typedef struct {
	u8		step;		// 0x00: sequencer current step 0..31 (increasing)
	u8		lincnt;		// 0x01: linear counter (max. 127 by quarter frame)
	u8		linrel;		// 0x02: linear counter reload value 0..127
	u8		reload;		// 0x03: <>0 linear counter reload request
	u8		ctrl;		// 0x04: control flag: <>0 halt linear counter
	u8		res[3];		// 0x05: ... reserved (align)
	u16		timer;		// 0x08: timer period 0..2047 (11 bits)
	u16		cnt;		// 0x0A: counter
	sApuLen		len;		// 0x0C: (4) length counter (max. 254 by half frame)
} sApuTri;
*/

// clock triangle generator counter by CPU clocks
void FASTCODE NOFLASH(NES_ApuTriClock)(sApuTri* t, u16 clk)
{
	u16 clk2;

	// process all clocks
	while (clk > 0)
	{
		// if counter reaches 0, reload counter
		if (t->cnt == 0)
		{
			// reload counter
			//   One step of sequencer is "timer+1" CPU cycles
			//   Example for t = 2: 3 CPU cycles 2 - 1 - 0
			u16 tim = t->timer; // load timer
			t->cnt = tim;

			// prepare conditions (timer must be valid - avoid ultrasonic, both linear and length counter must be non zero)
			Bool ok = (tim > 1) && (t->lincnt != 0) && NES_ApuLenOut(&t->len);

			// increment sequencer current step
			if (ok) t->step = (t->step + 1) & 0x1F;	// 31 steps of the sequencer

			// decrement clocks
			clk--;
		}
		else
		{
			// decrement counter and clocks
			clk2 = clk;		// use remaining clocks
			if (clk2 > t->cnt) clk2 = t->cnt; // limit clocks to the counter value
			t->cnt -= clk2;		// decrement counter
			clk -= clk2;		// decrement clocks
		}
	}
}

// clock triangle generator counter by quarter frame
void FASTCODE NOFLASH(NES_ApuTriQuartFrame)(sApuTri* t)
{
	// reload linear counter if necessary
	if (t->reload != 0)
		t->lincnt = t->linrel;
	else
		// decremet linear counter
		if (t->lincnt != 0) t->lincnt--;

	// reset linear counter reload flag, if not set control flag halting linear counter
	if (t->ctrl == 0) t->reload = 0;
}

// clock triangle generator counter by half frame
INLINE void NES_ApuTriHalfFrame(sApuTri* t)
{
	// clock length counter
	NES_ApuLenClock(&t->len);
}

// get output level (output level is 0..15)
u8 FASTCODE NOFLASH(NES_ApuTriOut)(sApuTri* t)
{
	// falling 15..0, rising 0..15 (values 0 and 15 are duplicated)
	u8 step = t->step;
	return (step < 16) ? (15 - step) : (step - 16);
}

// set registers of triangle generator (reg = 0..3)
void FASTCODE NOFLASH(NES_ApuTriSetReg)(sApuTri* t, u8 reg, u8 data)
{
	u8 k;

	switch (reg)
	{
	// 0x4008 control triangle generator
	case 0:
		// linear counter reload value
		t->linrel = data & 0x7f;

		// control flag: <>0 halt linear counter
		k = data >> 7;
		t->ctrl = k;

		// halt length counter
		NES_ApuLenSetHalt(&t->len, k);
		break;

	// 0x400A timer low
	case 2:
		// set timer low
		t->timer = (t->timer & 0x700) | data;
		break;

	// 0x400B timer high
	case 3:
		// set timer high
		t->timer = ((u16)(data & 0x07) << 8) | (t->timer & 0x00ff);

		// set length counter (len = 0..31)
		NES_ApuLenSetCnt(&t->len, data >> 3);
		
		// set linear counter reload flag
		t->reload = 1;
		break;

	// 0x4009 ... not used
	// case 1:
	default:
		break;

	}
}

// check length counter if output is enabled: length counter must not be zero
INLINE Bool NES_ApuTriLenOut(sApuTri* t) { return NES_ApuLenOut(&t->len); }

// set length counter enabled (<>0 enabled)
INLINE void NES_ApuTriLenSetEn(sApuTri* t, u8 en) { NES_ApuLenSetEn(&t->len, en); }

// ----------------------------------------------------------------------------
//                           Noise generator
// ----------------------------------------------------------------------------

/*
typedef struct {
	u16		shift;		// 0x00: shift register with noise sample
	u8		mode;		// 0x02: <>0 short sequence 93 or 31 steps, =0 long sequence 32767 steps
	u8		res;		// 0x03: ... reserved (align)
	u16		timer;		// 0x04: timer period 0..2047 (11 bits)
	u16		cnt;		// 0x06: counter
	sApuLen		len;		// 0x08: (4) length counter
	sApuEnv		env;		// 0x0C: (8) envelope unit
} sApuNoise;
*/

// timer periods
u16 NES_ApuNoisePeriods[16] = {
	  4,   8,  16,  32,  64,   96,  128,  160,
	202, 254, 380, 508, 762, 1016, 2034, 4068
};

// initialize noise generator
void FASTCODE NOFLASH(NES_ApuNoiseInit)(sApuNoise* n)
{
	// set timer at its minimal value
	n->timer = NES_ApuNoisePeriods[0];

	// set shift register (must not be 0)
	n->shift = 1;
}

// clock noise generator counter by CPU clock
void FASTCODE NOFLASH(NES_ApuNoiseClock)(sApuNoise* n, u16 clk)
{
	u16 clk2;

	// process all clocks
	while (clk > 0)
	{
		// if counter reaches 0, reload counter
		if (n->cnt == 0)
		{
			// reload counter
			//   One step of sequencer is "timer+1" CPU cycles
			//   Example for t = 2: 3 CPU cycles 2 - 1 - 0
			n->cnt = n->timer;

			// prepare feedback bit
			u8 bit = (n->mode == 0) ? 1 : 6;

			// shift register
			u16 shift = n->shift; // shift register
			u16 feedback = (shift ^ (shift >> bit)) & B0;
			n->shift = (feedback << 14) | (shift >> 1);

			// decrement clocks
			clk--;
		}
		else
		{
			// decrement counter and clocks
			clk2 = clk;		// use remaining clocks
			if (clk2 > n->cnt) clk2 = n->cnt; // limit clocks to the counter value
			n->cnt -= clk2;		// decrement counter
			clk -= clk2;		// decrement clocks
		}
	}
}

// clock noise generator counter by quarter frame
INLINE void NES_ApuNoiseQuartFrame(sApuNoise* n)
{
	// clock envelope counter
	NES_ApuEnvClock(&n->env);
}

// clock pulse generator counter by half frame
INLINE void NES_ApuNoiseHalfFrame(sApuNoise* n)
{
	// clock length counter
	NES_ApuLenClock(&n->len);
}

// get output level (output level is 0..15)
u8 FASTCODE NOFLASH(NES_ApuNoiseOut)(sApuNoise* n)
{
	return (((n->shift & B0) == 0) && NES_ApuLenOut(&n->len)) ? NES_ApuEnvGetVol(&n->env) : 0;
}

// set registers of noise generator (reg = 0..3)
void FASTCODE NOFLASH(NES_ApuNoiseSetReg)(sApuNoise* n, u8 reg, u8 data)
{
	u8 k;

	switch (reg)
	{
	// 0x400C control noise generator
	case 0:
		// set envelope divider period or constant volume 0..15
		NES_ApuEnvSetPerVol(&n->env, data & 0x0f);

		// set envelope constant volume mode (<>0 constant volume mode, =0 envelope volume mode)
		NES_ApuEnvSetMode(&n->env, data & B4);

		// set envelope loop mode (<>0 loop mode)
		k = data & B5; // loop mode
		NES_ApuEnvSetLoop(&n->env, k);

		// halt length counter if envelope loop mode (<>0 halt counting)
		NES_ApuLenSetHalt(&n->len, k);
		break;

	// 0x400E loop noise, noise period
	case 2:
		// setup timer from period
		k = data & 0x0f;
		n->timer = NES_ApuNoisePeriods[k];

		// mode (<>0 short sequence 93 or 31 steps, =0 long sequence 32767 steps)
		n->mode = data >> 7;
		break;

	// 0x400F length counter
	case 3:
		// set length counter (len = 0..31)
		NES_ApuLenSetCnt(&n->len, data >> 3);
		
		// start envelope
		NES_ApuEnvStart(&n->env);
		break;

	// 0x400D ... not used
	// case 1:
	default:
		break;
	}
}

// check length counter if output is enabled: length counter must not be zero
INLINE Bool NES_ApuNoiseLenOut(sApuNoise* n) { return NES_ApuLenOut(&n->len); }

// set length counter enabled (<>0 enabled)
INLINE void NES_ApuNoiseLenSetEn(sApuNoise* n, u8 en) { NES_ApuLenSetEn(&n->len, en); }

// ----------------------------------------------------------------------------
//                           DMC generator
// ----------------------------------------------------------------------------

/*
typedef struct {
	u8		inten;		// 0x00: <>0 enable interrupt
	u8		irq;		// 0x01: <>0 interrupt request
	u8		loop;		// 0x02: <>0 loop
	u8		silence;	// 0x03: <>0 silence
	u8		shift;		// 0x04: sample buffer shift register
	u8		bitcnt;		// 0x05: shift register bit counter (number of remaining bits)
	u8		outlev;		// 0x06: output level 0..126 (1=increment, 0=decrement)
	u8		en;		// 0x07: <>0 enabled
	u16		remain;		// 0x08: current sample remaining bytes
	u16		addr;		// 0x0A: current sample address
	u16		samplen;	// 0x0C: original sample length bytes
	u16		sampaddr;	// 0x0E: original sample address
	u16		timer;		// 0x10: timer 54..428
	u16		cnt;		// 0x12: counter
} sApuDmc;
*/

// DMC rates (in CPU cycles; f = 1789773/rate)
u16 NES_ApuDmcRates[16] = {
	428,	// 0: 4181.71 Hz
	380,	// 1: 4709.93 Hz
	340,	// 2: 5264.04 Hz
	320,	// 3: 5593.04 Hz
	286,	// 4: 6257.95 Hz
	254,	// 5: 7046.35 Hz
	226,	// 6: 7919.35 Hz
	214,	// 7: 8363.43 Hz
	190,	// 8: 9419.86 Hz
	160,	// 9: 11186.08 Hz
	142,	// 10: 12604.04 Hz
	128,	// 11: 13982.60 Hz
	106,	// 12: 16884.65 Hz
	84,	// 13: 21306.82 Hz
	72,	// 14: 24857.96 Hz
	54	// 15: 33143.94 Hz
};

// initialize DMC generator
void FASTCODE NOFLASH(NES_ApuDmcInit)(sApuDmc* d)
{
	// enable interrupt
//	d->inten = 1;

	// start silenced
	d->silence = 1;

	// setup timer to first value
	d->timer = NES_ApuDmcRates[0];

	// delay start 1025 CPU clock cycles later
	d->cnt = 1025;

	// initialize sample address
	d->sampaddr = 0xc000;
	d->shift = 0xff;
	d->bitcnt = 8;
	d->samplen = 1;
}

// clock DMC generator counter by CPU clocks
void FASTCODE NOFLASH(NES_ApuDmcClock)(sApuDmc* d, u16 clk)
{
	u16 clk2;

	// process all clocks
	while (clk > 0)
	{
		// decrement counter and clocks
		clk2 = clk;		// use remaining clocks
		if (clk2 > d->cnt) clk2 = d->cnt; // limit clocks to the counter value
		d->cnt -= clk2;		// decrement counter
		clk -= clk2;		// decrement clocks

		// if counter is 0
		if (d->cnt == 0)
		{
			// reload counter
			d->cnt = d->timer;

			// shift register is empty
			if (d->bitcnt == 0)
			{
				// restart bit counter
				d->bitcnt = 8;

				// no data remain, set silence
				if (d->remain == 0)
				{
					// no data, set cilence
					d->silence = 1;

					// loop sample (must be enabled)
					if ((d->loop != 0) && (d->en != 0))
					{
						// restart sample
						d->remain = d->samplen; // sample length
						d->addr = d->sampaddr; // sample address
					}
				}

				// some data remain
				if (d->remain != 0)
				{
					// read next sample
					d->shift = K6502_Read(d->addr);
					d->remain--;
					d->addr = (d->addr + 1) | 0x8000;

					// clear silence
					d->silence = 0;

					// end of data
					if (d->remain == 0)
					{
						// loop sample
						if (d->loop != 0)
						{
							// must be enabled
							if (d->en != 0)
							{
								// restart sample
								d->remain = d->samplen; // sample length
								d->addr = d->sampaddr; // sample address
							}
						}
						else
						{
							// interrupt
							if (d->inten != 0)
							{
								d->irq = 1;
#ifndef NES_NOIRQ_DMC		// if defined, disable IRQ on APU DMC end of sound
								IRQ_REQ;
#endif
							}
						}
					}
				}
			}

			// change output level if not silenced
			if (d->silence == 0)
			{
				// add DMC
				if ((d->shift & B0) != 0)
				{
					if (d->outlev <= 125) d->outlev += 2;
				}

				// subtract DMC
				else
				{
					if (d->outlev >= 2) d->outlev -= 2;
				}
			}

			// update shift register
			d->shift >>= 1;

			// update shift register bit counter
			d->bitcnt--;
		}
	}
}

// check if DMC generator is enabled
INLINE Bool NES_ApuDmcIsEn(sApuDmc* d) { return d->remain != 0; }

// set DMC generator enabled (<>0 enabled)
void FASTCODE NOFLASH(NES_ApuDmcSetEn)(sApuDmc* d, u8 en)
{
	// save enabled state
	d->en = en;

	// if disabled, reset remaining bytes
	if (en == 0)
	{
		d->remain = 0;
		return;
	}

	// if enabled, restart when nothing remains (= it was disabled)
	if (d->remain == 0)
	{
		// restart sample
		d->remain = d->samplen; // sample length
		d->addr = d->sampaddr; // sample address
	}

	// raise interrupt if required
//#ifndef NES_NOIRQ_DMC		// if defined, disable IRQ on APU DMC end of sound
//	if (d->irq != 0) IRQ_REQ;
//#endif
}

// get interrupt request flag
INLINE u8 NES_ApuDmcGetIrq(sApuDmc* d) { return d->irq; }

// reset interrupt request flag
INLINE void NES_ApuDmcClrIrq(sApuDmc* d) { d->irq = 0; }

// get output level (0..126)
INLINE u8 NES_ApuDmcOut(sApuDmc* d) { return d->outlev; }

// set registers of DMC generator (reg = 0..3)
void FASTCODE NOFLASH(NES_ApuDmcSetReg)(sApuDmc* d, u8 reg, u8 data)
{
	u8 k;

	switch (reg)
	{
	// 0x4010 control DMC generator
	case 0:
		// get timer from rate index
		d->timer = NES_ApuDmcRates[data & 0x0f];

		// get loop flag
		d->loop = data & 0x40;

		// get interrupt enable flag
		k = data >> 7;
		d->inten = k;

		// if interrupt disabled, reset interrupt request flag
		if (k == 0) d->irq = 0;
		break;

	// 0x4011 load counter
	case 1:
		d->outlev = data & 0x7f;
		break;

	// 0x4012 sample address
	case 2:
		d->sampaddr = 0xc000 | ((u16)data << 6);
		break;

	// 0x4013 sample length
	// case 3:
	default:
		d->samplen = ((u16)data << 4) | 1;
		break;
	}
}

// ----------------------------------------------------------------------------
//                             Sound chip
// ----------------------------------------------------------------------------
// The interrupt from the frame counter is used to timing long events in the program,
// so it is better to synchronize interrupt with CPU clock rather than with sound hardware.

/*
typedef struct {
	u8		irq;		// 0x00: <>0 frame counter interrupt request
					//	- controlled from the CPU emulator rather
					//	  than from the sound hardware
	u8		intdis;		// 0x01: <>0 frame counter interrupt disable
					//	- controlled from the CPU emulator rather
					//	  than from the sound hardware
	u8		mode5;		// 0x02: <>0 frame counter 5-step mode
	u8		step;		// 0x03: frame sequencer step (max. 4 or 5)
	u16		cnt;		// 0x04: frame counter - number of CPU clocks to the next frame step
	u16		framelen;	// 0x06: frame length (number of CPU clock ticks)
	sApuPulse	pulse1;		// 0x08: (0x20) pulse generator 1 (channel 0)
	sApuPulse	pulse2;		// 0x28: (0x20) pulse generator 2 (channel 1)
	sApuTri		tri;		// 0x48: (0x10) triangle generator (channel 2)
	sApuNoise	noise;		// 0x58: (0x14) noise generator (channel 3)
	sApuDmc		dmc;		// 0x6C: (0x14) DMC generator (channel 4)
} sApuSnd;
*/

#define NES_APU_FRAME_STEP	7456		// length of frame step in number of CPU clock cycles

// set frame interrupt dissable flag
//  - controlled from the CPU emulator rather than from the sound hardware
/*void FASTCODE NOFLASH(NES_ApuFrameSetIntDis)(sApuSnd* s, u8 intdis)
{
	s->intdis = intdis;

	// reset frame interrupt request, if interupt disable
	if (intdis != 0) s->irq = 0;
}
*/

// set frame 5-step mode (<>0 5-step mode, =0 4-step mode)
void NES_ApuFrameSetMode5(sApuSnd* s, u8 mode5)
{
	s->mode5 = mode5;

	// APU frame length
	s->framelen = NES_APU_FRAME_STEP * ((mode5 == 0) ? 4 : 5); // APU frame length

	// reset frame interrupt request on mode 5
//	if (mode5 != 0) s->irq = 0;
}

// clock sound chip by quarter frame
void FASTCODE NOFLASH(NES_ApuSndQuartFrame)(sApuSnd* s)
{
	NES_ApuPulseQuartFrame(&s->pulse1);	// pulse generator 1
	NES_ApuPulseQuartFrame(&s->pulse2);	// pulse generator 2
	NES_ApuTriQuartFrame(&s->tri);		// triangle generator
	NES_ApuNoiseQuartFrame(&s->noise);	// noise generator
}

// clock sound chip by half frame
void FASTCODE NOFLASH(NES_ApuSndHalfFrame)(sApuSnd* s)
{
	NES_ApuPulseHalfFrame(&s->pulse1);	// pulse generator 1
	NES_ApuPulseHalfFrame(&s->pulse2);	// pulse generator 2
	NES_ApuTriHalfFrame(&s->tri);		// triangle generator
	NES_ApuNoiseHalfFrame(&s->noise);	// noise generator
}

// set frame counter register 0x4017 and reset frame counter
void FASTCODE NOFLASH(NES_ApuFrameSetReg)(sApuSnd* s, u8 data)
{
	// set frame counter interrupt disable flag
//	NES_ApuFrameSetIntDis(s, data & 0x40);

	// set 5-step mode
	NES_ApuFrameSetMode5(s, data >> 7);

	// reset frame counter
	s->cnt = NES_APU_FRAME_STEP;
	s->step = 0;

	// reset interrupt request
//	s->irq = 0;

	// send frame signals
	NES_ApuSndQuartFrame(s);
	NES_ApuSndHalfFrame(s);
}

// set status register 0x4015
void FASTCODE NOFLASH(NES_ApuSndSetStatus)(sApuSnd* s, u8 data)
{
	// enable channels
	NES_ApuPulseLenSetEn(&s->pulse1, data & B0);	// pulse generator 1
	NES_ApuPulseLenSetEn(&s->pulse2, data & B1);	// pulse generator 2
	NES_ApuTriLenSetEn(&s->tri, data & B2);		// triangle generator
	NES_ApuNoiseLenSetEn(&s->noise, data & B3);	// noise generator
	NES_ApuDmcSetEn(&s->dmc, data & B4);		// DMC generator

	// reset DMC interrupt flag
	NES_ApuDmcClrIrq(&s->dmc);
}

// get status register
u8 FASTCODE NOFLASH(NES_ApuSndGetStatus)(sApuSnd* s)
{
	u8 data = 0;
	if (NES_ApuPulseLenOut(&s->pulse2)) data |= B0; // length counter of pulse generator 2 enabled
	if (NES_ApuPulseLenOut(&s->pulse1)) data |= B1; // length counter of pulse generator 2 enabled
	if (NES_ApuTriLenOut(&s->tri)) data |= B2;	// length counter of triangle generator enabled
	if (NES_ApuNoiseLenOut(&s->noise)) data |= B3;	// length counter of noise generator enabled
	if (NES_ApuDmcIsEn(&s->dmc)) data |= B4;	// DMC enabled (data remain)
//	if ((s->irq != 0) && (s->mode5 == 0)) data |= B6; // frame counter interrupt
	data |= APU_Reg[0x15] & B6;

	if (NES_ApuDmcGetIrq(&s->dmc) != 0) data |= B7;	// DMC interrupt

	// reset frame counter interrupt
//	s->irq = 0;					// reset frame counter interrupt
//	NES_ApuFrameClrIrq(s);
	APU_Reg[0x15] &= ~B6;
	
	return data;
}

// initialize sound chip (at first, clear sApuSnd descriptor)
void FASTCODE NOFLASH(NES_ApuSndInit)(sApuSnd* s)
{
	// initialize generators
	NES_ApuPulseInit(&s->pulse1, 1);	// initialize pulse generator 1
	NES_ApuPulseInit(&s->pulse2, 0);	// initialize pulse generator 2
	NES_ApuNoiseInit(&s->noise);		// initialize noise generator
	NES_ApuDmcInit(&s->dmc);		// initialize DMC generator

	// reset frame counter
	s->cnt = NES_APU_FRAME_STEP;
	s->step = 0;
}

// clock sound chip by CPU clocks
void FASTCODE NOFLASH(NES_ApuSndClock)(sApuSnd* s, u16 clk)
{
	u16 clk2;

	// process all clocks
	while (clk > 0)
	{
		// decrement frame counter and clocks
		clk2 = clk;		// use remaining clocks
		if (clk2 > s->cnt) clk2 = s->cnt; // limit clocks to the counter value
		s->cnt -= clk2;		// decrement counter
		clk -= clk2;		// decrement clocks

		// clock channels (should be run before frame signal)
		NES_ApuPulseClock(&s->pulse1, clk2);	// clock pulse generator 1
		NES_ApuPulseClock(&s->pulse2, clk2);	// clock pulse generator 2
		NES_ApuTriClock(&s->tri, clk2);		// clock triangle generator
		NES_ApuNoiseClock(&s->noise, clk2);	// clock noise generator
		NES_ApuDmcClock(&s->dmc, clk2);		// clock DMC generator

		// if frame counter is 0
		if (s->cnt == 0)
		{
			// reload counter
			s->cnt = NES_APU_FRAME_STEP;

			// increment frame sequencer step
			s->step++;

			// quarter frame step
			if ((s->step == 1) || (s->step == 3))
			{
				NES_ApuSndQuartFrame(s);
			}

			// half frame step
			else if (s->step == 2)
			{
				NES_ApuSndQuartFrame(s);
				NES_ApuSndHalfFrame(s);
			}

			// last step
			else
			{
				// 4-step mode
				if (s->mode5 == 0)
				{
					NES_ApuSndQuartFrame(s);
					NES_ApuSndHalfFrame(s);

					// reset counter
					s->step = 0;
				}

				// 5-step mode
				else if (s->step == 5)
				{
					NES_ApuSndQuartFrame(s);
					NES_ApuSndHalfFrame(s);

					// reset counter
					s->step = 0;
				}
			}
		}
	}
}


#define OUT_MODE	2	// output mode: 0=linear, 1=add tables, 2=table of adds


#if OUT_MODE == 1	// output mode: 0=linear, 1=add tables, 2=table of adds

// 16-step output table ... 459/(12/inx + 1)
u8 NES_OutTab16[16] = { 0, 35, 66, 92, 115, 135, 153, 169, 184, 197, 209, 220, 230, 239, 247, 255 };

// 128-step output table ... 456/(100/inx + 1)
u8 NES_OutTab128[128] = {
	   0,   5,   9,  13,  18,  22,  26,  30,
	  34,  38,  41,  45,  49,  52,  56,  59,
	  63,  66,  70,  73,  76,  79,  82,  85,
	  88,  91,  94,  97, 100, 103, 105, 108,
	 111, 113, 116, 118, 121, 123, 126, 128,
	 130, 133, 135, 137, 139, 142, 144, 146,
	 148, 150, 152, 154, 156, 158, 160, 162,
	 164, 166, 167, 169, 171, 173, 175, 176,
	 178, 180, 181, 183, 185, 186, 188, 189,
	 191, 192, 194, 195, 197, 198, 200, 201,
	 203, 204, 205, 207, 208, 210, 211, 212,
	 213, 215, 216, 217, 219, 220, 221, 222,
	 223, 225, 226, 227, 228, 229, 230, 231,
	 232, 234, 235, 236, 237, 238, 239, 240,
	 241, 242, 243, 244, 245, 246, 247, 248,
	 249, 250, 251, 252, 252, 253, 254, 255,
};

int NES_OldSum = 0; // low-pass filter

#endif // OUT_MODE == 1


#if OUT_MODE == 2	// output mode: 0=linear, 1=add tables, 2=table of adds

// Pulse output table ... 246/(60/inx+1)
u8 NES_PulseTab[31] = {
	 0,  4,  8, 12, 15, 19, 22, 26, 29, 32,
	35, 38, 41, 44, 47, 49, 52, 54, 57, 59,
	62, 64, 66, 68, 70, 72, 74, 76, 78, 80,
	82,
};

// TND output table ... 490/(202/inx+1)
u8 NES_TndTab[203] = {
	   0,   2,   5,   7,  10,  12,  14,  16,  19,  21,
	  23,  25,  27,  30,  32,  34,  36,  38,  40,  42,
	  44,  46,  48,  50,  52,  54,  56,  58,  60,  62,
	  63,  65,  67,  69,  71,  72,  74,  76,  78,  79,
	  81,  83,  84,  86,  88,  89,  91,  92,  94,  96,
	  97,  99, 100, 102, 103, 105, 106, 108, 109, 111,
	 112, 114, 115, 116, 118, 119, 121, 122, 123, 125,
	 126, 127, 129, 130, 131, 133, 134, 135, 137, 138,
	 139, 140, 141, 143, 144, 145, 146, 148, 149, 150,
	 151, 152, 153, 154, 156, 157, 158, 159, 160, 161,
	 162, 163, 164, 165, 167, 168, 169, 170, 171, 172,
	 173, 174, 175, 176, 177, 178, 179, 180, 181, 182,
	 183, 184, 185, 185, 186, 187, 188, 189, 190, 191,
	 192, 193, 194, 195, 195, 196, 197, 198, 199, 200,
	 201, 201, 202, 203, 204, 205, 206, 206, 207, 208,
	 209, 210, 210, 211, 212, 213, 214, 214, 215, 216,
	 217, 217, 218, 219, 220, 220, 221, 222, 222, 223,
	 224, 225, 225, 226, 227, 227, 228, 229, 230, 230,
	 231, 232, 232, 233, 234, 234, 235, 236, 236, 237,
	 238, 238, 239, 239, 240, 241, 241, 242, 243, 243,
	 244, 244, 245,
};

int NES_OldSum = 0; // low-pass filter
#if PWMSND_GPIO_R >= 0
int NES_OldSumR = 0; // low-pass filter
#endif

#endif // OUT_MODE == 2
 

// get output sample
// Mono output level: 0..1308
// Stereo output level: 12 LOW bits 0..1308, 12 HIGH bits 0..1308
int FASTCODE NOFLASH(NES_ApuSndOut)(sApuSnd* s)
{

#if OUT_MODE == 0	// output mode: 0=linear, 1=add tables, 2=table of adds

	// sum pulse channels (level is 0..2*15 = 30)
	int sum = NES_ApuPulseOut(&s->pulse1) + NES_ApuPulseOut(&s->pulse2);

	// add triangle channel (level is 0..3*15 = 45)
	sum += NES_ApuTriOut(&s->tri);

	// add noise channel (level is 0..4*15 = 60)
	sum += NES_ApuNoiseOut(&s->noise);

	// normalize to the same level as DMC channel (should be 4*126 = 504)
	sum <<= 3; // level is 0..4*120 = 480

	// add DMC channel (DMC level is 0..126, total level is 0..480+126 = 606)
	sum += NES_ApuDmcOut(&s->dmc);

	// use same level as tables
	sum <<= 1; // output level is 0..1212

#endif // OUT_MODE == 0


#if OUT_MODE == 1	// output mode: 0=linear, 1=add tables, 2=table of adds
	int sum = NES_OutTab16[NES_ApuPulseOut(&s->pulse1)];	// pulse channel 1 (0..15)
	sum += NES_OutTab16[NES_ApuPulseOut(&s->pulse2)];	// pulse channel 2 (0..15)
	sum += NES_OutTab16[NES_ApuTriOut(&s->tri)];		// triangle channel (0..15)
	sum += NES_OutTab16[NES_ApuNoiseOut(&s->noise)] >> 1;	// noise channel (0..15)
	sum += NES_OutTab128[NES_ApuDmcOut(&s->dmc)] << 2;	// DMC channel (0..127)

	// simple low pass filter
	sum = (NES_OldSum + sum*3) >> 2;
	NES_OldSum = sum;

#endif // OUT_MODE == 1


#if OUT_MODE == 2	// output mode: 0=linear, 1=add tables, 2=table of adds

	// https://www.nesdev.org/wiki/APU_Mixer
	// Input ranges: pulse1/pulse2/tri/noise = 0..(8)..15;  dmc = 0..(64)..127

	// Precise mixing:
	//  pulse_out = 95.88/(8128/(pulse1 + pulse2) + 100) ... mid=0.1577, max=0.2585
	//  tnd_out = 159.79/(1/(tri/8227 + noise/12241 + dmc/22638) + 100) ... mid=0.4923, max=0.74152
	//  output = pulse_out + tnd_out ... mid=0.6500, max=1.000

	// We need result output value:
	//  PWM = max. 1023 ... this is max. value of PWM output
	//  vol = normal 100 .. normal volume we will use
	//  PWM = (out * vol) >> 7 ... how we calculate PWM output
	//  out max. = PWM * 128 / vol = 1023*128/100 = 1309 ... output max. value we need

	// We need max. values from tables:
	//  out = (pulse + tnd) << 2 ... simple mix pulse and tnd outputs. We will use '<<2' so 'u8' range is enough.
	//    we will mix output with volume 'pulse*0.25 + tnd*0.75'
	//  result volume (pulse + tnd) should be max. 1309/4 = 327
	//  pulse max = (out/4)/4 = 82 ... this is max. value from table of 'pulse1+pulse2' (it will be 1/4 of result volume)
	//  tnd max = (out*3/4)/4 = 245 ... this is max. value from table of 'tri+noise+dmc' (it will be 3/4 of result volume)

	// How to calculate tables:
	//  pulse_sum = pulse1 + pulse2 ... max. index is 30
	//  tnd_sum = 3*tri + 2*noise + dmc ... max index is 202
	//  NES_PulseTab[inx=0..30] = 246/(60/inx+1)
	//  NES_TndTab[inx=0..202] = 490/(202/inx+1)

#if PWMSND_GPIO_R >= 0

// Stereo simulation, L + R:
// pulse 1 (0..15) main melody 1: 100% + 100%
// pulse 2 (0..15) main melody 2: 100% + 50%
// triangle (0..15) accompanying melody: 50% + 100%
// noise (0..15) percussion: 100% + 25%
// DMC (0..127): 25% + 100%

// Mono output level: 0..1308
// Stereo output level: 12 LOW bits 0..1308, 12 HIGH bits 0..1308

	// left channel
	int sum = NES_PulseTab[NES_ApuPulseOut(&s->pulse1) + NES_ApuPulseOut(&s->pulse2)]; // index 0..30, output 0..82
	sum += NES_TndTab[((NES_ApuTriOut(&s->tri)*3)>>1) + NES_ApuNoiseOut(&s->noise)*2 + (NES_ApuDmcOut(&s->dmc)>>2)]; // index 0..202, output 0..245

	// simple low pass filter (new=sum*4: value 0..1308, low pass mixing: (old + 3*new)/4)
	sum = (NES_OldSum + sum*12) >> 2; // output is max. (1308 + 327*12)/4 = 1308
	NES_OldSum = sum;

	// right channel
	int sumR = NES_PulseTab[NES_ApuPulseOut(&s->pulse1) + (NES_ApuPulseOut(&s->pulse2)>>1)]; // index 0..30, output 0..82
	sumR += NES_TndTab[NES_ApuTriOut(&s->tri)*3 + (NES_ApuNoiseOut(&s->noise)>>1) + NES_ApuDmcOut(&s->dmc)]; // index 0..202, output 0..245

	// simple low pass filter (new=sum*4: value 0..1308, low pass mixing: (old + 3*new)/4)
	sumR = (NES_OldSumR + sumR*12) >> 2; // output is max. (1308 + 327*12)/4 = 1308
	NES_OldSumR = sumR;

	sum |= sumR << 12;

#else // PWMSND_GPIO_R

	int sum = NES_PulseTab[NES_ApuPulseOut(&s->pulse1) + NES_ApuPulseOut(&s->pulse2)]; // index 0..30, output 0..82
	sum += NES_TndTab[NES_ApuTriOut(&s->tri)*3 + NES_ApuNoiseOut(&s->noise)*2 + NES_ApuDmcOut(&s->dmc)]; // index 0..202, output 0..245

	// simple low pass filter (new=sum*4: value 0..1308, low pass mixing: (old + 3*new)/4)
	sum = (NES_OldSum + sum*12) >> 2; // output is max. (1308 + 327*12)/4 = 1308
	NES_OldSum = sum;

#endif // PWMSND_GPIO_R

#endif // OUT_MODE == 2

	return sum;
}

// set register (reg = register 0x00-0x13)
void FASTCODE NOFLASH(NES_ApuSndSetReg)(sApuSnd* s, u8 reg, u8 data)
{
	// 0x00..0x03: pulse generator 1
	if (reg < 0x04)
		NES_ApuPulseSetReg(&s->pulse1, reg, data);

	// 0x04..0x07: pulse generator 2
	else if (reg < 0x08)
		NES_ApuPulseSetReg(&s->pulse2, reg - 4, data);

	// 0x08..0x0B: triangle generator
	else if (reg < 0x0C)
		NES_ApuTriSetReg(&s->tri, reg - 8, data);

	// 0x0C..0x0F: noise generator
	else if (reg < 0x10)
		NES_ApuNoiseSetReg(&s->noise, reg - 12, data);

	// 0x10..0x13: DMC generator
	else
		NES_ApuDmcSetReg(&s->dmc, reg - 16, data);
}

// write sound register 0x4000-0x4013 (reg = register 0x00-0x13)
void FASTCODE NOFLASH(NES_ApuWriteReg)(u8 reg, u8 data)
{
	// lock
	AUDIO_LOCK();

	// write sound register
//	APU_Reg[reg] = data;
	NES_ApuSndSetReg(APU, reg, data);

	// unlock
	AUDIO_UNLOCK();
}

// write sound enable register 0x4015
void FASTCODE NOFLASH(NES_ApuWriteEn)(u8 data)
{
	// lock
	AUDIO_LOCK();

	// write enable register
//	APU_Reg[0x15] = data;
	NES_ApuSndSetStatus(APU, data);

	// unlock
	AUDIO_UNLOCK();
}

// write frame counter register 0x4017
void FASTCODE NOFLASH(NES_ApuWriteFrame)(u8 data)
{
	// lock
	AUDIO_LOCK();

	// set frame counter register
//	APU_Reg[0x17] = data;
	NES_ApuFrameSetReg(APU, data);

	// unlock
	AUDIO_UNLOCK();
}

// read sound status 0x4015
u8 FASTCODE NOFLASH(NES_ApuReadState)()
{
	// lock
	AUDIO_LOCK();

	// read state register
	u8 res = NES_ApuSndGetStatus(APU);

	// unlock
	AUDIO_UNLOCK();

	return res;
}

// initialize APU
void InfoNES_ApuInit()
{
	// clear APU structure
	memset(APU, 0, sizeof(sApuSnd));

	// initialize sound chip
	NES_ApuSndInit(APU);

	// Sound Hardware Init
	InfoNES_SoundInit();
}
