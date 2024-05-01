
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

// current sound output
#define PC_SND_NO	0	// no sound
#define PC_SND_SPK	1	// output to PC speaker

#define PC_SND_CLK	1193182	// PC speaker divider clock (1.193182 MHz)
#define PC_VOLUME_MAX	3	// max. volume (steps 0..3)

extern u8 PC_SoundOut;	// current sound output
extern u8 PC_SoundOutSave; // current sound output - save
extern u8 PC_SoundMode; // sound counter 2 setup, port 43h
extern u16 PC_SoundDiv;	// sound counter divider, port 42h (default: 1 kHz)
extern u8 PC_SoundGate; // gate of sound counter 2, port 61h (bit 0: counting enable, bit 1: output to speaker enable)
extern Bool PC_SoundHigh; // flip/flop - read/write high byte
extern u16 PC_SoundLatch; // latch

// setup sound output to default state (should be called on program start)
void PC_SoundDef();

// update enable/disable sound output (after changing GlobalSoundOff; PC speaker: after changing PC_SoundGate)
void PC_SoundOnUpdate();

// update PC speaker frequency (PC speaker: after changing PC_SoundMode or PC_SoundDiv)
void PC_SoundFreqUpdate();

// initialize sound output to PC_SND_* (should be called on start of sound)
void PC_SoundInit(int out);

// terminate sound output (should be called on program pause or termination)
void PC_SoundTerm();
