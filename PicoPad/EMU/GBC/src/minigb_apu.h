/**
 * minigb_apu is released under the terms listed within the LICENSE file.
 *
 * minigb_apu emulates the audio processing unit (APU) of the Game Boy. This
 * project is based on MiniGBS by Alex Baines: https://github.com/baines/MiniGBS
 */

// Picopad modification: (2024) Miroslav Nemecek

#define AUDIO_SAMPBUF	4	// size of temporary sample buffer

#define AUDIO_SAMPLE_RATE  32768

#define DMG_CLOCK_FREQ    4194304.0
#define SCREEN_REFRESH_CYCLES  70224.0
#define VERTICAL_SYNC    (DMG_CLOCK_FREQ/SCREEN_REFRESH_CYCLES)	// = 59.7275

// (12 bytes)
struct chan_len_ctr {
		u8 load;
		unsigned enabled: 1;
		u8 res[2]; // align
		u32 counter;
		u32 inc;
};
STATIC_ASSERT(sizeof(struct chan_len_ctr) == 12, "Incorrect struct chan_len_ctr!");

// (12 bytes)
struct chan_vol_env {
		u8 step;
		unsigned up: 1;
		u8 res[2]; // align
		u32 counter;
		u32 inc;
};
STATIC_ASSERT(sizeof(struct chan_vol_env) == 12, "Incorrect struct chan_vol_env!");

// (16 bytes)
struct chan_freq_sweep {
		u16 freq;
		u8 rate;
		u8 shift;
		unsigned up: 1;
		u8 res[3]; // align
		u32 counter;
		u32 inc;
};
STATIC_ASSERT(sizeof(struct chan_freq_sweep) == 16, "Incorrect struct chan_freq_sweep!");

// 0x3C bytes
struct snd_chan {
	// 0x00
	unsigned enabled: 1;
	unsigned powered: 1;
	unsigned on_left: 1;
	unsigned on_right: 1;
	unsigned muted: 1;

	u8 volume;		// 0x01
	u8 volume_init;		// 0x02
	u8 res; 		// 0x03: align

	u16 freq;		// 0x04
	s16 val;		// 0x06
	u32 freq_counter;	// 0x08
	u32 freq_inc;		// 0x0C

	struct chan_len_ctr len;	// 0x10 (12 bytes)
	struct chan_vol_env env;	// 0x1C (12 bytes)
	struct chan_freq_sweep sweep;	// 0x28 (16 bytes)

	// 0x38 (4 bytes)
	union {
		struct {
			u8 duty;
			u8 duty_counter;
		} square;

		struct {
			u16 lfsr_reg;
			u8 lfsr_wide;
			u8 lfsr_div;
		} noise;

		struct {
			u8 sample;
		} wave;
	};
};
STATIC_ASSERT(sizeof(struct snd_chan) == 0x3c, "Incorrect struct snd_chan!");

// Fill allocated buffer "data" with samples in stereo interleaved format.
void FASTCODE NOFLASH(audio_callback)(s16 *data, int samples);

// Read audio register at given address "addr".
uint8_t FASTCODE NOFLASH(audio_read)(const u16 addr);

// Write "val" to audio register at given address "addr".
void FASTCODE NOFLASH(audio_write)(const u16 addr, const u8 val);

// Initialize audio
void audio_init(struct gb_s* gb);
