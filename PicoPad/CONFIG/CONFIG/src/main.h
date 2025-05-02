
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 16-bit pixel graphics
// image width: 32 pixels
// image height: 240 lines
// image pitch: 64 bytes
extern const u16 IconImg[7680] __attribute__ ((aligned(4)));

// sound format: Intel IMA ADPCM stereo 4-bit 32000Hz
// sound format = SNDFORM_ADPCM_S
// sound speed relative to 22050Hz = 1.45125f
//const u16 TestSnd_SampBlock = 1017; // number of samples per block
extern const u8 TestSnd[110472] __attribute__ ((aligned(4)));

#endif // _MAIN_H
