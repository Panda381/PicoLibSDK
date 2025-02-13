#include "../include.h"

// format: 4-bit paletted pixel graphics
// image width: 10 pixels
// image height: 10 lines
// image pitch: 5 bytes
const u16 Bullet3Img_Pal[7] __attribute__ ((aligned(4))) = {
	0xF81F, 0x731F, 0xA4FF, 0xB55F, 0x005F, 0x031F, 0x0000, 
};

const u8 Bullet3Img[50] __attribute__ ((aligned(4))) = {
	0x00, 0x00, 0x20, 0x00, 0x00, 0x02, 0x05, 0x44, 0x50, 0x20, 0x00, 0x44, 0x11, 0x44, 0x00, 0x05, 
	0x41, 0x33, 0x14, 0x50, 0x24, 0x13, 0x33, 0x31, 0x42, 0x04, 0x13, 0x33, 0x31, 0x40, 0x05, 0x41, 
	0x33, 0x14, 0x50, 0x00, 0x44, 0x11, 0x44, 0x00, 0x02, 0x05, 0x44, 0x50, 0x20, 0x00, 0x00, 0x02, 
	0x00, 0x00, 
};
