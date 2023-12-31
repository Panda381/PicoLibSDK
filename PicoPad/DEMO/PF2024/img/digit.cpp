#include "../include.h"

// format: 4-bit paletted pixel graphics
// image width: 200 pixels
// image height: 24 lines
// image pitch: 100 bytes
const u16 DigitImg_Pal[16] __attribute__ ((aligned(4))) = {
	0x2900, 0x7AC0, 0x6260, 0x51E0, 0x4180, 0x3140, 0xFDA0, 0xF560, 0xED40, 0xDCE0, 0xC460, 0xB400, 0xABC0, 0x9340, 0x28E0, 0x0000, 
};

const u8 DigitImg[2400] __attribute__ ((aligned(4))) = {
	0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x1C, 0xCD, 0x3E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xE4, 0x2E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x52, 0xDC, 0xC1, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0x3D, 0xCC, 0xD2, 0x0E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE1, 0x76, 0x2E, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x21, 0x34, 0x44, 0x31, 0xC9, 0x5E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xE0, 0x3D, 0xCC, 0xD3, 0xEE, 0xEE, 0xEE, 0xEE, 0x21, 0x44, 0x44, 0x44, 0x44, 0x44, 0x40, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xE5, 0x1C, 0xCC, 0xD3, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE5, 0x2C, 0xCC, 
	0x25, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0xA6, 0x66, 0x66, 0x6C, 0x0E, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xE5, 0xD8, 0x1E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x01, 0x86, 0x66, 0x66, 0x81, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEC, 0x66, 0x66, 0x66, 0xA4, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEC, 
	0x67, 0x5E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xB6, 0x66, 0x66, 0x66, 0x6D, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0x29, 0x66, 0x66, 0x66, 0xDE, 0xEE, 0xEE, 0xEE, 0xA6, 0x66, 0x66, 0x66, 
	0x66, 0x66, 0x75, 0xEE, 0xEE, 0xEE, 0xEE, 0x28, 0x69, 0xC9, 0x66, 0xC0, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0x28, 0x6A, 0xC8, 0x69, 0x3E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x28, 0x6C, 0x34, 0x19, 0x66, 
	0xDE, 0xEE, 0xEE, 0xEE, 0xEE, 0x20, 0x03, 0xC7, 0x66, 0x3E, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0xC6, 
	0x8D, 0x11, 0xA6, 0x68, 0x5E, 0xEE, 0xEE, 0xEE, 0xEE, 0x38, 0xD3, 0x42, 0x96, 0x6A, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0x09, 0x6C, 0xEE, 0xDC, 0x1E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x96, 0x66, 0x66, 
	0x66, 0xC0, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xED, 0x66, 0x81, 0x44, 0x2D, 0xAE, 0xEE, 0xEE, 0xEE, 
	0x96, 0x66, 0x66, 0x66, 0x66, 0x66, 0x9E, 0xEE, 0xEE, 0xEE, 0xE2, 0x66, 0xDE, 0xE0, 0xD6, 0x6C, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xED, 0x68, 0x3E, 0xE5, 0xC6, 0x83, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x68, 
	0x3E, 0xEE, 0xE4, 0x86, 0x74, 0xEE, 0xEE, 0xEE, 0xEE, 0x99, 0x96, 0x66, 0x67, 0x5E, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xED, 0x6C, 0x5E, 0xEE, 0xEA, 0x66, 0x1E, 0xEE, 0xEE, 0xEE, 0xEE, 0x23, 0xEE, 0xEE, 
	0x48, 0x66, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 0x26, 0x74, 0xEE, 0x86, 0x8E, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xE4, 0x6A, 0x31, 0x11, 0x14, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x16, 0x69, 0x4E, 0xEE, 0xEE, 
	0x5E, 0xEE, 0xEE, 0xEE, 0x53, 0x11, 0x1D, 0xCC, 0xCB, 0x86, 0xDE, 0xEE, 0xEE, 0xEE, 0xE9, 0x69, 
	0xEE, 0xEE, 0x09, 0x67, 0x5E, 0xEE, 0xEE, 0xEE, 0x26, 0x62, 0xEE, 0xEE, 0x58, 0x6A, 0xEE, 0xEE, 
	0xEE, 0xEE, 0x48, 0x61, 0xEE, 0xEE, 0xEE, 0xD6, 0x6D, 0xEE, 0xEE, 0xEE, 0xEE, 0x17, 0x69, 0xD9, 
	0x69, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE9, 0xDE, 0xEE, 0xEE, 0xE1, 0x66, 0x1E, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEC, 0x66, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 0xB6, 0xCE, 0xE0, 0x96, 0xAE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xE2, 0x6D, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x76, 
	0x94, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEC, 0x6C, 0x0E, 0xEE, 
	0xEE, 0xEE, 0x46, 0x6C, 0xEE, 0xEE, 0xEC, 0x66, 0x4E, 0xEE, 0xEE, 0xE5, 0x86, 0xAE, 0xEE, 0xEE, 
	0xED, 0x66, 0x3E, 0xEE, 0xEE, 0xE0, 0xA6, 0xA0, 0xEE, 0xEE, 0xEE, 0x36, 0x6B, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xE5, 0x40, 0x08, 0x6C, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE2, 0x0E, 0xEE, 0xEE, 0xE1, 0x66, 
	0x3E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEB, 0x69, 0x0E, 0xEE, 0xEE, 0xEE, 0xE4, 0x78, 
	0x4E, 0xE4, 0x66, 0xDE, 0xEE, 0xEE, 0xEE, 0xEE, 0xED, 0x63, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0x0A, 0x66, 0x3E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE4, 
	0xA6, 0xC0, 0xEE, 0xEE, 0xEE, 0xEE, 0x46, 0x69, 0x0E, 0xEE, 0xEB, 0x67, 0x5E, 0xEE, 0xEE, 0xE2, 
	0x66, 0x1E, 0xEE, 0xEE, 0xE2, 0x66, 0x1E, 0xEE, 0xEE, 0xE2, 0x66, 0x3E, 0xEE, 0xEE, 0xEE, 0x57, 
	0x69, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x46, 0x61, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEA, 0x69, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x47, 0x6D, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEC, 0x6D, 0xEE, 0xE2, 0x66, 0x2E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEB, 0x90, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x26, 0x6A, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0x59, 0x6C, 0x0E, 0xEE, 0xEE, 0xEE, 0xEE, 0x09, 0x66, 0xDE, 0xEE, 0x57, 0x6C, 
	0xEE, 0xEE, 0xEE, 0xED, 0x66, 0x3E, 0xEE, 0xEE, 0xE4, 0x66, 0xCE, 0xEE, 0xEE, 0xEA, 0x69, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xE8, 0x68, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x16, 0x63, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x47, 0x62, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE5, 
	0xB6, 0xC0, 0xEE, 0xEE, 0xEE, 0xEE, 0x47, 0x90, 0xEE, 0xE1, 0x66, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xE9, 0x99, 0x89, 0xC2, 0x0E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xA6, 0x62, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0xC6, 0xC0, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE2, 0x66, 
	0x6D, 0xE0, 0xC6, 0x95, 0xEE, 0xEE, 0xEE, 0xEC, 0x66, 0x4E, 0xEE, 0xEE, 0xE3, 0x66, 0xCE, 0xEE, 
	0xEE, 0x36, 0x6D, 0xEE, 0xEE, 0xEE, 0xEE, 0xE8, 0x68, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xD6, 
	0x75, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xC6, 0xAE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xE5, 0x45, 0x29, 0x7D, 0x0E, 0xEE, 0xEE, 0xEE, 0xEE, 0xC6, 0x3E, 0xEE, 0xEC, 0x69, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0x46, 0x66, 0x66, 0x66, 0xA4, 0xEE, 0xEE, 0xEE, 0xEE, 0xE5, 0x76, 0x75, 
	0x3D, 0x98, 0xAD, 0x5E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEC, 0x69, 0x5E, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xD6, 0x66, 0xDB, 0x6A, 0x4E, 0xEE, 0xEE, 0xEE, 0xEC, 0x66, 0x2E, 0xEE, 0xEE, 
	0xEC, 0x66, 0xCE, 0xEE, 0xEE, 0x16, 0x63, 0xEE, 0xEE, 0xEE, 0xEE, 0xE8, 0x6A, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xA6, 0x9E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x68, 0x4E, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xD7, 0x67, 0x68, 0x2E, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x7D, 0xEE, 
	0xEE, 0xEA, 0x6C, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x2A, 0xCC, 0xB7, 0x66, 0x69, 0x5E, 0xEE, 0xEE, 
	0xEE, 0xE3, 0x66, 0xA1, 0x76, 0x66, 0x66, 0x83, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x16, 0x84, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x5A, 0x66, 0x67, 0x10, 0xEE, 0xEE, 0xEE, 0xEE, 0xE2, 
	0x66, 0xA0, 0xEE, 0xEE, 0xD6, 0x66, 0xCE, 0xEE, 0xEE, 0xC6, 0x64, 0xEE, 0xEE, 0xEE, 0xEE, 0x57, 
	0x6C, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0x86, 0xCE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0x48, 0x73, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x66, 0x66, 0x66, 0x6C, 0x5E, 0xEE, 0xEE, 
	0xEE, 0x0A, 0x95, 0xEE, 0xEE, 0x49, 0x6D, 0x44, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE5, 0x18, 
	0x66, 0xDE, 0xEE, 0xEE, 0xEE, 0xE1, 0x66, 0x96, 0xA2, 0x43, 0xC6, 0x69, 0x5E, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xE3, 0x86, 0x1E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE2, 0x88, 0xD8, 0x66, 0xA4, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xE5, 0x96, 0x6C, 0x34, 0x2A, 0x69, 0x66, 0x1E, 0xEE, 0xEE, 0xA6, 0x8E, 0xEE, 
	0xEE, 0xEE, 0xEE, 0x46, 0x61, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x66, 0x1E, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0xA6, 0x1E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0x31, 0x22, 0xC7, 
	0x66, 0xA0, 0xEE, 0xEE, 0xEE, 0xD6, 0xD1, 0xCB, 0x98, 0x66, 0x66, 0x66, 0x93, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xE2, 0x66, 0x9E, 0xEE, 0xEE, 0xEE, 0xEC, 0x66, 0x6D, 0xEE, 0xEE, 0x0A, 0x66, 
	0x2E, 0xEE, 0xEE, 0xEE, 0xEE, 0x0A, 0x6A, 0x0E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0xD6, 0x94, 
	0xE4, 0x96, 0x69, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 0x38, 0x66, 0x66, 0x67, 0x1A, 0x66, 0x3E, 0xEE, 
	0xEE, 0x86, 0x8E, 0xEE, 0xEE, 0xEE, 0xEE, 0x16, 0x63, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE1, 0x66, 
	0x3E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x0C, 0x6D, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xE4, 0x96, 0x62, 0xEE, 0xEE, 0xEE, 0x96, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 
	0x6C, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0x96, 0x64, 0xEE, 0xEE, 0xEE, 0xEC, 0x66, 0xCE, 
	0xEE, 0xEE, 0xE2, 0x66, 0xCE, 0xEE, 0xEE, 0xEE, 0xEE, 0x16, 0x74, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xED, 0x68, 0x4E, 0xEE, 0x49, 0x66, 0xA0, 0xEE, 0xEE, 0xEE, 0xEE, 0xE4, 0xDA, 0x89, 0xD3, 
	0x57, 0x67, 0x5E, 0xEE, 0xEE, 0x86, 0x8E, 0xEE, 0xEE, 0xEE, 0xEE, 0xB6, 0x9E, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEC, 0x67, 0x5E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0xC6, 0xDE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x26, 0x6C, 0xEE, 0xEE, 0xEE, 0xA8, 0xAC, 0x11, 
	0x34, 0xB6, 0xA4, 0x41, 0xDA, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xC6, 0x64, 0xEE, 0xEE, 
	0xEE, 0xEC, 0x66, 0x3E, 0xEE, 0xEE, 0xE4, 0x66, 0xCE, 0xEE, 0xEE, 0xEE, 0xE5, 0x86, 0xCE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x36, 0x62, 0xEE, 0xEE, 0xE4, 0x96, 0x62, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0x26, 0x6A, 0xEE, 0xEE, 0xEE, 0x86, 0x8E, 0xEE, 0xEE, 0xEE, 0xE5, 0x76, 
	0x2E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEA, 0x69, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x0C, 
	0x6D, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x46, 0x6C, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x96, 0xCE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xC6, 0x64, 0xEE, 0xEE, 0xEE, 0xEC, 0x66, 0x4E, 0xEE, 0xEE, 0xE3, 0x66, 0xDE, 0xEE, 0xEE, 0xEE, 
	0xED, 0x67, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xB6, 0x75, 0xEE, 0xEE, 0xEE, 0x16, 0x6C, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xA6, 0x62, 0xEE, 0xEE, 0xEE, 0xA6, 0x64, 0xEE, 
	0xEE, 0xEE, 0xED, 0x6A, 0x0E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x57, 0x6C, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xE4, 0xA8, 0x2E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0x36, 0x6D, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE5, 0x76, 0x1E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xE0, 0x96, 0x9E, 0xEE, 0xEE, 0xEE, 0xE1, 0x66, 0x2E, 0xEE, 0xEE, 0xE1, 0x66, 
	0x2E, 0xEE, 0xEE, 0xEE, 0x09, 0x6A, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x96, 0x9E, 0xEE, 
	0xEE, 0xEE, 0x46, 0x6C, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x66, 0xA0, 0xEE, 0xEE, 
	0xEE, 0xD6, 0x62, 0xEE, 0xEE, 0xEE, 0x47, 0x84, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x36, 0x61, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x39, 0x83, 0xEE, 0x54, 0x44, 0x4E, 0xEE, 0xEE, 0xEE, 0x00, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xC6, 0x63, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x66, 0x1E, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE1, 0x66, 0xDE, 0xEE, 0xEE, 0xEE, 0xE3, 0x66, 0xDE, 
	0xEE, 0xEE, 0xEA, 0x68, 0x5E, 0xEE, 0xEE, 0xEE, 0x26, 0x61, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0x96, 0x75, 0xEE, 0xEE, 0xEE, 0x36, 0x61, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x49, 
	0x67, 0x3E, 0xEE, 0xEE, 0xEE, 0x47, 0x6A, 0xEE, 0xEE, 0xE4, 0x96, 0x3E, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0x16, 0x62, 0x11, 0x15, 0xEE, 0xEE, 0xEE, 0x0D, 0x76, 0xAA, 0x89, 0x76, 0x66, 0x69, 
	0x4E, 0xEE, 0xEE, 0x4C, 0x5E, 0xEE, 0xEE, 0x0D, 0x66, 0xCE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xE1, 0x66, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 0x23, 0xEE, 0xEE, 0xEE, 0x38, 0x69, 0x5E, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xA6, 0x85, 0xEE, 0xEE, 0x26, 0x62, 0xEE, 0xEE, 0xEE, 0xEE, 0xA6, 0x64, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xD6, 0x6D, 0xEE, 0xEE, 0xEE, 0xC6, 0x85, 0xEE, 0xEE, 0xEE, 0xE5, 
	0xEE, 0xEE, 0xE4, 0x96, 0x61, 0xEE, 0xEE, 0xEE, 0xEE, 0xED, 0x66, 0xD0, 0xEE, 0x49, 0x82, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0x30, 0x03, 0xA6, 0x66, 0x66, 0x68, 0x5E, 0xEE, 0xEE, 0x46, 0x66, 0x66, 
	0x66, 0x66, 0x66, 0x66, 0xDE, 0xEE, 0xEE, 0x57, 0x8D, 0x34, 0x41, 0xA6, 0x6A, 0x5E, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEC, 0x66, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 0x26, 0xA1, 0x34, 0x3C, 0x76, 
	0x93, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x48, 0x6C, 0x5E, 0xE3, 0x86, 0xDE, 0xEE, 0xEE, 0xEE, 0xE5, 
	0x76, 0x90, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x58, 0x66, 0xD5, 0xEE, 0x0D, 0x68, 0x3E, 
	0xEE, 0xEE, 0xEE, 0xEA, 0xD2, 0x44, 0x18, 0x66, 0xDE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0xC6, 0x69, 
	0xCA, 0x7A, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xD9, 0x96, 0x66, 0x66, 0x66, 0x66, 0x1E, 0xEE, 
	0xEE, 0x57, 0x69, 0xAC, 0xD1, 0x11, 0x1D, 0xC9, 0x9E, 0xEE, 0xEE, 0xE1, 0x66, 0x66, 0x66, 0x68, 
	0xD0, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEC, 0x66, 0x4E, 0xEE, 0xEE, 0xEE, 0xEE, 0x0B, 
	0x66, 0x66, 0x66, 0x6C, 0x5E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0x96, 0x8C, 0xA6, 0x82, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xE4, 0x66, 0x8E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE4, 0xA6, 0x68, 
	0xCC, 0x96, 0x93, 0xEE, 0xEE, 0xEE, 0xEE, 0xED, 0x66, 0x66, 0x66, 0x92, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xE3, 0xDC, 0xC1, 0x3E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x48, 0x69, 0x9C, 0xD1, 
	0x34, 0x43, 0x3E, 0xEE, 0xEE, 0xE4, 0x40, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0x3E, 0xEE, 0xEE, 0xEE, 
	0x31, 0xCC, 0xCD, 0x25, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE3, 0xCD, 0x5E, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xE0, 0x3D, 0xCC, 0xC1, 0x3E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x52, 
	0xCC, 0xC2, 0x5E, 0xEE, 0xEE, 0xEE, 0xEE, 0xE5, 0x96, 0x90, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xE3, 0x1C, 0xCC, 0x13, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x3D, 0xCC, 0xD3, 0x0E, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xE5, 0x40, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0x51, 0x15, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
};
