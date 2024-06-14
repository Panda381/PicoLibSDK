
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

// Note: Cannot use DMA, because sample is 8-bit, but CC port requires 16-bit write.

#include "../../global.h"	// globals

// global sound OFF
volatile Bool GlobalSoundOff = False;

#if USE_PWMSND		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)

#include "../../_sdk/inc/sdk_irq.h"
#include "../inc/lib_pwmsnd.h"
#include "../inc/lib_config.h"

// PWM sound channels
sPwmSnd PwmSound[USE_PWMSND];

// IMA ADPCM tables
#define ADPCM_MINVAL	-32768		// IMA ADPCM minimal value
#define ADPCM_MAXVAL	+32767		// IMA ADPCM maximal value
#define ADPCM_STEPS	89		// IMA ADPCM number of steps

const s8 ADPCM_TabInx[16] =	// IMA ADPCM table of index change steps
{
	-1,	-1,	-1,	-1,	// +0 .. +3, decrease step size
	2,	4,	6,	8,	// +4 .. +7, increase step size
	-1,	-1,	-1,	-1,	// -0 .. -3, decrease step size
	2,	4,	6,	8,	// -4 .. -7, increase step size
};

const s16 ADPCM_StepSize[ADPCM_STEPS] =	// IMA ADPCM table of step sizes
{
	7,	8,	9,	10,	11,	12,	13,	14,
	16,	17,	19,	21,	23,	25,	28,	31,
	34,	37,	41,	45,	50,	55,	60,	66,
	73,	80,	88,	97,	107,	118,	130,	143,
	157,	173,	190,	209,	230,	253,	279,	307,
	337,	371,	408,	449,	494,	544,	598,	658,
	724,	796,	876,	963,	1060,	1166,	1282,	1411,
	1552,	1707,	1878,	2066,	2272,	2499,	2749,	3024,
	3327,	3660,	4026,	4428,	4871,	5358,	5894,	6484,
	7132,	7845,	8630,	9493,	10442,	11487,	12635,	13899,
	15289,	16818,	18500,	20350,	22385,	24623,	27086,	29794,
	32767
};

// PWM sound interrupt service
void PWMSndIrq()
{
	// clear interrupt request
	PWM_IntClear(PWMSND_SLICE);

	// default sample if no sound		
	int samp = 128*SNDINT;

	// check if sound is playing
	sPwmSnd* s = PwmSound;
	int ch;
	for (ch = 0; ch < USE_PWMSND; ch++)
	{
		// get sound counter (<= 0 if no sound)
		int cnt = s->cnt;
		if (cnt > 0)
		{
			// get next sample
			const u8* snd = s->snd;
			int acc = s->acc;

			// PCM format
			u8 form = s->form;
			if (form == SNDFORM_PCM)
			{
				samp += (int)(*snd - 128) * s->vol;

				// increment pointer accumulator
				acc = acc + s->inc;
				int i = acc >> SNDFRAC; // whole increment
				snd += i; // shift sound pointer
				cnt -= i; // shift sound counter
				acc &= (SNDINT-1); // clear low invalid bits
			}

			// ADPCM format
			else if (form == SNDFORM_ADPCM)
			{
				int val;

				// preamble block
				if (s->sampcnt <= 0)
				{
					// get current value from block header
					val = (s16)(snd[0] + snd[1]*256);
					s->prevval = (s16)val;

					// get current index from block header
					s->stepinx = snd[2];
					snd += 4;

					// set counter of samples per block
					s->sampcnt = s->sampblock;
					s->odd = False; // odd sub-sample

					cnt -= 8; // sample count correction (= 4 bytes)
				}
				else
				{
					// get next sample
					u8 delta;
					if (s->odd) // odd sub-sample (higher 4 bits)
						delta = (u8)(s->subsample >> 4);
					else // even sub-sample (lower 4 bits)
					{
						delta = *snd++;
						s->subsample = delta;
						delta &= 0x0f;
					}
					cnt--; // sample counter
					s->odd = !s->odd; // flag of odd sub-sample

					// get step size
					s8 stepinx = s->stepinx;
					s16 step = ADPCM_StepSize[stepinx];

					// shift step index
					stepinx += ADPCM_TabInx[delta];

					// limit step index
					if ((uint)stepinx >= (uint)ADPCM_STEPS)
					{
						if (stepinx < 0)
							stepinx = 0;
						else
							stepinx = ADPCM_STEPS - 1;
					}
					s->stepinx = stepinx;

					// get next difference
					s16 dif = step >> 3;
					if ((delta & B0) != 0) dif += step >> 2;
					if ((delta & B1) != 0) dif += step >> 1;
					if ((delta & B2) != 0) dif += step ;
					if ((delta & B3) != 0) dif = -dif; // sign correction

					// add difference to previous value
					val = s->prevval + dif;
					if (val > ADPCM_MAXVAL) val = ADPCM_MAXVAL;
					if (val < ADPCM_MINVAL) val = ADPCM_MINVAL;
					s->prevval = (s16)val;
				}

				// sample counter
				s->sampcnt--;

				// add value to output
				samp += (val >> 8) * s->vol;
			}

			// repeated sample
			if (cnt <= 0) // end of sound?
			{
				s->sampcnt = 0; // reset counter of samples per block
				s->stepinx = 0; // step index
				s->prevval = 0; // previous value
				s->odd = False; // odd sub-sample
				cnt = s->nextcnt; // counter of next sound
				acc = 0; // reset accumulator
				snd = s->next; // pointer to next sound
			}

			// save new pointer
			s->cnt = cnt; // save new counter
			s->acc = acc; // save new accumulator
			s->snd = snd; // save new pointer
		}

		// shift pointer to next sound channel
		s++;
	}

	// limit sound sample
	samp >>= SNDFRAC;
	if (samp < 0) samp = 0;
	if (samp > 255) samp = 255;

	// write PWM sample
	PWM_Comp(PWMSND_SLICE, PWMSND_CHAN, (u8)samp);
}

// initialize PWM sound output (must be re-initialized after changing CLK_SYS system clock)
void PWMSndInit()
{
	// sound is OFF
	if (GlobalSoundOff) return;

	// reset PWM to default state
	PWM_Reset(PWMSND_SLICE);

	// set GPIO function to PWM
	PWM_GpioInit(PWMSND_GPIO);

	// clear descriptors
	int i;
	for (i = 0; i < USE_PWMSND; i++) PwmSound[i].cnt = 0;

	// set IRQ handler
	SetHandler(IRQ_PWM_WRAP, PWMSndIrq);
	NVIC_IRQEnable(IRQ_PWM_WRAP);

	// set clock divider
	//  125 MHz: 125000000/5644800 = 22.144, INT=22, FRAC=2,
	//     real sample rate = 125000000/(22+2/16)/256 = 22069Hz
	PWM_Clock(PWMSND_SLICE, PWMSND_CLOCK);

	// set period to 256 cycles
	PWM_Top(PWMSND_SLICE, PWMSND_TOP);

	// write default PWM sample
	PWM_Comp(PWMSND_SLICE, PWMSND_CHAN, 128);

	// enable PWM (will be enabled on 1st using of some sound to avoid speaker noise)
//	PWM_Enable(PWMSND_SLICE);

	// interrupt enable
	PWM_IntEnable(PWMSND_SLICE);
}

// terminate PWM sound output
void PWMSndTerm()
{
	// disable PWM
	PWM_Disable(PWMSND_SLICE);

	// interrupt disable
	PWM_IntDisable(PWMSND_SLICE);

	// clear interrupt request
	PWM_IntClear(PWMSND_SLICE);

	// reset PWM to default state
	PWM_Reset(PWMSND_SLICE);

	// set GPIO function to default
	GPIO_Init(PWMSND_GPIO);
}

// stop playing sound
void StopSoundChan(u8 chan)
{
	dmb();
	PwmSound[chan].cnt = 0;
	dmb();
}

void StopSound()
{
	StopSoundChan(0);
}

void StopAllSound()
{
	int i;
	for (i = 0; i < USE_PWMSND; i++) StopSoundChan(i);
}

// output PWM sound (sound must be PCM 8-bit mono 22050Hz)
//  chan = channel 0..
//  snd = pointer to sound
//  len = length of sound in number of samples (for ADPCM number of samples = 2 * number of bytes)
//  rep = True to repeat sample
//  speed = relative speed (1=normal; SNDFORM_ADPCM must have speed = 1)
//  volume = volume (1=normal)
//  form = sound format SNDFORM_*
//  ext = format extended data (ADPCM: number of samples per block)
void PlaySoundChan(u8 chan, const u8* snd, int len, Bool rep, float speed, float volume, u8 form, int ext)
{
	if (GlobalSoundOff) return;

	// prepare volume
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	int v = (int)(SNDINT * volume * Config.volume / CONFIG_VOLUME_FULLSTEP + 0.5f);
#else
	int v = (int)(SNDINT * volume + 0.5f);
#endif

	// prepare speed increment
	if (form == SNDFORM_ADPCM) speed = 1.0f; // ADPCM must have speed = 1.0
	int sinc = (int)(SNDINT * speed + 0.5f); // sample increment

	// stop playing sound
	StopSoundChan(chan);

	// pointer to sound channel
	sPwmSnd* s = &PwmSound[chan];

	// repeated sound
	s->nextcnt = 0;
	if (rep)
	{
		s->next = snd;
		s->nextcnt = len;
	}

	// sound speed
	s->inc = sinc;
	s->acc = 0;

	// sound volume
	s->vol0 = volume;
	s->vol = v;

	// prepare sound format
	s->sampblock = (s16)ext; // number of samples per block
	s->sampcnt = 0; // counter of samples per block
	s->form = form; // sound format
	s->stepinx = 0; // step index
	s->prevval = 0; // previous value
	s->odd = False; // odd sub-sample

	// start current sound
	s->snd = snd;
	dmb();
	s->cnt = len;
	dmb();

	// enable PWM
	PWM_Enable(PWMSND_SLICE);
}

void PlaySound(const u8* snd, int len)
{
	PlaySoundChan(0, snd, len, False, 1, 1, SNDFORM_PCM, 0);
}

// output PWM sound repeated
void PlaySoundRep(const u8* snd, int len)
{
	PlaySoundChan(0, snd, len, True, 1, 1, SNDFORM_PCM, 0);
}

// play ADPCM sound (len = number of samples = number of bytes * 2)
void PlayADPCMChan(u8 chan, const u8* snd, int len, int sampblock)
{
	PlaySoundChan(chan, snd, len, False, 1, 1, SNDFORM_ADPCM, sampblock);
}

// play ADPCM sound repeated (len = number of samples = number of bytes * 2)
void PlayADPCMRepChan(u8 chan, const u8* snd, int len, int sampblock)
{
	PlaySoundChan(chan, snd, len, True, 1, 1, SNDFORM_ADPCM, sampblock);
}

// update sound speed (1=normal speed; SNDFORM_ADPCM must have speed = 1)
void SpeedSoundChan(u8 chan, float speed)
{
	PwmSound[chan].inc = (int)(SNDINT * speed + 0.5f);
}

void SpeedSound(float speed)
{
	SpeedSoundChan(0, speed);
}

// update sound volume (1=normal volume)
void VolumeSoundChan(u8 chan, float volume)
{
	sPwmSnd* s = &PwmSound[chan];
	s->vol0 = volume;
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	s->vol = (int)(SNDINT * volume * Config.volume / CONFIG_VOLUME_FULLSTEP + 0.5f);
#else
	s->vol = (int)(SNDINT * volume + 0.5f);
#endif
}

void VolumeSound(float volume)
{
	VolumeSoundChan(0, volume);
}

// check if playing sound
Bool PlayingSoundChan(u8 chan)
{
	if (GlobalSoundOff) return False;

	sPwmSnd* s = &PwmSound[chan];
	return s->cnt > 0;
}

Bool PlayingSound()
{
	if (GlobalSoundOff) return False;

	return PlayingSoundChan(0);
}

// set next repeated sound
void SetNextSoundChan(u8 chan, const u8* snd, int len)
{
	sPwmSnd* s = &PwmSound[chan];

	// check if this sound is already next sound
	if (PlayingSoundChan(chan) && (s->next == snd) && (s->nextcnt == len)) return;

	// disable next sound
	s->nextcnt = 0;
	dmb();

	// start sound if not playing
	if (s->cnt == 0)
	{
		s->sampcnt = 0; // counter of samples per block
		s->stepinx = 0; // step index
		s->prevval = 0; // previous value
		s->odd = False; // odd sub-sample

		s->snd = snd;
		dmb();
		s->cnt = len;
		dmb();
	}

	// set next sound
	s->next = snd;
	dmb();
	s->nextcnt = len;
}

void SetNextSound(const u8* snd, int len)
{
	SetNextSoundChan(0, snd, len);
}

// global sound set OFF
void GlobalSoundSetOff()
{
	GlobalSoundOff = True;
	PWMSndTerm();
}

// global sound set ON
void GlobalSoundSetOn()
{
	PWMSndTerm();
	GlobalSoundOff = False;
	PWMSndInit();
}

// update sound volume after changing global volume
void GlobalVolumeUpdate()
{
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	int i;
	sPwmSnd* s = PwmSound;
	for (i = 0; i < USE_PWMSND; i++)
	{
		s->vol = (int)(SNDINT * s->vol0 * Config.volume / CONFIG_VOLUME_FULLSTEP + 0.5f);
		s++;
	}
#endif
}

#endif // USE_PWMSND		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
