#include "../include.h"

// format: 4-bit paletted pixel graphics
// image width: 39 pixels
// image height: 16 lines
// image pitch: 20 bytes
const u16 Picopad1Img_Pal[16] __attribute__ ((aligned(4))) = {
	0xF81F, 0x2945, 0x0000, 0x39E8, 0x10C3, 0x1989, 0x3A6C, 0x4330, 0x95FD, 0xE71B, 0xB513, 0x41C6, 0x8C30, 0x5269, 0x6B2C, 0x0000, 
};

const u8 Picopad1Img[320] __attribute__ ((aligned(4))) = {
	0x07, 0x55, 0x55, 0x66, 0x66, 0x66, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x66, 
	0x66, 0x66, 0x37, 0x00, 0x81, 0xBB, 0xDB, 0x57, 0x66, 0x76, 0xBD, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 
	0xBB, 0xDD, 0xE1, 0xEA, 0xAA, 0xCA, 0xD1, 0xC0, 0x53, 0xEC, 0xE4, 0xE7, 0x66, 0x76, 0x1E, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xD4, 0x42, 0xB1, 0xBD, 0xCE, 0x15, 0x50, 0x2C, 0x99, 0xA2, 0x51, 
	0x24, 0x32, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0xEC, 0xEE, 0x42, 0x24, 0x22, 0x48, 
	0x1C, 0x9C, 0xB1, 0x24, 0xCC, 0xAB, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0xDE, 0xAA, 
	0x32, 0x2D, 0xD2, 0x4F, 0x99, 0x9D, 0xC9, 0xC4, 0x99, 0x9E, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x72, 0x22, 0x43, 0x22, 0x38, 0x8E, 0x2F, 0xEB, 0x21, 0x88, 0xA2, 0x2B, 0x9E, 0x50, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xA4, 0x11, 0xE9, 0xC2, 0x28, 0x8A, 0x28, 0x24, 0x32, 0xC9, 0x61, 
	0x31, 0xAC, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8C, 0xC1, 0xC8, 0x91, 0x26, 0xC4, 0x2F, 
	0x48, 0x89, 0x62, 0xA8, 0x8C, 0xAA, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0xAB, 0x18, 
	0xC1, 0x1D, 0x62, 0x4F, 0x49, 0x88, 0xD2, 0xC8, 0x8C, 0x4B, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xC1, 0xBA, 0xB4, 0x44, 0x18, 0x97, 0x2F, 0x26, 0x74, 0xC9, 0x33, 0xD4, 0xB2, 0x60, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xC2, 0x24, 0xD9, 0xC2, 0x28, 0x8A, 0x20, 0x4B, 0x11, 0x88, 0xA2, 
	0x2A, 0xA2, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x22, 0xA8, 0x84, 0x26, 0x83, 0x20, 
	0x1A, 0xCB, 0x88, 0xA2, 0xB9, 0x9D, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x22, 0x68, 
	0xA1, 0xBD, 0x22, 0x20, 0xB9, 0xC2, 0xB3, 0x21, 0xEC, 0xAB, 0x23, 0xE1, 0x6E, 0x11, 0x31, 0x33, 
	0x16, 0x12, 0x22, 0x21, 0x42, 0xEA, 0xE2, 0x2F, 0xEC, 0xDA, 0xE2, 0x4C, 0xBD, 0xD1, 0x41, 0x92, 
	0xBE, 0x21, 0xE2, 0xBE, 0x2B, 0xC2, 0xBE, 0x2C, 0xE2, 0xA9, 0xA4, 0x50, 0x0B, 0x1E, 0xD2, 0x14, 
	0x4C, 0xDD, 0xCE, 0xC1, 0x4E, 0x46, 0xC4, 0xBD, 0x4E, 0xE4, 0x3E, 0x4E, 0xE4, 0xDD, 0xD5, 0x00, 
};
