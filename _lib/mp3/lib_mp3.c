
// ****************************************************************************
//
//                               MP3 decoder
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/

#include "../../global.h"	// globals

#if USE_MP3			// use MP3 decoder (lib_mp3*.c, lib_mp3*.h)

#include "../../_sdk/inc/sdk_timer.h"
#include "../inc/lib_malloc.h"
#include "../inc/lib_fat.h"
#include "../inc/lib_pwmsnd.h"
#include "../inc/lib_text.h"

// safe undef includes
#undef _MP3DEC_H
#undef _STATNAME_H
#undef _MP3COMMON_H
#undef _CODER_H
#undef _ASSEMBLY_H

// setup
#if RISCV
#define MP3DEC_GENERIC
#endif

#define MPDEC_ALLOCATOR MemAlloc
#define MPDEC_FREE MemFree

// PicoLibSDK public include
#include "lib_mp3.h"		// PicoLibSDK public interface

// MP3 library includes
#include "statname.h"		// name mangling macros for static linking
#include "mp3common.h"		// implementation-independent API's, datatypes, and definitions
#include "coder.h"		// private, implementation-specific header file
#include "assembly.h"		// assembly language functions and prototypes for supported platforms

// MP3 library code
#include "bitstream.c"
#include "buffers.c"
#include "dct32.c"
#include "dequant.c"
#include "dqchan.c"
#include "huffman.c"
#include "hufftabs.c"
#include "imdct.c"
#include "mp3dec.c"
#include "mp3tabs.c"
#include "polyphase.c"
#include "scalfact.c"
#include "stproc.c"
#include "subband.c"
#include "trigtabs.c"

#if MP3PLAYER_USEIRQ
sMP3Player* MP3Handler_ID;	// MP3 handler used in IRQ mode
#endif // MP3PLAYER_USEIRQ

// MP3 genre list
// https://en.wikipedia.org/wiki/List_of_ID3v1_genres
const char* MP3GenreList[MP3_GENRELIST_NUM] = {
	"Blues",		// 00
	"Classic Rock",		// 01
	"Country",		// 02
	"Dance",		// 03
	"Disco",		// 04
	"Funk",			// 05
	"Grunge",		// 06
	"Hip-Hop",		// 07
	"Jazz",			// 08
	"Metal",		// 09

	"New Age",		// 10
	"Oldies",		// 11
	"Other",		// 12
	"Pop",			// 13
	"R&B",			// 14 Rhythm And Blues
	"Rap",			// 15
	"Reggae",		// 16
	"Rock",			// 17
	"Techno",		// 18
	"Industrial",		// 19

	"Alternative",		// 20
	"Ska",			// 21
	"Death Metal",		// 22
	"Pranks",		// 23
	"Soundtrack",		// 24
	"Euro-Techno",		// 25
	"Ambient",		// 26
	"Trip-Hop",		// 27
	"Vocal",		// 28
	"Jazz & Funk",		// 29 Jazz+Funk

	"Fusion",		// 30
	"Trance",		// 31
	"Classical",		// 32
	"Instrumental",		// 33
	"Acid",			// 34
	"House",		// 35
	"Game",			// 36
	"Sound Clip",		// 37
	"Gospel",		// 38
	"Noise",		// 39

	"Alternative Rock",	// 40 AlternRock
	"Bass",			// 41
	"Soul",			// 42
	"Punk",			// 43
	"Space",		// 44
	"Meditative",		// 45
	"Instrumental Pop",	// 46
	"Instrumental Rock",	// 47
	"Ethnic",		// 48
	"Gothic",		// 49

	"Darkwave",		// 50
	"Techno-Industrial",	// 51
	"Electronic",		// 52
	"Pop-Folk",		// 53
	"Eurodance",		// 54
	"Dream",		// 55
	"Southern Rock",	// 56
	"Comedy",		// 57
	"Cult",			// 58
	"Gangsta",		// 59

	"Top 40",		// 60
	"Christian Rap",	// 61
	"Pop/Funk",		// 62
	"Jungle",		// 63 Jungle Music
	"Native US",		// 64 Native American
	"Cabaret",		// 65
	"New Wave",		// 66
	"Psychedelic",		// 67
	"Rave",			// 68
	"Showtunes",		// 69

	"Trailer",		// 70
	"Lo-Fi",		// 71
	"Tribal",		// 72
	"Acid Punk",		// 73
	"Acid Jazz",		// 74
	"Polka",		// 75
	"Retro",		// 76
	"Musical",		// 77
	"Rock & Roll",		// 78 Rock 'n' roll
	"Hard Rock",		// 79

	"Folk",			// 80
	"Folk-Rock",		// 81
	"National Folk",	// 82
	"Swing",		// 83
	"Fast Fusion",		// 84
	"Bebop",		// 85
	"Latin",		// 86
	"Revival",		// 87
	"Celtic",		// 88
	"Bluegrass",		// 89

	"Avantgarde",		// 90
	"Gothic Rock",		// 91
	"Progressive Rock",	// 92
	"Psychedelic Rock",	// 93
	"Symphonic Rock",	// 94
	"Slow Rock",		// 95
	"Big Band",		// 96
	"Chorus",		// 97
	"Easy Listening",	// 98
	"Acoustic",		// 99

	"Humour",		// 100
	"Speech",		// 101
	"Chanson",		// 102
	"Opera",		// 103
	"Chamber Music",	// 104
	"Sonata",		// 105
	"Symphony",		// 106
	"Booty Bass",		// 107
	"Primus",		// 108
	"Porn Groove",		// 109

	"Satire",		// 110
	"Slow Jam",		// 111
	"Club",			// 112
	"Tango",		// 113
	"Samba",		// 114
	"Folklore",		// 115
	"Ballad",		// 116
	"Power Ballad",		// 117
	"Rhythmic Soul",	// 118
	"Freestyle",		// 119

	"Duet",			// 120
	"Punk Rock",		// 121
	"Drum Solo",		// 122
	"A cappella",		// 123 Acappella
	"Euro-House",		// 124
	"Dance Hall",		// 125
	"Goa Music",		// 126
	"Drum & Bass",		// 127
	"Club-House",		// 128
	"Hardcore Techno",	// 129

	"Terror",		// 130
	"Indie",		// 131
	"Britpop",		// 132
	"Negerpunk",		// 133
	"Polsk Punk",		// 134
	"Beat",			// 135
	"Christian Gangsta Rap",// 136
	"Heavy Metal",		// 137
	"Black Metal",		// 138
	"Crossover",		// 139

	"Contemporary Christian",// 140 
	"Christian Rock",	// 141
	"Merengue",		// 142
	"Salsa",		// 143
	"Thrash Metal",		// 144
	"Anime",		// 145
	"Jpop",			// 146
	"Synthpop",		// 147
	"Christmas",		// 148
	"Art Rock",		// 149

	"Baroque",		// 150
	"Bhangra",		// 151
	"Big Beat",		// 152
	"Breakbeat",		// 153
	"Chillout",		// 154
	"Downtempo",		// 155
	"Dub",			// 156
	"EBM",			// 157
	"Eclectic",		// 158
	"Electro",		// 159

	"Electroclash",		// 160
	"Emo",			// 161
	"Experimental",		// 162
	"Garage",		// 163
	"Global",		// 164
	"IDM",			// 165
	"Illbient",		// 166
	"Industro-Goth",	// 167
	"Jam Band",		// 168
	"Krautrock",		// 169

	"Leftfield",		// 170
	"Lounge",		// 171
	"Math Rock",		// 172
	"New Romantic",		// 173
	"Nu-Breakz",		// 174
	"Post-Punk",		// 175
	"Post-Rock",		// 176
	"Psytrance",		// 177
	"Shoegaze",		// 178
	"Space Rock",		// 179

	"Trop Rock",		// 180
	"World Music",		// 181
	"Neoclassical",		// 182
	"Audiobook",		// 183
	"Audio Theatre",	// 184
	"Neue Deutsche Welle",	// 185
	"Podcast",		// 186
	"Indie Rock",		// 187
	"G-Funk",		// 188
	"Dubstep",		// 189

	"Garage Rock",		// 190
	"Psybient",		// 191
};

// Check ID3v1 tag on end of the MP3 (last 128 bytes)
//  buf ... pointer to input buffer with 10 bytes of data
// Return False if not valid ID3v1 tag.
Bool MP3CheckID3v1(const u8* buf)
{
	return	(buf[0] == 'T') &&
		(buf[1] == 'A') &&
		(buf[2] == 'G');
}

// Check ID3v2 tag on start of the MP3 (to skip it)
//  buf ... pointer to input buffer with 10 bytes of data
// Returns size of the Id3v2 tag in bytes, or 0 if ID3v2 tag not found.
int MP3CheckID3v2(const u8* buf)
{
// ID3v2 tag header http://id3.org/id3v2.3.0#ID3v2_header
	int s = 0;
	if (	(buf[0] == 'I') && (buf[1] == 'D') && (buf[2] == '3') && // tag ID
		(buf[3] != 0xff) && (buf[4] != 0xff) &&			// tag version
		((buf[5] & 0x1f) == 0) &&				// tag flags
		(buf[6] < 0x80) && (buf[7] < 0x80) && (buf[8] < 0x80) && (buf[9] < 0x80)) // tag size
	{
		// get tag size
		s = ((u32)buf[6] << 21) | ((u32)buf[7] << 14) | ((u32)buf[8] << 7) | ((u32)buf[9]);
		s += 10; // skip ID3v2 header
	}
	return s;
}

// refill input buffer (only in file mode)
void MP3Refill(sMP3Player* mp3)
{
	int i, j;
#if MP3_CHECK_LOAD
	u32 sum = 0;
#endif

	// check file mode
	if (mp3->filename != NULL)
	{
		// remain too many data
		if (mp3->inbufrem >= mp3->insize*5/8) return;

		// disable interrupts
#if MP3PLAYER_USEIRQ
		IRQ_LOCK;
#endif

		i = mp3->inbufrem;	// remaining data in input buffer
#if MP3_CHECK_LOAD
		if (i < mp3->poll_rem) mp3->poll_rem = i;
		u32 t1 = Time();
#endif

		// shift remaining data to the start of the buffer
		u8* dst = (u8*)mp3->inbuf;	// pointer to start of buffer
		const u8* src = (const u8*)mp3->inbufptr; // pointer to remaining data
		mp3->inbufptr = dst; // new start of data
		if (i <= 0) // no data left
		{
			// maybe we was behind end of data
			i = 0;
			mp3->inbufrem = i;
		}
		else // i > 0, some data remain
		{
			// move remaining data to start of buffer
			if (dst != src)
			{
				j = i;
#if MP3PLAYER_USEIRQ
				// Moving data can be very time-consuming - locking up all the time
				// can cause several audio player samples to drop out. Therefore,
				// we move only the beginning of the data, enable the interrupt,
				// and move the rest of the data at the same time as playing.
#define MP3REFILL_PRELOAD 100
				if (j >= MP3REFILL_PRELOAD)
				{
					for (j = MP3REFILL_PRELOAD; j > 0; j--) *dst++ = *src++;
					j = i - MP3REFILL_PRELOAD;
					IRQ_UNLOCK;
				}
#endif
				for (; j > 0; j--) *dst++ = *src++; // move remaining data to start of buffer
			}
		}

		// enable interrupts
#if MP3PLAYER_USEIRQ
		IRQ_UNLOCK;
#endif

		// load new data
		int off = FilePos(&mp3->file); // get current file position
		j = mp3->insize - i + off; // offset of new end
		if (j > mp3->filesize) j = mp3->filesize; // limit end of data
		j -= off; // remaining space
		j = FileRead(&mp3->file, (void*)(mp3->inbuf + i), j); // read new data

		// shift data size
#if MP3PLAYER_USEIRQ
		di();
#endif

		mp3->inbufrem += j;

#if MP3PLAYER_USEIRQ
		IRQ_UNLOCK;
#endif

#if MP3_CHECK_LOAD
		sum += j;
#endif

		// repeat sound
		i += j; // new size of input data in buffer
		if (mp3->rep && (i < mp3->insize))
		{
			mp3->pos = 0; // reset current frame
			off = mp3->frame0; // offset of first frame
			FileSeek(&mp3->file, off); // rewind back to the start
			j = mp3->insize - i + off; // offset of new end
			if (j > mp3->filesize) j = mp3->filesize; // limit end of data
			j -= off; // remaining space
			j = FileRead(&mp3->file, (void*)(mp3->inbuf + i), j); // read new data

			// file error
			if (j == 0) mp3->inerror = True;

			// shift data size
#if MP3PLAYER_USEIRQ
			di();
#endif

			mp3->inbufrem += j;

#if MP3PLAYER_USEIRQ
			IRQ_UNLOCK;
#endif

#if MP3_CHECK_LOAD
			sum += j;
#endif
		}

#if MP3_CHECK_LOAD
		mp3->poll_time += Time() - t1;
		mp3->poll_len += sum;
#endif
	}
}

// Initialize MP3 player
//  mp3 ... pointer to sMP3Player descriptor
//  filename ... filename, NULL=play MP3 from memory (inbuf)
//  inbuf ... pointer to MP3 in memory, or pointer to input bufer for file mode (recommended size MP3PLAYER_INSIZE)
//  insize ... size of input buffer in bytes
//  outbuf ... pointer to output buffer (must be aligned to u16 or better to u32; recommended size MP3PLAYER_OUTSIZE)
//  outsize ... size of output buffer in bytes
//  scan ... number of frames to scan file on open, -1=scan all file (count frames and length; recommended value is 100)
// Returns error code ERR_MP3_* (ERR_MP3_NONE = 0 if OK)
int MP3PlayerInit(sMP3Player* mp3, const char* filename, const u8* inbuf, int insize, u8* outbuf, int outsize, int scan)
{
	int i;

	// minimal number of scanned frames
	if ((scan >= 0) && (scan < 2)) scan = 2;

	// initialize MP3 decoder descriptor
	HMP3Decoder mp3dec = MP3InitDecoder(&mp3->decinfo);
	if (mp3dec == NULL) return ERR_MP3_OUT_OF_MEMORY;
	mp3->mp3dec = mp3dec;

	// input file
	mp3->filename = filename;
	mp3->filesize = insize;
	mp3->frame0 = 0;
	mp3->inerror = False;

	// input data buffer
	mp3->inbuf = inbuf;	// pointer to start of input buffer
	mp3->insize = insize;	// size of input buffer
	mp3->inbufptr = inbuf;	// current pointer to input buffer
	mp3->inbufrem = insize;	// remaining data in input buffer

	// output buffers
	outsize = (outsize >> 1) & ~3;
	mp3->outbuf[0] = outbuf;
	mp3->outbuf[1] = outbuf + outsize;
	mp3->outsize = outsize;

	// open input file
	mp3->id3v1_off = 0;
	if (filename != NULL)
	{
		// mount SD card
		if (!DiskAutoMount())
		{
			MP3FreeDecoder(mp3dec);
			return ERR_MP3_NOMOUNT;
		}

		// open input file
		if (!FileOpen(&mp3->file, filename))
		{
			MP3FreeDecoder(mp3dec);
			return ERR_MP3_INVALID_FILE;
		}

		// file size
		mp3->filesize = mp3->file.size;

		// seek to end of file - 128
		FileSeek(&mp3->file, mp3->filesize - ID3V1_SIZE);

		// read ID3v1 tag
		FileRead(&mp3->file, &mp3->id3v1, ID3V1_SIZE);

		// check ID3v1 tag on end of file
		if (MP3CheckID3v1((const u8*)&mp3->id3v1))
		{
			mp3->id3v1_off = mp3->filesize - ID3V1_SIZE;
			mp3->filesize -= ID3V1_SIZE;
		}

		// seek to start of file
		FileSeek(&mp3->file, 0);

		// refill input buffer
		mp3->inbufrem = 0;	// no remaining data in input buffer
		MP3Refill(mp3);

		// error reading data
		if (mp3->inerror)
		{
			MP3PlayerTerm(mp3dec);
			return ERR_MP3_INVALID_FILE;
		}
	}
	else
	{
		// check ID3v1 tag on end of buffer
		const u8* src = &inbuf[insize - ID3V1_SIZE];
		if (MP3CheckID3v1(src))
		{
			insize -= ID3V1_SIZE;
			mp3->id3v1_off = insize;
			mp3->insize = insize;
			mp3->inbufrem = insize;
			mp3->filesize -= ID3V1_SIZE;

			// copy ID3v1 tag to buffer
			u8* dst = (u8*)&mp3->id3v1;
			for (i = ID3V1_SIZE; i > 0; i--) *dst++ = *src++;			
		}
	}

	// ID3v1 invalid - clear buffer
	if (mp3->id3v1_off == 0) memset(&mp3->id3v1, 0, ID3V1_SIZE);

	// prepare ID3v1 entries
	memcpy(mp3->id3_title,		mp3->id3v1.title,	ID3V1_TITLE_LEN);
	memcpy(mp3->id3_artist,		mp3->id3v1.artist,	ID3V1_ARTIST_LEN);
	memcpy(mp3->id3_album,		mp3->id3v1.album,	ID3V1_ALBUM_LEN);
	memcpy(mp3->id3_comment,	mp3->id3v1.comment,	ID3V1_COMMENT_LEN);
	memcpy(mp3->id3_year,		mp3->id3v1.year,	ID3V1_YEAR_LEN);
	mp3->id3_title  [ID3V1_TITLE_LEN] = 0;
	mp3->id3_artist [ID3V1_ARTIST_LEN] = 0;
	mp3->id3_album  [ID3V1_ALBUM_LEN] = 0;
	mp3->id3_comment[ID3V1_COMMENT_LEN] = 0;
	mp3->id3_year   [ID3V1_YEAR_LEN] = 0;
	mp3->id3_title_len	= (u8)StrLen(mp3->id3_title);
	mp3->id3_artist_len	= (u8)StrLen(mp3->id3_artist);
	mp3->id3_album_len	= (u8)StrLen(mp3->id3_album);
	mp3->id3_comment_len	= (u8)StrLen(mp3->id3_comment);
	mp3->id3_year_len	= (u8)StrLen(mp3->id3_year);
	
	// detect ID3v2 on start of input buffer
	int n = MP3CheckID3v2(inbuf);
	mp3->id3v2_size = n;

	// skip ID3v2 tag
	if (n > 0)
	{
		// file mode
		if (filename != NULL)
		{
			while (n > 0)
			{
				i = n; // remaining data to skip
				if (i > mp3->inbufrem) i = mp3->inbufrem; // limit to data in buffer
				mp3->inbufrem -= i; // decrease remaining data
				mp3->inbufptr += i; // increase data pointer
				n -= i; // decrease remaining data
				MP3Refill(mp3); // refill next data

				// error reading data
				if (mp3->inerror)
				{
					MP3PlayerTerm(mp3dec);
					return ERR_MP3_INVALID_FILE;
				}
			}
		}

		// memory mode
		else
		{
			mp3->inbufptr += n;
			mp3->inbufrem -= n;
		}
	}

	// current buffer position (points behind ID3v2 tag)
	inbuf = (const u8*)mp3->inbufptr;
	insize = mp3->inbufrem;

	// find start of first frame
	n = MP3FindSyncWord(inbuf, insize); // find next sync word
	inbuf += n; // shift data pointer
	insize -= n; // decrease data size
	if ((n < 0) || (insize < 2))
	{
		MP3PlayerTerm(mp3dec);
		return ERR_MP3_INVALID_FRAMEHEADER; // not synced
	}
	n += mp3->id3v2_size; // offset of the sync
	mp3->frame0 = n; // offset of start of frame 0
	mp3->inbufptr = inbuf; // new buffer pointer
	mp3->inbufrem = insize; // new remaining data
	
	// parse frame info
	n = MP3GetNextFrameInfo(mp3dec, &mp3->info, inbuf); // unpack frame info
	if (n < 0) // error - invalid frame header
	{
		MP3PlayerTerm(mp3dec);
		return n;
	}

	// free mode not supported by the MP3 player
	if (mp3->info.bitrate == 0) return ERR_MP3_INVALID_FREE;

	// map frames
	int off = mp3->frame0; // offset of current frame
	int inx = 0;	// index of the seek entry
	int frames = 0; // total frame counter
	s64 bitrate = 0; // sum of bitrate
	int outsampmax = 0; // max. number of output samples per one frame (*2 = output bytes per frame, both channels)
	int sizemax = 0; // max. input size of frame
	int sampnum = 0; // total number of output samples (L and R are counted as 2 samples)
//	int seekshift = 0; // seek table granularity = number of bits to shift frame number right
//	int seekint = 1; // seek interval
//	int seekcnt = 1; // counter to skip seek entries
	mp3->rep = False; // no repeat
	MP3Refill(mp3); // refill next data
	while (True)
	{
/*

// The seek table is only useful if we scan
// the whole file. We can probably live without it.

		// store offset of the frame
		seekcnt--; // skip counter
		if (seekcnt == 0) // store this seek entry into table
		{
			// overflow seek table - need to reduce
			if (inx >= MP3PLAYER_SEEKN)
			{
				inx = 0; // start destination index
				int i = 0; // start source index
			
				for (; i < MP3PLAYER_SEEKN;) // loop all entries in the table
				{
					mp3->seek[inx] = mp3->seek[i]; // even entry will stay in the table
					inx++;
					i += 2;	// odd entry will be deleted
				}
				seekshift++; // increase seek table granularity
				seekint <<= 1; // increase seek interval
			}

			// MP3PLAYER_SEEKN is even number -> this offset will be stored as valid even after reduction
			mp3->seek[inx] = off; // store offset of the frame
			inx++;	// increase seek index
			seekcnt = seekint; // new counter to skip seek entries
		}
*/

		// add statistic
		frames++; // incrase frame counter
		bitrate += mp3->info.bitrate; // add sum of bitrate
		if (mp3->info.outputSamps > outsampmax) outsampmax = mp3->info.outputSamps; // check max. number of output samples
		sampnum += mp3->info.outputSamps; // add to total number of output samples
		n = mp3->info.size; // input size of this frame
		if (n > sizemax) sizemax = n; // check max. input size of the frame

		// skip current frame
		mp3->inbufptr += n; // shift pointer in input buffer
		mp3->inbufrem -= n; // decrease remaining data
		off += n; // shift offset in the file

		// end of scan
		if ((scan > 0) && (frames >= scan)) break;

		// refill next data
		if (scan != 0) MP3Refill(mp3); // refill next data
		if (mp3->inbufrem < 10) break; // end of data

		// find start of next frame
		n = MP3FindSyncWord((const u8*)mp3->inbufptr, mp3->inbufrem);
		if (n < 0) break; // no sync, end of data
		mp3->inbufptr += n; // shift pointer in input buffer
		mp3->inbufrem -= n; // decrease remaining data
		off += n; // shift offset in the file

		// parse frame info
		n = MP3GetNextFrameInfo(mp3dec, &mp3->info, (const u8*)mp3->inbufptr);
		if (n < 0) break; // some error or end of the sound
	}

	// seek setup
//	mp3->seekn = inx; // number of entries in seek table
//	mp3->seekshift = seekshift; // seek table granularity

	// interpolate rest of file
	mp3->bitrateavg = (int)((bitrate + frames/2) / frames); // average bit rate
	mp3->sampnumavg = (sampnum + frames/2) / frames; // average output samples per frame
	mp3->framesizeavg = (off - mp3->frame0 + frames/2)/frames; // average frame size
	if (off < mp3->filesize - 10)
	{
		frames = (mp3->filesize - mp3->frame0) / mp3->framesizeavg;
		sampnum = mp3->sampnumavg * frames;
	}

	// store statistics
	mp3->frames = frames; 		// number of frames
	mp3->outsampmax = outsampmax;	// max. number of output samples per one frame (L and R are counted as 2 samples)
	mp3->sizemax = sizemax;		// max. size of input frame in bytes
	mp3->sampnum = sampnum;		// total number of output samples (L and R are counted as 2 samples)
	mp3->timelen = (sampnum / mp3->info.nChans) / mp3->info.samprate; // total length in seconds
	// us_per_frame = frame_size * 8000000 / bitrate = samples_per_channel * 1000000 / (samprate * frames)
	mp3->frametime = (int)((s64)(sampnum / mp3->info.nChans)*1000000 / ((s64)mp3->info.samprate * frames)); // time of one frame in [us]

	// check output buffer size
	if (outsampmax > mp3->outsize) return ERR_MP3_OUTBUF_OVERFLOW;

	// reset current position to 1st frame
	mp3->pos = 0; // index of current frame
	if (mp3->filename != NULL) // file mode
	{
		// reset file mode
		FileSeek(&mp3->file, mp3->frame0); // rewind back to the start
		mp3->inbufptr = mp3->inbuf;
		mp3->inbufrem = 0;
		MP3Refill(mp3); // refill next data

		// error reading data
		if (mp3->inerror)
		{
			MP3PlayerTerm(mp3dec);
			return ERR_MP3_INVALID_FILE;
		}
	}
	else
	{
		// reset buffer mode
		mp3->inbufptr = mp3->inbuf + mp3->frame0;
		mp3->inbufrem = mp3->insize - mp3->frame0;
	}

	mp3->lastoutbuf = 0;
	mp3->chan = 255;
	mp3->playing = False;	// not playing
	mp3->speed = (float)mp3->info.samprate/SOUNDRATE;
	mp3->form = (mp3->info.nChans == 2) ? SNDFORM_PCM16_S : SNDFORM_PCM16;

	// Prepare batch size
	n = mp3->outsize / (mp3->outsampmax*2); // max. number of output frames
	if (mp3->filename != NULL)
	{
		i = mp3->insize / 2 / mp3->sizemax; // max. number of input frames (consume only half input buffer)
		if (n > i) n = i;
	}
	if (n < 1) n = 1;
	mp3->batch = n;

	return ERR_MP3_NONE;
}

// Terminate MP3 player
//  mp3 ... pointer to sMP3Player descriptor
void MP3PlayerTerm(sMP3Player* mp3)
{
	// stop playing sound
	MP3Stop(mp3);

	// close input file
	if (mp3->filename != NULL) FileClose(&mp3->file);

	// terminate MP3 decoder descriptor
	MP3FreeDecoder(mp3->mp3dec);
}

// refill output buffer (returns number of bytes)
int MP3RefillOut(sMP3Player* mp3, int bufinx)
{
	// decode frames
	int i, r;
	int outN = 0; // count output bytes
	u8* outbuf = mp3->outbuf[bufinx]; // output buffer
	const u8* buf = (const u8*)mp3->inbufptr; // pointer to data in input buffer
	int bufN = mp3->inbufrem; // remaining data in input buffer
	if ((mp3->filename == NULL) && mp3->rep) bufN -= mp3->framesizeavg; // repeat memory mode - trim last FadeOut frame
	for (i = mp3->batch; i > 0; i--) // batch frames
	{
		// find sync word
		r = MP3FindSyncWord(buf, bufN); // find sync
		if (r < 0)
		{
			if ((mp3->filename == NULL) && mp3->rep)  // memory mode and repeat sound
			{
				// rewind pointers to the start of the buffer
				buf = mp3->inbuf + mp3->frame0;
				bufN = mp3->insize - mp3->frame0 - mp3->framesizeavg; // without last FrameOut frame
				mp3->pos = 0; // reset current frame

				// find new sync word
				r = MP3FindSyncWord(buf, bufN); // find sync
				if (r < 0) break;
				buf +=  r; // shift pointer
				bufN -= r; // decrease remaining data

				// parse new frame info
				r = MP3GetNextFrameInfo(mp3->mp3dec, &mp3->info, buf);
				if ((r < 0) || (mp3->info.size > bufN)) break;

				// decode first frame with FadeIn, and skip it
				r = MP3Decode(mp3->mp3dec, &buf, &bufN, (s16*)&outbuf[outN], 0);
				if (r < 0) break;
				mp3->pos++; // increment current frame

				// find new sync word
				r = MP3FindSyncWord(buf, bufN); // find sync
				if (r < 0) break;
			}
			else
				break; // sync not found, end of sound
		}
		buf +=  r; // shift pointer
		bufN -= r; // decrease remaining data

		// parse frame info, to get output frame size
		r = MP3GetNextFrameInfo(mp3->mp3dec, &mp3->info, buf);
		if ((r < 0) || (mp3->info.size > bufN))
		{
			if ((mp3->filename == NULL) && mp3->rep)  // memory mode and repeat sound
			{
				// rewind pointers to the start of the buffer
				buf = mp3->inbuf + mp3->frame0;
				bufN = mp3->insize - mp3->frame0 - mp3->framesizeavg; // without last FrameOut frame
				mp3->pos = 0; // reset current frame

				// find new sync word
				r = MP3FindSyncWord(buf, bufN); // find sync
				if (r < 0) break;
				buf +=  r; // shift pointer
				bufN -= r; // decrease remaining data

				// parse new frame info
				r = MP3GetNextFrameInfo(mp3->mp3dec, &mp3->info, buf);
				if ((r < 0) || (mp3->info.size > bufN)) break;

				// decode first frame with FadeIn, and skip it
				r = MP3Decode(mp3->mp3dec, &buf, &bufN, (s16*)&outbuf[outN], 0);
				if (r < 0) break;
				mp3->pos++; // increment current frame

				// find new sync word
				r = MP3FindSyncWord(buf, bufN); // find sync
				if (r < 0) break;
				buf +=  r; // shift pointer
				bufN -= r; // decrease remaining data

				// parse frame info, to get output frame size
				r = MP3GetNextFrameInfo(mp3->mp3dec, &mp3->info, buf);
				if ((r < 0) || (mp3->info.size > bufN)) break;
			}
			else
				break; // invalid frame, end of sound
		}

		// decode one frame
		r = MP3Decode(mp3->mp3dec, &buf, &bufN, (s16*)&outbuf[outN], 0);
		if (r < 0) break;
		mp3->pos++; // increment current frame

		// shift output data
		outN += mp3->info.outputSamps*2;
	}
	if ((mp3->filename == NULL) && mp3->rep) bufN += mp3->framesizeavg; // repeat memory mode - return last FadeOut frame
	mp3->inbufptr = buf; // new pointer to input data
	mp3->inbufrem = bufN; // new remaining input data

	return outN;
}

// time service of playing the sound
void MP3Tick(sMP3Player* mp3)
{
	// not playing
	if (!mp3->playing) return;

	// check if streaming buffer is empty
	int chan = mp3->chan; // channel
	if (!SoundStreamIsEmpty(chan)) return; // not empty

#if MP3_CHECK_LOAD	// check MP3 load
	u32 t1 = Time();
	int rem = mp3->inbufrem;
	if (rem < mp3->decode_inrem) mp3->decode_inrem = rem;
#endif

	// decode next frame
	int bufinx = mp3->lastoutbuf ^ 1; // next buffer we will send to sound driver

	// refill output buffer (returns number of bytes)
	int outN = MP3RefillOut(mp3, bufinx);

	// no output data
	u8* outbuf = mp3->outbuf[bufinx];
	if (outN == 0)
	{
		// end of sound (or drop-out > 0.5 second)
		if (Time() - mp3->droptime > 500000)
		{
			mp3->playing = False;
			return;
		}

		// drop-out, use empty sound at least
		outN = mp3->outsampmax*2;
		memset(outbuf, 0, outN);
	}
	else
		mp3->droptime = Time();

#if MP3_CHECK_LOAD	// check MP3 load
	// remaining ouput buffer
	if (PlayingSoundChan(mp3->chan))
	{
		rem = PwmSound[mp3->chan].cnt*2 * mp3->info.nChans;
		if (rem < mp3->decode_outrem) mp3->decode_outrem = rem;
	}
#endif

	// send buffer to the stream
	SoundStreamSetNext(chan, outbuf, outN);

	// switch output buffer
	mp3->lastoutbuf = bufinx;

#if MP3_CHECK_LOAD	// check MP3 load
	u32 t2 = Time();
	mp3->decode_time += t2 - t1;
	mp3->decode_num += mp3->batch;
#endif
}

#if MP3PLAYER_USEIRQ
// MP3 IRQ handler
void MP3Handler()
{
	MP3Tick(MP3Handler_ID);
}
#endif // MP3PLAYER_USEIRQ

// Poll loading MP3 file to the player
// - Should be called from the program loop at least every 0.2 second
void MP3Poll(sMP3Player* mp3)
{
#if !MP3PLAYER_USEIRQ
	// decode sound to the sound buffer
	MP3Tick(mp3);
#endif

	// refill input buffer
	MP3Refill(mp3);
}

// start/continue playing the sound
//  mp3 ... pointer to sMP3Player descriptor
//  chan ... PWMSnd channel to play
//  rep ... repeat the sound
void MP3Play(sMP3Player* mp3, int chan, Bool rep)
{
	// stop playing
	MP3Stop(mp3);

	// save channel and repeat flag
	mp3->chan = chan;
	mp3->rep = rep;

	// refill input buffer
	MP3Refill(mp3);

	// refill output buffer (returns number of bytes)
	int bufinx = 0;
	mp3->lastoutbuf = bufinx; // last output buffer sent to sound driver
	int outN = MP3RefillOut(mp3, bufinx);

	// no data - try to start playing from start again
	if (outN == 0)
	{
		mp3->rep = True;
		outN = MP3RefillOut(mp3, bufinx);
		mp3->rep = rep;

		// error, not playing
		if (outN == 0) return;
	}

	// refill input buffer
	MP3Refill(mp3);

#if MP3_CHECK_LOAD	// check MP3 load
	mp3->decode_time = 0;
	mp3->decode_num = 0;
	mp3->decode_outrem = mp3->outsize;
	mp3->decode_inrem = mp3->insize;
	mp3->poll_time = 0;
	mp3->poll_len = 0;
	mp3->poll_rem = mp3->insize;
#endif

	// start playing the sound
	mp3->droptime = Time();
	mp3->playing = True;
	PlaySoundChan(chan, mp3->outbuf[bufinx], outN, SNDREPEAT_STREAM, mp3->speed, 1.0f, mp3->form, 0);

#if MP3PLAYER_USEIRQ
	// install IQ handler
	MP3Handler_ID = mp3;
	SetHandler(MP3IRQNUM, MP3Handler);
	NVIC_IRQEnable(MP3IRQNUM);
	NVIC_IRQPrio(MP3IRQNUM, IRQ_PRIO_PENDSV);

	// setup IRQ request
	sPwmSnd* s = &PwmSound[chan];
	s->useirq = True;
	s->irq = MP3IRQNUM;
	NVIC_IRQForce(MP3IRQNUM); // request to refill second buffer
#endif // MP3PLAYER_USEIRQ
}

// get current position in seconds
int MP3GetTimePos(sMP3Player* mp3)
{
	int pos = (int)((s64)mp3->pos * mp3->frametime / 1000000);
	if (pos > mp3->timelen) pos = mp3->timelen;
	return pos;
}

// seek to position in frames
void MP3SeekFrame(sMP3Player* mp3, int pos)
{
	// save playing state
	Bool playing = mp3->playing;

	// stop playing
	MP3Stop(mp3);
	WaitMs(2);

	// set new frame
	if (pos < 0) pos = 0;
	if (pos >= mp3->frames) pos = mp3->frames - 1;
	mp3->pos = pos;
	int off = mp3->frame0 + pos*mp3->framesizeavg; // offset of the frame

	if (mp3->filename == NULL) // buffer mode
	{
		mp3->inbufptr = mp3->inbuf + off;
		mp3->inbufrem = mp3->insize - off;
	}

	// file mode
	else
	{
		mp3->inbufptr = mp3->inbuf;
		mp3->inbufrem = 0;
		FileSeek(&mp3->file, off);
		MP3Refill(mp3); // refill input buffer
	}

	// save current frame setup
	sMP3FrameHeader *fh = &mp3->decinfo.FrameHeaderS;
	int ver = fh->ver;
	int layer = fh->layer;
	int nchans = fh->sMode;
	int samprate = fh->srIdx;
	int crc = fh->crc;
	int copy = fh->copyFlag;
	int orig = fh->origFlag;

	// synchronize to new frame
	int r;
	const u8* buf = (const u8*)mp3->inbufptr; // pointer to data in input buffer
	int bufN = mp3->inbufrem; // remaining data in input buffer
	while (bufN > 0)
	{
		// find sync word
		r = MP3FindSyncWord(buf, bufN); // find sync
		if (r < 0) return; 
		buf +=  r; // shift pointer
		bufN -= r; // decrease remaining data

		// parse frame info
		r = MP3GetNextFrameInfo(mp3->mp3dec, &mp3->info, buf);
		if (r >= 0)
		{
			if (	(ver == fh->ver) &&
				(layer == fh->layer) &&
				(nchans == fh->sMode) &&
				(samprate == fh->srIdx) &&
				(crc == fh->crc) &&
				(copy == fh->copyFlag) &&
				(orig == fh->origFlag))
				break;
		}

		// header is incorrect, skip this header
		buf++;
		bufN--;
	}
	if (bufN <= 0) return;

	mp3->inbufptr = buf;
	mp3->inbufrem = bufN;

	// continue playing
	if (playing) MP3Play(mp3, mp3->chan, mp3->rep);
}

// seek to position in seconds
void MP3SeekTime(sMP3Player* mp3, int pos)
{
	MP3SeekFrame(mp3, (int)((s64)pos * 1000000 / mp3->frametime));
}

// stop/pause playing sound
void MP3Stop(sMP3Player* mp3)
{
	// not playing
	if (!mp3->playing) return;
	mp3->playing = False;

	// stop playing sound
	StopSoundChan(mp3->chan);

#if MP3PLAYER_USEIRQ
	// uninstall IQ handler
	NVIC_IRQDisable(MP3IRQNUM);
	NVIC_IRQPrio(MP3IRQNUM, IRQ_PRIO_NORMAL);
#endif // MP3PLAYER_USEIRQ
}

#endif // USE_MP3
