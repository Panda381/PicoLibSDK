
// ****************************************************************************
//
//                           MP3 decoder - Public interface
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/

#if USE_MP3			// use MP3 decoder (lib_mp3*.c, lib_mp3*.h)

#ifndef _LIB_MP3_H
#define _LIB_MP3_H

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
//                     Internal MP3 structures
// ----------------------------------------------------------------------------

// determining MP3_MAINBUF_SIZE:
//  max mainDataBegin = (2^9 - 1) bytes (since 9-bit offset) = 511
//  max nSlots (concatenated with mainDataBegin bytes from before) = 1440 - 9 - 4 + 1 = 1428
//  511 + 1428 = 1939, round up to 1940 (4-byte align)
#define MP3_MAINBUF_SIZE	1940

#define MP3_MAX_NGRAN		2		// max granules
#define MP3_MAX_NCHAN		2		// max channels
#define MP3_MAX_NSAMP		576		// max samples per channel, per granule

#define MP3_MAX_SCFBD		4		// max scalefactor bands per channel

#define MP3_BLOCK_SIZE		18
#define	MP3_NBANDS		32
#define MP3_MAX_REORDER_SAMPS	((192-126)*3)	// largest critical band for short blocks (see sfBandTable)
#define MP3_VBUF_LENGTH		(17 * 2 * MP3_NBANDS)	// for double-sized vbuf FIFO

// StereoMode: map these to the corresponding 2-bit values in the frame header
enum {
	MP3_SM_Stereo = 0x00,	// two independent channels, but L and R frames might have different # of bits
	MP3_SM_Joint = 0x01,	// coupled channels - layer III: mix of M-S and intensity, Layers I/II: intensity and direct coding only
	MP3_SM_Dual = 0x02,	// two independent channels, L and R always have exactly 1/2 the total bitrate
	MP3_SM_Mono = 0x03	// one channel
};

// Band table
typedef struct {
	short l[23];
	short s[14];
} sMP3SFBandTable;

// Frame header
typedef struct {
	int ver;		// MPEG audio version ID MP3_VER_*
	int layer;		// MPEG layer index (1, 2, or 3)
	int crc;		// protected CRC flag: 0 = disabled, 1 = enabled
	int brIdx;		// bitrate index (0 - 14), 0 = "free" mode
	int srIdx;		// sample rate frequency index (0 - 2)
	int paddingBit;		// padding flag: 0 = no padding, 1 = single pad byte
	int privateBit;		// private flag is not used
	int sMode;		// mono/stereo mode MP3_SM_*: 0=stereo, 1=joint, 2=dual, 3=mono
	int modeExt;		// used to decipher joint stereo mode
	int copyFlag;		// copyright flag: 0 = no, 1 = yes
	int origFlag;		// original flag: 0 = copy, 1 = original
	int emphasis;		// deemphasis mode (0=none, 1=50/15 ms, 2=reserved, 3=CCIT J.17)
	int CRCWord;		// CRC word (16 bits, 0 if crc not enabled)
	const sMP3SFBandTable *sfBand; // pointer to band table
} sMP3FrameHeader;

// Side info subblock
typedef struct {
	int part23Length;	// number of bits in main data
	int nBigvals;		// 2x this = first set of Huffman cw's (maximum amplitude can be > 1)
	int globalGain;		// overall gain for dequantizer
	int sfCompress;		// unpacked to figure out number of bits in scale factors
	int winSwitchFlag;	// window switching flag
	int blockType;		// block type
	int mixedBlock;		// 0 = regular block (all short or long), 1 = mixed block
	int tableSelect[3];	// index of Huffman tables for the big values regions
	int subBlockGain[3];	// subblock gain offset, relative to global gain
	int region0Count;	// 1+region0Count = num scale factor bands in first region of bigvals
	int region1Count;	// 1+region1Count = num scale factor bands in second region of bigvals
	int preFlag;		// for optional high frequency boost
	int sfactScale;		// scaling of the scalefactors
	int count1TableSelect;	// index of Huffman table for quad codewords
} sMP3SideInfoSub;

// Side info
typedef struct {
	int mainDataBegin;
	int privateBits;
	int scfsi[MP3_MAX_NCHAN][MP3_MAX_SCFBD];	// 4 scalefactor bands per channel
	sMP3SideInfoSub	sis[MP3_MAX_NGRAN][MP3_MAX_NCHAN];
} sMP3SideInfo;

// max bits in scalefactors = 5, so use char's to save space
typedef struct {
	char l[23];            // [band]
	char s[13][3];         // [band][window]
} sMP3ScaleFactorInfoSub;  

// used in MPEG 2, 2.5 intensity (joint) stereo only
typedef struct {
	int intensityScale;		
	int	slen[4];
	int	nr[4];
} sMP3ScaleFactorJS;

typedef struct {
	sMP3ScaleFactorInfoSub sfis[MP3_MAX_NGRAN][MP3_MAX_NCHAN];
	sMP3ScaleFactorJS sfjs;
} sMP3ScaleFactorInfo;

// Huffman table info
typedef struct {
	int huffDecBuf[MP3_MAX_NCHAN][MP3_MAX_NSAMP];	// used both for decoded Huffman values and dequantized coefficients
	int nonZeroBound[MP3_MAX_NCHAN];		// number of coeffs in huffDecBuf[ch] which can be > 0
	int gb[MP3_MAX_NCHAN];				// minimum number of guard bits in huffDecBuf[ch]
} sMP3HuffmanInfo;

typedef struct {
	int cbType;			// pure long = 0, pure short = 1, mixed = 2
	int cbEndS[3];			// number nonzero short cb's, per subbblock
	int cbEndSMax;			// max of cbEndS[]
	int cbEndL;			// number nonzero long cb's
} sMP3CriticalBandInfo;

typedef struct {
	int workBuf[MP3_MAX_REORDER_SAMPS];		// workbuf for reordering short blocks
	sMP3CriticalBandInfo cbi[MP3_MAX_NCHAN];	// filled in dequantizer, used in joint stereo reconstruction
} sMP3DequantInfo;

typedef struct {
	int outBuf[MP3_MAX_NCHAN][MP3_BLOCK_SIZE][MP3_NBANDS];	// output of IMDCT
	int overBuf[MP3_MAX_NCHAN][MP3_MAX_NSAMP / 2];	// overlap-add buffer (by symmetry, only need 1/2 size)
	int numPrevIMDCT[MP3_MAX_NCHAN];		// how many IMDCT's calculated in this channel on prev. granule
	int prevType[MP3_MAX_NCHAN];
	int prevWinSwitch[MP3_MAX_NCHAN];
	int gb[MP3_MAX_NCHAN];
} sMP3IMDCTInfo;

// NOTE - could get by with smaller vbuf if memory is more important than speed
// (in Subband, instead of replicating each block in FDCT32 you would do a memmove on the
// last 15 blocks to shift them down one, a hardware style FIFO)
typedef struct {
	int vbuf[MP3_MAX_NCHAN * MP3_VBUF_LENGTH];	// vbuf for fast DCT-based synthesis PQMF - double size for speed (no modulo indexing)
	int vindex;					// internal index for tracking position in vbuf
} sMP3SubbandInfo;

// MP3 decoder info
typedef struct {
	// pointers to platform-specific data structures
	sMP3FrameHeader*	FrameHeaderPS;
	sMP3SideInfo*		SideInfoPS;
	sMP3ScaleFactorInfo*	ScaleFactorInfoPS;
	sMP3HuffmanInfo*	HuffmanInfoPS;
	sMP3DequantInfo*	DequantInfoPS;
	sMP3IMDCTInfo*		IMDCTInfoPS;
	sMP3SubbandInfo*	SubbandInfoPS;

	Bool	stat;	// structures are static - do not delete them

	// buffer which must be large enough to hold largest possible main_data section
	unsigned char mainBuf[MP3_MAINBUF_SIZE];

	// special info for "free" bitrate files
	int freeBitrateFlag;
	int freeBitrateSlots;

	// user-accessible info
	int bitrate;
	int nChans;
	int samprate;
	int nGrans;			// granules per frame
	int nGranSamps;			// samples per granule
	int nSlots;
	int layer;
	int version;
	int size;

	int frameheadersize;		// size of next frame header 4 or 6, or 0 = not unpacked yet

	int mainDataBegin;
	int mainDataBytes;

	int part23Length[MP3_MAX_NGRAN][MP3_MAX_NCHAN];
} sMP3DecInfo;

// Static MP3 decoder info (size 24 KB)
typedef struct {
	sMP3FrameHeader		FrameHeaderS;
	sMP3SideInfo		SideInfoS;
	sMP3ScaleFactorInfo	ScaleFactorInfoS;
	sMP3HuffmanInfo		HuffmanInfoS;
	sMP3DequantInfo		DequantInfoS;
	sMP3IMDCTInfo		IMDCTInfoS;
	sMP3SubbandInfo		SubbandInfoS;
	sMP3DecInfo		MP3DecInfoS;
} sMP3DecInfoStat;

// size of static structure for MP3 decoder info
#define MP3DECODER_SIZE	sizeof(sMP3DecInfoStat)

// ----------------------------------------------------------------------------
//                       Public API interface
// ----------------------------------------------------------------------------

// MP3 MPEG audio version ID
//  Note: MPEG Version 2.5 was added lately to the MPEG 2 standard. It is an
//  extension used for very low bitrate files, allowing the use of lower sampling
//  frequencies. If your decoder does not support this extension, it is recommended
//  for you to use 12 bits for synchronization instead of 11 bits.
enum {
	MP3_VER_MPEG1 = 0,	// MPEG version 1 (ISO/IEC 11172-3)
	MP3_VER_MPEG2 = 1,	// MPEG version 2 (ISO/IEC 13818-3)
	MP3_VER_MPEG25 = 2,	// MPEG version 2.5 (later version of MPEG 2)
};

// MP3 error codes
enum {
	ERR_MP3_NONE =			0,	// no error
	ERR_MP3_INDATA_UNDERFLOW =	-1,	// out of data - assume last or truncated frame
	ERR_MP3_MAINDATA_UNDERFLOW =	-2,	// not enough data in bit reservoir from previous frames (perhaps starting in middle of file)
	ERR_MP3_FREE_BITRATE_SYNC =	-3,	// cannot find sync in free mode
	ERR_MP3_OUT_OF_MEMORY =		-4,	// cannot allocate buffers
	ERR_MP3_NULL_POINTER =		-5,	// NULL pointer passes instead of decoder handler
	ERR_MP3_INVALID_FRAMEHEADER =	-6,	// invalid frame header
	ERR_MP3_INVALID_SIDEINFO =	-7,	// invalid side info header
	ERR_MP3_INVALID_SCALEFACT =	-8,	// invalid scale factors
	ERR_MP3_INVALID_HUFFCODES =	-9,	// invalid Huffman codes
	ERR_MP3_INVALID_DEQUANTIZE =	-10,	// invalid dequantize coefficients
	ERR_MP3_INVALID_IMDCT =		-11,	// invalid IMDCT (Inverse Modified Discrete Cosine Transform)
	ERR_MP3_INVALID_SUBBAND =	-12,	// invalid subband transform
	ERR_MP3_INVALID_FREE =		-13,	// free mode not supported by MP3 player
	ERR_MP3_OUTBUF_OVERFLOW =	-14,	// output buffer overflow (output buffer is too small)
	ERR_MP3_NOMOUNT =		-15,	// cannot mount SD disk
	ERR_MP3_INVALID_FILE =		-16,	// invalid input file, cannot open
};

// MP3 frame info
typedef struct {
	int	bitrate;		// bit rate (input bits per second = 8000 - 384000), 0 = "free" mode
	int	nChans;			// number of channels (= 1 or 2)
	int	samprate;		// sample rate (samples per second per one channel = 8000 - 48000)
	int	bitsPerSample;		// number of bits per sample (always 16)
	int	outputSamps;		// number of output samples per one frame (= mono 384 - 1152, stereo 768 - 2304; 2304 is 1152 L+R double-samples = 4608 bytes)
	int	layer;			// MPEG layer description (Layer = 1 - 3)
	int	version;		// MPEG audio version ID (= MP3_VER_*)
	int	size;			// size of MP3 frame in number of bytes, including headers and CRC and padding (= 24 - 1441)
} sMP3FrameInfo;

// MP3 handler (pointer to MP3 descriptor sMP3DecInfo*)
typedef void* HMP3Decoder;

// Initialize MP3 decoder
//  mp3stat ... pointer to static buffer for MP3 decoder info structure of size MP3DECODER_SIZE bytes, or NULL to auto-allocate
// Returns MP3 handler, or NULL on error (pointer to the MP3 descriptor; requires about 24 KB of RAM).
// If static buffer is used, caller must free it after finalize.
// If auto-allocation is used, MP3FreeDecoder() must be called after finalize, to free allocated structures.
HMP3Decoder MP3InitDecoder(void* mp3stat);

// Free MP3 decoder (static descriptor will not be deallocated)
//  hMP3Decoder ... MP3 decoder handler
void MP3FreeDecoder(HMP3Decoder hMP3Decoder);

// Check ID3v2 tag on start of the MP3 (to skip it)
//  buf ... pointer to input buffer (with 10 bytes of data)
// Returns size of the Id3v2 tag in bytes, or 0 if ID3v2 tag not found.
int MP3CheckID3v2(const u8* buf);

// Find start of next frame
//  buf ... pointer to source buffer
//  nBytes ... size of data in source buffer
// Returns offset of start of next frame (if >= 0), or -1 if sync word not found.
int MP3FindSyncWord(const u8* buf, int nBytes);

// Get info from next frame (parse frame header, can be used at start of frame after MP3FindSyncWord() and before MP3Decode())
//  hMP3Decoder ... MP3 decoder handler
//  mp3FrameInfo ... pointer to get MP3 frame info
//  buf ... pointer to source buffer (with 4 or 6 bytes of data)
// Returns error code ERR_MP3_*.
int MP3GetNextFrameInfo(HMP3Decoder hMP3Decoder, sMP3FrameInfo* mp3FrameInfo, const u8* buf);

// Get info from last frame (can be used after MP3Decode())
//  hMP3Decoder ... MP3 decoder handler
//  mp3FrameInfo ... pointer to get MP3 frame info
void MP3GetLastFrameInfo(HMP3Decoder hMP3Decoder, sMP3FrameInfo* mp3FrameInfo);

// Decode one frame of the MP3
//  hMP3Decoder ... MP3 decoder handler
//  inbuf ... pointer to pointer to start of frame in input buffer (will be increased, required data size sMP3FrameInfo.size)
//  bytesLeft ... pointer to bytes left in input buffer (will be decreased)
//  outbuf ... pointer to output buffer, big enough to hold one frame of decodec PCM samples (sMP3FrameInfo.outputSamps, interleave LRLRLR...)
//		number of output samples = nGrans * nGranSamps * nChans
//  useSize ... flag indicating whether MP3 data is normal MPEG format (=0) or reformated "self-contained" frames (=1)
// Returns error code ERR_MP3_*. Use MP3FindSyncWord() to find start of next frame before decode next frame.
int MP3Decode(HMP3Decoder hMP3Decoder, const u8** inbuf, int* bytesLeft, s16* outbuf, int useSize);

// ----------------------------------------------------------------------------
//                          MP3 player (PWMSnd output)
// ----------------------------------------------------------------------------
// Total RAM needed to play MP3 file:
// - MP3 player descriptor 28 KB
// - input buffer 12 KB
// - output buffers 70 KB (poll mode) or 10 KB (IRQ mode)
// Total = 110 KB of RAM (poll mode) or 50 KB of RAM (IRQ mode).

// Use IRQ mode instead of poll mode.
// RISC-V needs to use poll mode, because IRQ handler cannot set lower priority and would disable sound output.
#ifndef MP3PLAYER_USEIRQ
#if RISCV
#define MP3PLAYER_USEIRQ	0
#else
#define MP3PLAYER_USEIRQ	1
#endif
#endif

// IRQ of MP3 player to decode frame, in IRQ mode
//#if MP3PLAYER_USEIRQ
//#define MP3PLAYER_IRQ		IRQ_SPAREIRQ_0	// IRQ used to decode next frame
//#endif

// To hold 0.2 second of the sound in input buffer:
//   Input bitrate can be in range 8 to 448 kbps, it means 1 to 56 KB/sec.
//   Interval of 0.2 second requires input buffer up to 11 KB.
#ifndef MP3PLAYER_INSIZE
#define MP3PLAYER_INSIZE	12000		// size of input buffer in bytes
#endif

// To hold 0.2 second of the sound in output buffer:
//   Output sample rate is in range 8 kHz to 48 kHz. With stereo 16-bit it is 32 to 192 KB.
//   Interval 0.2 second requires output buffer up to 38 KB.
//   If we limit it to 44kHz, we need buffer 35 KB. 2 buffers are 70 KB.
// In IRQ mode we need hold 1 frame = max. 2304 samples = 4608 bytes, we need 2 buffers = 10 KB
#ifndef MP3PLAYER_OUTSIZE
#if MP3PLAYER_USEIRQ
#define MP3PLAYER_OUTSIZE	10000		// size of output buffer in bytes ... IRQ mode
#else
#define MP3PLAYER_OUTSIZE	70000		// size of output buffer in bytes ... poll mode
#endif
#endif

// Number of entries in seek table (every entry is 4 bytes)
//   5-minute song - one seek entry is 0.3 second, it means 2 to 40 frames.
//#ifndef MP3PLAYER_SEEKN
//#define MP3PLAYER_SEEKN	1000		// number of entries in seek table (1000 entries = 4 KB; must be even number)
//#endif

// ID3v1.1 tag (size 128 bytes)
#define ID3V1_TITLE_LEN		30	// ID3v1 title max. length
#define ID3V1_ARTIST_LEN	30	// ID3v1 artist max. length
#define ID3V1_ALBUM_LEN		30	// ID3v1 album max. length
#define ID3V1_YEAR_LEN		4	// ID3v1 year max. length
#define ID3V1_COMMENT_LEN	29	// ID3v1 comment max. length

#pragma pack(push,1)
typedef struct {
	char	tag[3];				// 0-2: tag "TAG" (3 characters)
	char	title[ID3V1_TITLE_LEN];		// 3-32: title (30 characters)
	char	artist[ID3V1_ARTIST_LEN];	// 33-62: artist (30 characters)
	char	album[ID3V1_ALBUM_LEN];		// 63-92: album (30 characters)
	char	year[ID3V1_YEAR_LEN];		// 93-96: year (4 characters)
	char	comment[ID3V1_COMMENT_LEN];	// 97-125: comment (29 characters)
	u8	track;				// 126: track number
	u8	genre;				// 127: genre identifier
} sID3v1;
#pragma pack(pop)
#define ID3V1_SIZE	sizeof(sID3v1)		// size of ID3v1 tag

STATIC_ASSERT(ID3V1_SIZE == 128, "Incorrect ID3v1 size!");

// MP3 player descriptor (size 28 KB)
typedef struct {
	// playing setup
	u8		chan;				// PWMSnd channel to play (255 = not defined yet)
	Bool		rep;				// flag - repeat the sound
	Bool		playing;			// sound is playing
	u8		form;				// format SNDFORM_PCM16*
	float		speed;				// sound speed (sample rate relative to SOUNDRATE = 22050)
	u32		droptime;			// last drop-out time

	// input file
	const char*	filename;			// filename (NULL = buffer mode, play from buffer)
	sFile		file;				// open file
	int		filesize;			// file size (without ID3v1 on the end)
	int		frame0;				// offset of start of frame 0 (start after ID3v2, synced)

	// input data buffer
	const u8*	inbuf;				// pointer to input data buffer with the sound in MP3 format
	int		insize;				// size of input data buffer
	volatile const u8* inbufptr;			// current pointer
	volatile int	inbufrem;			// remaining data
	Bool		inerror;			// error reading data

	// output buffers
	u8*		outbuf[2];			// pointers to output buffers
	int		outsize;			// size of one output buffer
	int		lastoutbuf;			// last output buffer sent to sound driver
	int		batch;				// number of frames to decode in one batch

	// seek position
// Total_time = frames * frametime ... total length of the sound in [us]
// Current_pos = pos * frametime ... current playing position in [us]
	volatile int	pos;				// index of current frame
	int		frames;				// total number of frames
	u32		frametime;			// time of one frame in [us] (= 8000 to 144000, 7 to 125 frames per second)
//	int		seekn;				// number of entries in seek table
//	int		seekshift;			// seek table granularity = number of bits to shift frame number right
//	int		seek[MP3PLAYER_SEEKN];		// seek table (offset of frames in the buffer; MP3PLAYER_SEEKN must be even number)

	// sound info
	sMP3FrameInfo	info;				// MP3 frame info
	int		bitrateavg;			// average bit rate
	int		framesizeavg;			// average frame input size
	int		sampnumavg;			// average output samples per frame (L and R are counted as 2 samples)
	int		outsampmax;			// max. number of output samples per one frame (L and R are counted as 2 samples)
	int		sizemax;			// max. size of input frame in bytes
	int		sampnum;			// total number of samples (L and R are counted as 2 samples)
	int		timelen;			// total length in number of seconds

	// ID3v1 tag buffer
	int		id3v1_off;			// offset of ID3v1 tag on end of MP3 (size 128 bytes; 0 = no ID3v1 tag)
	int		id3v2_size;			// size of ID3v2 tag on start of MP3 (0 = no ID3v2 tag)
	sID3v1		id3v1;				// ID3v1 tag (valid if id3v1_off > 0) ... all entries are zero padded

	u8		id3_title_len;			// title length
	u8		id3_artist_len;			// artist length
	u8		id3_album_len;			// album length
	u8		id3_comment_len;		// comment length
	u8		id3_year_len;			// year length
	char		id3_title[ID3V1_TITLE_LEN+1];	// title with erminating 0
	char		id3_artist[ID3V1_ARTIST_LEN+1];	// artist with erminating 0
	char		id3_album[ID3V1_ALBUM_LEN+1];	// album with erminating 0
	char		id3_comment[ID3V1_COMMENT_LEN+1]; // comment with erminating 0
	char		id3_year[ID3V1_YEAR_LEN+1];	// year with erminating 0

	// check MP3 load
#if MP3_CHECK_LOAD
	volatile u32	decode_time;			// sum of decode time [us]
	volatile int	decode_num;			// count of decoded frames
	volatile int	decode_outrem;			// lowest remaining data in output buffer (full: mp3->outsize)
	volatile int	decode_inrem;			// lowest remaining data in input buffer (full: mp3->insize)
	volatile u32	poll_time;			// sum of poll time [us]
	volatile int	poll_len;			// number of polled bytes
	volatile int	poll_rem;			// lowest remaining data in buffer (full: mp3->insize)
#endif

	// internal structures of MP3 decoder
	HMP3Decoder	mp3dec;				// handle of MP3 decoder
	sMP3DecInfoStat	decinfo;			// MP3 decoder info (size 24 KB)

} sMP3Player;

#if MP3PLAYER_USEIRQ

#ifndef MP3IRQNUM
#define MP3IRQNUM IRQ_SPAREIRQ_0
#endif

extern sMP3Player* MP3Handler_ID;	// MP3 handler used in IRQ mode

#endif // MP3PLAYER_USEIRQ

// MP3 genre list
#define MP3_GENRELIST_NUM	192
extern const char* MP3GenreList[MP3_GENRELIST_NUM];

// Initialize MP3 player
//  mp3 ... pointer to sMP3Player descriptor
//  filename ... filename, NULL=play MP3 from memory (inbuf)
//  inbuf ... pointer to MP3 in memory, or pointer to input bufer for file mode (recommended size MP3PLAYER_INSIZE)
//  insize ... size of input buffer in bytes
//  outbuf ... pointer to output buffer (must be aligned to u16 or better to u32; recommended size MP3PLAYER_OUTSIZE)
//  outsize ... size of output buffer in bytes
//  scan ... number of frames to scan file on open, -1=scan all file (count frames and length; recommended value is 100)
// Returns error code ERR_MP3_* (ERR_MP3_NONE = 0 if OK)
int MP3PlayerInit(sMP3Player* mp3, const char* filename, const u8* inbuf, int insize, u8* outbuf, int outsize, int scan);

// Terminate MP3 player
//  mp3 ... pointer to sMP3Player descriptor
void MP3PlayerTerm(sMP3Player* mp3);

// Poll loading MP3 file to the player
// - Should be called from the program loop at least every 0.2 second
void MP3Poll(sMP3Player* mp3);

// start/continue playing the sound
//  mp3 ... pointer to sMP3Player descriptor
//  chan ... PWMSnd channel to play
//  rep ... repeat the sound
void MP3Play(sMP3Player* mp3, int chan, Bool rep);

// check if MP3 Player is plaing (Wait a few milliseconds after termination for the play buffers to empty)
INLINE Bool MP3Playing(sMP3Player* mp3) { return mp3->playing || PlayingSoundChan(mp3->chan); }

// get total length in seconds
INLINE int MP3GetTimeLen(sMP3Player* mp3) { return mp3->timelen; }

// get total length in frames
INLINE int MP3GetFrameLen(sMP3Player* mp3) { return mp3->frames; }

// get current position in seconds
int MP3GetTimePos(sMP3Player* mp3);

// get current position in frames
INLINE int MP3GetFramePos(sMP3Player* mp3) { return mp3->pos; }

// seek to position in frames
void MP3SeekFrame(sMP3Player* mp3, int pos);

// seek to position in seconds
void MP3SeekTime(sMP3Player* mp3, int pos);

// stop/pause playing sound
void MP3Stop(sMP3Player* mp3);

#ifdef __cplusplus
}
#endif

#endif // _LIB_MP3_H

#endif // USE_MP3
