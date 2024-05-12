#include "../include.h"

// format: 4-bit paletted pixel graphics
// image width: 256 pixels
// image height: 32 lines
// image pitch: 128 bytes
const u16 ExplosionImg_Pal[6] __attribute__ ((aligned(4))) = {
	0xF81F, 0xFFE0, 0xFDA0, 0xFB80, 0xF800, 0x0000, 
};

const u8 ExplosionImg[4096] __attribute__ ((aligned(4))) = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x04, 0x44, 0x40, 0x00, 0x44, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x44, 0x44, 0x40, 0x00, 0x44, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x44, 0x44, 0x40, 0x00, 0x44, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x02, 0x00, 0x44, 0x44, 0x40, 0x00, 0x44, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x02, 0x00, 0x44, 0x40, 0x40, 0x00, 0x44, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x02, 0x00, 0x44, 0x40, 0x40, 0x00, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x02, 0x44, 0x04, 0x44, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x44, 0x40, 0x44, 0x40, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x44, 0x40, 0x44, 0x34, 0x44, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x44, 0x40, 0x44, 0x34, 0x44, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x04, 0x44, 0x20, 0x42, 0x32, 0x44, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x04, 0x40, 0x20, 0x42, 0x32, 0x04, 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x04, 0x40, 0x00, 0x42, 0x30, 0x04, 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x04, 0x00, 0x44, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x40, 0x04, 0x00, 0x00, 0x44, 0x33, 0x43, 0x34, 0x44, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x40, 0x04, 0x00, 0x00, 0x44, 0x33, 0x43, 0x44, 0x44, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x40, 0x04, 0x03, 0x00, 0x44, 0x33, 0x43, 0x44, 0x44, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x40, 0x04, 0x03, 0x24, 0x44, 0x33, 0x43, 0x44, 0x24, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x40, 0x04, 0x03, 0x04, 0x43, 0x30, 0x44, 0x00, 0x24, 0x00, 0x40, 0x40, 0x40, 0x00, 0x00, 
	0x00, 0x40, 0x00, 0x03, 0x04, 0x43, 0x30, 0x44, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x40, 0x04, 0x04, 0x40, 0x40, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x04, 0x43, 0x34, 0x44, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x40, 0x34, 0x04, 0x43, 0x43, 0x44, 0x34, 0x44, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x40, 0x44, 0x44, 0x33, 0x43, 0x44, 0x34, 0x44, 0x04, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0x40, 0x44, 0x34, 0x33, 0x43, 0x24, 0x34, 0x44, 0x04, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x03, 0x44, 0x44, 0x34, 0x32, 0x33, 0x24, 0x24, 0x44, 0x04, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x03, 0x44, 0x44, 0x33, 0x33, 0x33, 0x44, 0x23, 0x04, 0x04, 0x04, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x03, 0x44, 0x44, 0x33, 0x00, 0x33, 0x44, 0x20, 0x04, 0x04, 0x04, 0x00, 0x00, 
	0x00, 0x04, 0x02, 0x44, 0x44, 0x40, 0x44, 0x44, 0x40, 0x40, 0x00, 0x44, 0x04, 0x04, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x44, 0x44, 0x42, 0x44, 0x34, 0x04, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x33, 0x24, 0x43, 0x44, 0x44, 0x40, 0x04, 0x40, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x40, 0x33, 0x34, 0x33, 0x23, 0x43, 0x44, 0x34, 0x43, 0x44, 0x40, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x43, 0x33, 0x33, 0x22, 0x23, 0x43, 0x24, 0x33, 0x43, 0x44, 0x40, 0x00, 
	0x00, 0x04, 0x00, 0x04, 0x43, 0x33, 0x43, 0x33, 0x24, 0x24, 0x44, 0x34, 0x43, 0x44, 0x40, 0x00, 
	0x00, 0x04, 0x00, 0x04, 0x40, 0x43, 0x43, 0x33, 0x23, 0x23, 0x34, 0x34, 0x43, 0x44, 0x44, 0x00, 
	0x00, 0x04, 0x00, 0x04, 0x40, 0x43, 0x43, 0x43, 0x23, 0x23, 0x00, 0x34, 0x40, 0x04, 0x44, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x04, 0x44, 0x40, 0x44, 0x44, 0x44, 0x44, 0x44, 0x40, 0x04, 0x44, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x43, 0x44, 0x42, 0x32, 0x43, 0x23, 0x34, 0x44, 0x40, 0x44, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x44, 0x44, 0x42, 0x33, 0x32, 0x33, 0x34, 0x44, 0x40, 0x44, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x30, 0x34, 0x44, 0x42, 0x33, 0x32, 0x33, 0x34, 0x44, 0x40, 0x44, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x30, 0x34, 0x43, 0x43, 0x33, 0x22, 0x33, 0x24, 0x44, 0x40, 0x44, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x42, 0x44, 0x34, 0x43, 0x32, 0x34, 0x34, 0x24, 0x24, 0x44, 0x34, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x42, 0x44, 0x33, 0x43, 0x33, 0x34, 0x43, 0x33, 0x20, 0x30, 0x04, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x42, 0x44, 0x33, 0x43, 0x44, 0x34, 0x43, 0x33, 0x20, 0x00, 0x04, 0x00, 0x00, 
	0x40, 0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x40, 0x40, 0x04, 0x44, 0x40, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x43, 0x44, 0x42, 0x33, 0x33, 0x33, 0x33, 0x34, 0x44, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x43, 0x44, 0x33, 0x32, 0x31, 0x33, 0x23, 0x32, 0x44, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x04, 0x43, 0x44, 0x33, 0x32, 0x33, 0x23, 0x23, 0x32, 0x44, 0x00, 0x04, 0x00, 
	0x00, 0x00, 0x40, 0x34, 0x33, 0x43, 0x23, 0x32, 0x33, 0x23, 0x23, 0x32, 0x24, 0x00, 0x04, 0x00, 
	0x00, 0x40, 0x40, 0x34, 0x33, 0x43, 0x24, 0x32, 0x33, 0x43, 0x22, 0x32, 0x22, 0x00, 0x44, 0x00, 
	0x00, 0x44, 0x44, 0x34, 0x43, 0x43, 0x23, 0x32, 0x33, 0x43, 0x22, 0x32, 0x32, 0x04, 0x04, 0x00, 
	0x00, 0x04, 0x44, 0x34, 0x43, 0x43, 0x43, 0x33, 0x33, 0x43, 0x23, 0x34, 0x32, 0x04, 0x04, 0x00, 
	0x04, 0x04, 0x44, 0x40, 0x44, 0x44, 0x44, 0x44, 0x40, 0x40, 0x40, 0x04, 0x00, 0x04, 0x44, 0x00, 
	0x00, 0x00, 0x00, 0x33, 0x24, 0x22, 0x23, 0x33, 0x32, 0x33, 0x33, 0x33, 0x34, 0x44, 0x00, 0x00, 
	0x44, 0x00, 0x00, 0x44, 0x43, 0x33, 0x11, 0x22, 0x21, 0x22, 0x13, 0x33, 0x34, 0x44, 0x00, 0x00, 
	0x44, 0x40, 0x04, 0x44, 0x43, 0x33, 0x11, 0x22, 0x21, 0x22, 0x13, 0x33, 0x34, 0x44, 0x00, 0x40, 
	0x44, 0x40, 0x03, 0x44, 0x43, 0x33, 0x21, 0x22, 0x22, 0x22, 0x13, 0x32, 0x34, 0x44, 0x00, 0x40, 
	0x32, 0x40, 0x03, 0x44, 0x44, 0x32, 0x44, 0x24, 0x32, 0x22, 0x23, 0x22, 0x32, 0x24, 0x00, 0x40, 
	0x32, 0x40, 0x03, 0x44, 0x43, 0x33, 0x34, 0x24, 0x32, 0x22, 0x33, 0x23, 0x32, 0x24, 0x00, 0x40, 
	0x32, 0x40, 0x03, 0x44, 0x44, 0x43, 0x44, 0x34, 0x44, 0x33, 0x33, 0x33, 0x32, 0x24, 0x00, 0x40, 
	0x44, 0x44, 0x40, 0x44, 0x40, 0x40, 0x40, 0x04, 0x44, 0x34, 0x34, 0x04, 0x00, 0x44, 0x00, 0x40, 
	0x00, 0x00, 0x00, 0x44, 0x32, 0x32, 0x21, 0x22, 0x22, 0x22, 0x33, 0x33, 0x44, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x44, 0x33, 0x33, 0x22, 0x21, 0x11, 0x21, 0x22, 0x13, 0x33, 0x44, 0x40, 0x00, 
	0x00, 0x00, 0x40, 0x34, 0x33, 0x33, 0x22, 0x21, 0x12, 0x22, 0x22, 0x13, 0x33, 0x43, 0x40, 0x00, 
	0x00, 0x33, 0x40, 0x34, 0x33, 0x23, 0x22, 0x22, 0x22, 0x22, 0x22, 0x23, 0x33, 0x43, 0x40, 0x00, 
	0x00, 0x33, 0x20, 0x34, 0x33, 0x23, 0x22, 0x24, 0x42, 0x42, 0x22, 0x22, 0x23, 0x43, 0x40, 0x00, 
	0x00, 0x33, 0x20, 0x34, 0x33, 0x23, 0x23, 0x34, 0x42, 0x42, 0x23, 0x33, 0x33, 0x43, 0x40, 0x00, 
	0x00, 0x33, 0x43, 0x44, 0x34, 0x23, 0x23, 0x34, 0x33, 0x34, 0x33, 0x44, 0x43, 0x43, 0x40, 0x00, 
	0x44, 0x30, 0x43, 0x44, 0x00, 0x04, 0x04, 0x04, 0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x00, 
	0x00, 0x00, 0x00, 0x44, 0x23, 0x32, 0x12, 0x12, 0x22, 0x22, 0x23, 0x23, 0x23, 0x34, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x44, 0x23, 0x32, 0x21, 0x11, 0x11, 0x22, 0x12, 0x22, 0x33, 0x34, 0x04, 0x00, 
	0x40, 0x04, 0x03, 0x43, 0x23, 0x32, 0x21, 0x22, 0x11, 0x22, 0x12, 0x33, 0x33, 0x33, 0x44, 0x00, 
	0x40, 0x03, 0x03, 0x42, 0x23, 0x32, 0x22, 0x22, 0x21, 0x22, 0x12, 0x32, 0x23, 0x22, 0x44, 0x00, 
	0x33, 0x23, 0x33, 0x42, 0x44, 0x32, 0x22, 0x22, 0x31, 0x33, 0x12, 0x32, 0x23, 0x23, 0x34, 0x00, 
	0x33, 0x24, 0x44, 0x42, 0x43, 0x33, 0x34, 0x23, 0x31, 0x33, 0x13, 0x32, 0x23, 0x23, 0x34, 0x00, 
	0x33, 0x24, 0x44, 0x44, 0x43, 0x33, 0x44, 0x43, 0x33, 0x33, 0x33, 0x32, 0x34, 0x43, 0x34, 0x00, 
	0x40, 0x04, 0x04, 0x44, 0x04, 0x44, 0x44, 0x40, 0x40, 0x04, 0x40, 0x44, 0x04, 0x04, 0x34, 0x00, 
	0x00, 0x00, 0x04, 0x43, 0x23, 0x32, 0x21, 0x11, 0x11, 0x21, 0x12, 0x32, 0x23, 0x34, 0x40, 0x00, 
	0x00, 0x04, 0x44, 0x34, 0x33, 0x21, 0x22, 0x12, 0x21, 0x12, 0x11, 0x22, 0x13, 0x34, 0x34, 0x00, 
	0x00, 0x44, 0x44, 0x34, 0x33, 0x21, 0x22, 0x12, 0x21, 0x22, 0x11, 0x22, 0x33, 0x34, 0x34, 0x00, 
	0x00, 0x43, 0x44, 0x22, 0x33, 0x21, 0x22, 0x12, 0x23, 0x22, 0x11, 0x22, 0x33, 0x34, 0x34, 0x00, 
	0x03, 0x43, 0x44, 0x22, 0x33, 0x41, 0x22, 0x22, 0x33, 0x33, 0x33, 0x32, 0x33, 0x24, 0x32, 0x00, 
	0x03, 0x43, 0x44, 0x33, 0x33, 0x44, 0x24, 0x22, 0x43, 0x43, 0x33, 0x32, 0x33, 0x20, 0x32, 0x00, 
	0x03, 0x44, 0x44, 0x44, 0x44, 0x44, 0x34, 0x22, 0x33, 0x43, 0x34, 0x44, 0x33, 0x40, 0x32, 0x00, 
	0x04, 0x40, 0x40, 0x40, 0x40, 0x44, 0x40, 0x04, 0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x00, 
	0x00, 0x04, 0x44, 0x44, 0x32, 0x32, 0x11, 0x21, 0x11, 0x21, 0x21, 0x13, 0x34, 0x44, 0x40, 0x00, 
	0x00, 0x44, 0x44, 0x42, 0x22, 0x22, 0x21, 0x22, 0x21, 0x12, 0x22, 0x21, 0x22, 0x33, 0x44, 0x00, 
	0x00, 0x44, 0x34, 0x32, 0x22, 0x22, 0x21, 0x22, 0x21, 0x22, 0x22, 0x21, 0x22, 0x33, 0x44, 0x00, 
	0x00, 0x44, 0x34, 0x32, 0x22, 0x32, 0x21, 0x22, 0x21, 0x32, 0x22, 0x21, 0x23, 0x33, 0x44, 0x00, 
	0x33, 0x44, 0x34, 0x42, 0x42, 0x32, 0x22, 0x24, 0x43, 0x32, 0x32, 0x21, 0x23, 0x34, 0x44, 0x00, 
	0x33, 0x44, 0x34, 0x33, 0x34, 0x32, 0x22, 0x44, 0x43, 0x34, 0x33, 0x31, 0x33, 0x34, 0x44, 0x00, 
	0x33, 0x44, 0x44, 0x44, 0x34, 0x33, 0x40, 0x40, 0x34, 0x00, 0x33, 0x31, 0x44, 0x34, 0x44, 0x00, 
	0x34, 0x00, 0x44, 0x44, 0x04, 0x00, 0x44, 0x00, 0x30, 0x00, 0x40, 0x00, 0x44, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x44, 0x33, 0x32, 0x22, 0x21, 0x12, 0x11, 0x11, 0x22, 0x22, 0x33, 0x44, 0x40, 0x40, 
	0x40, 0x00, 0x33, 0x43, 0x32, 0x11, 0x11, 0x12, 0x12, 0x21, 0x21, 0x12, 0x22, 0x23, 0x30, 0x40, 
	0x40, 0x40, 0x33, 0x33, 0x32, 0x11, 0x12, 0x22, 0x12, 0x22, 0x21, 0x12, 0x22, 0x23, 0x30, 0x44, 
	0x40, 0x40, 0x33, 0x33, 0x32, 0x22, 0x12, 0x22, 0x12, 0x33, 0x21, 0x12, 0x22, 0x23, 0x20, 0x44, 
	0x42, 0x30, 0x32, 0x34, 0x42, 0x22, 0x42, 0x34, 0x32, 0x33, 0x31, 0x22, 0x32, 0x23, 0x20, 0x44, 
	0x42, 0x30, 0x32, 0x34, 0x34, 0x23, 0x42, 0x34, 0x34, 0x44, 0x33, 0x22, 0x32, 0x03, 0x00, 0x44, 
	0x42, 0x30, 0x34, 0x34, 0x43, 0x23, 0x33, 0x34, 0x00, 0x04, 0x04, 0x43, 0x43, 0x03, 0x00, 0x44, 
	0x44, 0x44, 0x34, 0x34, 0x44, 0x00, 0x03, 0x30, 0x40, 0x04, 0x00, 0x04, 0x00, 0x40, 0x00, 0x44, 
	0x00, 0x00, 0x44, 0x43, 0x23, 0x22, 0x21, 0x11, 0x11, 0x11, 0x22, 0x12, 0x33, 0x34, 0x40, 0x00, 
	0x00, 0x44, 0x44, 0x33, 0x21, 0x22, 0x12, 0x12, 0x22, 0x22, 0x11, 0x12, 0x22, 0x23, 0x34, 0x00, 
	0x40, 0x44, 0x44, 0x33, 0x21, 0x22, 0x22, 0x12, 0x33, 0x22, 0x22, 0x12, 0x22, 0x23, 0x34, 0x04, 
	0x40, 0x44, 0x44, 0x33, 0x22, 0x22, 0x23, 0x12, 0x33, 0x33, 0x32, 0x12, 0x22, 0x23, 0x34, 0x04, 
	0x43, 0x22, 0x24, 0x33, 0x22, 0x22, 0x33, 0x44, 0x33, 0x44, 0x32, 0x32, 0x22, 0x32, 0x44, 0x44, 
	0x43, 0x24, 0x24, 0x33, 0x33, 0x22, 0x34, 0x44, 0x43, 0x44, 0x34, 0x33, 0x22, 0x32, 0x44, 0x44, 
	0x44, 0x44, 0x44, 0x44, 0x33, 0x33, 0x40, 0x04, 0x03, 0x44, 0x34, 0x44, 0x33, 0x44, 0x44, 0x00, 
	0x40, 0x04, 0x44, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x34, 0x44, 0x44, 0x00, 
	0x00, 0x00, 0x43, 0x32, 0x22, 0x22, 0x21, 0x11, 0x11, 0x11, 0x12, 0x22, 0x33, 0x44, 0x40, 0x44, 
	0x40, 0x04, 0x44, 0x23, 0x21, 0x22, 0x12, 0x22, 0x22, 0x12, 0x21, 0x12, 0x22, 0x23, 0x34, 0x44, 
	0x40, 0x44, 0x44, 0x23, 0x21, 0x22, 0x22, 0x23, 0x22, 0x22, 0x21, 0x12, 0x22, 0x23, 0x34, 0x44, 
	0x40, 0x44, 0x44, 0x22, 0x22, 0x32, 0x32, 0x33, 0x33, 0x33, 0x31, 0x32, 0x22, 0x23, 0x34, 0x34, 
	0x42, 0x44, 0x42, 0x42, 0x22, 0x32, 0x42, 0x44, 0x04, 0x33, 0x33, 0x32, 0x22, 0x23, 0x34, 0x32, 
	0x42, 0x43, 0x42, 0x42, 0x32, 0x34, 0x42, 0x43, 0x04, 0x44, 0x34, 0x33, 0x32, 0x23, 0x34, 0x42, 
	0x44, 0x44, 0x42, 0x44, 0x34, 0x34, 0x44, 0x40, 0x04, 0x44, 0x44, 0x34, 0x43, 0x43, 0x44, 0x40, 
	0x44, 0x04, 0x44, 0x44, 0x44, 0x00, 0x04, 0x40, 0x00, 0x00, 0x44, 0x00, 0x04, 0x44, 0x44, 0x40, 
	0x00, 0x00, 0x44, 0x34, 0x33, 0x21, 0x12, 0x11, 0x11, 0x11, 0x11, 0x12, 0x23, 0x43, 0x40, 0x34, 
	0x00, 0x04, 0x43, 0x33, 0x22, 0x21, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x23, 0x34, 0x44, 
	0x40, 0x04, 0x43, 0x33, 0x22, 0x22, 0x22, 0x32, 0x22, 0x22, 0x32, 0x22, 0x22, 0x23, 0x34, 0x44, 
	0x40, 0x34, 0x43, 0x33, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x32, 0x32, 0x22, 0x23, 0x34, 0x44, 
	0x30, 0x34, 0x43, 0x33, 0x22, 0x33, 0x24, 0x44, 0x34, 0x44, 0x42, 0x32, 0x22, 0x23, 0x34, 0x44, 
	0x30, 0x34, 0x44, 0x33, 0x32, 0x34, 0x24, 0x44, 0x34, 0x43, 0x42, 0x32, 0x32, 0x33, 0x34, 0x04, 
	0x40, 0x44, 0x44, 0x43, 0x34, 0x30, 0x44, 0x04, 0x30, 0x00, 0x40, 0x33, 0x44, 0x43, 0x30, 0x00, 
	0x44, 0x44, 0x44, 0x44, 0x04, 0x04, 0x04, 0x04, 0x30, 0x00, 0x00, 0x04, 0x40, 0x43, 0x44, 0x00, 
	0x00, 0x04, 0x43, 0x32, 0x22, 0x22, 0x21, 0x11, 0x11, 0x11, 0x12, 0x12, 0x33, 0x43, 0x40, 0x30, 
	0x00, 0x33, 0x34, 0x31, 0x21, 0x11, 0x12, 0x11, 0x22, 0x22, 0x21, 0x12, 0x21, 0x23, 0x24, 0x30, 
	0x00, 0x33, 0x34, 0x31, 0x21, 0x12, 0x22, 0x21, 0x33, 0x33, 0x32, 0x22, 0x21, 0x23, 0x24, 0x40, 
	0x03, 0x33, 0x34, 0x31, 0x22, 0x33, 0x23, 0x33, 0x44, 0x44, 0x32, 0x22, 0x21, 0x22, 0x24, 0x40, 
	0x34, 0x33, 0x32, 0x31, 0x22, 0x34, 0x33, 0x34, 0x44, 0x44, 0x44, 0x32, 0x21, 0x22, 0x24, 0x40, 
	0x34, 0x33, 0x32, 0x31, 0x24, 0x44, 0x33, 0x40, 0x00, 0x44, 0x34, 0x32, 0x31, 0x32, 0x24, 0x40, 
	0x34, 0x33, 0x44, 0x33, 0x34, 0x30, 0x03, 0x00, 0x00, 0x44, 0x04, 0x43, 0x33, 0x32, 0x40, 0x00, 
	0x34, 0x33, 0x44, 0x44, 0x30, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x44, 0x44, 0x40, 
	0x00, 0x00, 0x44, 0x42, 0x33, 0x22, 0x12, 0x11, 0x11, 0x11, 0x12, 0x22, 0x32, 0x33, 0x34, 0x30, 
	0x03, 0x04, 0x44, 0x32, 0x22, 0x22, 0x12, 0x12, 0x22, 0x22, 0x22, 0x22, 0x22, 0x23, 0x33, 0x44, 
	0x03, 0x04, 0x43, 0x32, 0x22, 0x22, 0x12, 0x33, 0x33, 0x32, 0x32, 0x22, 0x22, 0x23, 0x33, 0x44, 
	0x03, 0x04, 0x42, 0x32, 0x22, 0x23, 0x13, 0x33, 0x34, 0x34, 0x32, 0x22, 0x22, 0x23, 0x33, 0x43, 
	0x04, 0x04, 0x42, 0x22, 0x22, 0x23, 0x34, 0x44, 0x44, 0x04, 0x44, 0x23, 0x23, 0x24, 0x33, 0x42, 
	0x04, 0x04, 0x42, 0x22, 0x32, 0x23, 0x34, 0x40, 0x40, 0x04, 0x43, 0x23, 0x33, 0x34, 0x33, 0x42, 
	0x04, 0x04, 0x44, 0x44, 0x44, 0x30, 0x00, 0x00, 0x00, 0x04, 0x44, 0x24, 0x33, 0x44, 0x33, 0x00, 
	0x04, 0x04, 0x40, 0x44, 0x00, 0x00, 0x04, 0x00, 0x00, 0x40, 0x00, 0x44, 0x00, 0x44, 0x44, 0x44, 
	0x00, 0x04, 0x34, 0x44, 0x43, 0x22, 0x12, 0x21, 0x11, 0x11, 0x12, 0x22, 0x32, 0x34, 0x30, 0x00, 
	0x00, 0x04, 0x33, 0x33, 0x22, 0x22, 0x21, 0x11, 0x22, 0x22, 0x22, 0x12, 0x22, 0x23, 0x34, 0x44, 
	0x00, 0x04, 0x33, 0x33, 0x22, 0x22, 0x23, 0x13, 0x33, 0x22, 0x22, 0x22, 0x22, 0x23, 0x34, 0x44, 
	0x00, 0x04, 0x33, 0x33, 0x22, 0x23, 0x33, 0x33, 0x43, 0x34, 0x32, 0x22, 0x32, 0x23, 0x34, 0x43, 
	0x34, 0x24, 0x33, 0x33, 0x23, 0x23, 0x43, 0x44, 0x44, 0x44, 0x42, 0x32, 0x32, 0x23, 0x24, 0x43, 
	0x34, 0x24, 0x33, 0x33, 0x33, 0x23, 0x44, 0x44, 0x44, 0x40, 0x42, 0x33, 0x32, 0x33, 0x20, 0x40, 
	0x34, 0x44, 0x34, 0x33, 0x34, 0x20, 0x04, 0x00, 0x04, 0x40, 0x00, 0x33, 0x34, 0x43, 0x20, 0x00, 
	0x04, 0x44, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x44, 0x03, 0x00, 0x40, 
	0x00, 0x00, 0x04, 0x44, 0x43, 0x33, 0x22, 0x11, 0x21, 0x11, 0x12, 0x32, 0x23, 0x24, 0x44, 0x40, 
	0x40, 0x04, 0x43, 0x22, 0x22, 0x21, 0x21, 0x11, 0x12, 0x22, 0x11, 0x21, 0x22, 0x23, 0x33, 0x40, 
	0x40, 0x44, 0x43, 0x23, 0x22, 0x21, 0x22, 0x21, 0x32, 0x32, 0x12, 0x21, 0x22, 0x23, 0x33, 0x40, 
	0x40, 0x44, 0x23, 0x22, 0x22, 0x23, 0x22, 0x23, 0x33, 0x33, 0x32, 0x31, 0x22, 0x23, 0x33, 0x40, 
	0x40, 0x44, 0x22, 0x22, 0x22, 0x23, 0x22, 0x44, 0x04, 0x43, 0x42, 0x33, 0x22, 0x23, 0x32, 0x30, 
	0x40, 0x44, 0x23, 0x32, 0x22, 0x43, 0x24, 0x00, 0x04, 0x34, 0x44, 0x33, 0x33, 0x20, 0x00, 0x30, 
	0x40, 0x44, 0x34, 0x43, 0x23, 0x34, 0x20, 0x00, 0x00, 0x34, 0x43, 0x33, 0x44, 0x44, 0x00, 0x00, 
	0x04, 0x44, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x34, 0x40, 0x30, 0x04, 0x04, 0x00, 0x00, 
	0x00, 0x04, 0x44, 0x44, 0x44, 0x32, 0x21, 0x22, 0x22, 0x11, 0x12, 0x33, 0x22, 0x34, 0x04, 0x00, 
	0x44, 0x44, 0x43, 0x43, 0x32, 0x22, 0x11, 0x12, 0x12, 0x21, 0x22, 0x12, 0x22, 0x33, 0x43, 0x44, 
	0x44, 0x44, 0x43, 0x33, 0x32, 0x22, 0x11, 0x12, 0x12, 0x22, 0x22, 0x12, 0x22, 0x33, 0x33, 0x34, 
	0x44, 0x44, 0x42, 0x33, 0x32, 0x22, 0x11, 0x32, 0x13, 0x33, 0x32, 0x12, 0x23, 0x23, 0x33, 0x34, 
	0x44, 0x44, 0x42, 0x32, 0x32, 0x23, 0x33, 0x34, 0x44, 0x44, 0x32, 0x12, 0x23, 0x22, 0x34, 0x34, 
	0x44, 0x44, 0x42, 0x32, 0x33, 0x33, 0x43, 0x40, 0x04, 0x00, 0x34, 0x33, 0x23, 0x22, 0x34, 0x44, 
	0x44, 0x44, 0x42, 0x34, 0x33, 0x34, 0x00, 0x44, 0x00, 0x04, 0x33, 0x30, 0x33, 0x44, 0x04, 0x00, 
	0x44, 0x04, 0x44, 0x44, 0x44, 0x40, 0x00, 0x00, 0x00, 0x04, 0x34, 0x00, 0x43, 0x40, 0x04, 0x00, 
	0x00, 0x00, 0x00, 0x43, 0x34, 0x43, 0x33, 0x22, 0x12, 0x22, 0x33, 0x32, 0x34, 0x40, 0x40, 0x00, 
	0x00, 0x00, 0x44, 0x22, 0x32, 0x22, 0x12, 0x12, 0x11, 0x11, 0x12, 0x22, 0x12, 0x23, 0x30, 0x40, 
	0x00, 0x00, 0x44, 0x32, 0x32, 0x22, 0x22, 0x12, 0x21, 0x11, 0x12, 0x22, 0x12, 0x23, 0x30, 0x40, 
	0x03, 0x33, 0x43, 0x32, 0x32, 0x22, 0x22, 0x12, 0x21, 0x33, 0x12, 0x22, 0x12, 0x23, 0x30, 0x40, 
	0x03, 0x33, 0x43, 0x32, 0x42, 0x22, 0x23, 0x34, 0x43, 0x43, 0x32, 0x22, 0x12, 0x24, 0x30, 0x43, 
	0x03, 0x43, 0x33, 0x32, 0x33, 0x34, 0x24, 0x04, 0x00, 0x44, 0x33, 0x33, 0x32, 0x04, 0x34, 0x43, 
	0x03, 0x43, 0x33, 0x44, 0x44, 0x34, 0x44, 0x04, 0x40, 0x03, 0x03, 0x33, 0x42, 0x04, 0x30, 0x43, 
	0x03, 0x43, 0x44, 0x44, 0x00, 0x40, 0x00, 0x00, 0x00, 0x04, 0x00, 0x40, 0x44, 0x40, 0x30, 0x40, 
	0x00, 0x00, 0x00, 0x44, 0x43, 0x43, 0x32, 0x33, 0x23, 0x11, 0x32, 0x24, 0x33, 0x44, 0x04, 0x00, 
	0x00, 0x40, 0x44, 0x23, 0x33, 0x22, 0x22, 0x21, 0x11, 0x11, 0x21, 0x12, 0x22, 0x23, 0x43, 0x04, 
	0x04, 0x40, 0x44, 0x23, 0x33, 0x22, 0x22, 0x22, 0x12, 0x11, 0x21, 0x22, 0x22, 0x23, 0x43, 0x34, 
	0x04, 0x40, 0x34, 0x22, 0x33, 0x22, 0x22, 0x22, 0x12, 0x11, 0x31, 0x22, 0x22, 0x23, 0x43, 0x33, 
	0x44, 0x40, 0x24, 0x22, 0x34, 0x24, 0x22, 0x33, 0x43, 0x41, 0x33, 0x22, 0x22, 0x23, 0x44, 0x43, 
	0x44, 0x40, 0x24, 0x22, 0x33, 0x23, 0x43, 0x34, 0x44, 0x43, 0x43, 0x22, 0x33, 0x23, 0x44, 0x43, 
	0x44, 0x40, 0x44, 0x44, 0x33, 0x24, 0x43, 0x44, 0x44, 0x03, 0x30, 0x33, 0x44, 0x44, 0x40, 0x40, 
	0x44, 0x00, 0x44, 0x44, 0x40, 0x40, 0x40, 0x04, 0x00, 0x00, 0x30, 0x30, 0x40, 0x44, 0x40, 0x40, 
	0x00, 0x00, 0x00, 0x43, 0x43, 0x33, 0x41, 0x22, 0x22, 0x33, 0x44, 0x43, 0x44, 0x40, 0x00, 0x00, 
	0x04, 0x40, 0x03, 0x43, 0x33, 0x32, 0x12, 0x22, 0x12, 0x11, 0x12, 0x11, 0x23, 0x34, 0x24, 0x04, 
	0x04, 0x40, 0x04, 0x43, 0x33, 0x32, 0x12, 0x22, 0x12, 0x11, 0x12, 0x11, 0x23, 0x34, 0x23, 0x04, 
	0x04, 0x40, 0x24, 0x33, 0x33, 0x32, 0x22, 0x22, 0x12, 0x13, 0x22, 0x11, 0x22, 0x34, 0x23, 0x04, 
	0x04, 0x42, 0x24, 0x33, 0x34, 0x42, 0x22, 0x33, 0x33, 0x33, 0x22, 0x21, 0x24, 0x34, 0x33, 0x04, 
	0x04, 0x42, 0x24, 0x33, 0x34, 0x42, 0x23, 0x33, 0x33, 0x33, 0x32, 0x33, 0x33, 0x33, 0x43, 0x00, 
	0x04, 0x42, 0x24, 0x44, 0x44, 0x33, 0x43, 0x43, 0x33, 0x33, 0x33, 0x34, 0x44, 0x40, 0x40, 0x00, 
	0x04, 0x44, 0x24, 0x04, 0x44, 0x04, 0x40, 0x40, 0x00, 0x00, 0x00, 0x30, 0x40, 0x44, 0x40, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x34, 0x33, 0x34, 0x33, 0x33, 0x33, 0x43, 0x44, 0x40, 0x40, 0x40, 0x00, 
	0x40, 0x40, 0x04, 0x32, 0x32, 0x32, 0x22, 0x22, 0x12, 0x22, 0x12, 0x23, 0x33, 0x40, 0x30, 0x00, 
	0x40, 0x40, 0x04, 0x32, 0x32, 0x32, 0x22, 0x22, 0x12, 0x22, 0x12, 0x23, 0x33, 0x40, 0x30, 0x00, 
	0x40, 0x43, 0x04, 0x23, 0x32, 0x32, 0x22, 0x22, 0x12, 0x22, 0x22, 0x22, 0x33, 0x40, 0x30, 0x00, 
	0x40, 0x44, 0x04, 0x23, 0x32, 0x22, 0x22, 0x22, 0x22, 0x23, 0x22, 0x22, 0x33, 0x34, 0x34, 0x40, 
	0x40, 0x44, 0x04, 0x23, 0x32, 0x22, 0x22, 0x22, 0x33, 0x23, 0x32, 0x32, 0x33, 0x30, 0x30, 0x40, 
	0x40, 0x44, 0x04, 0x44, 0x33, 0x44, 0x34, 0x33, 0x33, 0x24, 0x33, 0x44, 0x33, 0x40, 0x30, 0x00, 
	0x00, 0x44, 0x44, 0x40, 0x44, 0x44, 0x04, 0x00, 0x30, 0x40, 0x40, 0x40, 0x00, 0x40, 0x30, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x44, 0x44, 0x33, 0x33, 0x33, 0x43, 0x34, 0x44, 0x04, 0x44, 0x00, 0x00, 
	0x00, 0x40, 0x00, 0x44, 0x44, 0x33, 0x23, 0x12, 0x22, 0x22, 0x21, 0x23, 0x34, 0x24, 0x00, 0x00, 
	0x00, 0x40, 0x00, 0x44, 0x44, 0x33, 0x33, 0x12, 0x22, 0x22, 0x21, 0x23, 0x34, 0x23, 0x00, 0x00, 
	0x00, 0x40, 0x00, 0x44, 0x43, 0x33, 0x33, 0x12, 0x22, 0x22, 0x32, 0x23, 0x24, 0x23, 0x03, 0x00, 
	0x44, 0x40, 0x00, 0x44, 0x42, 0x33, 0x33, 0x22, 0x23, 0x22, 0x22, 0x23, 0x23, 0x24, 0x03, 0x00, 
	0x44, 0x40, 0x04, 0x44, 0x42, 0x33, 0x33, 0x32, 0x33, 0x32, 0x33, 0x23, 0x23, 0x20, 0x03, 0x00, 
	0x44, 0x40, 0x04, 0x44, 0x34, 0x43, 0x33, 0x33, 0x34, 0x33, 0x33, 0x24, 0x43, 0x40, 0x03, 0x00, 
	0x40, 0x00, 0x44, 0x44, 0x44, 0x04, 0x44, 0x44, 0x04, 0x44, 0x43, 0x04, 0x44, 0x00, 0x03, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x40, 0x34, 0x33, 0x44, 0x34, 0x44, 0x34, 0x40, 0x44, 0x40, 0x00, 0x00, 
	0x00, 0x40, 0x40, 0x04, 0x34, 0x33, 0x33, 0x23, 0x33, 0x33, 0x33, 0x32, 0x24, 0x30, 0x00, 0x00, 
	0x00, 0x40, 0x44, 0x04, 0x34, 0x33, 0x33, 0x23, 0x33, 0x33, 0x33, 0x32, 0x24, 0x30, 0x00, 0x00, 
	0x00, 0x40, 0x44, 0x04, 0x34, 0x33, 0x23, 0x23, 0x33, 0x33, 0x33, 0x22, 0x24, 0x30, 0x30, 0x00, 
	0x00, 0x40, 0x44, 0x04, 0x34, 0x33, 0x23, 0x22, 0x22, 0x23, 0x22, 0x22, 0x33, 0x30, 0x30, 0x00, 
	0x00, 0x40, 0x44, 0x04, 0x34, 0x33, 0x23, 0x33, 0x23, 0x33, 0x23, 0x33, 0x33, 0x30, 0x30, 0x00, 
	0x04, 0x44, 0x44, 0x04, 0x33, 0x43, 0x43, 0x33, 0x34, 0x33, 0x43, 0x43, 0x43, 0x30, 0x30, 0x00, 
	0x00, 0x44, 0x44, 0x44, 0x43, 0x44, 0x44, 0x44, 0x44, 0x40, 0x43, 0x44, 0x44, 0x30, 0x30, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x44, 0x34, 0x24, 0x33, 0x33, 0x32, 0x34, 0x24, 0x03, 0x40, 0x00, 0x00, 
	0x00, 0x44, 0x00, 0x40, 0x44, 0x34, 0x23, 0x33, 0x33, 0x32, 0x34, 0x24, 0x03, 0x33, 0x04, 0x00, 
	0x00, 0x44, 0x00, 0x42, 0x24, 0x34, 0x23, 0x33, 0x33, 0x32, 0x34, 0x24, 0x03, 0x33, 0x04, 0x00, 
	0x00, 0x44, 0x00, 0x42, 0x24, 0x34, 0x23, 0x32, 0x22, 0x22, 0x33, 0x33, 0x03, 0x34, 0x44, 0x00, 
	0x00, 0x44, 0x40, 0x42, 0x24, 0x34, 0x23, 0x32, 0x32, 0x23, 0x33, 0x33, 0x03, 0x34, 0x44, 0x00, 
	0x00, 0x44, 0x40, 0x32, 0x24, 0x44, 0x24, 0x34, 0x44, 0x23, 0x33, 0x34, 0x00, 0x34, 0x44, 0x00, 
	0x00, 0x44, 0x40, 0x34, 0x44, 0x44, 0x24, 0x34, 0x40, 0x40, 0x03, 0x04, 0x40, 0x34, 0x44, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x30, 0x04, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x44, 0x03, 0x43, 0x44, 0x34, 0x44, 0x32, 0x23, 0x44, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x44, 0x03, 0x43, 0x44, 0x34, 0x44, 0x33, 0x23, 0x44, 0x40, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x42, 0x03, 0x43, 0x44, 0x32, 0x44, 0x33, 0x23, 0x44, 0x40, 0x30, 0x00, 0x00, 
	0x04, 0x40, 0x04, 0x42, 0x02, 0x43, 0x44, 0x32, 0x44, 0x33, 0x23, 0x34, 0x40, 0x44, 0x44, 0x00, 
	0x04, 0x40, 0x04, 0x42, 0x02, 0x43, 0x44, 0x33, 0x44, 0x43, 0x33, 0x34, 0x40, 0x44, 0x44, 0x00, 
	0x04, 0x40, 0x44, 0x42, 0x32, 0x33, 0x44, 0x43, 0x34, 0x43, 0x43, 0x44, 0x44, 0x40, 0x44, 0x00, 
	0x04, 0x40, 0x44, 0x44, 0x44, 0x44, 0x00, 0x44, 0x44, 0x44, 0x40, 0x40, 0x40, 0x40, 0x44, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x40, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x34, 0x44, 0x43, 0x44, 0x34, 0x04, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x43, 0x34, 0x34, 0x04, 0x03, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x20, 0x03, 0x44, 0x33, 0x44, 0x43, 0x34, 0x34, 0x04, 0x03, 0x03, 0x00, 0x00, 
	0x04, 0x44, 0x00, 0x40, 0x23, 0x43, 0x33, 0x44, 0x42, 0x34, 0x32, 0x44, 0x04, 0x04, 0x00, 0x00, 
	0x04, 0x44, 0x00, 0x44, 0x23, 0x44, 0x33, 0x44, 0x44, 0x34, 0x33, 0x04, 0x04, 0x04, 0x00, 0x00, 
	0x00, 0x44, 0x00, 0x44, 0x23, 0x44, 0x43, 0x44, 0x44, 0x44, 0x34, 0x44, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x44, 0x00, 0x44, 0x44, 0x44, 0x40, 0x44, 0x44, 0x40, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x43, 0x00, 0x34, 0x03, 0x40, 0x40, 0x40, 0x44, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x43, 0x00, 0x34, 0x03, 0x40, 0x43, 0x40, 0x44, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x43, 0x30, 0x34, 0x03, 0x40, 0x43, 0x40, 0x44, 0x00, 0x00, 
	0x00, 0x44, 0x00, 0x04, 0x00, 0x03, 0x43, 0x30, 0x34, 0x33, 0x40, 0x44, 0x44, 0x44, 0x00, 0x00, 
	0x00, 0x44, 0x00, 0x04, 0x04, 0x43, 0x43, 0x30, 0x34, 0x33, 0x00, 0x44, 0x44, 0x40, 0x00, 0x00, 
	0x00, 0x40, 0x00, 0x04, 0x44, 0x43, 0x44, 0x44, 0x34, 0x03, 0x04, 0x44, 0x44, 0x40, 0x00, 0x00, 
	0x00, 0x40, 0x00, 0x44, 0x44, 0x43, 0x40, 0x44, 0x44, 0x44, 0x04, 0x44, 0x44, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x03, 0x03, 0x40, 0x04, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x40, 0x03, 0x03, 0x40, 0x04, 0x30, 0x40, 0x00, 0x04, 0x40, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x40, 0x03, 0x03, 0x40, 0x04, 0x33, 0x40, 0x03, 0x04, 0x40, 0x00, 
	0x00, 0x04, 0x00, 0x04, 0x04, 0x40, 0x23, 0x23, 0x40, 0x22, 0x33, 0x40, 0x03, 0x44, 0x40, 0x00, 
	0x00, 0x04, 0x00, 0x04, 0x04, 0x40, 0x23, 0x43, 0x40, 0x22, 0x33, 0x40, 0x03, 0x40, 0x00, 0x00, 
	0x00, 0x04, 0x00, 0x04, 0x04, 0x40, 0x43, 0x40, 0x40, 0x22, 0x33, 0x40, 0x00, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x04, 0x40, 0x43, 0x40, 0x40, 0x24, 0x40, 0x04, 0x00, 0x40, 0x00, 0x00, 
};