
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

// Based on NESEmu source code by Jonathan Baliko

/* 
APU https://www.nesdev.org/wiki/APU

Length counter and envelope uints are clocked by the frame counter.

Pulse generator 1/2
-------------------
0x4000/0x4004 control pulse generator
	B0-B3: (0..15) volume or envelope rate
	B4: 1=volume is constant, 0=envelope (decreasing volume from 15 to 0)
	B5: 1=infinite play (envelope will repeat forever), 0=one shot
	B6-B7: (0..3) duty, width of the pulse 0=12.5%, 1=25%, 2=50%, 3=75%

0x4001/0x4005 control sweep unit (periodically adjust pulse channel's period up or down)
	B0-B2: (S=0..7) sweep shifts
	B3: negate, 0=add to period (to lower frequency), 1=subtract from period (to higher frequency)
	B4-B6: (P=0..7) sweep length (divider's period is P+1 half-frames)
	B7: sweep enable

0x4002/0x4006 timer low
	B0-B7: timer low 8 bits
0x4003/0x4007 timer high
	B0-B2: timer high 3 bits
	B3-B7: length counter load

   Timer is updated every 2nd CPU cycle. Timer counts down.
   Frequency 'f' of the pulse channels is division of CPU clock (fcpu = 1.789773MHz)
   by 11-bit value 't' to 0x4002-0x4003/0x4006-0x4007 (if t<8, channel is silenced).
   Writing to 0x4003/0x4007 reloads length counter.
	f = fcpu / (16 * (t + 1))
	t = (fcpu / (16 * f)) - 1


Triangle generator
------------------
0x4008 control triangle generator
	B0-B6: (0..127) linear counter reload value
	B7: control flag - length counter halt flag

0x400A timer low
	B0-B7: timer low 8 bits
0x400B timer high
	B0-B2: timer high 3 bits
	B3-B7: length counter load

    Sequencer is clocked by 11-bit timer 't' updated every CPU cycle. Timer counts down.
	f = fcpu / (32*(t + 1))
	t = fcpu/(32*f) - 1


Noise generator
---------------
0x400C control noise generator
	B0-B3: volume or envelope rate
	B4: 1=volume is constant, 0=envelope (decreasing volume from 15 to 0)
	B5: 1=infinite play (envelope will repeat forever), 0=one shot

0x400E mode and period
	B0-B3: (0..15) timer period CPU cycles: 4, 8, 16, 32, 64, 96, 128, 160, 202, 254,
			380, 508, 762, 1016, 2034, 4068
	B7: mode flag

0x400F length counter
	B3-B7: length counter and envelope restart


DMC channel (stream of 1-bit deltas that control 7-bit PCM counter; clamp value, 1=increment, 0=decrement)
-----------
0x4010 control DMC channel
	B0-B3: frequency
	B6: loop
	B7: IRQ enable

0x4011 B0-B6 load counter

0x4012 sample address %11AA AAAA AA00 0000 (address 0xC000-0xFFFF)

0x4013 sample length in bytes %LLLL LLLL 0001 (length 1-4081)


Control
-------
0x4015 Write: enable
	B0: 1=enable pulse channel 1
	B1: 1=enable pulse channel 2
	B2: 1=enable triangle channel
	B3: 1=enable noise channel
	B4: 1=enable DMC channel
       Read: status

0x4017 frame counter (240 Hz)
	B6: IRQ inhibit flag
	B7: mode 0=4-step, 1=5-step
*/

// === length counter (used in pulse, triangle and noise generators)
// To reset: clear all variables
typedef struct {
	u8	cnt;		// 0x00 length counter (0 = mute channel)
	u8	en;		// 0x01: <>0 enabled
	u8	halt;		// 0x02: <>0 halt counting
	u8	res;		// 0x03: ... reserved (align)
} sApuLen;

STATIC_ASSERT(sizeof(sApuLen) == 4, "Incorrect sApuLen!");

// === sweep unit (used in pulse generators)
// To reset: clear all variables
typedef struct {
	u8	cnt;		// 0x00: counter
	u8	period;		// 0x01: divider's period 0..7
	u8	shift;		// 0x02: shift count 0..7 (0=sweep unit is disabled)
	u8	comp;		// 0x03: <>0 one complement mode (pulse generator 1), =0 negate mode (pulse generator 2)
// aligned
	u8	en;		// 0x04: <>0 enabled
	u8	reload;		// 0x05: <>0 reload request
	u8	neg;		// 0x06: <>0 decrease period, =0 increase period
	u8	res;		// 0x07: ... reserved (align)
// aligned
	u16*	target;		// 0x08: target channel 11-bit period ('timer')
} sApuSweep;

STATIC_ASSERT(sizeof(sApuSweep) == 0x0C, "Incorrect sApuSweep!");

// === envelope unit (used in pulse and noise generators)
// To reset: clear all variables
typedef struct {
	u8	cnt;		// 0x00: counter
	u8	decay;		// 0x01: decay level counter 15..0
	u8	pervol;		// 0x02: divider period or constant volume 0..15
	u8	start;		// 0x03: <>0 start flag
// aligned
	u8	loop;		// 0x04: <>0 loop mode
	u8	mode;		// 0x05: <>0 constant volume mode, =0 envelope volume mode
	u8	res[2];		// 0x06: ... reserved (align)
} sApuEnv;

STATIC_ASSERT(sizeof(sApuEnv) == 8, "Incorrect sApuEnv!");

// === pulse generator (channel 0 and 1)
// To reset: clear all variables and call NES_ApuPulseInit()
typedef struct {
	u8		waveform;	// 0x00: duty waveform
	u8		step;		// 0x01: sequencer current step 0..7 (increasing)
	u8		res[2];		// 0x02: ... reserved (align)
// aligned
	u16		timer;		// 0x04: timer period 0..2047 (11 bits)
	u16		cnt;		// 0x06: counter (loaded with timer*2+1)
// aligned
	sApuLen		len;		// 0x08: (4) length counter
	sApuSweep	sweep;		// 0x0C: (12) sweep unit
	sApuEnv		env;		// 0x18: (8) envelope unit
} sApuPulse;

STATIC_ASSERT(sizeof(sApuPulse) == 0x20, "Incorrect sApuPulse!");

// === triangle generator (channel 2)
// To reset: clear all variables
typedef struct {
	u8		step;		// 0x00: sequencer current step 0..31 (increasing)
	u8		lincnt;		// 0x01: linear counter (max. 127 by quarter frame)
	u8		linrel;		// 0x02: linear counter reload value 0..127
	u8		reload;		// 0x03: <>0 linear counter reload request
// aligned
	u8		ctrl;		// 0x04: control flag: <>0 halt linear counter
	u8		res[3];		// 0x05: ... reserved (align)
// aligned
	u16		timer;		// 0x08: timer period 0..2047 (11 bits)
	u16		cnt;		// 0x0A: counter
// aligned
	sApuLen		len;		// 0x0C: (4) length counter (max. 254 by half frame)
} sApuTri;

STATIC_ASSERT(sizeof(sApuTri) == 0x10, "Incorrect sApuTri!");

// === noise generator (channel 3)
// To reset: clear all variables and call NES_ApuNoiseInit()
typedef struct {
	u16		shift;		// 0x00: shift register with noise sample
	u8		mode;		// 0x02: <>0 short sequence 93 or 31 steps, =0 long sequence 32767 steps
	u8		res;		// 0x03: ... reserved (align)
// aligned
	u16		timer;		// 0x04: timer period 0..2047 (11 bits)
	u16		cnt;		// 0x06: counter
// aligned
	sApuLen		len;		// 0x08: (4) length counter
	sApuEnv		env;		// 0x0C: (8) envelope unit
} sApuNoise;

STATIC_ASSERT(sizeof(sApuNoise) == 0x14, "Incorrect sApuNoise!");

// === DMC generator (channel 4)
// To reset: clear all variables and call NES_ApuDmcInit()
typedef struct {
	u8		inten;		// 0x00: <>0 enable interrupt
	u8		irq;		// 0x01: <>0 interrupt request
	u8		loop;		// 0x02: <>0 loop
	u8		silence;	// 0x03: <>0 silence
// aligned
	u8		shift;		// 0x04: sample buffer shift register
	u8		bitcnt;		// 0x05: shift register bit counter (number of remaining bits)
	u8		outlev;		// 0x06: output level 0..127 (1=increment, 0=decrement)
	u8		en;		// 0x07: <>0 enabled
// aligned
	u16		remain;		// 0x08: current sample remaining bytes
	u16		addr;		// 0x0A: current sample address
// aligned
	u16		samplen;	// 0x0C: original sample length bytes
	u16		sampaddr;	// 0x0E: original sample address
// aligned
	u16		timer;		// 0x10: timer 54..428
	u16		cnt;		// 0x12: counter
} sApuDmc;

STATIC_ASSERT(sizeof(sApuDmc) == 0x14, "Incorrect sApuDmc!");

// === Sound chip
// To reset: clear all variables and call NES_ApuSndInit()
typedef struct {
	u8		irq;		// 0x00: <>0 frame counter interrupt request
					//	- controlled from the CPU emulator rather
					//	  than from the sound hardware
	u8		intdis;		// 0x01: <>0 frame counter interrupt disable
					//	- controlled from the CPU emulator rather
					//	  than from the sound hardware
	u8		mode5;		// 0x02: <>0 frame counter 5-step mode
	u8		step;		// 0x03: frame sequencer step (max. 4 or 5)
// aligned
	u16		cnt;		// 0x04: frame counter - number of CPU clocks to the next frame step
	u16		framelen;	// 0x06: frame length (number of CPU clock ticks)
// aligned
	sApuPulse	pulse1;		// 0x08: (size 0x20) pulse generator 1 (channel 0)
	sApuPulse	pulse2;		// 0x28: (size 0x20) pulse generator 2 (channel 1)
	sApuTri		tri;		// 0x48: (size 0x10) triangle generator (channel 2)
	sApuNoise	noise;		// 0x58: (size 0x14) noise generator (channel 3)
	sApuDmc		dmc;		// 0x6C: (size 0x14) DMC generator (channel 4)
// aligned
	u32		res2[8];	// 0x80: (size 0x20) ... reserved
} sApuSnd;

STATIC_ASSERT(sizeof(sApuSnd) == 0xA0, "Incorrect sApuSnd!");

// get APU frame length
INLINE u16 NES_ApuFrameLen(sApuSnd* s) { return s->framelen; }

// get frame interrupt request flag
//  - controlled from the CPU emulator rather than from the sound hardware
//INLINE Bool NES_ApuFrameGetIrq(sApuSnd* s) { return (s->irq != 0) && (s->mode5 == 0); }

// set frame interrupt request flag
//  - controlled from the CPU emulator rather than from the sound hardware
//INLINE void NES_ApuFrameSetIrq(sApuSnd* s) { if (s->mode5 == 0) s->irq = 1; }

// reset frame interrupt request flag
//  - controlled from the CPU emulator rather than from the sound hardware
//INLINE void NES_ApuFrameClrIrq(sApuSnd* s) { s->irq = 0; }

// set frame interrupt disabled flag
//  - controlled from the CPU emulator rather than from the sound hardware
//void FASTCODE NOFLASH(NES_ApuFrameSetIntDis)(sApuSnd* s, u8 intdis);

// get frame interrupt disable flag
//  - controlled from the CPU emulator rather than from the sound hardware
//INLINE Bool NES_ApuFrameGetIntDis(sApuSnd* s) { return s->intdis != 0; }

// set frame 5-step mode (<>0 5-step mode, =0 4-step mode)
void NES_ApuFrameSetMode5(sApuSnd* s, u8 mode5);

// clock sound chip by CPU clocks
void FASTCODE NOFLASH(NES_ApuSndClock)(sApuSnd* s, u16 clk);

// get output sample
// Mono output level: 0..1308
// Stereo output level: 12 LOW bits 0..1308, 12 HIGH bits 0..1308
int FASTCODE NOFLASH(NES_ApuSndOut)(sApuSnd* s);

// write sound register 0x4000-0x4013 (reg = register 0x00-0x13)
void FASTCODE NOFLASH(NES_ApuWriteReg)(u8 reg, u8 data);

// write sound enable register 0x4015
void FASTCODE NOFLASH(NES_ApuWriteEn)(u8 data);

// write frame counter register 0x4017
void FASTCODE NOFLASH(NES_ApuWriteFrame)(u8 data);

// read sound status 0x4015
u8 FASTCODE NOFLASH(NES_ApuReadState)();

// initialize APU
void InfoNES_ApuInit();
