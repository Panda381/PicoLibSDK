
// ****************************************************************************
//
//                                PWM sound output
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

// Sounds are at format 8-bit unsigned (middle at 128), 22050 samples per second, mono
// PWM cycle is 256: TOP = 255
// Required PWM clock: 22050*256 = 5644800 Hz

// ... GP19 ... MOSI + sound output (PWM1 B)

#if USE_PWMSND		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)

#ifndef _LIB_PWMSND_H
#define _LIB_PWMSND_H

#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define PWMSND_GPIO	0	// PWM output GPIO pin (0..29)

#define PWMSND_SLICE	PWM_GPIOTOSLICE(PWMSND_GPIO) // PWM slice index
#define PWMSND_CHAN	PWM_GPIOTOCHAN(PWMSND_GPIO) // PWM channel index (=1)

#define SOUNDRATE	22050	// sound rate [Hz]
#define PWMSND_TOP	255	// PRM top (period = PWMSND_TOP + 1 = 256)
#define PWMSND_CLOCK	(SOUNDRATE*(PWMSND_TOP+1)) // PWM clock (= 22050*256 = 5644800)

#define SNDFRAC	10	// number of fraction bits
#define SNDINT (1<<SNDFRAC) // integer part of sound fraction (= 1024)

// sound formats
#define SNDFORM_PCM	0	// no compression
#define SNDFORM_ADPCM	1	// IMA ADPCM

// PWM sound channel
typedef struct {
	// current sound
	const u8*	snd;		// pointer to current playing sound
	volatile int	cnt;		// counter of current playing sound (<= 0 if no sound)
	int		inc;		// increment of the pointer
	volatile int	acc;		// accumulator of the pointer
	// next sound
	const u8*	next;		// pointer to next sound to play repeated sound
	int		nextcnt;	// counter of next sound (0 = no repeated sound)
	// volume
	int		vol;		// sound volume (SNDINT = normal = 1024)
	float		vol0;		// initial sound volume (1.0 = normal)
	// IMA ADPCM
	s16		sampblock;	// number of samples per block (0 = no preamble)
	s16		sampcnt;	// counter of samples per block (0 = end of block)
	u8		form;		// sound format SNDFORM_*
	s8		stepinx;	// step index
	s16		prevval;	// previous value
	Bool		odd;		// odd sub-sample
	u8		subsample;	// save sub-sample
} sPwmSnd;

// PWM sound channels
extern sPwmSnd PwmSound[USE_PWMSND];

// global sound OFF
extern Bool GlobalSoundOff;

// initialize PWM sound output (must be re-initialized after changing CLK_SYS system clock)
void PWMSndInit();

// terminate PWM sound output
void PWMSndTerm();

// stop playing sound
void StopSoundChan(u8 chan);
void StopSound();
void StopAllSound();

// output PWM sound (sound must be PCM 8-bit mono 22050Hz)
//  chan = channel 0..
//  snd = pointer to sound
//  len = length of sound in number of samples (for ADPCM number of samples = 2 * number of bytes)
//  rep = True to repeat sample
//  speed = relative speed (1=normal; SNDFORM_ADPCM must have speed = 1)
//  volume = volume (1=normal)
//  form = sound format SNDFORM_*
//  ext = format extended data (ADPCM: number of samples per block)
void PlaySoundChan(u8 chan, const u8* snd, int len, Bool rep, float speed, float volume, u8 form, int ext);
void PlaySound(const u8* snd, int len);
#define PLAYSOUND(snd) PlaySound((snd), count_of(snd))

// output PWM sound repeated
void PlaySoundRep(const u8* snd, int len);
#define PLAYSOUNDREP(snd) PlaySoundRep((snd), count_of(snd))

// play ADPCM sound (len = number of samples = number of bytes * 2)
void PlayADPCMChan(u8 chan, const u8* snd, int len, int sampblock);
INLINE void PlayADPCM(const u8* snd, int len, int sampblock) { PlayADPCMChan(0, snd, len, sampblock); }
#define PLAYADPCM(snd, sampblock) PlayADPCM((snd), count_of(snd)*2, (sampblock))

// play ADPCM sound repeated (len = number of samples = number of bytes * 2)
void PlayADPCMRepChan(u8 chan, const u8* snd, int len, int sampblock);
INLINE void PlayADPCMRep(const u8* snd, int len, int sampblock) { PlayADPCMRepChan(0, snd, len, sampblock); }
#define PLAYADPCMREP(snd, sampblock) PlayADPCMRep((snd), count_of(snd)*2, (sampblock))

// update sound speed (1=normal speed; SNDFORM_ADPCM must have speed = 1)
void SpeedSoundChan(u8 chan, float speed);
void SpeedSound(float speed);

// update sound volume (1=normal volume)
void VolumeSoundChan(u8 chan, float volume);
void VolumeSound(float volume);

// check if playing sound
Bool PlayingSoundChan(u8 chan);
Bool PlayingSound();

// set next repeated sound
void SetNextSoundChan(u8 chan, const u8* snd, int len);
void SetNextSound(const u8* snd, int len);

// global sound set OFF
void GlobalSoundSetOff();

// global sound set ON
void GlobalSoundSetOn();

// update sound volume after changing global volume
void GlobalVolumeUpdate();

#ifdef __cplusplus
}
#endif

#endif // _LIB_PWMSND_H

#endif // USE_PWMSND		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
