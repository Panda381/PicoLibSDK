#include "include.h"

// format: 8-bit pixel graphics
// image width: 192 pixels
// image height: 20 lines
// image pitch: 192 bytes
const u8 FlagsImg[3840] __attribute__ ((aligned(4))) = {
	0x68, 0x44, 0x86, 0x86, 0x86, 0x7B, 0x7B, 0x7B, 0x7B, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x7B, 0x7B, 0x76, 0x76, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x7B, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0xFA, 0xFA, 0xFA, 0xFA, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0xFA, 0xFA, 0xD6, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0xFA, 0x86, 0x86, 
	0x6D, 0x29, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x76, 0x76, 0x76, 0x76, 0x76, 0x7B, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x7B, 0x86, 0x86, 0x6D, 0x29, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x7B, 0x7B, 0x00, 0x00, 0x7B, 0x00, 0x00, 0x7B, 0x76, 0x76, 0x76, 0x76, 0x86, 0x86, 0x86, 0x86, 
	0x6D, 0x29, 0x6D, 0x52, 0x52, 0x86, 0x86, 0x86, 0x86, 0x86, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x7B, 0x7B, 0x76, 0x86, 0x86, 0x52, 0x86, 0x86, 0x6D, 0x29, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x76, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x7B, 0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 
	0x6D, 0x49, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xD6, 0xD6, 0xD6, 0xD6, 0xD6, 0xFA, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0xFA, 0x86, 0x86, 0x6D, 0x49, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0xFA, 0xFA, 0x64, 0x64, 0xFA, 0x64, 0x44, 0xFA, 0xD6, 0xD6, 0xD6, 0xD6, 0x86, 0x86, 0x86, 0x86, 
	0x6D, 0x49, 0x6D, 0x92, 0x92, 0x86, 0x86, 0x86, 0x86, 0x86, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 
	0xFA, 0xFA, 0xD6, 0x86, 0x86, 0x92, 0x86, 0x86, 0x6D, 0x49, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xD6, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0xFA, 0xFA, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x76, 0x00, 0x00, 0x76, 0x00, 0x00, 0x76, 
	0x7B, 0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x7B, 0x7B, 0x00, 0x7B, 0x7B, 0x00, 0x7B, 0x76, 0x7B, 0x76, 0x76, 0x86, 0x86, 0x7B, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x76, 0x52, 0x76, 0x76, 0x76, 0x7B, 0x00, 0x00, 0x7B, 0x00, 0x00, 0x7B, 
	0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x76, 0x7B, 
	0x76, 0x76, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xD6, 0x44, 0x44, 0xD6, 0x44, 0x44, 0xD6, 
	0xFA, 0xFA, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0xFA, 0xFA, 0x64, 0xFA, 0xFA, 0x64, 0xFA, 0xD6, 0xFA, 0xD6, 0xD6, 0x86, 0x86, 0xFA, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0xD6, 0x92, 0xD6, 0xD6, 0xD6, 0xFA, 0x64, 0x64, 0xFA, 0x64, 0x64, 0xFA, 
	0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xD6, 0xFA, 
	0xD6, 0xD6, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x00, 0x00, 0x76, 0x76, 0x00, 0x76, 0x7B, 
	0x76, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x76, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x52, 0x76, 0x76, 0x76, 0x7B, 0x76, 0x00, 0x7B, 0x7B, 0x00, 0x7B, 0x7B, 
	0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x76, 
	0x76, 0x76, 0x00, 0x00, 0x7B, 0x00, 0x00, 0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x64, 0x44, 0xD6, 0xD6, 0x44, 0xD6, 0xFA, 
	0xD6, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0xFA, 0x64, 0x64, 0x64, 0x64, 0x64, 0x44, 0xFA, 0xD6, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0x92, 0xD6, 0xD6, 0xD6, 0xFA, 0xD6, 0x64, 0xFA, 0xFA, 0x64, 0xFA, 0xFA, 
	0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xD6, 
	0xD6, 0xD6, 0x64, 0x64, 0xFA, 0x64, 0x64, 0xFA, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 
	0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x76, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x76, 0x52, 0x76, 0x76, 0x76, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 
	0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x76, 0x7B, 
	0x76, 0x76, 0x00, 0x7B, 0x7B, 0x00, 0x7B, 0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x64, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0xD6, 
	0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0x64, 0x64, 0xFF, 0x64, 0x64, 0xFF, 0x64, 0x44, 0xD6, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0xD6, 0x92, 0xD6, 0xD6, 0xD6, 0xFA, 0x64, 0x64, 0x64, 0x64, 0x64, 0xFA, 
	0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xD6, 0xFA, 
	0xD6, 0xD6, 0x44, 0xFA, 0xFA, 0x64, 0xFA, 0xFA, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x00, 0x00, 0xB6, 0x00, 0x00, 0xB6, 0x00, 0x00, 
	0x76, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x52, 0x76, 0x76, 0x76, 0x7B, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 
	0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x76, 
	0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x64, 0x64, 0xB6, 0x44, 0x44, 0xB6, 0x44, 0x64, 
	0xD6, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0x64, 0x64, 0xFF, 0x64, 0x64, 0xFF, 0x44, 0x64, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0x92, 0xD6, 0xD6, 0xD6, 0xFA, 0x44, 0xFF, 0x64, 0x64, 0xFF, 0x64, 0x64, 
	0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xD6, 
	0xD6, 0x44, 0x64, 0x64, 0x64, 0x64, 0x64, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x00, 0x00, 0xB6, 0x00, 0x00, 0xB6, 0x00, 0x00, 
	0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x76, 0x52, 0x76, 0x76, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 
	0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x76, 0x7B, 
	0x00, 0x00, 0xB6, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x64, 0x44, 0xB6, 0x44, 0x44, 0xB6, 0x44, 0x44, 
	0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x44, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0xD6, 0x92, 0xD6, 0xD6, 0x44, 0x64, 0xFF, 0x64, 0x64, 0xFF, 0x64, 0x64, 
	0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xD6, 0xFA, 
	0x44, 0x44, 0xB6, 0x64, 0x64, 0xFF, 0x64, 0x64, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x52, 0x76, 0x76, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x76, 
	0x00, 0x00, 0xB6, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x64, 0x64, 0x44, 0x44, 0x44, 0x44, 0x44, 0x64, 
	0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0x64, 0x64, 0xFF, 0x64, 0x64, 0xFF, 0x44, 0x64, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0x92, 0xD6, 0xD6, 0xD6, 0x64, 0x44, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 
	0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xD6, 
	0x44, 0x44, 0xB6, 0x64, 0x64, 0xFF, 0x64, 0x64, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x00, 0x00, 0xB6, 0x00, 0x00, 0xB6, 0x00, 0x00, 
	0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x76, 0x52, 0x76, 0x76, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 
	0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x76, 0x7B, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x64, 0x44, 0xB6, 0x44, 0x44, 0xB6, 0x44, 0x44, 
	0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0x64, 0x64, 0x64, 0xFF, 0xFF, 0x64, 0x64, 0x44, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0xD6, 0x92, 0xD6, 0xD6, 0x44, 0x64, 0xFF, 0x64, 0x64, 0xFF, 0x64, 0x64, 
	0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xD6, 0xFA, 
	0x44, 0x44, 0x44, 0x64, 0x64, 0x64, 0x64, 0x64, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x00, 0x00, 0xB6, 0xB6, 0x00, 0x00, 0x00, 
	0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x76, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x52, 0x76, 0x76, 0x76, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 
	0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x76, 
	0x00, 0x00, 0xB6, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x64, 0x44, 0xB6, 0xB6, 0x44, 0x44, 0x64, 
	0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0xFA, 0x64, 0x64, 0x64, 0x64, 0x64, 0x44, 0xD6, 0xD6, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0x92, 0xD6, 0xD6, 0xD6, 0x64, 0x44, 0x64, 0xFF, 0xFF, 0x64, 0x64, 0x64, 
	0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xD6, 
	0x44, 0x44, 0xB6, 0x64, 0x64, 0xFF, 0x64, 0x64, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 
	0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x7B, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x76, 0x7B, 0x76, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x76, 0x52, 0x76, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 
	0x7B, 0x7B, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x76, 0x7B, 
	0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xD6, 0x44, 0x44, 0x44, 0x44, 0x44, 0xD6, 
	0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0xFA, 0xFA, 0x64, 0x64, 0x64, 0x64, 0xD6, 0xFA, 0xD6, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0xD6, 0x92, 0xD6, 0xD6, 0x44, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0xFA, 
	0xFA, 0xFA, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xD6, 0xFA, 
	0x44, 0x44, 0x44, 0xFF, 0xFF, 0x64, 0x64, 0x64, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x52, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x76, 0x7B, 
	0x76, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x76, 0x76, 0x76, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x52, 0x52, 0x76, 0x76, 0x76, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x7B, 
	0x7B, 0x7B, 0x76, 0x76, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x76, 
	0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x7B, 0x86, 0x86, 
	0x68, 0x44, 0x92, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x44, 0x44, 0x44, 0x44, 0xD6, 0xFA, 
	0xD6, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xD6, 0xD6, 0xD6, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x6D, 0x92, 0x92, 0xD6, 0xD6, 0xD6, 0xFA, 0x44, 0x64, 0x64, 0x64, 0x64, 0xFA, 0xFA, 
	0xFA, 0xFA, 0xD6, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xD6, 
	0xD6, 0x44, 0x64, 0x64, 0x64, 0x64, 0x64, 0xFA, 0xFA, 0xFA, 0xFA, 0x86, 0x86, 0xFA, 0x86, 0x86, 
	0x6D, 0x29, 0x52, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x76, 0x76, 0x76, 0x76, 0x7B, 0x76, 
	0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x7B, 0x86, 0x86, 0x6D, 0x29, 0x52, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x76, 0x76, 0x76, 0x76, 0x86, 0x86, 0x86, 0x86, 
	0x6D, 0x29, 0x6D, 0x52, 0x76, 0x52, 0x76, 0x76, 0x76, 0x7B, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x52, 0x86, 0x86, 0x6D, 0x29, 0x52, 0x76, 0x7B, 0x7B, 0x76, 0x7B, 
	0x76, 0x76, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 
	0x6D, 0x49, 0x92, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0xD6, 0xD6, 0xD6, 0xD6, 0xFA, 0xD6, 
	0xFA, 0xFA, 0xFA, 0x86, 0x86, 0xFA, 0x86, 0x86, 0x6D, 0x49, 0x92, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0xD6, 0xD6, 0xD6, 0xD6, 0x86, 0x86, 0x86, 0x86, 
	0x6D, 0x49, 0x6D, 0x92, 0xD6, 0x92, 0xD6, 0xD6, 0xD6, 0xFA, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x92, 0x86, 0x86, 0x6D, 0x49, 0x92, 0xD6, 0xFA, 0xFA, 0xD6, 0xFA, 
	0xD6, 0xD6, 0x44, 0x64, 0x64, 0x64, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x76, 0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x7B, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x76, 0x76, 0x76, 0x7B, 0x76, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x76, 0x76, 0x76, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0xD6, 0xFA, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0xFA, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0xD6, 0xD6, 0xD6, 0xFA, 0xD6, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0xD6, 0xD6, 0xD6, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x68, 0x44, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
	0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 
};
