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
 * mp3common.h - implementation-independent API's, datatypes, and definitions
 **************************************************************************************/

#ifndef _MP3COMMON_H
#define _MP3COMMON_H

#include "statname.h"	/* do name-mangling for static linking */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && !defined(_WIN32_WCE)
#
#elif defined(_WIN32) && defined(_WIN32_WCE) && defined(ARM)
#
#elif defined(_WIN32) && defined(WINCE_EMULATOR)
#
#elif defined(ARM_ADS)
#
#elif defined(_SYMBIAN) && defined(__WINS__)	/* Symbian emulator for Ix86 */
#
#elif defined(__GNUC__) && defined(ARM)
#
#elif defined(__GNUC__) && defined(__ARMEL__)
#
#elif defined(__GNUC__) && defined(__i386__)
#
#elif defined(__GNUC__) && defined(__amd64__)
#
#elif defined(_OPENWAVE_SIMULATOR) || defined(_OPENWAVE_ARMULATOR)
#
#elif defined(__GNUC__) && defined(__AVR32_UC__)
#
#elif defined(__CORTEX_M) && __CORTEX_M == 0x04U
#
#elif defined(__MK66FX1M0__) || defined(__MK64FX512__) || defined(__MK20DX256__)	/* teensy 3.6, 3.5, or 3.1/2 */
#
#elif defined(MP3DEC_GENERIC)
#
#else
#error No platform defined. See valid options in mp3dec.h
#endif

//#define MP3_MAX_SCFBD	4		/* max scalefactor bands per channel */
#define NGRANS_MPEG1	2
#define NGRANS_MPEG2	1

/* 11-bit syncword if MPEG 2.5 extensions are enabled */
#define	SYNCWORDH		0xff
#define	SYNCWORDL		0xe0

/* 12-bit syncword if MPEG 1,2 only are supported 
 * #define	SYNCWORDH		0xff
 * #define	SYNCWORDL		0xf0
 */

/* decoder functions which must be implemented for each platform */
sMP3DecInfo *AllocateBuffers(void* mp3stat);
void FreeBuffers(sMP3DecInfo *mp3DecInfo);
int CheckPadBit(sMP3DecInfo *mp3DecInfo);
int UnpackFrameHeader(sMP3DecInfo *mp3DecInfo, const u8 *buf);
int UnpackSideInfo(sMP3DecInfo *mp3DecInfo, const u8 *buf);
int DecodeHuffman(sMP3DecInfo *mp3DecInfo, const u8 *buf, int *bitOffset, int huffBlockBits, int gr, int ch);
int Dequantize(sMP3DecInfo *mp3DecInfo, int gr);
int IMDCT(sMP3DecInfo *mp3DecInfo, int gr, int ch);
int UnpackScaleFactors(sMP3DecInfo *mp3DecInfo, const u8 *buf, int *bitOffset, int bitsAvail, int gr, int ch);
int Subband(sMP3DecInfo *mp3DecInfo, short *pcmBuf);

/* mp3tabs.c - global ROM tables */
extern const int samplerateTab[3][3];
extern const short bitrateTab[3][3][15];
extern const short samplesPerFrameTab[3][3];
extern const short bitsPerSlotTab[3];
extern const short sideBytesTab[3][2];
extern const short slotTab[3][3][15];
extern const sMP3SFBandTable sfBandTable[3][3];

#ifdef __cplusplus
}
#endif

#endif	/* _MP3COMMON_H */
