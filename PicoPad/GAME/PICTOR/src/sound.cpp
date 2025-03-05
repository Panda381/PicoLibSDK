
// ****************************************************************************
//
//                                 Sounds
//
// ****************************************************************************

#include "../include.h"

#ifdef DEB_SOUND	// initial sound mode SOUNDMODE_OFF, SOUNDMODE_SOUND, SOUNDMODE_MUSIC
Bool SoundMode = DEB_SOUND; // sound mode
#else
Bool SoundMode = SOUNDMODE_MUSIC; // sound mode
#endif

// music samples
const u8* SndSamp[SND_NUM] = {
	Music1Snd,
	Music2Snd,
	Music3Snd,
	Music4Snd,
	Music5Snd,
	Music6Snd,
	Music7Snd,
	Music8Snd,
	Music9Snd,
	Music10Snd,
	Music11Snd,
	Music12Snd,
};

const int SndLen[SND_NUM] = {
	count_of(Music1Snd),
	count_of(Music2Snd),
	count_of(Music3Snd),
	count_of(Music4Snd),
	count_of(Music5Snd),
	count_of(Music6Snd),
	count_of(Music7Snd),
	count_of(Music8Snd),
	count_of(Music9Snd),
	count_of(Music10Snd),
	count_of(Music11Snd),
	count_of(Music12Snd),
};

// play music
void MusicStart()
{
	if (SoundMode == SOUNDMODE_MUSIC)
		// output PWM sound (sound must be PCM 8-bit mono 22050Hz)
		//  chan = channel 0..
		//  snd = pointer to sound
		//  size = length of sound in bytes
		//  rep = True to repeat sample
		//  speed = relative speed (1=normal; SNDFORM_ADPCM must have speed = 1)
		//  volume = volume (1=normal)
		//  form = sound format SNDFORM_*
		//  ext = format extended data (ADPCM: number of samples per block)
		PlaySoundChan(SOUNDCHAN_MUSIC, SndSamp[BackInx], SndLen[BackInx], True, 1, MUSIC_VOL, SNDFORM_ADPCM, IMA_SAMPBLOCK);
}
