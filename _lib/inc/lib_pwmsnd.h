
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

// #define USE_PWMSND		4		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
// #define PWMSND_GPIO		20		// PWM sound output GPIO pin (left or single channel)
// #define PWMSND_GPIO_R	21		// PWM sound output GPIO pin (right channel; -1 = not used, only single channel is used)

// Default sound format: 8-bit unsigned (middle at 128), 22050 samples per second, mono

#ifndef _LIB_PWMSND_H
#define _LIB_PWMSND_H

#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_pwm.h"

#ifndef PWMSND_GPIO_R
#define PWMSND_GPIO_R	-1	// PWM sound output GPIO pin (right channel; -1 = not used, only single channel is used)
#endif

#define PWMSND_SLICE	PWM_GPIOTOSLICE(PWMSND_GPIO) // PWM slice index, left or single channel
#define PWMSND_CHAN	PWM_GPIOTOCHAN(PWMSND_GPIO) // PWM channel index, left or single channel

#define PWMSND_SLICE_R	PWM_GPIOTOSLICE(PWMSND_GPIO_R) // PWM slice index, right channel ... must be on the same slice as left channel
#define PWMSND_CHAN_R	PWM_GPIOTOCHAN(PWMSND_GPIO_R) // PWM channel index, right channel

// global sound OFF
extern volatile Bool GlobalSoundOff;

#if USE_PWMSND		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)

// both channels must be on the same PWM slice
#if PWMSND_GPIO_R >= 0
#if PWMSND_SLICE != PWMSND_SLICE_R
#error Both PWM sound channels must be on the same PWM slice!
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SOUNDRATE	22050	// reference sound rate [Hz]
#define PWMSND_BITS	11	// number of bits per PWM sample (= 2048) (... + 1 bit with time dithering)
#define PWMSND_TOP	((1<<PWMSND_BITS)-1)	// PWM top (period = PWMSND_TOP + 1 = 2048)
#define PWMSND_CLOCK	(SOUNDRATE*(PWMSND_TOP+1)) // default PWM clock (= 22050*2048 = 45158400)

#define SNDFRAC	10		// number of fraction bits
#define SNDINT (1<<SNDFRAC)	// integer part of sound fraction (= 1024)

// predefined sound speeds
#define SNDSPEED_8K	((float)8000/SOUNDRATE)		//  8000 Hz (= 0.36281f)
#define SNDSPEED_11K	((float)11025/SOUNDRATE)	// 11025 Hz (= 0.50000f)
#define SNDSPEED_12K	((float)12000/SOUNDRATE)	// 12000 Hz (= 0.54422f)
#define SNDSPEED_16K	((float)16000/SOUNDRATE)	// 16000 Hz (= 0.72562f)
#define SNDSPEED_22K	((float)22050/SOUNDRATE)	// 22050 Hz (= 1.00000f)
#define SNDSPEED_24K	((float)24000/SOUNDRATE)	// 24000 Hz (= 1.08844f)
#define SNDSPEED_32K	((float)32000/SOUNDRATE)	// 32000 Hz (= 1.45125f)
#define SNDSPEED_44K	((float)44100/SOUNDRATE)	// 44100 Hz (= 2.00000f)
#define SNDSPEED_48K	((float)48000/SOUNDRATE)	// 48000 Hz (= 2.17687f)

// sound formats
#define SNDFORM_PCM	0	// no compression, 8-bits unsigned per sample (middle at 128), recommended sample rate 22050 Hz
#define SNDFORM_ADPCM	1	// IMA ADPCM, 4-bit compression, recommended sample rate 16000 Hz (speed = 0.726
#define SNDFORM_PCM16	2	// no compression, 16-bits signed per sample (middle at 0), recommended sample rate 32000 Hz

#define SNDFORM_STEREO	B6	// add this flag to the sound format - stereo 2 channels

#define SNDFORM_PCM_S	(SNDFORM_PCM|SNDFORM_STEREO) 	// 8-bits unsigned per sample, stereo
#define SNDFORM_ADPCM_S	(SNDFORM_ADPCM|SNDFORM_STEREO) 	// IMA ADPCM, 4-bit compression, stereo
#define SNDFORM_PCM16_S	(SNDFORM_PCM16|SNDFORM_STEREO)	// 16-bits signed per sample, stereo

// repeat mode
#define SNDREPEAT_NO		0	// no repeat (or use False)
#define SNDREPEAT_REPEAT	1	// repeat sound (or use True)
#define SNDREPEAT_STREAM	2	// streaming sound

// PWM sound channel
typedef struct {
	// current sound
	const u8*	snd;		// pointer to current playing sound (4-bit, 8-bit or 16-bit format)
	volatile int	cnt;		// counter of current playing sound (<= 0 if no sound)
	volatile int	acc;		// accumulator of the pointer * SNDINT
	// speed
	int		inc;		// increment of the pointer * SNDINT
	float		speed;		// initial sound speed (relative to 22050 Hz)
	// next sound
	const u8*	next;		// pointer to next sound to play repeated sound
	int		nextcnt;	// counter of next sound (0 = no repeated sound)
	Bool		stream;		// use streaming sound
	Bool		useirq;		// use streaming IRQ (set this entry manually, after calling PlaySoundChan() function)
	u8		irq;		// streaming IRQ (set this entry manually, after calling PlaySoundChan() function)
	// volume
	int		vol;		// sound volume (SNDINT = normal = 1024)
	float		vol0;		// initial sound volume (1.0 = normal)
	// IMA ADPCM
	s16		sampblock;	// number of samples per block (0 = no preamble)
	s16		sampcnt;	// counter of samples per block (0 = end of block)
	s16		oldval[2];	// old value
	s16		prevval[2];	// previous value (current value)
	u8		form;		// sound format SNDFORM_*
	s8		stepinx[2];	// step index
	Bool		odd;		// odd sub-sample
	u8		byte4;		// counter of 4-byte
	u8		subsample[2];	// save sub-sample
} sPwmSnd;

// PWM sound output pins (default set to PWMSND_GPIO and PWMSND_GPIO_R)
// - Must be initialized before calling PWMSndInit().
// - Set PWMSndGpioR to -1 to use mono output to PWMSndGpio.
// - Both PWM sound channels L+R must be on the same PWM slice.
extern int PWMSndGpio, PWMSndGpioR;
extern int PWMSndChan;
extern int PWMSndChanR;
extern int PWMSndSlice;
extern Bool PWMSndWasInit;

// PWM sound channels
extern sPwmSnd PwmSound[USE_PWMSND];

// initialize PWM sound output (needs not be re-initialized after changing CLK_SYS system clock,
// but if playing any sound, SpeedSoundUpdate() should be called to update speed of current sounds).
void PWMSndInit();

// terminate PWM sound output
void PWMSndTerm();

// stop playing sound
void StopSoundChan(int chan);

// stop playing sound of channel 0
void StopSound();

// stop playing sounds of all channels
void StopAllSound();

// Convert length of sound in bytes to number of samples
//  size = length of sound in number of bytes (use sizeof(array))
//  form = sound format SNDFORM_* (4-bit, 8-bit, 16-bit, mono or stereo)
// Returns length of sound in samples (or double-samples for stereo)
int SoundByteToLen(int size, int form);

// play sound
//  chan = channel 0..
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
//  rep = repeat mode SNDREPEAT_*
//  speed = speed relative to sample rate 22050 Hz (1=normal, you can use constants SNDSPEED_*)
//  volume = volume (1=normal)
//  form = sound format SNDFORM_* (4-bit, 8-bit, 16-bit, mono or stereo)
//  ext = format extended data (ADPCM: number of samples per block, see WAV file)
// If want to use streaming mode with IRQ, set the 'useirq' and 'irq' entries manually, after calling PlaySoundChan() function.
void PlaySoundChan(int chan, const void* snd, int size, int rep, float speed, float volume, int form, int ext);

// play sound at channel 0, format SNDFORM_PCM: 8-bit 22050 Hz mono
//  snd = pointer to sound in format SNDFORM_PCM: 8-bit 22050 Hz mono
//  size = length of sound in number of bytes (use sizeof(array))
void PlaySound(const void* snd, int size);
#define PLAYSOUND(snd) PlaySound((snd), sizeof(snd))

// play sound at channel 0 repeated, format SNDFORM_PCM: 8-bit 22050 Hz mono
//  snd = pointer to sound in format SNDFORM_PCM: 8-bit 22050 Hz mono
//  size = length of sound in number of bytes (use sizeof(array))
void PlaySoundRep(const void* snd, int size);
#define PLAYSOUNDREP(snd) PlaySoundRep((snd), sizeof(snd))

// play ADPCM sound, format SNDFORM_ADPCM: 4-bit 22050 Hz mono
//  chan = channel 0..
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
//  sampblock = number of samples per block (see WAV file)
void PlayADPCMChan(int chan, const void* snd, int size, int sampblock);

// play ADPCM sound at channel 0, format SNDFORM_ADPCM: 4-bit 22050 Hz mono
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
//  sampblock = number of samples per block (see WAV file)
void PlayADPCM(const void* snd, int size, int sampblock);
#define PLAYADPCM(snd, sampblock) PlayADPCM((snd), sizeof(snd), (sampblock))

// play ADPCM sound repeated, format SNDFORM_ADPCM: 4-bit 22050 Hz mono
//  chan = channel 0..
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
//  sampblock = number of samples per block (see WAV file)
void PlayADPCMRepChan(int chan, const void* snd, int size, int sampblock);

// play ADPCM sound at channel 0 repeated, format SNDFORM_ADPCM: 4-bit 22050 Hz mono
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
//  sampblock = number of samples per block (see WAV file)
void PlayADPCMRep(const void* snd, int len, int sampblock);
#define PLAYADPCMREP(snd, sampblock) PlayADPCMRep((snd), sizeof(snd), (sampblock))

// update sound speed (1=normal speed)
void SpeedSoundChan(int chan, float speed);

// update sound speed of channel 0 (1=normal speed)
void SpeedSound(float speed);

// update all sound speeds after changing system clock
void SpeedSoundUpdate();

// update sound volume (1=normal volume)
void VolumeSoundChan(int chan, float volume);

// update sound volume of channel 0 (1=normal volume)
void VolumeSound(float volume);

// check if sound is playing
Bool PlayingSoundChan(int chan);

// check if sound of channel 0 is playing
Bool PlayingSound();

// set next repeated sound in the same format
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
void SetNextSoundChan(int chan, const void* snd, int size);

// set next repeated sound of channel 0 in the same format
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
void SetNextSound(const void* snd, int size);

// check if streaming buffer is empty
Bool SoundStreamIsEmpty(int chan);

// set next streaming buffer
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
void SoundStreamSetNext(int chan, const void* snd, int size);

// global sound set OFF
void GlobalSoundSetOff();

// global sound set ON
void GlobalSoundSetOn();

// update sound volume after changing global volume
void GlobalVolumeUpdate();

#ifdef __cplusplus
}
#endif

#endif // USE_PWMSND		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)

#endif // _LIB_PWMSND_H
