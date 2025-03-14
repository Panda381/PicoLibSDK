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
 * buffers.c - allocation and freeing of internal MP3 decoder buffers
 *
 * All memory allocation for the codec is done in this file, so if you don't want 
 *  to use other the default system malloc() and free() for heap management this is 
 *  the only file you'll need to change.
 **************************************************************************************/

#include "coder.h"

#ifndef _WIN32
#ifdef DEMO_HELIX_FOOTPRINT
#include "dv_debug_usart.h"
#endif
#endif

/**************************************************************************************
 * Function:    ClearBuffer
 *
 * Description: fill buffer with 0's
 *
 * Inputs:      pointer to buffer
 *              number of bytes to fill with 0
 *
 * Outputs:     cleared buffer
 *
 * Return:      none
 *
 * Notes:       slow, platform-independent equivalent to memset(buf, 0, nBytes)
 **************************************************************************************/
#ifndef MPDEC_ALLOCATOR
static void ClearBuffer(void *buf, int nBytes)
{
	int i;
	unsigned char *cbuf = (unsigned char *)buf;

	for (i = 0; i < nBytes; i++)
		cbuf[i] = 0;

	return;
}
#endif

/**************************************************************************************
 * Function:    AllocateBuffers
 *
 * Description: allocate all the memory needed for the MP3 decoder
 *
 * Inputs:      none
 *
 * Outputs:     none
 *
 * Return:      pointer to sMP3DecInfo structure (initialized with pointers to all 
 *                the internal buffers needed for decoding, all other members of 
 *                sMP3DecInfo structure set to 0)
 *
 * Notes:       if one or more mallocs fail, function frees any buffers already
 *                allocated before returning
 **************************************************************************************/

//  mp3stat ... pointer to static buffer for MP3 decoder info structure of size MP3DECODER_SIZE, or NULL to auto-allocate
// Returns MP3 handler, or NULL on error (pointer to the MP3 descriptor; requires about 24 KB of RAM).
// If static buffer is used, caller must free it after finalize.
// If auto-allocation is used, MP3FreeDecoder() must be called after finalize, to free allocated structures.

sMP3DecInfo *AllocateBuffers(void* mp3stat)
{
	sMP3DecInfo *mp3DecInfo;

	// statis buffer used
	if (mp3stat != NULL)
	{
		// clear structures
		memset(mp3stat, 0, MP3DECODER_SIZE);

		// setup pointers
		sMP3DecInfoStat* s = (sMP3DecInfoStat*)mp3stat;
		mp3DecInfo = &s->MP3DecInfoS;
		mp3DecInfo->FrameHeaderPS = &s->FrameHeaderS;
		mp3DecInfo->SideInfoPS = &s->SideInfoS;
		mp3DecInfo->ScaleFactorInfoPS = &s->ScaleFactorInfoS;
		mp3DecInfo->HuffmanInfoPS = &s->HuffmanInfoS;
		mp3DecInfo->DequantInfoPS = &s->DequantInfoS;
		mp3DecInfo->IMDCTInfoPS = &s->IMDCTInfoS;
		mp3DecInfo->SubbandInfoPS = &s->SubbandInfoS;
		mp3DecInfo->stat = True;

		return mp3DecInfo;
	}

#ifdef MPDEC_ALLOCATOR
	sMP3FrameHeader *fh;
	sMP3SideInfo *si;
	sMP3ScaleFactorInfo *sfi;
	sMP3HuffmanInfo *hi;
	sMP3DequantInfo *di;
	sMP3IMDCTInfo *mi;
	sMP3SubbandInfo *sbi;

	mp3DecInfo = (sMP3DecInfo *)MPDEC_ALLOCATOR(sizeof(sMP3DecInfo));
	if (!mp3DecInfo)
	{
#ifndef _WIN32
#ifdef DEMO_HELIX_FOOTPRINT
	  sprintf(COPY_DEBUG_BUFFER,"mp3DecInfo size: %d\n", (int)sizeof(sMP3DecInfo));
    DV_DEBUG_USART_Trace( COPY_DEBUG_BUFFER );
#endif
#endif
		return 0;
	}

	hi =  (sMP3HuffmanInfo *)     MPDEC_ALLOCATOR(sizeof(sMP3HuffmanInfo));
	sbi = (sMP3SubbandInfo *)     MPDEC_ALLOCATOR(sizeof(sMP3SubbandInfo));
	mi =  (sMP3IMDCTInfo *)       MPDEC_ALLOCATOR(sizeof(sMP3IMDCTInfo));
	di =  (sMP3DequantInfo *)     MPDEC_ALLOCATOR(sizeof(sMP3DequantInfo));
	si =  (sMP3SideInfo *)        MPDEC_ALLOCATOR(sizeof(sMP3SideInfo));
	sfi = (sMP3ScaleFactorInfo *) MPDEC_ALLOCATOR(sizeof(sMP3ScaleFactorInfo));
	fh =  (sMP3FrameHeader *)     MPDEC_ALLOCATOR(sizeof(sMP3FrameHeader));

	if (!fh || !si || !sfi || !hi || !di || !mi || !sbi) {
#ifndef _WIN32
#ifdef DEMO_HELIX_FOOTPRINT
    sprintf(COPY_DEBUG_BUFFER,"mp3DecInfo:%d[%d] | fh:%d[%d] | si:%d[%d] \
      | sfi:%d[%d] | hi:%d[%d] | di:%d[%d] | mi:%d[%d] | sbi:%d[%d]\n",
      (int)mp3DecInfo, (int)sizeof(sMP3DecInfo), (int)fh, (int)sizeof(sMP3FrameHeader),
      (int)si, (int)sizeof(sMP3SideInfo), (int)sfi, (int)sizeof(sMP3ScaleFactorInfo),
      (int)hi, (int)sizeof(sMP3HuffmanInfo), (int)di, (int)sizeof(sMP3DequantInfo),
      (int)mi, (int)sizeof(sMP3IMDCTInfo), (int)sbi, (int)sizeof(sMP3SubbandInfo) );
    DV_DEBUG_USART_Trace( COPY_DEBUG_BUFFER );
#endif
#endif
		FreeBuffers(mp3DecInfo);	// safe to call - only frees memory that was successfully allocated
		return 0;
	}
#else

	// Buffers:
	static char s_mp3DecInfo[sizeof(sMP3DecInfo)];
	static char fh[sizeof(sMP3FrameHeader)];
	static char si[sizeof(sMP3SideInfo)];
	static char sfi[sizeof(sMP3ScaleFactorInfo)];
	static char hi[sizeof(sMP3HuffmanInfo)];
	static char di[sizeof(sMP3DequantInfo)];
	static char mi[sizeof(sMP3IMDCTInfo)];
	static char sbi[sizeof(sMP3SubbandInfo)];

	mp3DecInfo = (sMP3DecInfo *)s_mp3DecInfo;
	ClearBuffer(mp3DecInfo, sizeof(sMP3DecInfo));

	/* important to do this - DSP primitives assume a bunch of state variables are 0 on first use */
	ClearBuffer(fh,  sizeof(sMP3FrameHeader));
	ClearBuffer(si,  sizeof(sMP3SideInfo));
	ClearBuffer(sfi, sizeof(sMP3ScaleFactorInfo));
	ClearBuffer(hi,  sizeof(sMP3HuffmanInfo));
	ClearBuffer(di,  sizeof(sMP3DequantInfo));
	ClearBuffer(mi,  sizeof(sMP3IMDCTInfo));
	ClearBuffer(sbi, sizeof(sMP3SubbandInfo));

#endif

	mp3DecInfo->FrameHeaderPS =     (void *)fh;
	mp3DecInfo->SideInfoPS =        (void *)si;
	mp3DecInfo->ScaleFactorInfoPS = (void *)sfi;
	mp3DecInfo->HuffmanInfoPS =     (void *)hi;
	mp3DecInfo->DequantInfoPS =     (void *)di;
	mp3DecInfo->IMDCTInfoPS =       (void *)mi;
	mp3DecInfo->SubbandInfoPS =     (void *)sbi;

#ifndef _WIN32
#ifdef DEMO_HELIX_FOOTPRINT
	sprintf(COPY_DEBUG_BUFFER, "Total decoder malloc size: %d\n",
					(int)(sizeof(sMP3DecInfo) + sizeof(sMP3FrameHeader) + sizeof(sMP3SideInfo)
					+ sizeof(sMP3ScaleFactorInfo) + sizeof(sMP3HuffmanInfo) + sizeof(sMP3DequantInfo)
					+ sizeof(sMP3IMDCTInfo) + sizeof(sMP3SubbandInfo)));
	DV_DEBUG_USART_Trace( COPY_DEBUG_BUFFER );
#endif
#endif

	return mp3DecInfo;
}

#ifdef MPDEC_FREE
#define SAFE_FREE(x)	{if (x)	MPDEC_FREE(x);	(x) = 0;}	/* helper macro */
#else
#define SAFE_FREE(x)    { (x) = 0; }
#endif

/**************************************************************************************
 * Function:    FreeBuffers
 *
 * Description: frees all the memory used by the MP3 decoder
 *
 * Inputs:      pointer to initialized sMP3DecInfo structure
 *
 * Outputs:     none
 *
 * Return:      none
 *
 * Notes:       safe to call even if some buffers were not allocated (uses SAFE_FREE)
 **************************************************************************************/
void FreeBuffers(sMP3DecInfo *mp3DecInfo)
{
	if (!mp3DecInfo || mp3DecInfo->stat)
		return;

	SAFE_FREE(mp3DecInfo->FrameHeaderPS);
	SAFE_FREE(mp3DecInfo->SideInfoPS);
	SAFE_FREE(mp3DecInfo->ScaleFactorInfoPS);
	SAFE_FREE(mp3DecInfo->HuffmanInfoPS);
	SAFE_FREE(mp3DecInfo->DequantInfoPS);
	SAFE_FREE(mp3DecInfo->IMDCTInfoPS);
	SAFE_FREE(mp3DecInfo->SubbandInfoPS);

	SAFE_FREE(mp3DecInfo);
}
