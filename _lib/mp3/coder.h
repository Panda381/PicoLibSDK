/* ***** BEGIN LICENSE BLOCK ***** 
 * Version: RCSL 1.0/RPSL 1.0 
 *  
 * Portions Copyright (c) 1995-2002 RealNetworks, Inc. All Rights Reserved. 
 *      
 * The contents of this file, and the files included with this file, are 
 * subject to the current version of the RealNetworks Public Source License 
 * Version 1.0 (the "RPSL") available at 
 * http://www.helixcommunity.org/content/rpsl unless you have licensed 
 * the file under the RealNetworks Community Source License Version 1.0 
 * (the "RCSL") available at http://www.helixcommunity.org/content/rcsl, 
 * in which case the RCSL will apply. You may also obtain the license terms 
 * directly from RealNetworks.  You may not use this file except in 
 * compliance with the RPSL or, if you have a valid RCSL with RealNetworks 
 * applicable to this file, the RCSL.  Please see the applicable RPSL or 
 * RCSL for the rights, obligations and limitations governing use of the 
 * contents of the file.  
 *  
 * This file is part of the Helix DNA Technology. RealNetworks is the 
 * developer of the Original Code and owns the copyrights in the portions 
 * it created. 
 *  
 * This file, and the files included with this file, is distributed and made 
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. 
 * 
 * Technology Compatibility Kit Test Suite(s) Location: 
 *    http://www.helixcommunity.org/content/tck 
 * 
 * Contributor(s): 
 *  
 * ***** END LICENSE BLOCK ***** */ 

/**************************************************************************************
 * Fixed-point MP3 decoder
 * Jon Recker (jrecker@real.com), Ken Cooke (kenc@real.com)
 * June 2003
 *
 * coder.h - private, implementation-specific header file
 **************************************************************************************/

#ifndef _CODER_H
#define _CODER_H

#include "mp3common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ASSERT)
#undef ASSERT
#endif
#if defined(_WIN32) && defined(_M_IX86) && (defined (_DEBUG) || defined (REL_ENABLE_ASSERTS))
#define ASSERT(x) if (!(x)) __asm int 3;
#else
#define ASSERT(x) /* do nothing */
#endif

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

/* clip to range [-2^n, 2^n - 1] */
#define CLIP_2N(y, n) { \
	int sign = (y) >> 31;  \
	if (sign != (y) >> (n))  { \
		(y) = sign ^ ((1 << (n)) - 1); \
	} \
}

#define SIBYTES_MPEG1_MONO		17
#define SIBYTES_MPEG1_STEREO	32
#define SIBYTES_MPEG2_MONO		 9
#define SIBYTES_MPEG2_STEREO	17

/* number of fraction bits for pow43Tab (see comments there) */
#define POW43_FRACBITS_LOW		22
#define POW43_FRACBITS_HIGH		12

#define DQ_FRACBITS_OUT			25	/* number of fraction bits in output of dequant */
#define	IMDCT_SCALE				2	/* additional scaling (by sqrt(2)) for fast IMDCT36 */

#define	HUFF_PAIRTABS			32

/* additional external symbols to name-mangle for static linking */
#define	SetBitstreamPointer	STATNAME(SetBitstreamPointer)
#define	GetBits				STATNAME(GetBits)
#define	CalcBitsUsed		STATNAME(CalcBitsUsed)
#define	DequantChannel		STATNAME(DequantChannel)
#define	MidSideProc			STATNAME(MidSideProc)
#define	IntensityProcMPEG1	STATNAME(IntensityProcMPEG1)
#define	IntensityProcMPEG2	STATNAME(IntensityProcMPEG2)
#define PolyphaseMono		STATNAME(PolyphaseMono)
#define PolyphaseStereo		STATNAME(PolyphaseStereo)
#define FDCT32				STATNAME(FDCT32)

#define	ISFMpeg1			STATNAME(ISFMpeg1)
#define	ISFMpeg2			STATNAME(ISFMpeg2)
#define	ISFIIP				STATNAME(ISFIIP)
#define uniqueIDTab			STATNAME(uniqueIDTab)
#define	coef32				STATNAME(coef32)
#define	polyCoef			STATNAME(polyCoef)
#define	csa					STATNAME(csa)
#define	imdctWin			STATNAME(imdctWin)

#define	huffTable			STATNAME(huffTable)
#define	huffTabOffset		STATNAME(huffTabOffset)
#define	huffTabLookup		STATNAME(huffTabLookup)
#define	quadTable			STATNAME(quadTable)
#define	quadTabOffset		STATNAME(quadTabOffset)
#define	quadTabMaxBits		STATNAME(quadTabMaxBits)

typedef struct _BitStreamInfo {
	const u8 *bytePtr;
	unsigned int iCache;
	int cachedBits;
	int nBytes;
} BitStreamInfo;

typedef enum _HuffTabType {
	noBits,
	oneShot,
	loopNoLinbits,
	loopLinbits,
	quadA,
	quadB,
	invalidTab
} HuffTabType;

typedef struct _HuffTabLookup {
	int	linBits;
	HuffTabType tabType;
} HuffTabLookup;

typedef struct _BlockCount {
	int nBlocksLong;
	int nBlocksTotal;
	int nBlocksPrev; 
	int prevType;
	int prevWinSwitch;
	int currWinSwitch;
	int gbIn;
	int gbOut;
} BlockCount;

/* bitstream.c */
void SetBitstreamPointer(BitStreamInfo *bsi, int nBytes, const u8 *buf);
unsigned int GetBits(BitStreamInfo *bsi, int nBits);
int CalcBitsUsed(BitStreamInfo *bsi, const u8 *startBuf, int startOffset);

/* dequant.c, dqchan.c, stproc.c */
int DequantChannel(int *sampleBuf, int *workBuf, int *nonZeroBound, sMP3FrameHeader *fh, sMP3SideInfoSub *sis, 
					sMP3ScaleFactorInfoSub *sfis, sMP3CriticalBandInfo *cbi);
void MidSideProc(int x[MP3_MAX_NCHAN][MP3_MAX_NSAMP], int nSamps, int mOut[2]);
void IntensityProcMPEG1(int x[MP3_MAX_NCHAN][MP3_MAX_NSAMP], int nSamps, sMP3FrameHeader *fh, sMP3ScaleFactorInfoSub *sfis, 
						sMP3CriticalBandInfo *cbi, int midSideFlag, int mixFlag, int mOut[2]);
void IntensityProcMPEG2(int x[MP3_MAX_NCHAN][MP3_MAX_NSAMP], int nSamps, sMP3FrameHeader *fh, sMP3ScaleFactorInfoSub *sfis, 
						sMP3CriticalBandInfo *cbi, sMP3ScaleFactorJS *sfjs, int midSideFlag, int mixFlag, int mOut[2]);

/* dct32.c */
// about 1 ms faster in RAM, but very large
void FDCT32(int *x, int *d, int offset, int oddBlock, int gb);// __attribute__ ((section (".data")));

/* hufftabs.c */
extern const HuffTabLookup huffTabLookup[HUFF_PAIRTABS];
extern const int huffTabOffset[HUFF_PAIRTABS];
extern const unsigned short huffTable[];
extern const unsigned char quadTable[64+16];
extern const int quadTabOffset[2];
extern const int quadTabMaxBits[2];

/* polyphase.c (or asmpoly.s)
 * some platforms require a C++ compile of all source files,
 * so if we're compiling C as C++ and using native assembly
 * for these functions we need to prevent C++ name mangling.
 */
#ifdef __cplusplus
extern "C" {
#endif
void PolyphaseMono(short *pcm, int *vbuf, const int *coefBase);
void PolyphaseStereo(short *pcm, int *vbuf, const int *coefBase);
#ifdef __cplusplus
}
#endif

/* trigtabs.c */
extern const int imdctWin[4][36];
extern const int ISFMpeg1[2][7];
extern const int ISFMpeg2[2][2][16];
extern const int ISFIIP[2][2];
extern const int csa[8][2];
extern const int coef32[31];
extern const int polyCoef[264];

#ifdef __cplusplus
}
#endif
#endif	/* _CODER_H */
