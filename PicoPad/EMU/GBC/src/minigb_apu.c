/**
 * minigb_apu is released under the terms listed within the LICENSE file.
 *
 * minigb_apu emulates the audio processing unit (APU) of the Game Boy. This
 * project is based on MiniGBS by Alex Baines: https://github.com/baines/MiniGBS
 */

// Picopad modification: (2024) Miroslav Nemecek

#include "../include.h"

#define DMG_CLOCK_FREQ_U  ((unsigned)DMG_CLOCK_FREQ)

#define AUDIO_MEM_SIZE    (0xFF3F - 0xFF10 + 1)
#define AUDIO_ADDR_COMPENSATION  0xFF10

#define VOL_INIT_MAX    (INT16_MAX/8)
#define VOL_INIT_MIN    (INT16_MIN/8)

// Handles time keeping for sound generation.
// FREQ_INC_REF must be equal to, or larger than AUDIO_SAMPLE_RATE in order
// to avoid a division by zero error.
// Using a square of 2 simplifies calculations.
#define FREQ_INC_REF    (AUDIO_SAMPLE_RATE * 16)

#define MAX_CHAN_VOLUME    15

struct gb_s* pgb;
u8* audio_mem;	// Memory holding audio registers between 0xFF10 and 0xFF3F inclusive.
struct snd_chan* chans;

// audio lock (this alternative works only on the same core)
#define AUDIO_LOCK() IRQ_LOCK
#define AUDIO_UNLOCK() IRQ_UNLOCK

// ----------------------------------------------------------------------------
//                            Render functions
// ----------------------------------------------------------------------------

static void FASTCODE NOFLASH(set_note_freq)(struct snd_chan *c, const u32 freq)
{
	// Lowest expected value of freq is 64.
	c->freq_inc = freq * (u32) (FREQ_INC_REF / AUDIO_SAMPLE_RATE);
}

static void FASTCODE NOFLASH(chan_enable)(const uint_fast8_t i, const bool enable)
{
	u8 val;

	chans[i].enabled = enable;
	val = (audio_mem[0xFF26 - AUDIO_ADDR_COMPENSATION] & 0x80) |
				(chans[3].enabled << 3) | (chans[2].enabled << 2) |
				(chans[1].enabled << 1) | (chans[0].enabled << 0);

	audio_mem[0xFF26 - AUDIO_ADDR_COMPENSATION] = val;
}

static void FASTCODE NOFLASH(update_env)(struct snd_chan *c)
{
	c->env.counter += c->env.inc;

	while (c->env.counter > FREQ_INC_REF)
	{
		if (c->env.step)
		{
			c->volume += c->env.up ? 1 : -1;
			if (c->volume == 0 || c->volume == MAX_CHAN_VOLUME)
			{
				c->env.inc = 0;
			}
			c->volume = MAX(0, MIN(MAX_CHAN_VOLUME, c->volume));
		}
		c->env.counter -= FREQ_INC_REF;
	}
}

static void FASTCODE NOFLASH(update_len)(struct snd_chan *c)
{
	if (!c->len.enabled) return;

	c->len.counter += c->len.inc;
	if (c->len.counter > FREQ_INC_REF)
	{
		chan_enable(c - chans, 0);
		c->len.counter = 0;
	}
}

static bool FASTCODE NOFLASH(update_freq)(struct snd_chan *c, u32 *pos)
{
	u32 inc = c->freq_inc - *pos;
	c->freq_counter += inc;

	if (c->freq_counter > FREQ_INC_REF)
	{
		*pos = c->freq_inc - (c->freq_counter - FREQ_INC_REF);
		c->freq_counter = 0;
		return true;
	}

	*pos = c->freq_inc;
	return false;
}

static void FASTCODE NOFLASH(update_sweep)(struct snd_chan *c)
{
	c->sweep.counter += c->sweep.inc;

	while (c->sweep.counter > FREQ_INC_REF)
	{
		if (c->sweep.shift)
		{
			u16 inc = (c->sweep.freq >> c->sweep.shift);
			if (!c->sweep.up) inc *= -1;

			c->freq += inc;
			if (c->freq > 2047)
			{
				c->enabled = 0;
			}
			else
			{
				set_note_freq(c, DMG_CLOCK_FREQ_U / ((2048 - c->freq) << 5));
				c->freq_inc *= 8;
			}
		}
		else if (c->sweep.rate)
		{
			c->enabled = 0;
		}
		c->sweep.counter -= FREQ_INC_REF;
	}
}

// render square channel 0 or 1
static void FASTCODE NOFLASH(update_square)(s16 *samples, const bool ch2, int samples_num)
{
	u32 freq;
	struct snd_chan *c = chans + ch2;

	if (!c->powered || !c->enabled) return;

	freq = DMG_CLOCK_FREQ_U / ((2048 - c->freq) << 5);
	set_note_freq(c, freq);
	c->freq_inc *= 8;

	s32 vol_l = pgb->vol_l;
	s32 vol_r = pgb->vol_r;

	for (uint_fast16_t i = 0; i < samples_num*2; i += 2)
	{
		update_len(c);

		if (!c->enabled) continue;

		update_env(c);
		if (!ch2) update_sweep(c);

		u32 pos = 0;
		u32 prev_pos = 0;
		s32 sample = 0;

		while (update_freq(c, &pos))
		{
			c->square.duty_counter = (c->square.duty_counter + 1) & 7;
			sample += ((pos - prev_pos) / c->freq_inc) * c->val;
			c->val = (c->square.duty & (1 << c->square.duty_counter)) ?
						 VOL_INIT_MAX / MAX_CHAN_VOLUME :
						 VOL_INIT_MIN / MAX_CHAN_VOLUME;
			prev_pos = pos;
		}

		if (c->muted) continue;

		sample += c->val;
		sample *= c->volume;
		sample /= 4;

		samples[i + 0] += sample * c->on_left * vol_l;
		samples[i + 1] += sample * c->on_right * vol_r;
	}
}

static u8 FASTCODE NOFLASH(wave_sample)(const unsigned int pos, const unsigned int volume)
{
	u8 sample;

	sample = audio_mem[(0xFF30 + pos / 2) - AUDIO_ADDR_COMPENSATION];
	if (pos & 1)
		sample &= 0xF;
	else
		sample >>= 4;
	return volume ? (sample >> (volume - 1)) : 0;
}

// render wave channel 2
static void FASTCODE NOFLASH(update_wave)(s16 *samples, int samples_num)
{
	u32 freq;
	struct snd_chan *c = chans + 2;

	if (!c->powered || !c->enabled) return;

	freq = (DMG_CLOCK_FREQ_U / 64) / (2048 - c->freq);
	set_note_freq(c, freq);

	c->freq_inc *= 32;

	s32 vol_l = pgb->vol_l;
	s32 vol_r = pgb->vol_r;

	for (uint_fast16_t i = 0; i < samples_num*2; i += 2)
	{
		update_len(c);

		if (!c->enabled) continue;

		u32 pos = 0;
		u32 prev_pos = 0;
		s32 sample = 0;

		c->wave.sample = wave_sample(c->val, c->volume);

		while (update_freq(c, &pos))
		{
			c->val = (c->val + 1) & 31;
			sample += ((pos - prev_pos) / c->freq_inc) *
					((int) c->wave.sample - 8) * (INT16_MAX / 64);
			c->wave.sample = wave_sample(c->val, c->volume);
			prev_pos = pos;
		}

		sample += ((int) c->wave.sample - 8) * (int) (INT16_MAX / 64);

		if (c->volume == 0) continue;

		{
			// First element is unused.
			s16 div[] = {INT16_MAX, 1, 2, 4};
			sample = sample / (div[c->volume]);
		}

		if (c->muted) continue;

		sample /= 4;

		samples[i + 0] += sample * c->on_left * vol_l;
		samples[i + 1] += sample * c->on_right * vol_r;
	}
}

// render noise channel 3
static void FASTCODE NOFLASH(update_noise)(s16 *samples, int samples_num)
{
	struct snd_chan *c = chans + 3;

	s32 vol_l = pgb->vol_l;
	s32 vol_r = pgb->vol_r;

	if (!c->powered) return;

	{
		const u32 lfsr_div_lut[] = { 8, 16, 32, 48, 64, 80, 96, 112 };
		u32 freq = DMG_CLOCK_FREQ_U / (lfsr_div_lut[c->noise.lfsr_div] << c->freq);
		set_note_freq(c, freq);
	}

	if (c->freq >= 14) c->enabled = 0;

	for (uint_fast16_t i = 0; i < samples_num*2; i += 2)
	{
		update_len(c);

		if (!c->enabled) continue;

		update_env(c);

		u32 pos = 0;
		u32 prev_pos = 0;
		s32 sample = 0;

		while (update_freq(c, &pos))
		{
			c->noise.lfsr_reg = (c->noise.lfsr_reg << 1) |
						(c->val >= VOL_INIT_MAX / MAX_CHAN_VOLUME);

			if (c->noise.lfsr_wide)
			{
				c->val = !(((c->noise.lfsr_reg >> 14) & 1) ^
						 ((c->noise.lfsr_reg >> 13) & 1)) ?
						 VOL_INIT_MAX / MAX_CHAN_VOLUME :
						 VOL_INIT_MIN / MAX_CHAN_VOLUME;
			}
			else
			{
				c->val = !(((c->noise.lfsr_reg >> 6) & 1) ^
						 ((c->noise.lfsr_reg >> 5) & 1)) ?
						 VOL_INIT_MAX / MAX_CHAN_VOLUME :
						 VOL_INIT_MIN / MAX_CHAN_VOLUME;
			}

			sample += ((pos - prev_pos) / c->freq_inc) * c->val;
			prev_pos = pos;
		}

		if (c->muted) continue;

		sample += c->val;
		sample *= c->volume;
		sample /= 4;

		samples[i + 0] += sample * c->on_left * vol_l;
		samples[i + 1] += sample * c->on_right * vol_r;
	}
}

// Render sound buffer
// SDL2 style audio callback function.
// Fill allocated buffer "data" with samples in stereo interleaved format.
// - stream of s16 values, range -32768 to +32767, 1st sample is left channel, 2nd sample is right channel
void FASTCODE NOFLASH(audio_callback)(s16 *stream, int samples_num)
{
	memset(stream, 0, samples_num*4);

	// audio lock
	AUDIO_LOCK();

	update_square(stream, 0, samples_num);
	update_square(stream, 1, samples_num);
	update_wave(stream, samples_num);
	update_noise(stream, samples_num);

	// audio unlock
	AUDIO_UNLOCK();
}

// ----------------------------------------------------------------------------
//                            Control functions
// ----------------------------------------------------------------------------

// restart sound channel
static void FASTCODE NOFLASH(chan_trigger)(uint_fast8_t i)
{
	struct snd_chan *c = chans + i;

	chan_enable(i, 1);
	c->volume = c->volume_init;

	// volume envelope
	{
		u8 val = audio_mem[(0xFF12 + (i * 5)) - AUDIO_ADDR_COMPENSATION];

		c->env.step = val & 0x07;
		c->env.up = val & 0x08 ? 1 : 0;
		c->env.inc = c->env.step ?
			 (FREQ_INC_REF * 64ul) / ((u32) c->env.step * AUDIO_SAMPLE_RATE) :
			 (8ul * FREQ_INC_REF) / AUDIO_SAMPLE_RATE;
		c->env.counter = 0;
	}

	// freq sweep
	if (i == 0)
	{
		u8 val = audio_mem[0xFF10 - AUDIO_ADDR_COMPENSATION];

		c->sweep.freq = c->freq;
		c->sweep.rate = (val >> 4) & 0x07;
		c->sweep.up = !(val & 0x08);
		c->sweep.shift = (val & 0x07);
		c->sweep.inc = c->sweep.rate ?
				 ((128 * FREQ_INC_REF) / (c->sweep.rate * AUDIO_SAMPLE_RATE)) : 0;
		c->sweep.counter = FREQ_INC_REF;
	}

	int len_max = 64;

	if (i == 2)
	{	// wave
		len_max = 256;
		c->val = 0;
	}
	else if (i == 3)
	{	// noise
		c->noise.lfsr_reg = 0xFFFF;
		c->val = VOL_INIT_MIN / MAX_CHAN_VOLUME;
	}

	c->len.inc = (256 * FREQ_INC_REF) / (AUDIO_SAMPLE_RATE * (len_max - c->len.load));
	c->len.counter = 0;
}

// Read audio register.
//	addr ... Address of audio register. Must be 0xFF10 <= addr <= 0xFF3F.
//		This is not checked in this function.
// return Byte at address.
u8 FASTCODE NOFLASH(audio_read)(const u16 addr)
{
	static u8 ortab[] = {
			0x80, 0x3f, 0x00, 0xff, 0xbf,
			0xff, 0x3f, 0x00, 0xff, 0xbf,
			0x7f, 0xff, 0x9f, 0xff, 0xbf,
			0xff, 0xff, 0x00, 0x00, 0xbf,
			0x00, 0x00, 0x70,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	return audio_mem[addr - AUDIO_ADDR_COMPENSATION] |
				 ortab[addr - AUDIO_ADDR_COMPENSATION];
}

// Write audio register.
//	addr ... Address of audio register. Must be 0xFF10 <= addr <= 0xFF3F.
//		This is not checked in this function.
//	val ... Byte to write at address.
void FASTCODE NOFLASH(audio_write)(const u16 addr, const u8 val)
{
	// Find sound channel corresponding to register address.
	uint_fast8_t i;

	// audio lock
	AUDIO_LOCK();

	if (addr == 0xFF26)
	{
		audio_mem[addr - AUDIO_ADDR_COMPENSATION] = val & 0x80;

		// On APU power off, clear all registers apart from wave RAM.
		if ((val & 0x80) == 0)
		{
			memset(audio_mem, 0x00, 0xFF26 - AUDIO_ADDR_COMPENSATION);
			chans[0].enabled = false;
			chans[1].enabled = false;
			chans[2].enabled = false;
			chans[3].enabled = false;
		}

		// audio unlock
		AUDIO_UNLOCK();
		return;
	}

	// Ignore register writes if APU powered off.
	if (audio_mem[0xFF26 - AUDIO_ADDR_COMPENSATION] == 0x00)
	{
		// audio unlock
		AUDIO_UNLOCK();
		return;
	}

	audio_mem[addr - AUDIO_ADDR_COMPENSATION] = val;
	i = (addr - AUDIO_ADDR_COMPENSATION) / 5; // channel

	switch (addr)
	{
		case 0xFF12:
		case 0xFF17:
		case 0xFF21:
		  {
			chans[i].volume_init = val >> 4;
			chans[i].powered = (val >> 3) != 0;

			// "zombie mode" stuff, needed for Prehistorik Man and probably
			// others
			if (chans[i].powered && chans[i].enabled)
			{
				if ((chans[i].env.step == 0 && chans[i].env.inc != 0))
				{
					if (val & 0x08)
						chans[i].volume++;
					else
						chans[i].volume += 2;
				}
				else
					chans[i].volume = 16 - chans[i].volume;

				chans[i].volume &= 0x0F;
				chans[i].env.step = val & 0x07;
			}
			break;
		  }

		case 0xFF1C:
			chans[i].volume = chans[i].volume_init = (val >> 5) & 0x03;
			break;

		case 0xFF11:
		case 0xFF16:
		case 0xFF20:
		  {
			const u8 duty_lookup[] = {0x10, 0x30, 0x3C, 0xCF};
			chans[i].len.load = val & 0x3f;
			chans[i].square.duty = duty_lookup[val >> 6];
			break;
		  }

		case 0xFF1B:
			chans[i].len.load = val;
			break;

		case 0xFF13:
		case 0xFF18:
		case 0xFF1D:
			chans[i].freq &= 0xFF00;
			chans[i].freq |= val;
			break;

		case 0xFF1A:
			chans[i].powered = (val & 0x80) != 0;
			chan_enable(i, val & 0x80);
			break;

		case 0xFF14:
		case 0xFF19:
		case 0xFF1E:
			chans[i].freq &= 0x00FF;
			chans[i].freq |= ((val & 0x07) << 8);
			// Intentional fall-through.
		case 0xFF23:
			chans[i].len.enabled = val & 0x40 ? 1 : 0;
			if (val & 0x80)	chan_trigger(i); // restart sound channel
			break;

		case 0xFF22:
			chans[3].freq = val >> 4;
			chans[3].noise.lfsr_wide = !(val & 0x08);
			chans[3].noise.lfsr_div = val & 0x07;
			break;

		case 0xFF24:
			pgb->vol_l = ((val >> 4) & 0x07);
			pgb->vol_r = (val & 0x07);
			break;

		case 0xFF25:
			for (uint_fast8_t j = 0; j < 4; j++)
			{
				chans[j].on_left = (val >> (4 + j)) & 1;
				chans[j].on_right = (val >> j) & 1;
			}
			break;
	}

	// audio unlock
	AUDIO_UNLOCK();
}

// Initialize audio (mem = pointer to first register GB_IO_NR)
void audio_init(struct gb_s* gb)
{
	audio_mem = &gb->hram_io[IO_SND];
	chans = &gb->chans[0];
	pgb = gb;

//#if AUDIO_SPINLOCK
//	SpinInit(AUDIO_SPINLOCK);
//#endif

	// Initialise channels and samples.
	memset(chans, 0, sizeof(chans));
	chans[0].val = chans[1].val = -1;

	// Initialise IO registers.
	{
		const u8 regs_init[] = {0x80, 0xBF, 0xF3, 0xFF, 0x3F,
					 0xFF, 0x3F, 0x00, 0xFF, 0x3F,
					 0x7F, 0xFF, 0x9F, 0xFF, 0x3F,
					 0xFF, 0xFF, 0x00, 0x00, 0x3F,
					 0x77, 0xF3, 0xF1};

		for (uint_fast8_t i = 0; i < sizeof(regs_init); ++i)
			audio_write(0xFF10 + i, regs_init[i]);
	}

	// Initialise Wave Pattern RAM.
	{
		const u8 wave_init[] = {0xac, 0xdd, 0xda, 0x48,
					 0x36, 0x02, 0xcf, 0x16,
					 0x2c, 0x04, 0xe5, 0x2c,
					 0xac, 0xdd, 0xda, 0x48};

		for (uint_fast8_t i = 0; i < sizeof(wave_init); ++i)
			audio_write(0xFF30 + i, wave_init[i]);
	}
}
