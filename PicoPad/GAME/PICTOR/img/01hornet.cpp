#include "../include.h"

// format: 8-bit paletted pixel graphics
// image width: 100 pixels
// image height: 43 lines
// image pitch: 100 bytes
const u16 HornetImg_Pal[256] __attribute__ ((aligned(4))) = {
	0xF81F, 0x7BAE, 0xBDB7, 0x8C10, 0x8C51, 0x83CF, 0xA4F4, 0xAD55, 0x9CB3, 0xB596, 0xBDD7, 0xB5B6, 0xA514, 0x0861, 0x6B6E, 0xAD76, 
	0xAD75, 0xBDD7, 0xB5B6, 0xB596, 0x0022, 0xBDF7, 0xB5B7, 0xC67A, 0xCEBA, 0xBDF7, 0xB5B6, 0xB5B6, 0xB618, 0xBE59, 0xBE38, 0xB5F7, 
	0xC679, 0xB617, 0xD6FB, 0xDF1B, 0xCEB9, 0xC638, 0xBDD7, 0xE75C, 0xF7BD, 0xF7FD, 0xF7DC, 0xFFFC, 0xEF7C, 0xFFFC, 0xDEFA, 0xAD74, 
	0xFFDD, 0xFFDC, 0xFFD2, 0xFFF9, 0xFFFC, 0xFFFD, 0xFFFE, 0xBDD7, 0xFFFB, 0xD696, 0xFFFC, 0xFFDB, 0xFFDB, 0xFFDC, 0xB548, 0xAD2C, 
	0xD697, 0xCE56, 0xEF5A, 0xE6D6, 0xFF0E, 0xDED7, 0xFFBB, 0xEF19, 0xC5F5, 0xFFBC, 0xF79A, 0xFFBA, 0xFF99, 0xE6F8, 0xDEB8, 0xC616, 
	0xF77B, 0xB4C9, 0x942B, 0xE693, 0xD654, 0xEF18, 0xCE14, 0xE6D7, 0xF759, 0xDE97, 0xBDF7, 0x8B86, 0x7B26, 0xE6B5, 0xEED6, 0xD634, 
	0xCDF3, 0xE695, 0xC5F4, 0xDE97, 0xEF39, 0xD656, 0xB573, 0xFD80, 0x8BCA, 0xCDD2, 0xDE54, 0xD633, 0xDE55, 0xEED7, 0xDE96, 0xD655, 
	0xBDB4, 0xD677, 0x5A02, 0x6A84, 0xC52B, 0x7329, 0xD5D1, 0xC571, 0xC592, 0xE675, 0xFF38, 0xE6B6, 0xCDF4, 0xAD11, 0xEEF8, 0xD635, 
	0xBD93, 0xE6B7, 0xCE15, 0xE6B8, 0xE6D9, 0x4181, 0xA3C5, 0xCD91, 0xD5F2, 0xDE34, 0xDE76, 0xC5F6, 0x59E1, 0xB406, 0x5A44, 0xDD6C, 
	0xC5B3, 0xCE16, 0x7AC5, 0xCDB2, 0xBD30, 0xBDB5, 0xBDD6, 0xC617, 0xB5B6, 0x4940, 0x7221, 0xBD52, 0xBB62, 0x5226, 0x942E, 0xC5B5, 
	0xFC41, 0xECE8, 0x1881, 0x93CB, 0xACF1, 0xD636, 0x8A82, 0x9C90, 0xBD74, 0xDE78, 0xBDD7, 0xBDD7, 0xCB20, 0x59C3, 0x4983, 0x6287, 
	0xAC6E, 0xC552, 0xCDB4, 0x79C0, 0xAD13, 0x3943, 0x72A7, 0x938A, 0x834A, 0x838C, 0x7B8D, 0xBA60, 0x1040, 0x2080, 0x4942, 0x0840, 
	0x6AC9, 0xDE37, 0x630C, 0x5940, 0x38E1, 0xE678, 0x30A0, 0x40E0, 0x20C2, 0xABCB, 0xB533, 0x9180, 0x48C0, 0x1061, 0x6227, 0x8BAD, 
	0x39A6, 0x734C, 0xB554, 0xBDB6, 0x1840, 0x30A1, 0x8AE8, 0x3880, 0xA2A6, 0xB596, 0x68E0, 0x50C0, 0x9A65, 0xA308, 0x2040, 0xA44F, 
	0xAD13, 0x8983, 0x93EE, 0xA4D2, 0xBDD7, 0x1820, 0x4A08, 0x5269, 0x9430, 0x69C6, 0x834C, 0x5124, 0x3144, 0x5840, 0x1861, 0x4020, 
	0x3000, 0x38C2, 0xA800, 0x2000, 0x1000, 0x0800, 0x0000, 0x2800, 0xB575, 0xB596, 0xB5B6, 0xBDF7, 0xB5B6, 0xB5B6, 0x0841, 0x0000, 
};

const u8 HornetImg[4300] __attribute__ ((aligned(4))) = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xC2, 0x98, 0xAB, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0x01, 0xE7, 0x75, 0x7C, 0x62, 0x95, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xB5, 0x52, 0x79, 
	0x7A, 0x6D, 0x6E, 0x6B, 0x8B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xAD, 0x5D, 0x88, 0x79, 0x57, 0x6F, 0x7F, 0x6F, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0xBE, 0x53, 0x88, 0x6C, 0x6D, 0x8A, 0x8A, 0x7F, 0x78, 0x5A, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xB9, 0x25, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xAD, 0x33, 0x77, 0x79, 0x55, 0x6E, 0x5F, 0x60, 0x69, 0x90, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9E, 0x2E, 0x25, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xC7, 0x38, 0x6B, 0x79, 0x4C, 0x7B, 0x5F, 
	0x88, 0x6C, 0x60, 0x97, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xE3, 0x9E, 0x23, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDE, 0x4B, 0x61, 0x88, 
	0x4B, 0x6D, 0x6C, 0x7B, 0x7B, 0x77, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xA3, 0x40, 0x1D, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 
	0x7B, 0x47, 0x93, 0x3B, 0x55, 0x7E, 0x58, 0x89, 0x94, 0x78, 0xFB, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x95, 0x25, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x07, 0x06, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB4, 0xB0, 0x23, 0x25, 0x11, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xE6, 0x94, 0x3C, 0x77, 0x3B, 0x4C, 0x4B, 0x7E, 0x69, 0x94, 0x77, 0x97, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB4, 0x70, 0x18, 
	0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x06, 0xD1, 0xC0, 0x75, 0x75, 0x9D, 0xB4, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB9, 0x63, 0x20, 0x1E, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0xB8, 0x35, 0x77, 0x3B, 0x34, 0x4A, 0x79, 0x6B, 0x89, 0x83, 
	0x56, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xAB, 0xCF, 0x24, 0x21, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0xE6, 0xAF, 0x77, 0x76, 0x6A, 0x55, 0x7A, 0x5E, 0x69, 0x98, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xA3, 
	0x2A, 0x1E, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xAE, 0x38, 0x78, 0x3A, 0x36, 0x57, 0x6B, 
	0x5D, 0x64, 0x4E, 0x48, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9E, 0x9B, 0x18, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0xE6, 0xB8, 0x5E, 0x53, 0x76, 0x69, 0x79, 0x6D, 0x63, 0x65, 
	0x57, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xB9, 0x5D, 0x23, 0x1F, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB5, 0x89, 0x5F, 0x3D, 
	0x30, 0x56, 0x54, 0x4C, 0x4A, 0x40, 0x4F, 0x66, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFA, 0x52, 0x2E, 0x1E, 0x1E, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0xE7, 0xCE, 0x6D, 0x3B, 0x69, 0x93, 0x6D, 0x55, 
	0x7B, 0x6E, 0x8A, 0x65, 0x6F, 0x56, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xF9, 0xB7, 0x34, 0x22, 0x1E, 0x25, 0x00, 0x00, 0x00, 0x00, 0xC2, 
	0xB8, 0x6E, 0x50, 0x2E, 0x7C, 0x55, 0x35, 0x58, 0x8A, 0x82, 0x66, 0x91, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9E, 
	0x5F, 0x23, 0x1F, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xB5, 0x90, 0x35, 0x54, 0x87, 
	0x5E, 0x4B, 0x7E, 0x79, 0x5F, 0x7C, 0x7C, 0x60, 0x78, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x94, 0x30, 0x24, 0x1C, 0x11, 0x00, 
	0x00, 0x00, 0x0C, 0xF1, 0xC1, 0x42, 0x59, 0x7F, 0x3C, 0x3A, 0x58, 0x57, 0x41, 0x66, 0x9F, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xF8, 0xA3, 0x2B, 0x24, 0x1F, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0xD0, 0xB9, 0x46, 
	0x83, 0x77, 0x79, 0x3B, 0x4C, 0x6D, 0x61, 0x6C, 0x89, 0x89, 0x5F, 0x78, 0x8B, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB8, 0x7E, 0x2C, 
	0x20, 0x1C, 0x00, 0x00, 0x00, 0x11, 0xFE, 0xCF, 0x50, 0x6E, 0x47, 0x36, 0x38, 0x58, 0x47, 0x63, 
	0x48, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB8, 0x5D, 0x27, 0x20, 0x21, 0x19, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xD0, 0xB2, 0x64, 0x90, 0x5F, 0x3B, 0x38, 0x4C, 0x4C, 0x55, 0x7E, 0x7B, 0x89, 0x69, 0x78, 0x97, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xF8, 0xA3, 0x35, 0x27, 0x1C, 0x1E, 0x00, 0x00, 0x00, 0xC2, 0xB8, 0x31, 0x57, 0x3D, 0x3C, 0x4D, 
	0x58, 0x46, 0x45, 0x70, 0x66, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xB7, 0x35, 0x27, 
	0x1D, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x16, 0xE6, 0xC0, 0x8A, 0x65, 0x6F, 0x35, 0x36, 0x46, 0x55, 0x6D, 0x61, 0x6A, 0x87, 0x94, 
	0x78, 0x80, 0xE4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x9B, 0x36, 0x23, 0x1F, 0x15, 0x00, 0x00, 0x10, 0xB5, 0x50, 0x45, 
	0x84, 0x71, 0x63, 0x83, 0x7C, 0x80, 0x48, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xEA, 0x69, 0x36, 0x23, 0x1F, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0C, 0xFE, 0xCF, 0x64, 0x4E, 0x50, 0x49, 0x83, 0x65, 0x6F, 0x61, 0x5D, 
	0x61, 0x6C, 0x6F, 0x7C, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD1, 0x8A, 0x2C, 0x24, 0x1C, 0x00, 0x00, 0x00, 
	0xD0, 0xA4, 0x4E, 0x4D, 0x63, 0x46, 0x35, 0x3C, 0x47, 0xA7, 0xE3, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0xB6, 0x4A, 0x28, 0x22, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xC8, 0xA7, 0xC5, 0xA5, 0x84, 0x65, 0x65, 0x63, 
	0x55, 0x4C, 0x4B, 0x58, 0x45, 0x40, 0x41, 0x62, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD1, 0x63, 0x27, 0x1D, 
	0x15, 0x00, 0x00, 0x04, 0xE6, 0x84, 0x7F, 0x4D, 0x3A, 0x55, 0x57, 0x39, 0x9B, 0xD9, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x03, 0xB0, 0x36, 0x27, 0x1D, 0x15, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0xE6, 0xE7, 0x83, 0x42, 0x47, 0x4D, 
	0x64, 0x46, 0x34, 0x34, 0x4B, 0x64, 0x63, 0x65, 0x41, 0x48, 0x80, 0x5A, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0xDC, 0x5A, 0x00, 0x00, 0x00, 0x00, 
	0x05, 0x4E, 0x24, 0x1C, 0x00, 0x00, 0x0A, 0xEC, 0xA8, 0xB1, 0x4A, 0x35, 0x46, 0x43, 0x52, 0xB4, 
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0xD8, 0x19, 0x00, 0xCE, 
	0x6D, 0x30, 0x22, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0xEC, 0xBA, 0x50, 
	0x91, 0x65, 0x42, 0x50, 0x58, 0x3A, 0x38, 0x58, 0x47, 0x63, 0x41, 0x48, 0x66, 0x95, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEA, 0xCB, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x71, 0x20, 0x11, 0x00, 0x00, 0xC2, 0xC2, 0x48, 0x35, 0x35, 0x4A, 
	0x56, 0xA4, 0xD9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xEA, 0xCB, 0x00, 0x09, 0xB5, 0x7E, 0x2C, 0x18, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 
	0xCD, 0xA7, 0xA9, 0x82, 0x84, 0x4D, 0x4D, 0x83, 0x81, 0x56, 0x47, 0x64, 0x59, 0x41, 0x9F, 0x70, 
	0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xE9, 0xDC, 0x00, 0x00, 0x00, 0x01, 0x25, 0x18, 0x0A, 0x18, 0x04, 0xE6, 0x70, 
	0x42, 0x36, 0x42, 0x9F, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xE9, 0xD8, 0x00, 0x0F, 0xD0, 0x82, 0x23, 0x22, 0x25, 0x17, 0x16, 
	0xF8, 0xFA, 0x08, 0xD0, 0xCA, 0xA5, 0x84, 0x47, 0x4A, 0x46, 0x34, 0x35, 0x3A, 0x65, 0x7D, 0x41, 
	0x70, 0x66, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE9, 0xC9, 0x00, 0x00, 0x01, 0xE4, 0xE4, 0xEA, 
	0xA6, 0xF6, 0xD7, 0xE1, 0xE9, 0xC0, 0xE4, 0x16, 0xAB, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE9, 0xC9, 0x00, 0x16, 0x0E, 0xE0, 
	0x95, 0xD1, 0xA6, 0xCC, 0xD5, 0xCD, 0xD4, 0xBA, 0x48, 0x24, 0x7A, 0x38, 0x29, 0x2D, 0x69, 0x56, 
	0x41, 0x4F, 0xA4, 0xE3, 0xD9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE2, 0xCA, 0x11, 
	0xB9, 0x86, 0x3E, 0xF0, 0xDA, 0xBC, 0xEF, 0xEF, 0xBB, 0xF0, 0xEB, 0x0D, 0xF7, 0xA0, 0x8F, 0x09, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE2, 
	0xCA, 0x26, 0xE3, 0xA6, 0x3E, 0xF4, 0xBB, 0xAC, 0xD4, 0xF5, 0xED, 0xEF, 0xD6, 0xE7, 0xCE, 0xA1, 
	0x8F, 0x9B, 0x2F, 0xB4, 0xD2, 0xD3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x11, 0xDF, 0xDB, 0x3E, 0x33, 0x8E, 0xFF, 0xE5, 0xC3, 0xDA, 0x67, 0xB3, 0xF3, 0xC6, 
	0xFF, 0xB3, 0xAC, 0xCB, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x11, 0xDF, 0xDB, 0x3E, 0x33, 0x9D, 0xF0, 0xCC, 0xFF, 0xCC, 0xCB, 0xDA, 
	0xEF, 0xB5, 0x14, 0xC3, 0xBB, 0xCB, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8D, 0xED, 0xE1, 0x33, 0x52, 0xFF, 0xF4, 0xA6, 0x9C, 
	0x9A, 0xF5, 0xC4, 0xC4, 0xF6, 0xF0, 0xAC, 0xAC, 0xF4, 0xE6, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8D, 0xED, 0xE1, 0x33, 0x52, 0xFF, 0xFF, 
	0xC3, 0xA6, 0x9A, 0xF6, 0xC4, 0xC6, 0xFF, 0xEF, 0xAC, 0xAC, 0xF4, 0xE7, 0x19, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xF9, 0xFA, 0x0A, 0x95, 0x3E, 0xED, 0xF2, 0x44, 0x3F, 
	0xF5, 0xF3, 0xFF, 0xA2, 0xA2, 0x75, 0x8E, 0xC7, 0xF6, 0xDA, 0x67, 0x67, 0xCB, 0xFF, 0xD2, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x0F, 0xFA, 0x0A, 0x95, 0x3E, 0xED, 0xF2, 
	0x44, 0x3F, 0xF5, 0xF3, 0xF5, 0xBD, 0xC8, 0x75, 0x8E, 0xC4, 0xF6, 0xDA, 0x67, 0x67, 0xCB, 0xFF, 
	0xD2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xE8, 0x04, 0x0C, 0xF9, 0xD9, 0xE3, 0xB5, 
	0xF7, 0xEF, 0x8D, 0x32, 0xAE, 0xDE, 0x85, 0xC4, 0x74, 0x3F, 0xC4, 0xBD, 0xFF, 0xDB, 0xA0, 0x67, 
	0xBB, 0xF5, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0xE8, 0x04, 0x0C, 0xF9, 0xD9, 
	0xE3, 0xB5, 0xF7, 0xEF, 0x8D, 0x32, 0xAE, 0xDE, 0x85, 0xC4, 0x74, 0x3F, 0xC4, 0xBD, 0xFF, 0xDB, 
	0xA0, 0x67, 0xBB, 0xF5, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0xD1, 0xE3, 0xAB, 0x00, 
	0x00, 0x00, 0x00, 0xB6, 0xE5, 0xFF, 0x92, 0x32, 0x51, 0xF5, 0x8D, 0x8F, 0x74, 0xD5, 0xE5, 0xFF, 
	0xFF, 0xD7, 0xBB, 0xAC, 0xC3, 0xF5, 0xCB, 0xB0, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0xD1, 0xE3, 
	0xAB, 0x00, 0x00, 0x00, 0x00, 0xB6, 0xE5, 0xFF, 0x92, 0x32, 0x51, 0xF5, 0x8D, 0x8F, 0x74, 0xD5, 
	0xE5, 0xFF, 0xFF, 0xD7, 0xBB, 0xAC, 0xC3, 0xF5, 0xCB, 0xB0, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0xE2, 
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0x99, 0x85, 0x5C, 0x74, 0x51, 0xB9, 0xB6, 0x86, 
	0x51, 0x8C, 0xAD, 0xFF, 0xFF, 0xDE, 0xCC, 0xC7, 0xFF, 0xFF, 0xDA, 0xFF, 0x10, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x37, 0xE2, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0x99, 0x85, 0x5C, 0x74, 0x51, 0xB9, 
	0xB6, 0x86, 0x51, 0x8C, 0xAD, 0xFF, 0xFF, 0xDE, 0xCC, 0xC7, 0xFF, 0xFF, 0xDA, 0xFF, 0x10, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xEA, 0xE4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9E, 0x8C, 0x72, 0x73, 
	0x92, 0x37, 0x00, 0xC7, 0x8E, 0x60, 0x51, 0x5C, 0xAD, 0xCB, 0xC6, 0xFF, 0xFF, 0xCC, 0xC0, 0x06, 
	0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xE2, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9E, 0x8C, 
	0x72, 0x73, 0x92, 0x37, 0x00, 0xC7, 0x8E, 0x60, 0x51, 0x5C, 0xAD, 0xCB, 0xC6, 0xFF, 0xFF, 0xCC, 
	0xC0, 0x0C, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xE0, 0xB9, 0xAF, 0xCA, 0x00, 0x00, 0xD3, 0xB8, 0xAA, 0xBA, 0x68, 0x5B, 0xB3, 0xCC, 0xCD, 
	0xA3, 0xB1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xE0, 0xB9, 0xAF, 0xCA, 0x00, 0x00, 0xD3, 0xB8, 0xAA, 0xBA, 0x68, 0x5B, 0xB3, 
	0xCC, 0xCD, 0xA3, 0xB1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xF9, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA8, 0xBD, 0x08, 0x26, 
	0x13, 0xFF, 0xFF, 0xF3, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF9, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA8, 0xBD, 
	0x08, 0x26, 0x13, 0xFF, 0xFF, 0xF3, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF6, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x06, 0xF1, 0xD0, 0xE7, 0x02, 0x26, 0x01, 0xEC, 0xFF, 0x9D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF6, 0x08, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x06, 0xF1, 0xD0, 0xE7, 0xD3, 0x26, 0x01, 0xEC, 0xFF, 0x9D, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xE5, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xF8, 0xF3, 0xE4, 0x00, 0xEE, 0x97, 0x00, 0x00, 0xF8, 0xD9, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
	0xE5, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xF3, 0xE4, 0x00, 0xEE, 0x97, 0x00, 0x00, 0xF8, 0xD9, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xA8, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0xB5, 0x00, 0x00, 0x96, 0x25, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xA8, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0xB5, 0x00, 0x00, 0x96, 
	0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0xE8, 0x11, 0x00, 0x00, 0x00, 0x00, 0x11, 0xBF, 0xA8, 
	0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0xE8, 0x11, 0x00, 0x00, 0x00, 0x00, 0x11, 
	0xBF, 0xA8, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x1A, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0xFC, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD1, 0x08, 0xFB, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD1, 0x08, 0xFB, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x0A, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 
	0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};