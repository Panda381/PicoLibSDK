#include "include.h"

// sound format: Intel IMA ADPCM mono 4-bit 22050Hz
const u16 ZingSnd_SampBlock = 1017; // number of samples per block

const u8 ZingSnd[4712] = {
	0x00, 0xFD, 0x00, 0x00, 0x77, 0x77, 0x77, 0xF7, 0x0F, 0xA2, 0x0D, 0x27, 0xA0, 0x28, 0xD0, 0x0B, 
	0x02, 0x9A, 0x36, 0xB0, 0x2A, 0xB1, 0x9F, 0x33, 0xA8, 0x52, 0xA1, 0x8D, 0x01, 0xAB, 0x52, 0x92, 
	0x29, 0xA2, 0xAF, 0x10, 0xA9, 0x62, 0x83, 0x0A, 0x81, 0xAE, 0x20, 0x91, 0x58, 0x83, 0xAC, 0x11, 
	0xDA, 0x38, 0x83, 0x2A, 0x05, 0xBC, 0x18, 0xB0, 0x49, 0x06, 0x99, 0x22, 0xEB, 0x29, 0x91, 0x2A, 
	0x16, 0xA8, 0x20, 0xD9, 0x1B, 0x03, 0x8A, 0x36, 0xC9, 0x19, 0xB0, 0x0C, 0x25, 0xA8, 0x42, 0xC8, 
	0x0B, 0x82, 0x9C, 0x35, 0x90, 0x38, 0xD0, 0x8D, 0x21, 0x89, 0x62, 0x90, 0x0B, 0x90, 0x9D, 0x32, 
	0xA0, 0x51, 0xA3, 0x9C, 0x01, 0xBC, 0x61, 0x81, 0x28, 0x93, 0xBE, 0x10, 0xA8, 0x68, 0x03, 0x8B, 
	0x12, 0xEC, 0x28, 0x91, 0x29, 0x15, 0xAA, 0x28, 0xE9, 0x19, 0x04, 0x09, 0x33, 0xDB, 0x19, 0xC1, 
	0x1B, 0x16, 0x98, 0x41, 0xC8, 0x0C, 0x02, 0x8A, 0x53, 0xB0, 0x1A, 0x92, 0xAE, 0x42, 0x81, 0x18, 
	0xB1, 0x9C, 0x01, 0x89, 0x46, 0xA0, 0x2A, 0xC1, 0x9D, 0x22, 0xB9, 0x72, 0xB3, 0x8C, 0x12, 0xAC, 
	0x62, 0x91, 0x19, 0x82, 0xAE, 0x11, 0xB9, 0x68, 0x83, 0x0B, 0x12, 0xBD, 0x48, 0x91, 0x39, 0x05, 
	0xBB, 0x32, 0xFA, 0x29, 0x02, 0x0B, 0x15, 0xCA, 0x28, 0xC0, 0x1A, 0x16, 0x99, 0x32, 0xD9, 0x2A, 
	0x92, 0x8C, 0x25, 0xB9, 0x48, 0xB1, 0x8C, 0x13, 0xAB, 0x54, 0xB0, 0x1A, 0x93, 0x8E, 0x42, 0xA9, 
	0x48, 0xA1, 0x0C, 0x12, 0xAC, 0x52, 0xA1, 0x2A, 0x93, 0x9F, 0x31, 0xB9, 0x50, 0x92, 0x8B, 0x03, 
	0xBD, 0x40, 0xA1, 0x4A, 0x04, 0x9C, 0x11, 0xC9, 0x38, 0x84, 0x0A, 0x15, 0xDB, 0x28, 0xA0, 0x29, 
	0x05, 0xAA, 0x22, 0xF9, 0x29, 0x92, 0x1B, 0x35, 0xBA, 0x38, 0xD0, 0x2B, 0x15, 0xAA, 0x53, 0xC9, 
	0x2A, 0x91, 0x0C, 0x24, 0xA9, 0x31, 0xE1, 0x0B, 0x03, 0x9C, 0x44, 0xB0, 0x29, 0xA2, 0x9E, 0x23, 
	0xB9, 0x61, 0xA1, 0x1B, 0x83, 0xBD, 0x42, 0xA0, 0x49, 0x93, 0x9E, 0x12, 0xBA, 0x60, 0x92, 0x0A, 
	0x03, 0xBD, 0x40, 0xA0, 0x39, 0x05, 0xAB, 0x32, 0xFB, 0x28, 0x92, 0x1A, 0x16, 0xBA, 0x28, 0xC8, 
	0x3A, 0x05, 0x8A, 0x33, 0xFA, 0x29, 0x90, 0x1A, 0x16, 0x99, 0x21, 0xEA, 0x1B, 0x83, 0x9A, 0x45, 
	0x90, 0x30, 0xC1, 0x9E, 0x01, 0xAA, 0x50, 0x82, 0x29, 0x03, 0xBC, 0x21, 0xC9, 0x48, 0xA2, 0x0B, 
	0x02, 0x9C, 0x34, 0xEB, 0x48, 0x93, 0x9C, 0x22, 0xC8, 0x31, 0xB2, 0x1D, 0x03, 0xBC, 0x51, 0xA0, 
	0x39, 0x04, 0x9C, 0x00, 0xCD, 0x48, 0x93, 0x19, 0x14, 0xBB, 0x41, 0xC8, 0x3A, 0x04, 0x8C, 0x42, 
	0xD9, 0x19, 0x91, 0x0A, 0x15, 0xB9, 0x31, 0xE0, 0x1B, 0x14, 0x8A, 0x43, 0xC8, 0x2A, 0xB2, 0x0E, 
	0x33, 0xA9, 0x40, 0xD1, 0x0B, 0x02, 0xAB, 0x45, 0xB0, 0x29, 0xA3, 0x8E, 0x21, 0xBA, 0x62, 0xA1, 
	0x0A, 0x83, 0xAE, 0x41, 0xA1, 0x30, 0xA3, 0xAD, 0x22, 0xDB, 0x40, 0x92, 0x1B, 0x13, 0xBD, 0x30, 
	0xD8, 0x49, 0x04, 0x9A, 0x22, 0xEB, 0x39, 0x82, 0x2A, 0x06, 0xCA, 0x20, 0xC8, 0x19, 0x04, 0x8A, 
	0x43, 0xD9, 0x29, 0xB1, 0x0B, 0x26, 0xA9, 0x41, 0xC0, 0x0A, 0x02, 0x8D, 0x43, 0xA8, 0x39, 0xB1, 
	0x9F, 0x22, 0x99, 0x62, 0xA1, 0x0B, 0x91, 0x9D, 0x42, 0x90, 0x48, 0x92, 0x9E, 0x11, 0xAA, 0x51, 
	0x92, 0x1A, 0x83, 0xAF, 0x20, 0x90, 0x48, 0x84, 0xAB, 0x22, 0xCC, 0x38, 0x92, 0x3A, 0x16, 0xCB, 
	0x20, 0xC9, 0x39, 0x05, 0x8A, 0x23, 0xFB, 0x18, 0xA1, 0x2A, 0x25, 0xB9, 0x31, 0xF8, 0x1A, 0x82, 
	0x89, 0x44, 0xB9, 0x28, 0xB0, 0x0E, 0x23, 0xA9, 0x62, 0xB0, 0x0B, 0x92, 0x9D, 0x53, 0xA0, 0x30, 
	0x00, 0x53, 0x4E, 0x00, 0xEB, 0x29, 0xA1, 0x2A, 0x16, 0xA9, 0x30, 0xE9, 0x1A, 0x83, 0x09, 0x44, 
	0xB9, 0x19, 0xD1, 0x0B, 0x24, 0xA8, 0x52, 0xC0, 0x0A, 0x81, 0x8C, 0x53, 0xA0, 0x38, 0xC1, 0x9D, 
	0x12, 0x99, 0x72, 0x91, 0x0A, 0x91, 0xAD, 0x32, 0xA0, 0x50, 0x92, 0x8D, 0x01, 0xBA, 0x51, 0xA2, 
	0x19, 0x85, 0xAC, 0x20, 0xC8, 0x48, 0x84, 0x0A, 0x12, 0xEB, 0x29, 0xA1, 0x39, 0x16, 0xAA, 0x20, 
	0xE9, 0x19, 0x03, 0x0A, 0x26, 0xBA, 0x19, 0xC0, 0x1C, 0x15, 0x98, 0x32, 0xE8, 0x0A, 0x81, 0x8A, 
	0x35, 0xB0, 0x39, 0xD1, 0x8D, 0x22, 0x99, 0x62, 0xA0, 0x0A, 0xA2, 0x9D, 0x42, 0xA0, 0x40, 0xA2, 
	0x9C, 0x11, 0xAC, 0x61, 0x91, 0x29, 0x82, 0xAE, 0x20, 0xA8, 0x68, 0x82, 0x99, 0x02, 0xCC, 0x38, 
	0x91, 0x39, 0x16, 0xBB, 0x11, 0xEA, 0x39, 0x84, 0x09, 0x24, 0xCB, 0x19, 0xC0, 0x3A, 0x16, 0x99, 
	0x22, 0xE9, 0x1A, 0x81, 0x0A, 0x26, 0xA8, 0x29, 0xD0, 0x0B, 0x23, 0x99, 0x54, 0xB0, 0x0B, 0xA1, 
	0x8E, 0x43, 0xA0, 0x40, 0xB1, 0x9D, 0x02, 0xAA, 0x63, 0x91, 0x19, 0xA2, 0xAE, 0x21, 0xA0, 0x60, 
	0x82, 0x9B, 0x02, 0xBD, 0x40, 0x91, 0x38, 0x85, 0xBB, 0x10, 0xDA, 0x48, 0x04, 0x0A, 0x13, 0xDC, 
	0x18, 0xA0, 0x39, 0x16, 0xA9, 0x21, 0xEA, 0x2A, 0x82, 0x19, 0x25, 0xB9, 0x19, 0xD8, 0x1B, 0x15, 
	0x98, 0x53, 0xC8, 0x0A, 0x90, 0x8B, 0x36, 0xA0, 0x20, 0xD1, 0x9C, 0x12, 0x8A, 0x73, 0x90, 0x09, 
	0xA1, 0xAD, 0x32, 0xA0, 0x52, 0xA2, 0x8C, 0x81, 0xAD, 0x51, 0x91, 0x28, 0x93, 0xAD, 0x10, 0xB9, 
	0x70, 0x82, 0x09, 0x02, 0xCD, 0x28, 0x90, 0x48, 0x04, 0x9A, 0x10, 0xEB, 0x29, 0x03, 0x19, 0x25, 
	0xCB, 0x19, 0xC8, 0x29, 0x15, 0x98, 0x23, 0xF9, 0x0A, 0x81, 0x1A, 0x25, 0x98, 0x28, 0xE8, 0x0B, 
	0x12, 0x89, 0x45, 0xB0, 0x0A, 0xB1, 0x8E, 0x32, 0x90, 0x51, 0xA1, 0x8D, 0x80, 0xAA, 0x63, 0x81, 
	0x29, 0xB2, 0xAE, 0x11, 0xA8, 0x71, 0x92, 0x89, 0x81, 0xAD, 0x20, 0x91, 0x40, 0x04, 0x9C, 0x00, 
	0xCB, 0x48, 0x83, 0x29, 0x14, 0xCC, 0x19, 0xB0, 0x49, 0x15, 0x99, 0x12, 0xEB, 0x2A, 0x81, 0x2A, 
	0x26, 0xA9, 0x18, 0xD9, 0x1B, 0x14, 0x09, 0x53, 0xC8, 0x1A, 0xB0, 0x8B, 0x36, 0x98, 0x31, 0xD0, 
	0x8C, 0x01, 0x9A, 0x54, 0x90, 0x18, 0xB1, 0x8F, 0x20, 0x98, 0x51, 0x91, 0x8A, 0x91, 0xAD, 0x41, 
	0x91, 0x30, 0x94, 0x9D, 0x00, 0xBA, 0x51, 0x83, 0x19, 0x03, 0xBF, 0x28, 0xA0, 0x59, 0x04, 0x8A, 
	0x01, 0xEB, 0x29, 0x82, 0x29, 0x16, 0xAA, 0x18, 0xE9, 0x19, 0x14, 0x89, 0x33, 0xEA, 0x09, 0xA1, 
	0x1B, 0x26, 0x98, 0x30, 0xE8, 0x0B, 0x02, 0x8A, 0x45, 0x98, 0x09, 0xC1, 0x8C, 0x32, 0x98, 0x62, 
	0xB1, 0x8B, 0x91, 0x9D, 0x52, 0x91, 0x38, 0xA2, 0x9F, 0x10, 0xA9, 0x61, 0x92, 0x09, 0x92, 0xBD, 
	0x21, 0x90, 0x68, 0x83, 0x9B, 0x01, 0xBD, 0x48, 0x82, 0x29, 0x06, 0xBB, 0x28, 0xD9, 0x49, 0x13, 
	0x0A, 0x13, 0xFB, 0x1A, 0x91, 0x3A, 0x26, 0xA9, 0x20, 0xEA, 0x1A, 0x12, 0x0A, 0x35, 0xC8, 0x09, 
	0xB0, 0x0D, 0x24, 0x98, 0x42, 0xC0, 0x0C, 0x91, 0x9A, 0x35, 0x90, 0x28, 0xC1, 0x8E, 0x11, 0x99, 
	0x62, 0x91, 0x0A, 0x91, 0x9E, 0x21, 0x90, 0x50, 0x92, 0xAB, 0x01, 0xCC, 0x41, 0x82, 0x29, 0x84, 
	0xBD, 0x10, 0xA8, 0x58, 0x04, 0x8A, 0x12, 0xCD, 0x28, 0x91, 0x39, 0x16, 0xAA, 0x10, 0xEA, 0x29, 
	0x03, 0x09, 0x34, 0xEA, 0x19, 0xB0, 0x1A, 0x26, 0x99, 0x22, 0xE8, 0x0A, 0x81, 0x0A, 0x35, 0xB0, 
	0x28, 0xE0, 0x0C, 0x12, 0x98, 0x53, 0xB1, 0x0B, 0xA1, 0x9F, 0x33, 0x90, 0x40, 0xB2, 0x9E, 0x01, 
	0x00, 0xE1, 0x4B, 0x00, 0x2A, 0x16, 0x89, 0x20, 0xD9, 0x0A, 0x02, 0x0A, 0x26, 0xA8, 0x29, 0xD8, 
	0x0C, 0x23, 0x89, 0x53, 0xC0, 0x8A, 0xA2, 0x8D, 0x43, 0x90, 0x30, 0xD2, 0x9C, 0x02, 0xAA, 0x73, 
	0x91, 0x19, 0xA1, 0xAD, 0x31, 0x98, 0x60, 0x82, 0x9B, 0x81, 0xBD, 0x41, 0x92, 0x38, 0x04, 0xBD, 
	0x10, 0xC9, 0x58, 0x83, 0x09, 0x03, 0xCD, 0x18, 0xA1, 0x49, 0x14, 0x9A, 0x20, 0xFB, 0x19, 0x83, 
	0x19, 0x25, 0xC9, 0x19, 0xB8, 0x2C, 0x24, 0x89, 0x42, 0xE8, 0x0A, 0x81, 0x0B, 0x35, 0xA8, 0x30, 
	0xE0, 0x8C, 0x12, 0x99, 0x44, 0xA0, 0x1A, 0xB1, 0x9E, 0x32, 0xA0, 0x52, 0xB2, 0x8C, 0x81, 0xAC, 
	0x52, 0x81, 0x29, 0x93, 0xAF, 0x01, 0xA8, 0x60, 0x82, 0x89, 0x82, 0xBD, 0x38, 0x91, 0x58, 0x04, 
	0xAB, 0x10, 0xEB, 0x39, 0x84, 0x08, 0x14, 0xCB, 0x29, 0xC8, 0x39, 0x15, 0x89, 0x22, 0xFB, 0x09, 
	0x81, 0x19, 0x25, 0xB8, 0x10, 0xE8, 0x0B, 0x04, 0x88, 0x53, 0xB8, 0x1A, 0xB0, 0x8D, 0x34, 0x98, 
	0x51, 0xB0, 0x8D, 0x81, 0x9A, 0x44, 0x91, 0x29, 0xB1, 0x9F, 0x11, 0x98, 0x61, 0x81, 0x8A, 0x91, 
	0xAD, 0x31, 0x91, 0x50, 0x93, 0xAC, 0x00, 0xCB, 0x50, 0x83, 0x19, 0x04, 0xBD, 0x18, 0xB0, 0x58, 
	0x14, 0x8A, 0x01, 0xFB, 0x29, 0x81, 0x29, 0x25, 0xBA, 0x18, 0xE9, 0x19, 0x04, 0x08, 0x33, 0xEA, 
	0x1A, 0xA0, 0x1B, 0x26, 0x88, 0x30, 0xF8, 0x8A, 0x82, 0x89, 0x35, 0xA0, 0x19, 0xC0, 0x9D, 0x32, 
	0x98, 0x63, 0xB1, 0x9A, 0x91, 0xAD, 0x43, 0x81, 0x30, 0xB3, 0xAF, 0x00, 0xA9, 0x62, 0x82, 0x09, 
	0x92, 0xBE, 0x20, 0x90, 0x50, 0x84, 0x9A, 0x81, 0xEB, 0x28, 0x83, 0x28, 0x15, 0xCB, 0x08, 0xC9, 
	0x49, 0x13, 0x88, 0x14, 0xEB, 0x09, 0x90, 0x29, 0x17, 0x98, 0x10, 0xD9, 0x0A, 0x02, 0x19, 0x35, 
	0xB8, 0x1A, 0xE8, 0x0B, 0x43, 0x88, 0x52, 0xC0, 0x8A, 0x90, 0x8C, 0x44, 0x90, 0x20, 0xC1, 0x9C, 
	0x10, 0x99, 0x73, 0x81, 0x89, 0xA1, 0xAD, 0x30, 0x80, 0x61, 0x82, 0xAB, 0x80, 0xBD, 0x41, 0x83, 
	0x38, 0x84, 0xBD, 0x08, 0xB9, 0x60, 0x04, 0x09, 0x01, 0xCC, 0x19, 0x91, 0x48, 0x15, 0xA9, 0x00, 
	0xDB, 0x1A, 0x04, 0x18, 0x25, 0xBA, 0x0A, 0xC8, 0x1B, 0x17, 0x80, 0x21, 0xD8, 0x0B, 0x80, 0x1B, 
	0x36, 0x90, 0x18, 0xE0, 0x9B, 0x12, 0x88, 0x64, 0x90, 0x0A, 0xB0, 0x9D, 0x32, 0x91, 0x52, 0xA2, 
	0x9D, 0x80, 0xAB, 0x72, 0x81, 0x10, 0x91, 0xBD, 0x10, 0x99, 0x71, 0x02, 0x8A, 0x81, 0xBD, 0x38, 
	0x91, 0x50, 0x04, 0xAA, 0x08, 0xEB, 0x28, 0x13, 0x29, 0x15, 0xDB, 0x09, 0xA8, 0x4A, 0x24, 0x88, 
	0x21, 0xFB, 0x0A, 0x81, 0x19, 0x26, 0xA0, 0x08, 0xD9, 0x0B, 0x13, 0x08, 0x36, 0xB8, 0x0B, 0xC8, 
	0x8C, 0x34, 0x91, 0x42, 0xC0, 0x9C, 0x80, 0x8A, 0x54, 0x81, 0x18, 0xB0, 0xAE, 0x20, 0x88, 0x62, 
	0x82, 0x8B, 0x90, 0xAE, 0x30, 0x82, 0x50, 0x93, 0xCB, 0x08, 0xCB, 0x50, 0x03, 0x18, 0x02, 0xCD, 
	0x19, 0xA8, 0x58, 0x14, 0x89, 0x00, 0xEB, 0x1A, 0x82, 0x38, 0x26, 0xB9, 0x08, 0xDA, 0x1A, 0x14, 
	0x18, 0x43, 0xD9, 0x8A, 0xA0, 0x1B, 0x36, 0x90, 0x31, 0xE9, 0x9B, 0x01, 0x89, 0x36, 0x91, 0x09, 
	0xD0, 0x9C, 0x31, 0x88, 0x63, 0xA2, 0x9B, 0xA0, 0xAD, 0x52, 0x81, 0x21, 0xA3, 0x9F, 0x08, 0x9A, 
	0x51, 0x83, 0x08, 0x92, 0xBE, 0x18, 0x90, 0x51, 0x04, 0xA9, 0x81, 0xDC, 0x28, 0x82, 0x20, 0x15, 
	0xCA, 0x88, 0xC9, 0x39, 0x15, 0x08, 0x22, 0xEB, 0x0A, 0x90, 0x2A, 0x27, 0x98, 0x20, 0xDA, 0x0B, 
	0x02, 0x19, 0x36, 0xA8, 0x0A, 0xC8, 0x8D, 0x23, 0x80, 0x63, 0xB0, 0x8B, 0xA0, 0x8D, 0x53, 0x81, 
	0x00, 0x11, 0x48, 0x00, 0x12, 0xEB, 0x1A, 0x90, 0x39, 0x26, 0x99, 0x10, 0xEA, 0x0A, 0x02, 0x18, 
	0x35, 0xB8, 0x0A, 0xD9, 0x0B, 0x34, 0x80, 0x43, 0xD0, 0x9B, 0xA1, 0x8B, 0x46, 0x91, 0x10, 0xC1, 
	0x9D, 0x10, 0x89, 0x63, 0x81, 0x0A, 0xA0, 0xAD, 0x21, 0x81, 0x61, 0x82, 0x9C, 0x88, 0xBB, 0x51, 
	0x03, 0x28, 0x84, 0xCD, 0x18, 0xA9, 0x50, 0x13, 0x89, 0x82, 0xDD, 0x18, 0x80, 0x38, 0x16, 0xA9, 
	0x80, 0xDA, 0x19, 0x13, 0x18, 0x25, 0xD9, 0x09, 0xB8, 0x2B, 0x26, 0x88, 0x32, 0xF9, 0x8A, 0x81, 
	0x1A, 0x44, 0x90, 0x18, 0xD8, 0x8C, 0x12, 0x88, 0x63, 0x90, 0x8A, 0xB0, 0x9D, 0x42, 0x91, 0x32, 
	0xC3, 0x9D, 0x80, 0xAA, 0x72, 0x01, 0x19, 0xA2, 0xAD, 0x28, 0xA8, 0x61, 0x03, 0x9A, 0x81, 0xBE, 
	0x38, 0x92, 0x40, 0x05, 0xAB, 0x08, 0xDA, 0x39, 0x05, 0x18, 0x13, 0xCC, 0x1A, 0xA8, 0x39, 0x27, 
	0x89, 0x11, 0xEB, 0x1A, 0x81, 0x18, 0x35, 0xB8, 0x08, 0xD9, 0x0C, 0x23, 0x88, 0x44, 0xB8, 0x8B, 
	0xB0, 0x8D, 0x35, 0x80, 0x30, 0xD1, 0x8D, 0x00, 0x99, 0x63, 0x91, 0x08, 0xB1, 0x9E, 0x11, 0x90, 
	0x52, 0x92, 0x8B, 0x90, 0xBD, 0x41, 0x82, 0x30, 0x85, 0xAD, 0x18, 0xBA, 0x60, 0x83, 0x08, 0x02, 
	0xCD, 0x08, 0xA1, 0x30, 0x07, 0x99, 0x01, 0xCB, 0x2A, 0x03, 0x29, 0x17, 0xB9, 0x19, 0xD9, 0x2A, 
	0x24, 0x88, 0x33, 0xFA, 0x0A, 0x90, 0x1A, 0x26, 0x90, 0x10, 0xD8, 0x8C, 0x02, 0x88, 0x44, 0xA0, 
	0x09, 0xC0, 0x9C, 0x42, 0x90, 0x42, 0xB2, 0x8D, 0x80, 0x9C, 0x52, 0x81, 0x28, 0xA2, 0x9F, 0x00, 
	0x98, 0x50, 0x02, 0x0A, 0x91, 0xBD, 0x38, 0x91, 0x50, 0x85, 0xAA, 0x00, 0xDB, 0x38, 0x04, 0x29, 
	0x04, 0xDB, 0x08, 0xB8, 0x49, 0x15, 0x89, 0x12, 0xEB, 0x1A, 0x81, 0x29, 0x35, 0xA9, 0x18, 0xF9, 
	0x0A, 0x22, 0x09, 0x44, 0xB8, 0x0B, 0xB0, 0x8D, 0x25, 0x90, 0x32, 0xD0, 0x8C, 0x00, 0x8B, 0x35, 
	0xA2, 0x18, 0xC1, 0x9E, 0x11, 0x98, 0x62, 0x92, 0x9A, 0x91, 0xAD, 0x31, 0x92, 0x50, 0x93, 0xAD, 
	0x00, 0xCA, 0x41, 0x03, 0x19, 0x83, 0xCE, 0x18, 0xA0, 0x58, 0x04, 0x99, 0x01, 0xCC, 0x29, 0x82, 
	0x28, 0x16, 0xB9, 0x08, 0xDA, 0x2A, 0x15, 0x19, 0x32, 0xEA, 0x0A, 0x90, 0x2B, 0x26, 0x98, 0x21, 
	0xF8, 0x8A, 0x02, 0x0A, 0x35, 0xA0, 0x1A, 0xD0, 0x8C, 0x32, 0x98, 0x63, 0xB1, 0x8B, 0x90, 0x9D, 
	0x53, 0x80, 0x20, 0xA2, 0x9F, 0x00, 0x99, 0x61, 0x92, 0x19, 0x91, 0xAE, 0x20, 0x90, 0x50, 0x83, 
	0x9B, 0x00, 0xBD, 0x48, 0x02, 0x39, 0x05, 0xCB, 0x19, 0xC9, 0x38, 0x16, 0x89, 0x03, 0xEB, 0x09, 
	0x91, 0x39, 0x26, 0xA9, 0x10, 0xF9, 0x09, 0x02, 0x08, 0x34, 0xC8, 0x09, 0xC8, 0x0B, 0x25, 0x88, 
	0x42, 0xC8, 0x8B, 0x91, 0x8B, 0x46, 0x90, 0x28, 0xC0, 0x9C, 0x11, 0x89, 0x73, 0x91, 0x89, 0xA1, 
	0xAD, 0x31, 0x91, 0x61, 0x92, 0x9C, 0x80, 0xBB, 0x71, 0x01, 0x18, 0x82, 0xBD, 0x18, 0xA8, 0x60, 
	0x03, 0x0A, 0x01, 0xDD, 0x18, 0x81, 0x38, 0x15, 0xAA, 0x18, 0xEB, 0x29, 0x13, 0x19, 0x25, 0xDA, 
	0x09, 0xA8, 0x2B, 0x17, 0x90, 0x12, 0xD9, 0x0B, 0x81, 0x1A, 0x27, 0xA0, 0x18, 0xD8, 0x8B, 0x13, 
	0x88, 0x45, 0xA0, 0x0B, 0xB0, 0x8E, 0x32, 0x81, 0x41, 0xC2, 0xAC, 0x81, 0xAA, 0x73, 0x82, 0x19, 
	0xA2, 0xAF, 0x11, 0x98, 0x51, 0x82, 0x8A, 0x91, 0xCD, 0x20, 0x82, 0x30, 0x05, 0xCB, 0x80, 0xCA, 
	0x48, 0x04, 0x19, 0x03, 0xCC, 0x19, 0xA8, 0x49, 0x25, 0x99, 0x11, 0xFB, 0x09, 0x82, 0x18, 0x25, 
	0xA9, 0x19, 0xD9, 0x0B, 0x24, 0x08, 0x53, 0xC8, 0x8A, 0xA0, 0x0C, 0x44, 0x90, 0x21, 0xD0, 0x9B, 
	0x00, 0xEF, 0x46, 0x00, 0xA0, 0x58, 0x14, 0x99, 0x01, 0xCC, 0x1A, 0x83, 0x28, 0x17, 0xA9, 0x19, 
	0xD9, 0x1A, 0x14, 0x08, 0x24, 0xD9, 0x0A, 0xA0, 0x1B, 0x36, 0x88, 0x20, 0xD8, 0x8C, 0x01, 0x89, 
	0x35, 0xA1, 0x09, 0xC0, 0x9E, 0x22, 0x80, 0x52, 0xA1, 0x9B, 0x90, 0x9E, 0x51, 0x81, 0x20, 0xA2, 
	0xAE, 0x00, 0x99, 0x61, 0x82, 0x19, 0x91, 0xBD, 0x28, 0x90, 0x60, 0x03, 0x9A, 0x80, 0xEC, 0x28, 
	0x02, 0x28, 0x15, 0xBB, 0x09, 0xD9, 0x29, 0x16, 0x88, 0x22, 0xEB, 0x09, 0x90, 0x29, 0x26, 0x98, 
	0x18, 0xD9, 0x0B, 0x12, 0x19, 0x45, 0xA8, 0x0A, 0xD0, 0x8B, 0x24, 0x80, 0x43, 0xC0, 0x8C, 0x90, 
	0x8B, 0x54, 0x91, 0x20, 0xC1, 0x9D, 0x01, 0x99, 0x63, 0x92, 0x09, 0xA0, 0xBD, 0x31, 0x80, 0x61, 
	0x82, 0xAB, 0x80, 0xBD, 0x50, 0x02, 0x28, 0x03, 0xBE, 0x08, 0xA9, 0x58, 0x05, 0x88, 0x01, 0xEB, 
	0x19, 0x91, 0x38, 0x16, 0x99, 0x08, 0xDA, 0x1A, 0x13, 0x29, 0x26, 0xC9, 0x09, 0xC8, 0x1A, 0x34, 
	0x88, 0x42, 0xD9, 0x0B, 0x80, 0x0B, 0x27, 0x80, 0x18, 0xC8, 0x9C, 0x12, 0x89, 0x54, 0xA1, 0x89, 
	0xB0, 0xAD, 0x42, 0x91, 0x42, 0xA2, 0x9D, 0x80, 0xBB, 0x72, 0x01, 0x18, 0x92, 0xAE, 0x00, 0x98, 
	0x60, 0x02, 0x89, 0x81, 0xBD, 0x39, 0x81, 0x58, 0x04, 0xAA, 0x08, 0xEB, 0x28, 0x13, 0x29, 0x15, 
	0xDB, 0x09, 0xB0, 0x3A, 0x17, 0x88, 0x21, 0xEA, 0x0A, 0x81, 0x19, 0x26, 0x98, 0x08, 0xD8, 0x0B, 
	0x22, 0x09, 0x45, 0xB0, 0x0B, 0xB0, 0x8E, 0x33, 0x91, 0x42, 0xC1, 0x8D, 0x80, 0x9A, 0x73, 0x91, 
	0x18, 0xA1, 0xAD, 0x20, 0x98, 0x62, 0x92, 0x0A, 0x90, 0xBD, 0x30, 0x82, 0x60, 0x83, 0xAC, 0x08, 
	0xCB, 0x40, 0x04, 0x19, 0x03, 0xCD, 0x18, 0xA8, 0x48, 0x14, 0x89, 0x01, 0xFB, 0x19, 0x81, 0x28, 
	0x25, 0xAA, 0x08, 0xE9, 0x1A, 0x23, 0x08, 0x34, 0xE9, 0x0A, 0xA0, 0x1B, 0x35, 0x90, 0x32, 0xF8, 
	0x8B, 0x81, 0x0A, 0x45, 0xA1, 0x19, 0xC0, 0x9C, 0x22, 0x88, 0x72, 0x91, 0x8B, 0x90, 0x9D, 0x41, 
	0x81, 0x30, 0xA3, 0x9F, 0x80, 0xA9, 0x61, 0x82, 0x19, 0x81, 0xBD, 0x28, 0x98, 0x60, 0x03, 0x9A, 
	0x81, 0xCD, 0x28, 0x02, 0x49, 0x14, 0xBB, 0x09, 0xE9, 0x39, 0x14, 0x08, 0x13, 0xFB, 0x09, 0xA0, 
	0x29, 0x26, 0x98, 0x10, 0xE9, 0x0A, 0x01, 0x19, 0x35, 0xB0, 0x1A, 0xD8, 0x8C, 0x33, 0x88, 0x44, 
	0xB0, 0x8D, 0xA1, 0x8B, 0x44, 0x91, 0x30, 0xC1, 0x9E, 0x01, 0x99, 0x53, 0x92, 0x09, 0xA1, 0xAF, 
	0x21, 0x90, 0x41, 0x83, 0x9C, 0x80, 0xBC, 0x50, 0x82, 0x28, 0x85, 0xCB, 0x18, 0xB9, 0x58, 0x04, 
	0x09, 0x02, 0xDC, 0x19, 0x91, 0x38, 0x16, 0xA9, 0x10, 0xEA, 0x09, 0x03, 0x29, 0x34, 0xD9, 0x09, 
	0xB8, 0x1B, 0x26, 0x88, 0x32, 0xF8, 0x0A, 0x90, 0x1A, 0x44, 0x90, 0x28, 0xD0, 0x9C, 0x12, 0x89, 
	0x44, 0xA1, 0x0A, 0xA0, 0x9F, 0x22, 0x80, 0x41, 0xA2, 0x9D, 0x81, 0x9C, 0x51, 0x92, 0x28, 0x92, 
	0xAE, 0x00, 0xA8, 0x61, 0x02, 0x0A, 0x81, 0xBE, 0x20, 0x91, 0x40, 0x04, 0xBA, 0x00, 0xEB, 0x39, 
	0x04, 0x08, 0x14, 0xDA, 0x09, 0xB0, 0x29, 0x17, 0x88, 0x11, 0xDA, 0x0A, 0x81, 0x19, 0x27, 0xA8, 
	0x18, 0xC9, 0x0C, 0x13, 0x88, 0x44, 0xC0, 0x0A, 0xA0, 0x8C, 0x34, 0x90, 0x41, 0xC0, 0x8C, 0x00, 
	0x9A, 0x54, 0x91, 0x08, 0xA1, 0xAE, 0x21, 0x98, 0x52, 0x92, 0x9A, 0x91, 0xAE, 0x40, 0x81, 0x30, 
	0x94, 0xAC, 0x00, 0xCB, 0x50, 0x03, 0x19, 0x83, 0xBE, 0x29, 0xA8, 0x68, 0x13, 0xA9, 0x02, 0xEC, 
	0x19, 0x02, 0x29, 0x16, 0xB9, 0x08, 0xC9, 0x1A, 0x15, 0x08, 0x33, 0xEA, 0x0A, 0xA0, 0x2B, 0x36, 
	0x00, 0x0F, 0x44, 0x00, 0x19, 0x92, 0xCD, 0x28, 0xA0, 0x50, 0x03, 0x9A, 0x81, 0xDC, 0x29, 0x03, 
	0x39, 0x25, 0xCB, 0x19, 0xCA, 0x3A, 0x16, 0x88, 0x13, 0xEA, 0x09, 0xA0, 0x19, 0x27, 0x98, 0x10, 
	0xD9, 0x8A, 0x02, 0x08, 0x35, 0xB0, 0x0A, 0xD0, 0x8C, 0x33, 0x88, 0x53, 0xC1, 0x9B, 0x91, 0x8D, 
	0x53, 0x91, 0x20, 0xB1, 0x9F, 0x00, 0x98, 0x62, 0x81, 0x09, 0x90, 0xBD, 0x21, 0x91, 0x51, 0x83, 
	0xBB, 0x81, 0xBE, 0x40, 0x83, 0x38, 0x04, 0xBD, 0x19, 0xB9, 0x68, 0x23, 0x99, 0x03, 0xFC, 0x19, 
	0x91, 0x28, 0x15, 0xA8, 0x18, 0xEA, 0x09, 0x13, 0x19, 0x25, 0xB9, 0x0B, 0xC8, 0x1C, 0x25, 0x08, 
	0x31, 0xF8, 0x8A, 0x81, 0x0A, 0x44, 0x90, 0x18, 0xC0, 0x9D, 0x12, 0x88, 0x53, 0x91, 0x8B, 0xB1, 
	0x9F, 0x31, 0x81, 0x41, 0xA2, 0x9D, 0x08, 0xAB, 0x71, 0x02, 0x19, 0x92, 0xAE, 0x18, 0x98, 0x51, 
	0x03, 0x8A, 0x81, 0xCE, 0x28, 0x81, 0x30, 0x06, 0xAA, 0x08, 0xCB, 0x39, 0x14, 0x28, 0x14, 0xEB, 
	0x09, 0xA8, 0x3A, 0x17, 0x90, 0x11, 0xDA, 0x8A, 0x82, 0x29, 0x36, 0xA8, 0x09, 0xE8, 0x0B, 0x32, 
	0x08, 0x44, 0xB0, 0x8C, 0xA0, 0x9C, 0x44, 0x80, 0x21, 0xC1, 0xAC, 0x00, 0x99, 0x54, 0x92, 0x08, 
	0xB1, 0x9F, 0x10, 0x80, 0x51, 0x82, 0xAA, 0x90, 0xBD, 0x40, 0x02, 0x30, 0x84, 0xBC, 0x09, 0xCA, 
	0x68, 0x03, 0x08, 0x83, 0xDC, 0x09, 0x90, 0x48, 0x15, 0x98, 0x80, 0xEA, 0x1A, 0x02, 0x28, 0x16, 
	0xB8, 0x09, 0xD9, 0x1A, 0x24, 0x08, 0x33, 0xE9, 0x8B, 0xA0, 0x0A, 0x37, 0x80, 0x28, 0xD8, 0x8C, 
	0x10, 0x09, 0x44, 0x91, 0x0A, 0xB8, 0x9E, 0x31, 0x80, 0x53, 0xA2, 0x9C, 0x98, 0xAC, 0x62, 0x01, 
	0x28, 0x92, 0xAE, 0x08, 0x99, 0x61, 0x02, 0x88, 0x81, 0xBE, 0x28, 0x80, 0x50, 0x13, 0xAA, 0x88, 
	0xCD, 0x39, 0x03, 0x20, 0x16, 0xCA, 0x89, 0xC8, 0x39, 0x25, 0x88, 0x22, 0xFB, 0x0A, 0x80, 0x29, 
	0x35, 0x98, 0x18, 0xEA, 0x8B, 0x22, 0x08, 0x36, 0xA8, 0x8A, 0xC8, 0x9C, 0x34, 0x80, 0x42, 0xC1, 
	0x9C, 0x80, 0x8B, 0x73, 0x81, 0x10, 0xA0, 0xAE, 0x10, 0x88, 0x52, 0x82, 0x89, 0xA0, 0xBE, 0x30, 
	0x01, 0x41, 0x84, 0xBB, 0x88, 0xCC, 0x40, 0x03, 0x20, 0x03, 0xCE, 0x09, 0xA8, 0x58, 0x14, 0x88, 
	0x81, 0xEB, 0x1A, 0x81, 0x38, 0x16, 0xA8, 0x08, 0xDA, 0x1B, 0x23, 0x10, 0x35, 0xC9, 0x8B, 0xB8, 
	0x0C, 0x36, 0x80, 0x21, 0xD8, 0x9C, 0x81, 0x0A, 0x45, 0x91, 0x08, 0xC0, 0x9D, 0x11, 0x80, 0x53, 
	0x81, 0x9B, 0xA0, 0xAE, 0x32, 0x82, 0x51, 0x92, 0xAD, 0x88, 0xAA, 0x71, 0x02, 0x08, 0x92, 0xBD, 
	0x08, 0x90, 0x60, 0x13, 0x99, 0x80, 0xCD, 0x29, 0x82, 0x30, 0x16, 0xBA, 0x08, 0xDB, 0x29, 0x24, 
	0x18, 0x14, 0xEA, 0x89, 0x98, 0x3A, 0x35, 0x88, 0x11, 0xFA, 0x8A, 0x01, 0x19, 0x35, 0x90, 0x0A, 
	0xE8, 0x9B, 0x33, 0x18, 0x44, 0xB1, 0x9C, 0xB0, 0x9C, 0x63, 0x01, 0x20, 0xB1, 0xAE, 0x00, 0x9A, 
	0x73, 0x82, 0x09, 0xA1, 0xAD, 0x28, 0x90, 0x62, 0x83, 0xAA, 0x90, 0xCC, 0x38, 0x03, 0x40, 0x04, 
	0xBC, 0x09, 0xBA, 0x69, 0x14, 0x08, 0x02, 0xDC, 0x09, 0x90, 0x38, 0x26, 0x99, 0x10, 0xEB, 0x1A, 
	0x11, 0x18, 0x35, 0xA9, 0x8A, 0xD9, 0x0B, 0x25, 0x00, 0x42, 0xC8, 0x8C, 0x90, 0x8A, 0x35, 0x81, 
	0x20, 0xD8, 0x9D, 0x01, 0x88, 0x63, 0x81, 0x0A, 0xB0, 0x9E, 0x21, 0x80, 0x52, 0x92, 0x9C, 0x90, 
	0xAC, 0x51, 0x82, 0x20, 0x93, 0xBE, 0x00, 0xA9, 0x60, 0x03, 0x09, 0x81, 0xCD, 0x29, 0x80, 0x40, 
	0x14, 0x9A, 0x08, 0xDC, 0x29, 0x12, 0x28, 0x25, 0xCA, 0x0A, 0xB9, 0x3B, 0x27, 0x08, 0x12, 0xF9, 
	0x00, 0xF1, 0x47, 0x00, 0x00, 0x99, 0x41, 0x03, 0x89, 0x81, 0xCE, 0x10, 0x00, 0x48, 0x04, 0xAA, 
	0x80, 0xDB, 0x39, 0x13, 0x28, 0x16, 0xCB, 0x09, 0xB9, 0x39, 0x27, 0x88, 0x21, 0xEB, 0x0A, 0x91, 
	0x18, 0x26, 0xA0, 0x18, 0xE9, 0x8A, 0x22, 0x19, 0x44, 0xB0, 0x0B, 0xB8, 0x8E, 0x43, 0x80, 0x41, 
	0xB0, 0x9D, 0x80, 0x8A, 0x73, 0x81, 0x18, 0xB0, 0xAD, 0x11, 0x98, 0x63, 0x82, 0x8A, 0xA0, 0xAE, 
	0x30, 0x01, 0x50, 0x83, 0x9D, 0x88, 0xBA, 0x50, 0x03, 0x28, 0x03, 0xBF, 0x19, 0xA8, 0x58, 0x14, 
	0x89, 0x01, 0xDC, 0x19, 0x92, 0x38, 0x16, 0xA9, 0x08, 0xDA, 0x1A, 0x14, 0x18, 0x33, 0xE9, 0x0A, 
	0xA8, 0x1B, 0x36, 0x88, 0x31, 0xE9, 0x9B, 0x82, 0x0A, 0x46, 0x90, 0x19, 0xB8, 0x8E, 0x21, 0x88, 
	0x62, 0xA1, 0x8A, 0xA0, 0x9D, 0x42, 0x91, 0x31, 0xB3, 0x9F, 0x80, 0x9A, 0x61, 0x82, 0x08, 0x92, 
	0xAE, 0x28, 0x98, 0x51, 0x03, 0x9B, 0x81, 0xCD, 0x28, 0x02, 0x38, 0x06, 0xBA, 0x08, 0xDA, 0x39, 
	0x15, 0x19, 0x22, 0xDC, 0x09, 0x90, 0x3A, 0x17, 0x98, 0x20, 0xDA, 0x0A, 0x82, 0x18, 0x35, 0xB8, 
	0x09, 0xE8, 0x0B, 0x14, 0x80, 0x53, 0xB8, 0x8C, 0xA1, 0x8B, 0x45, 0x80, 0x20, 0xC0, 0x9D, 0x01, 
	0x89, 0x63, 0x91, 0x19, 0xA0, 0xAE, 0x21, 0x80, 0x51, 0x93, 0x9C, 0x80, 0xBC, 0x51, 0x01, 0x38, 
	0x93, 0xAE, 0x18, 0xAA, 0x68, 0x03, 0x09, 0x02, 0xBE, 0x29, 0x90, 0x58, 0x14, 0xA9, 0x00, 0xFB, 
	0x29, 0x02, 0x18, 0x24, 0xCA, 0x09, 0xC8, 0x1A, 0x16, 0x08, 0x31, 0xDA, 0x0B, 0x91, 0x2B, 0x36, 
	0xA0, 0x20, 0xE9, 0x8C, 0x12, 0x09, 0x44, 0xA0, 0x0A, 0xB0, 0x9E, 0x32, 0x91, 0x52, 0xA1, 0x9D, 
	0x80, 0xAA, 0x63, 0x92, 0x28, 0xA2, 0xAF, 0x10, 0x98, 0x51, 0x83, 0x8A, 0x91, 0xBE, 0x30, 0x91, 
	0x50, 0x03, 0xCB, 0x00, 0xDB, 0x38, 0x04, 0x18, 0x14, 0xCC, 0x08, 0xA8, 0x39, 0x17, 0x89, 0x02, 
	0xEA, 0x09, 0x81, 0x18, 0x25, 0xA8, 0x08, 0xE9, 0x0A, 0x23, 0x19, 0x44, 0xB9, 0x0B, 0xB8, 0x0D, 
	0x44, 0x80, 0x21, 0xD0, 0x8C, 0x00, 0x8A, 0x54, 0x80, 0x19, 0xB0, 0x9E, 0x11, 0x90, 0x53, 0xA2, 
	0x9A, 0xA1, 0xAE, 0x41, 0x81, 0x40, 0x92, 0x9D, 0x08, 0xBA, 0x61, 0x02, 0x08, 0x93, 0xBE, 0x18, 
	0x90, 0x50, 0x04, 0x99, 0x00, 0xCC, 0x29, 0x02, 0x38, 0x16, 0xBA, 0x19, 0xEA, 0x29, 0x14, 0x08, 
	0x23, 0xFB, 0x09, 0x98, 0x2A, 0x26, 0x98, 0x11, 0xD9, 0x8B, 0x02, 0x19, 0x36, 0xB0, 0x19, 0xD9, 
	0x8C, 0x23, 0x80, 0x63, 0xA0, 0x8C, 0xA0, 0x9B, 0x54, 0x81, 0x20, 0xC1, 0xAC, 0x10, 0x9A, 0x73, 
	0x82, 0x09, 0x90, 0xAE, 0x20, 0x90, 0x52, 0x82, 0xAA, 0x90, 0xDC, 0x30, 0x02, 0x30, 0x85, 0xDB, 
	0x08, 0xB9, 0x58, 0x04, 0x08, 0x02, 0xDC, 0x09, 0x91, 0x38, 0x16, 0x98, 0x08, 0xEA, 0x09, 0x02, 
	0x10, 0x25, 0xC8, 0x09, 0xB9, 0x1C, 0x24, 0x00, 0x42, 0xD9, 0x8A, 0x88, 0x0B, 0x36, 0x91, 0x20, 
	0xE8, 0x9B, 0x01, 0x88, 0x45, 0x91, 0x89, 0xC0, 0x9D, 0x31, 0x00, 0x52, 0x91, 0x9C, 0x98, 0xBB, 
	0x63, 0x83, 0x20, 0x92, 0xBF, 0x00, 0x99, 0x51, 0x04, 0x89, 0x81, 0xBD, 0x19, 0x92, 0x51, 0x14, 
	0xAA, 0x88, 0xEB, 0x29, 0x13, 0x38, 0x15, 0xDA, 0x89, 0xA9, 0x29, 0x17, 0x80, 0x12, 0xEA, 0x0A, 
	0x90, 0x28, 0x35, 0x98, 0x08, 0xE9, 0x8B, 0x12, 0x10, 0x45, 0xB0, 0x8A, 0xC8, 0x8C, 0x43, 0x00, 
	0x32, 0xD1, 0x9C, 0x90, 0x8A, 0x54, 0x81, 0x10, 0xB0, 0xAE, 0x00, 0x90, 0x63, 0x82, 0x99, 0x90, 
	0xAE, 0x20, 0x01, 0x51, 0x02, 0xAC, 0x88, 0xCB, 0x58, 0x12, 0x10, 0x83, 0xCD, 0x09, 0xA8, 0x40, 
	0x00, 0x09, 0x3F, 0x00, 0x82, 0x29, 0xD8, 0x9E, 0x10, 0x08, 0x62, 0x81, 0x8A, 0x98, 0xAD, 0x30, 
	0x02, 0x51, 0x82, 0xBC, 0x98, 0xCA, 0x51, 0x03, 0x10, 0x93, 0xCE, 0x08, 0x98, 0x40, 0x14, 0x88, 
	0x80, 0xCD, 0x19, 0x01, 0x30, 0x16, 0xA9, 0x89, 0xDA, 0x19, 0x33, 0x20, 0x24, 0xEA, 0x8A, 0xA9, 
	0x2A, 0x27, 0x00, 0x10, 0xD9, 0x9B, 0x00, 0x18, 0x36, 0x91, 0x09, 0xD9, 0x8D, 0x11, 0x01, 0x53, 
	0xA1, 0x9B, 0xA9, 0x9D, 0x52, 0x01, 0x31, 0xB2, 0xAF, 0x08, 0x9A, 0x63, 0x02, 0x08, 0xA0, 0xBD, 
	0x18, 0x80, 0x62, 0x03, 0x9A, 0x98, 0xCD, 0x28, 0x12, 0x31, 0x15, 0xCB, 0x8A, 0xCA, 0x38, 0x16, 
	0x00, 0x03, 0xCC, 0x8A, 0xA0, 0x30, 0x27, 0x88, 0x00, 0xDB, 0x0B, 0x11, 0x28, 0x36, 0xB0, 0x99, 
	0xD9, 0x0C, 0x32, 0x01, 0x34, 0xD0, 0x9B, 0xA8, 0x8B, 0x46, 0x01, 0x10, 0xC0, 0x9D, 0x08, 0x08, 
	0x72, 0x81, 0x88, 0xA8, 0xAC, 0x20, 0x81, 0x63, 0x82, 0xAB, 0xA8, 0xAD, 0x40, 0x03, 0x31, 0x94, 
	0xCC, 0x89, 0xA9, 0x60, 0x13, 0x00, 0x00, 0xBE, 0x0A, 0x80, 0x51, 0x24, 0x99, 0x98, 0xEB, 0x1A, 
	0x12, 0x21, 0x35, 0xD9, 0x99, 0xB8, 0x2B, 0x35, 0x01, 0x22, 0xF9, 0x9B, 0x90, 0x19, 0x36, 0x81, 
	0x18, 0xE9, 0x9B, 0x20, 0x18, 0x54, 0x91, 0x8A, 0xB9, 0x9E, 0x31, 0x02, 0x43, 0xB2, 0xBC, 0x99, 
	0x9C, 0x62, 0x03, 0x20, 0xB1, 0xAF, 0x08, 0x89, 0x52, 0x13, 0x89, 0xA0, 0xCE, 0x18, 0x01, 0x41, 
	0x13, 0xBA, 0x8A, 0xCD, 0x39, 0x14, 0x20, 0x04, 0xDA, 0x8A, 0xA9, 0x38, 0x27, 0x80, 0x01, 0xDB, 
	0x8B, 0x81, 0x38, 0x27, 0x90, 0x09, 0xDA, 0x8B, 0x22, 0x11, 0x45, 0xB0, 0xAA, 0xB8, 0x8D, 0x53, 
	0x01, 0x31, 0xC0, 0x9D, 0x88, 0x0A, 0x63, 0x82, 0x18, 0xB8, 0xAE, 0x10, 0x08, 0x53, 0x83, 0x9A, 
	0xB8, 0xBE, 0x30, 0x13, 0x51, 0x83, 0xCC, 0x88, 0xBB, 0x60, 0x13, 0x18, 0x02, 0xBE, 0x0A, 0x88, 
	0x50, 0x14, 0x88, 0x80, 0xDC, 0x09, 0x01, 0x30, 0x16, 0xA8, 0x89, 0xCB, 0x0A, 0x25, 0x10, 0x23, 
	0xD9, 0x9B, 0xA9, 0x1A, 0x37, 0x01, 0x10, 0xE9, 0x9B, 0x00, 0x19, 0x55, 0x91, 0x09, 0xC8, 0x9C, 
	0x21, 0x01, 0x62, 0x91, 0x9B, 0x99, 0x9D, 0x51, 0x11, 0x20, 0xA2, 0xAE, 0x88, 0xA9, 0x63, 0x12, 
	0x09, 0x91, 0xBE, 0x08, 0x81, 0x51, 0x04, 0xA9, 0x90, 0xCC, 0x18, 0x13, 0x30, 0x15, 0xCA, 0x8A, 
	0xBA, 0x4A, 0x26, 0x18, 0x11, 0xEB, 0x8A, 0x80, 0x29, 0x36, 0x88, 0x08, 0xDA, 0x8B, 0x21, 0x28, 
	0x36, 0xA0, 0x8B, 0xCA, 0x8D, 0x33, 0x01, 0x63, 0xB0, 0x9C, 0x98, 0x8B, 0x45, 0x01, 0x28, 0xC0, 
	0xAD, 0x00, 0x80, 0x72, 0x01, 0x89, 0x98, 0xAD, 0x20, 0x81, 0x52, 0x83, 0xAC, 0x88, 0xBC, 0x50, 
	0x12, 0x10, 0x83, 0xCD, 0x09, 0xA8, 0x40, 0x15, 0x88, 0x81, 0xCC, 0x1A, 0x81, 0x30, 0x17, 0xA8, 
	0x88, 0xD9, 0x0A, 0x23, 0x10, 0x25, 0xD8, 0x8A, 0xA8, 0x2B, 0x35, 0x81, 0x22, 0xF9, 0x8B, 0x88, 
	0x08, 0x36, 0x91, 0x19, 0xD9, 0x9C, 0x21, 0x80, 0x44, 0xA2, 0x8B, 0xB8, 0x9E, 0x41, 0x01, 0x41, 
	0xA1, 0xBC, 0x88, 0xAA, 0x73, 0x83, 0x18, 0x91, 0xBE, 0x18, 0x88, 0x61, 0x03, 0x99, 0x88, 0xCD, 
	0x28, 0x11, 0x30, 0x15, 0xCB, 0x88, 0xCA, 0x39, 0x15, 0x10, 0x12, 0xDC, 0x89, 0xA0, 0x38, 0x26, 
	0x90, 0x00, 0xEA, 0x0B, 0x11, 0x28, 0x35, 0xB0, 0x89, 0xE9, 0x8B, 0x33, 0x10, 0x35, 0xC0, 0x9C, 
	0xA0, 0x8B, 0x45, 0x81, 0x20, 0xD1, 0x9C, 0x00, 0x89, 0x73, 0x81, 0x09, 0xA0, 0xAD, 0x20, 0x81, 
	0x52, 0x93, 0xAB, 0x98, 0xAE, 0x40, 0x02, 0x40, 0x82, 0xBD, 0x88, 0xA9, 0x60, 0x13, 0x08, 0x81, 
	0x00, 0x02, 0x38, 0x00, 0x9F, 0x01, 0x08, 0x63, 0x91, 0x89, 0xB0, 0xAD, 0x31, 0x81, 0x53, 0x92, 
	0xAC, 0x98, 0xCB, 0x52, 0x83, 0x30, 0xA2, 0xAF, 0x08, 0xA9, 0x61, 0x03, 0x09, 0x91, 0xCD, 0x18, 
	0x80, 0x41, 0x04, 0xA9, 0x88, 0xEB, 0x29, 0x13, 0x20, 0x15, 0xCB, 0x89, 0xB9, 0x3A, 0x27, 0x00, 
	0x11, 0xFA, 0x0A, 0x80, 0x29, 0x25, 0x90, 0x08, 0xE9, 0x8B, 0x22, 0x18, 0x35, 0xA0, 0x9B, 0xC9, 
	0x8D, 0x43, 0x01, 0x31, 0xC1, 0x9E, 0x80, 0x8A, 0x53, 0x82, 0x18, 0xB1, 0xAF, 0x10, 0x88, 0x52, 
	0x83, 0x8A, 0xA8, 0xAE, 0x38, 0x02, 0x51, 0x83, 0xAD, 0x88, 0xBA, 0x58, 0x04, 0x10, 0x02, 0xCD, 
	0x09, 0x98, 0x40, 0x14, 0x88, 0x81, 0xDC, 0x09, 0x01, 0x20, 0x26, 0xA9, 0x89, 0xD9, 0x1A, 0x23, 
	0x10, 0x35, 0xCA, 0x8B, 0xB8, 0x1C, 0x36, 0x80, 0x21, 0xE9, 0x8B, 0x80, 0x09, 0x36, 0x91, 0x09, 
	0xC8, 0x9D, 0x21, 0x00, 0x53, 0xA2, 0x9B, 0xB8, 0x9E, 0x32, 0x12, 0x41, 0xB3, 0xAF, 0x88, 0xA9, 
	0x63, 0x02, 0x08, 0xA1, 0xBD, 0x08, 0x80, 0x71, 0x03, 0xA9, 0x80, 0xBD, 0x39, 0x02, 0x41, 0x14, 
	0xCB, 0x98, 0xCA, 0x39, 0x25, 0x10, 0x12, 0xDC, 0x0A, 0x98, 0x39, 0x27, 0x88, 0x00, 0xDA, 0x0B, 
	0x01, 0x28, 0x36, 0xA0, 0x8A, 0xD9, 0x8B, 0x33, 0x11, 0x35, 0xC0, 0x9C, 0xA8, 0x8B, 0x45, 0x82, 
	0x20, 0xC0, 0x9D, 0x08, 0x09, 0x72, 0x82, 0x09, 0xA8, 0xAD, 0x20, 0x00, 0x52, 0x83, 0x9C, 0x98, 
	0xAC, 0x30, 0x23, 0x50, 0x02, 0xCD, 0x88, 0xA9, 0x50, 0x23, 0x19, 0x81, 0xDD, 0x09, 0x80, 0x40, 
	0x14, 0x89, 0x88, 0xDB, 0x0A, 0x13, 0x30, 0x26, 0xB9, 0x9A, 0xE9, 0x19, 0x33, 0x10, 0x34, 0xFA, 
	0x8A, 0x98, 0x19, 0x45, 0x80, 0x00, 0xD8, 0x9B, 0x01, 0x00, 0x45, 0x80, 0x8A, 0xB9, 0x9D, 0x31, 
	0x82, 0x63, 0x91, 0xAC, 0x88, 0x9C, 0x52, 0x11, 0x10, 0x91, 0xAF, 0x08, 0x98, 0x52, 0x02, 0x08, 
	0x88, 0xBE, 0x18, 0x81, 0x42, 0x04, 0x9A, 0x89, 0xBD, 0x28, 0x23, 0x31, 0x16, 0xCB, 0x0B, 0xAA, 
	0x49, 0x25, 0x00, 0x82, 0xFA, 0x8A, 0x80, 0x28, 0x26, 0x90, 0x08, 0xEA, 0x0A, 0x11, 0x10, 0x34, 
	0xC1, 0x99, 0xAA, 0x8D, 0x43, 0x01, 0x23, 0xC8, 0x9D, 0x98, 0x99, 0x35, 0x84, 0x00, 0xC0, 0xBC, 
	0x01, 0x08, 0x44, 0x82, 0x0A, 0xB9, 0xCD, 0x21, 0x01, 0x42, 0x94, 0xCA, 0x90, 0xCA, 0x40, 0x22, 
	0x10, 0x02, 0xBE, 0x0A, 0x98, 0x68, 0x23, 0x09, 0x08, 0xEC, 0x09, 0x81, 0x31, 0x24, 0xA9, 0xA8, 
	0xBC, 0x2C, 0x13, 0x22, 0x35, 0xE9, 0x99, 0x99, 0x0A, 0x26, 0x01, 0x11, 0xC9, 0x9D, 0x91, 0x08, 
	0x35, 0x81, 0x89, 0xD0, 0x8D, 0x10, 0x10, 0x43, 0x91, 0x9A, 0xAB, 0xAD, 0x52, 0x81, 0x42, 0x91, 
	0xBC, 0xA8, 0x0A, 0x72, 0x11, 0x80, 0xA2, 0xBD, 0x08, 0x88, 0x33, 0x17, 0x89, 0x0A, 0x9D, 0x2A, 
	0x82, 0x33, 0x33, 0xBC, 0x8B, 0xCC, 0x3B, 0x07, 0x82, 0x22, 0xEA, 0x0A, 0x98, 0x38, 0x42, 0xB2, 
	0x84, 0xFB, 0x89, 0x02, 0x08, 0x25, 0xA8, 0x80, 0xBB, 0x8B, 0x04, 0x84, 0x34, 0xB8, 0x8C, 0xC0, 
	0x8B, 0x34, 0x84, 0x30, 0xC0, 0xBC, 0x80, 0x80, 0x70, 0x01, 0x08, 0xC0, 0x0C, 0x08, 0x08, 0x44, 
	0x08, 0xC8, 0x80, 0x0C, 0x08, 0x04, 0x58, 0x08, 0xC8, 0x80, 0x0C, 0x48, 0x30, 0x80, 0x80, 0x0E, 
	0x0C, 0x88, 0x40, 0x84, 0x80, 0xC0, 0xC8, 0x80, 0x04, 0x08, 0x84, 0xC0, 0x08, 0xD8, 0x80, 0x04, 
	0x08, 0x84, 0xC0, 0x08, 0xD8, 0x80, 0x04, 0x08, 0x58, 0x8B, 0x80, 0x80, 0x80, 0x80, 0xB7, 0x80, 
	0x80, 0xF0, 0x83, 0x80, 0x80, 0xB6, 0x80, 0x08, 0x80, 0x80, 0x08, 0x80, 0x08, 0x80, 0x08, 0x08, 
	0x00, 0x00, 0x11, 0x00, 0x80, 0x80, 0x80, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};