
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
#include "../../_sdk/inc/sdk_clocks.h"
#include "../inc/lib_pwmsnd.h"
#include "../inc/lib_config.h"

// PWM sound output pins (default set to PWMSND_GPIO and PWMSND_GPIO_R)
// - Must be initialized before calling PWMSndInit().
// - Set PWMSndGpioR to -1 to use mono output to PWMSndGpio.
// - Both PWM sound channels L+R must be on the same PWM slice.
int PWMSndGpio = PWMSND_GPIO;
int PWMSndGpioR = PWMSND_GPIO_R;
int PWMSndChan = PWM_GPIOTOCHAN(PWMSND_GPIO);
int PWMSndChanR = PWM_GPIOTOCHAN(PWMSND_GPIO_R);
int PWMSndSlice = PWM_GPIOTOSLICE(PWMSND_GPIO);
Bool PWMSndWasInit = False;

// PWM sound channels
sPwmSnd PwmSound[USE_PWMSND];

// time dithering
int PwmSoundDither = 0;

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
	PWM_IntClear(PWMSndSlice);

	// default sample if no sound		
	int samp = SNDINT*PWMSND_TOP/2;
	int sampR = samp;

	// loop sound channels
	sPwmSnd* s = PwmSound;
	int i, ch, k, k2, kk;
	for (ch = 0; ch < USE_PWMSND; ch++)
	{
		// get sound counter (<= 0 if no sound)
		int cnt = s->cnt;
		if (cnt > 0)
		{
			// prepare pointer to the sample
			const u8* snd = s->snd;
			int acc = s->acc;
			int sndover = 0;

			// PCM 8-bit format mono
			u8 form = s->form;
			if (form == SNDFORM_PCM)
			{
				// get next sample, interpolate with next value
				k = ((int)*snd - 128);
				if (cnt <= 1)
					kk = k;
				else
					kk = ((int)snd[1] - 128);
				k = kk*acc + k*(SNDINT - acc);
				k *= s->vol;
				k >>= 8 - PWMSND_BITS + SNDFRAC;

				samp += k;
				sampR += k;

				// increment pointer accumulator
				acc = acc + s->inc;
				i = acc >> SNDFRAC; // whole increment
				snd += i; // shift sound pointer
				cnt -= i; // shift sound counter
				sndover = -cnt; // overflow correction
				acc &= (SNDINT-1); // clear high bits
			}

			// PCM 8-bit format stereo
			else if (form == SNDFORM_PCM_S)
			{
				// get next sample L and R, interpolate with next value
				k = ((int)snd[0] - 128);
				if (cnt <= 1)
					kk = k;
				else
					kk = ((int)snd[2] - 128);
				k = kk*acc + k*(SNDINT - acc);
				k *= s->vol;
				k >>= 8 - PWMSND_BITS + SNDFRAC;

				k2 = ((int)snd[1] - 128);
				if (cnt <= 1)
					kk = k;
				else
					kk = ((int)snd[3] - 128);
				k2 = kk*acc + k2*(SNDINT - acc);
				k2 *= s->vol;
				k2 >>= 8 - PWMSND_BITS + SNDFRAC;

				if (PWMSndGpioR >= 0)
				{
					samp += k;
					sampR += k2;
				}
				else
					samp += (k+k2) >> 1;

				// increment pointer accumulator
				acc = acc + s->inc;
				i = acc >> SNDFRAC; // whole increment
				snd += i*2; // shift sound pointer
				cnt -= i; // shift sound counter
				sndover = -2*cnt; // overflow correction
				acc &= (SNDINT-1); // clear high bits
			}

			// PCM 16-bit format mono
			else if (form == SNDFORM_PCM16)
			{
				// get next sample, interpolate with next value
				k = (u8)snd[0] + ((int)(s8)snd[1] << 8);
				if (cnt <= 1)
					kk = k;
				else
					kk = (u8)snd[2] + ((int)(s8)snd[3] << 8);
				k = kk*acc + k*(SNDINT - acc);
				k >>= SNDFRAC;
				k *= s->vol;
				k >>= 16 - PWMSND_BITS;

				samp += k;
				sampR += k;

				// increment pointer accumulator
				acc = acc + s->inc;
				i = acc >> SNDFRAC; // whole increment
				snd += i*2; // shift sound pointer
				cnt -= i; // shift sound counter
				sndover = -2*cnt; // overflow correction
				acc &= (SNDINT-1); // clear high bits
			}

			// PCM 16-bit format stereo
			else if (form == SNDFORM_PCM16_S)
			{
				// get next sample L and R, interpolate with next value
				k = (u8)snd[0] + ((int)(s8)snd[1] << 8);
				if (cnt <= 1)
					kk = k;
				else
					kk = (u8)snd[4] + ((int)(s8)snd[5] << 8);
				k = kk*acc + k*(SNDINT - acc);
				k >>= SNDFRAC;
				k *= s->vol;
				k >>= 16 - PWMSND_BITS;

				k2 = (u8)snd[2] + ((int)(s8)snd[3] << 8);
				if (cnt <= 1)
					kk = k2;
				else
					kk = (u8)snd[6] + ((int)(s8)snd[7] << 8);
				k2 = kk*acc + k2*(SNDINT - acc);
				k2 >>= SNDFRAC;
				k2 *= s->vol;
				k2 >>= 16 - PWMSND_BITS;

				if (PWMSndGpioR >= 0)
				{
					samp += k;
					sampR += k2;
				}
				else
					samp += (k+k2) >> 1;

				// increment pointer accumulator
				acc = acc + s->inc;
				i = acc >> SNDFRAC; // whole increment
				snd += i*4; // shift sound pointer
				cnt -= i; // shift sound counter
				sndover = -4*cnt; // overflow correction
				acc &= (SNDINT-1); // clear high bits
			}

			// ADPCM format mono
			else if (form == SNDFORM_ADPCM)
			{
				int val;

				// prepare increment counter
				acc = acc + s->inc;
				i = acc >> SNDFRAC; // whole increment
				acc &= (SNDINT-1); // clear high bits

				if (i > 0)
				{
					s->oldval[0] = s->prevval[0];
				}

				// skip to next valid value
				for (; i > 0; i--)
				{
					// preamble block
					if (s->sampcnt <= 0)
					{
						// get current value from block header
						val = (s16)(snd[0] + snd[1]*256);
						s->prevval[0] = (s16)val;

						// get current index from block header
						s->stepinx[0] = snd[2];
						snd += 4;

						// set counter of samples per block
						s->sampcnt = s->sampblock;
						s->odd = False; // odd sub-sample

						cnt -= 8; // sample count correction (= 4 bytes)
					}
					else
					{
						// get next sample delta
						u8 delta;
						if (s->odd) // odd sub-sample (higher 4 bits)
							delta = (u8)(s->subsample[0] >> 4);
						else // even sub-sample (lower 4 bits)
						{
							delta = *snd++;
							s->subsample[0] = delta;
							delta &= 0x0f;
						}
						cnt--; // sample counter
						s->odd = !s->odd; // flag of odd sub-sample

						// get step size
						s8 stepinx = s->stepinx[0];
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
						s->stepinx[0] = stepinx;

						// get next difference
						s16 dif = step >> 3;
						if ((delta & B0) != 0) dif += step >> 2;
						if ((delta & B1) != 0) dif += step >> 1;
						if ((delta & B2) != 0) dif += step ;
						if ((delta & B3) != 0) dif = -dif; // sign correction

						// add difference to previous value
						val = s->prevval[0] + dif;
						if (val > ADPCM_MAXVAL) val = ADPCM_MAXVAL;
						if (val < ADPCM_MINVAL) val = ADPCM_MINVAL;
						s->prevval[0] = (s16)val;
					}

					// sample counter
					s->sampcnt--;
					if (cnt <= 0)
					{
						cnt = 0;
						break;
					}
				}

				// add value to output, interpolate
				k = s->prevval[0]*acc + s->oldval[0]*(SNDINT - acc);
				k >>= SNDFRAC;
				k *= s->vol;
				k >>= 16 - PWMSND_BITS;
				samp += k;
				sampR += k;
			}

			// ADPCM format stereo
			else if (form == SNDFORM_ADPCM_S)
			{
				int val, val2;

				// prepare increment counter
				acc = acc + s->inc;
				i = acc >> SNDFRAC; // whole increment
				acc &= (SNDINT-1); // clear high bits

				if (i > 0)
				{
					s->oldval[0] = s->prevval[0];
					s->oldval[1] = s->prevval[1];
				}

				// skip to next valid value
				for (; i > 0; i--)
				{
					// preamble block
					if (s->sampcnt <= 0)
					{
						// get current value from block header
						val = (s16)(snd[0] + snd[1]*256);
						s->prevval[0] = (s16)val;

						val2 = (s16)(snd[4] + snd[5]*256);
						s->prevval[1] = (s16)val2;

						// get current index from block header
						s->stepinx[0] = snd[2];
						s->stepinx[1] = snd[6];
						snd += 8;

						// set counter of samples per block
						s->sampcnt = s->sampblock;
						s->odd = False; // odd sub-sample
						s->byte4 = 0;	// counter of 4-byte

						cnt -= 8; // sample count correction (= 8 bytes)
					}
					else
					{
						// get next sample delta
						u8 delta, delta2;
						if (s->odd) // odd sub-sample (higher 4 bits)
						{
							delta = (u8)(s->subsample[0] >> 4);
							delta2 = (u8)(s->subsample[1] >> 4);
						}
						else // even sub-sample (lower 4 bits)
						{
							delta = snd[0];
							s->subsample[0] = delta;
							delta &= 0x0f;

							delta2 = snd[4];
							s->subsample[1] = delta2;
							delta2 &= 0x0f;

							snd++;
							s->byte4++;
							if (s->byte4 >= 4)
							{
								s->byte4 = 0;
								snd += 4; // skip right channel
							}
						}
						cnt--; // sample counter
						s->odd = !s->odd; // flag of odd sub-sample

						// get step size
						s8 stepinx = s->stepinx[0];
						s16 step = ADPCM_StepSize[stepinx];

						s8 stepinx2 = s->stepinx[1];
						s16 step2 = ADPCM_StepSize[stepinx2];

						// shift step index
						stepinx += ADPCM_TabInx[delta];
						stepinx2 += ADPCM_TabInx[delta2];

						// limit step index
						if ((uint)stepinx >= (uint)ADPCM_STEPS)
						{
							if (stepinx < 0)
								stepinx = 0;
							else
								stepinx = ADPCM_STEPS - 1;
						}
						s->stepinx[0] = stepinx;

						if ((uint)stepinx2 >= (uint)ADPCM_STEPS)
						{
							if (stepinx2 < 0)
								stepinx2 = 0;
							else
								stepinx2 = ADPCM_STEPS - 1;
						}
						s->stepinx[1] = stepinx2;

						// get next difference
						s16 dif = step >> 3;
						if ((delta & B0) != 0) dif += step >> 2;
						if ((delta & B1) != 0) dif += step >> 1;
						if ((delta & B2) != 0) dif += step ;
						if ((delta & B3) != 0) dif = -dif; // sign correction

						s16 dif2 = step2 >> 3;
						if ((delta2 & B0) != 0) dif2 += step2 >> 2;
						if ((delta2 & B1) != 0) dif2 += step2 >> 1;
						if ((delta2 & B2) != 0) dif2 += step2 ;
						if ((delta2 & B3) != 0) dif2 = -dif2; // sign correction

						// add difference to previous value
						val = s->prevval[0] + dif;
						if (val > ADPCM_MAXVAL) val = ADPCM_MAXVAL;
						if (val < ADPCM_MINVAL) val = ADPCM_MINVAL;
						s->prevval[0] = (s16)val;

						val2 = s->prevval[1] + dif2;
						if (val2 > ADPCM_MAXVAL) val2 = ADPCM_MAXVAL;
						if (val2 < ADPCM_MINVAL) val2 = ADPCM_MINVAL;
						s->prevval[1] = (s16)val2;
					}

					// sample counter
					s->sampcnt--;
					if (cnt <= 0)
					{
						cnt = 0;
						break;
					}
				}

				// add value to output, interpolate
				k = s->prevval[0]*acc + s->oldval[0]*(SNDINT - acc);
				k >>= SNDFRAC;
				k *= s->vol;
				k >>= 16 - PWMSND_BITS;

				k2 = s->prevval[1]*acc + s->oldval[1]*(SNDINT - acc);
				k2 >>= SNDFRAC;
				k2 *= s->vol;
				k2 >>= 16 - PWMSND_BITS;

				if (PWMSndGpioR >= 0)
				{
					samp += k;
					sampR += k2;
				}
				else
					samp += (k + k2) >> 1;
			}

			// repeated sample
			if (cnt <= 0) // end of sound?
			{
				s->sampcnt = 0; // reset counter of samples per block
				s->stepinx[0] = 0; // step index L
				s->stepinx[1] = 0; // step index R
				s->prevval[0] = 0; // previous value L
				s->prevval[1] = 0; // previous value R
				s->odd = False; // odd sub-sample
				s->oldval[0] = 0; // old value L
				s->oldval[1] = 0; // old value R
				snd = s->next + sndover; // pointer to next sound
				cnt += s->nextcnt; // counter of next sound
				if (s->stream)
				{
					s->nextcnt = 0; // streaming
					if (s->useirq)
					{
						// streaming IRQ
						dmb();
						NVIC_IRQForce(s->irq);
					}
				}
			}

			// save new pointer
			s->cnt = cnt; // save new counter
			s->acc = acc; // save new accumulator
			s->snd = snd; // save new pointer
		}

		// shift pointer to next sound channel
		s++;
	}

	// time dithering
	int di = PwmSoundDither;
	di ^= 1;
	PwmSoundDither = di;
	di *= SNDINT/2;
	di += SNDINT/4;
	samp += di;
	sampR += di;

	// limit sound sample
	samp >>= SNDFRAC;
	if (samp < 0) samp = 0;
	if (samp > PWMSND_TOP) samp = PWMSND_TOP;

	if (PWMSndGpioR >= 0)
	{
		sampR >>= SNDFRAC;
		if (sampR < 0) sampR = 0;
		if (sampR > PWMSND_TOP) sampR = PWMSND_TOP;

		PWM_Comp(PWMSndSlice, PWMSndChanR, sampR);
	}

	// write PWM samples
	PWM_Comp(PWMSndSlice, PWMSndChan, samp);
}

// initialize PWM sound output (needs not be re-initialized after changing CLK_SYS system clock,
// but if playing any sound, SpeedSoundUpdate() should be called to update speed of current sounds).
void PWMSndInit()
{
	// sound is OFF
	if (GlobalSoundOff) return;

	// prepare output channels
	PWMSndChan = PWM_GpioToChan(PWMSndGpio);
	PWMSndChanR = PWM_GpioToChan(PWMSndGpioR);
	PWMSndSlice = PWM_GpioToSlice(PWMSndGpio);

	// reset PWM to default state
	PWM_Reset(PWMSndSlice);

	// set GPIO function to PWM
	if (PWMSndGpioR >= 0)
	{
		PWM_GpioInit(PWMSndGpioR);
//		GPIO_Drive12mA(PWMSndGpioR);
	}

	PWM_GpioInit(PWMSndGpio);
//	GPIO_Drive12mA(PWMSndGpio);

	// clear descriptors
	int i;
	for (i = 0; i < USE_PWMSND; i++) PwmSound[i].cnt = 0;

	// set IRQ handler
	SetHandler(IRQ_PWM_WRAP, PWMSndIrq);
	NVIC_IRQEnable(IRQ_PWM_WRAP);

	// set clock divider to 1.00
	PWM_ClkDiv(PWMSndSlice, 0x010);

	// set period to top cycles
	PWM_Top(PWMSndSlice, PWMSND_TOP);

	// write default PWM sample
	if (PWMSndGpioR >= 0) PWM_Comp(PWMSndSlice, PWMSndChanR, PWMSND_TOP/2);
	PWM_Comp(PWMSndSlice, PWMSndChan, PWMSND_TOP/2);

	// reset time dithering
	PwmSoundDither = 0;

	// enable PWM (will be enabled on 1st using of some sound to avoid speaker noise)
//	PWM_Enable(PWMSndSlice);

	// interrupt enable
	PWM_IntEnable(PWMSndSlice);

	// was initialized
	PWMSndWasInit = True;
}

// terminate PWM sound output
void PWMSndTerm()
{
	// was initialized
	if (!PWMSndWasInit) return;
	PWMSndWasInit = False;

	// disable PWM
	PWM_Disable(PWMSndSlice);

	// interrupt disable
	PWM_IntDisable(PWMSndSlice);

	// clear interrupt request
	PWM_IntClear(PWMSndSlice);

	// reset PWM to default state
	PWM_Reset(PWMSndSlice);

	// set GPIO function to default
	GPIO_Reset(PWMSndGpio);
	if (PWMSndGpioR >= 0) GPIO_Reset(PWMSndGpioR);
}

// stop playing sound
void StopSoundChan(int chan)
{
	dmb();
	PwmSound[chan].cnt = 0;
	dmb();
}

// stop playing sound of channel 0
void StopSound()
{
	StopSoundChan(0);
}

// stop playing sounds of all channels
void StopAllSound()
{
	int i;
	for (i = 0; i < USE_PWMSND; i++) StopSoundChan(i);
}

// Convert length of sound in bytes to number of samples
//  size = length of sound in bytes (use sizeof(array))
//  form = sound format SNDFORM_* (4-bit, 8-bit, 16-bit, mono or stereo)
// Returns length of sound in samples (or double-samples for stereo)
int SoundByteToLen(int size, int form)
{
	switch(form)
	{
	// 8-bits unsigned per sample, stereo
	// 16-bits signed per sample, mono
	case SNDFORM_PCM_S:
	case SNDFORM_PCM16: return size/2;

	// IMA ADPCM, 4-bit compression, mono
	case SNDFORM_ADPCM: return size*2;

	// 16-bits signed per sample, stereo
	case SNDFORM_PCM16_S: return size/4;

	// 8-bits unsigned per sample, mono
	// IMA ADPCM, 4-bit compression, stereo
	case SNDFORM_PCM:
	case SNDFORM_ADPCM_S:
	default: return size;
	}
}

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
void PlaySoundChan(int chan, const void* snd, int size, int rep, float speed, float volume, int form, int ext)
{
	// global sound is OFF
	if (GlobalSoundOff) return;

	// convert sound size to the length
	int len = SoundByteToLen(size, form);

	// prepare volume
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	int v = (int)(SNDINT * volume * Config.volume / CONFIG_VOLUME_FULLSTEP + 0.5f);
#else
	int v = (int)(SNDINT * volume + 0.5f);
#endif

	// stop playing sound
	StopSoundChan(chan);

	// pointer to sound channel
	sPwmSnd* s = &PwmSound[chan];

	// system speed
	float speed0 = (float)PWMSND_CLOCK/CurrentFreq[CLK_SYS];
	s->speed = speed;

	// prepare speed increment
	int sinc = (int)(SNDINT * speed * speed0 + 0.5f); // sample increment

	// repeated sound
	s->nextcnt = 0;
	if ((rep != SNDREPEAT_NO) && (rep != SNDREPEAT_STREAM))
	{
		s->next = (const u8*)snd;
		s->nextcnt = len;
	}
	s->stream = (rep == SNDREPEAT_STREAM);
	s->useirq = False;
	s->irq = 0;

	// sound speed
	s->inc = sinc;
	s->acc = 0;

	// sound volume
	s->vol0 = volume;
	s->vol = v;

	// prepare sound format
	s->sampblock = (s16)ext; // number of samples per block
	s->sampcnt = 0; // counter of samples per block
	s->form = (u8)form; // sound format
	s->stepinx[0] = 0; // step index L
	s->stepinx[1] = 0; // step index R
	s->prevval[0] = 0; // previous value L
	s->prevval[1] = 0; // previous value R
	s->odd = False; // odd sub-sample
	s->oldval[0] = 0; // old value L
	s->oldval[1] = 0; // old value R

	// start current sound
	s->snd = (const u8*)snd;
	dmb();
	s->cnt = len;
	dmb();

	// enable PWM
	PWM_Enable(PWMSndSlice);
}

// play sound at channel 0, format SNDFORM_PCM: 8-bit 22050 Hz mono
//  snd = pointer to sound in format SNDFORM_PCM: 8-bit 22050 Hz mono
//  size = length of sound in number of bytes (use sizeof(array))
void PlaySound(const void* snd, int size)
{
	PlaySoundChan(0, snd, size, SNDREPEAT_NO, 1, 1, SNDFORM_PCM, 0);
}

// play sound at channel 0 repeated, format SNDFORM_PCM: 8-bit 22050 Hz mono
//  snd = pointer to sound in format SNDFORM_PCM: 8-bit 22050 Hz mono
//  size = length of sound in number of bytes (use sizeof(array))
void PlaySoundRep(const void* snd, int size)
{
	PlaySoundChan(0, snd, size, SNDREPEAT_REPEAT, 1, 1, SNDFORM_PCM, 0);
}

// play ADPCM sound, format SNDFORM_ADPCM: 4-bit 22050 Hz mono
//  chan = channel 0..
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
//  sampblock = number of samples per block (see WAV file)
void PlayADPCMChan(int chan, const void* snd, int size, int sampblock)
{
	PlaySoundChan(chan, snd, size, SNDREPEAT_NO, 1, 1, SNDFORM_ADPCM, sampblock);
}

// play ADPCM sound at channel 0, format SNDFORM_ADPCM: 4-bit 22050 Hz mono
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
//  sampblock = number of samples per block (see WAV file)
void PlayADPCM(const void* snd, int size, int sampblock)
{
	PlayADPCMChan(0, snd, size, sampblock);
}

// play ADPCM sound repeated, format SNDFORM_ADPCM: 4-bit 22050 Hz mono
//  chan = channel 0..
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
//  sampblock = number of samples per block (see WAV file)
void PlayADPCMRepChan(int chan, const void* snd, int size, int sampblock)
{
	PlaySoundChan(chan, snd, size, SNDREPEAT_REPEAT, 1, 1, SNDFORM_ADPCM, sampblock);
}

// play ADPCM sound at channel 0 repeated, format SNDFORM_ADPCM: 4-bit 22050 Hz mono
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
//  sampblock = number of samples per block (see WAV file)
void PlayADPCMRep(const void* snd, int size, int sampblock)
{
	PlayADPCMRepChan(0, snd, size, sampblock);
}

// update sound speed (1=normal speed)
void SpeedSoundChan(int chan, float speed)
{
	float speed0 = (float)PWMSND_CLOCK/CurrentFreq[CLK_SYS];
	PwmSound[chan].speed = speed;
	PwmSound[chan].inc = (int)(SNDINT * speed * speed0 + 0.5f);
}

// update sound speed of channel 0 (1=normal speed)
void SpeedSound(float speed)
{
	SpeedSoundChan(0, speed);
}

// update all sound speeds after changing system clock
void SpeedSoundUpdate()
{
	float speed0 = (float)PWMSND_CLOCK/CurrentFreq[CLK_SYS];
	int i;
	for (i = 0; i < USE_PWMSND; i++)
	{
		PwmSound[i].inc = (int)(SNDINT * PwmSound[i].speed * speed0 + 0.5f);
	}
}

// update sound volume (1=normal volume)
void VolumeSoundChan(int chan, float volume)
{
	sPwmSnd* s = &PwmSound[chan];
	s->vol0 = volume;
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	s->vol = (int)(SNDINT * volume * Config.volume / CONFIG_VOLUME_FULLSTEP + 0.5f);
#else
	s->vol = (int)(SNDINT * volume + 0.5f);
#endif
}

// update sound volume of channel 0 (1=normal volume)
void VolumeSound(float volume)
{
	VolumeSoundChan(0, volume);
}

// check if sound is playing
Bool PlayingSoundChan(int chan)
{
	if (GlobalSoundOff) return False;
	if ((uint)chan >= (uint)USE_PWMSND) return False;
	sPwmSnd* s = &PwmSound[chan];
	return s->cnt > 0;
}

// check if sound of channel 0 is playing
Bool PlayingSound()
{
	if (GlobalSoundOff) return False;

	return PlayingSoundChan(0);
}

// set next repeated sound in the same format
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
void SetNextSoundChan(int chan, const void* snd, int size)
{
	sPwmSnd* s = &PwmSound[chan];

	// convert sound size to the length
	int len = SoundByteToLen(size, s->form);

	// check if this sound is already next sound
	if (PlayingSoundChan(chan) && ((const void*)s->next == snd) && (s->nextcnt == len)) return;

	// disable next sound
	s->nextcnt = 0;
	dmb();

	// start sound if not playing
	if (s->cnt == 0)
	{
		s->sampcnt = 0; // counter of samples per block
		s->stepinx[0] = 0; // step index L
		s->stepinx[1] = 0; // step index R
		s->prevval[0] = 0; // previous value L
		s->prevval[1] = 0; // previous value R
		s->odd = False; // odd sub-sample
		s->oldval[0] = 0; // old value L
		s->oldval[1] = 0; // old value R

		s->snd = (const u8*)snd;
		dmb();
		s->cnt = len;
		dmb();
	}

	// set next sound
	s->next = (const u8*)snd;
	dmb();
	s->nextcnt = len;
}

// set next repeated sound of channel 0 in the same format
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
void SetNextSound(const void* snd, int size)
{
	SetNextSoundChan(0, snd, size);
}

// check if streaming buffer is empty
Bool SoundStreamIsEmpty(int chan)
{
	sPwmSnd* s = &PwmSound[chan];
	return s->nextcnt == 0;
}

// set next streaming buffer
//  snd = pointer to sound
//  size = length of sound in number of bytes (use sizeof(array))
void SoundStreamSetNext(int chan, const void* snd, int size)
{
	sPwmSnd* s = &PwmSound[chan];

	// convert sound size to the length
	int len = SoundByteToLen(size, s->form);

	// disable interrupt
	IRQ_LOCK;

	// start sound if not playing
	if (s->cnt == 0)
	{
		s->sampcnt = 0; // counter of samples per block
		s->stepinx[0] = 0; // step index L
		s->stepinx[1] = 0; // step index R
		s->prevval[0] = 0; // previous value L
		s->prevval[1] = 0; // previous value R
		s->odd = False; // odd sub-sample
		s->oldval[0] = 0; // old value L
		s->oldval[1] = 0; // old value R

		s->snd = (const u8*)snd;
		dmb();
		s->cnt = len;
		dmb();
		s->nextcnt = 0;

		// buffer is still hungry - request one more interrupt
		dmb();
		if (s->useirq) NVIC_IRQForce(s->irq);

		IRQ_UNLOCK;
		return;
	}

	// set next sound
	s->next = (const u8*)snd;
	dmb();
	s->nextcnt = len;
	dmb();

	IRQ_UNLOCK;
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
