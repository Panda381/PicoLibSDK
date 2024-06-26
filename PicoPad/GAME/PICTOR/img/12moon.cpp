#include "../include.h"

// format: 8-bit paletted pixel graphics
// image width: 35 pixels
// image height: 36 lines
// image pitch: 35 bytes
const u16 MoonImg_Pal[240] __attribute__ ((aligned(4))) = {
	0xF81F, 0xFFFF, 0xFFFF, 0xFFDF, 0xFFDF, 0xF7BE, 0xF7BE, 0xF7BE, 0xF7BE, 0xF79E, 0xF79E, 0xF79E, 0xEF7D, 0xEF7D, 0xEF5D, 0xEF5D, 
	0xEF5D, 0xE73C, 0xE73C, 0xE73C, 0xE73C, 0xE71C, 0xE71C, 0xE71C, 0xE71C, 0xDEFB, 0xDEFB, 0xDEFB, 0xDEFB, 0xDEDB, 0xDEDB, 0xDEDB, 
	0xDEDB, 0xD6BA, 0xD6BA, 0xD6BA, 0xD6BA, 0xD69A, 0xD69A, 0xD69A, 0xD69A, 0xCE79, 0xCE79, 0xCE79, 0xCE79, 0xCE59, 0xCE59, 0xCE59, 
	0xCE59, 0xC638, 0xC638, 0xC638, 0xC638, 0xC618, 0xC618, 0xC618, 0xC618, 0xBDF7, 0xBDF7, 0xBDF7, 0xBDF7, 0xBDD7, 0xBDD7, 0xBDD7, 
	0xBDD7, 0xB5B6, 0xB5B6, 0xB5B6, 0xB5B6, 0xB596, 0xB596, 0xB596, 0xB596, 0xAD75, 0xAD75, 0xAD75, 0xAD75, 0xAD55, 0xAD55, 0xAD55, 
	0xAD55, 0xA534, 0xA534, 0xA534, 0xA534, 0xA514, 0xA514, 0xA514, 0xA514, 0x9CF3, 0x9CF3, 0x9CF3, 0x9CF3, 0x9CD3, 0x9CD3, 0x9CD3, 
	0x9CD3, 0x94B2, 0x94B2, 0x94B2, 0x94B2, 0x9492, 0x9492, 0x9492, 0x9492, 0x8C71, 0x8C71, 0x8C71, 0x8C71, 0x8C51, 0x8C51, 0x8C51, 
	0x8430, 0x8430, 0x8430, 0x8430, 0x8410, 0x8410, 0x8410, 0x8410, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BCF, 0x7BCF, 0x7BCF, 0x7BCF, 
	0x73AE, 0x73AE, 0x73AE, 0x73AE, 0x738E, 0x738E, 0x738E, 0x738E, 0x6B6D, 0x6B6D, 0x6B6D, 0x6B4D, 0x6B4D, 0x6B4D, 0x6B4D, 0x632C, 
	0x632C, 0x632C, 0x632C, 0x630C, 0x630C, 0x630C, 0x630C, 0x5AEB, 0x5AEB, 0x5AEB, 0x5AEB, 0x5ACB, 0x5ACB, 0x5ACB, 0x5ACB, 0x52AA, 
	0x52AA, 0x52AA, 0x52AA, 0x528A, 0x528A, 0x528A, 0x4A69, 0x4A69, 0x4A69, 0x4A69, 0x4A49, 0x4A49, 0x4A49, 0x4228, 0x4228, 0x4228, 
	0x4228, 0x4208, 0x4208, 0x4208, 0x4208, 0x39E7, 0x39E7, 0x39E7, 0x39E7, 0x39C7, 0x39C7, 0x39C7, 0x39C7, 0x31A6, 0x31A6, 0x31A6, 
	0x3186, 0x3186, 0x3186, 0x3186, 0x2965, 0x2965, 0x2965, 0x2965, 0x2945, 0x2945, 0x2945, 0x2945, 0x2124, 0x2124, 0x2124, 0x2104, 
	0x2104, 0x2104, 0x2104, 0x18E3, 0x18E3, 0x18E3, 0x18E3, 0x18C3, 0x18C3, 0x18C3, 0x18C3, 0x10A2, 0x10A2, 0x10A2, 0x10A2, 0x1082, 
	0x1082, 0x1082, 0x1082, 0x0861, 0x0861, 0x0861, 0x0841, 0x0841, 0x0841, 0x0841, 0x0020, 0x0020, 0x0020, 0x0020, 0x0000, 0x0000, 
};

const u8 MoonImg[1260] __attribute__ ((aligned(4))) = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x2B, 0x26, 0x25, 0x2B, 0x34, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x2F, 0x33, 0x24, 0x0B, 0x19, 0x15, 0x0C, 0x19, 0x33, 0x3C, 0x34, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x33, 0x2A, 0x11, 0x47, 0x01, 0x01, 0x0B, 0x15, 0x04, 0x01, 0x01, 0x03, 
	0x0D, 0x25, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x02, 0x01, 0x0A, 0x01, 0x33, 0x76, 0x45, 0x19, 0x16, 
	0x15, 0x17, 0x15, 0x11, 0x04, 0x01, 0x2C, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3A, 0x06, 0x06, 0x16, 0x15, 0x03, 0x76, 0x9A, 
	0x59, 0x60, 0x28, 0x29, 0x19, 0x13, 0x16, 0x22, 0x2B, 0x23, 0x34, 0x4E, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x06, 0x01, 0x3C, 0x26, 0x14, 0x0F, 
	0x20, 0x25, 0x1A, 0x4D, 0x45, 0x20, 0x34, 0x21, 0x2B, 0x28, 0x23, 0x32, 0x68, 0x87, 0x19, 0x48, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x94, 0x82, 0x68, 0x42, 0x06, 0x0F, 0x7C, 0x35, 
	0x38, 0x21, 0x24, 0x65, 0x2D, 0x20, 0x34, 0x22, 0x21, 0x1E, 0x16, 0x2B, 0x59, 0x4C, 0x21, 0x49, 
	0x3C, 0x14, 0x01, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0xB3, 0xCE, 0xB3, 0x93, 0x5B, 0x0F, 
	0x15, 0x9C, 0x34, 0x29, 0x4A, 0x3C, 0x28, 0x1A, 0x23, 0x1D, 0x22, 0x1C, 0x16, 0x24, 0x40, 0x73, 
	0x57, 0x1B, 0x33, 0x53, 0x2B, 0x25, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x93, 0xBB, 0xB4, 0xCA, 
	0x7B, 0xA3, 0x89, 0x48, 0x0E, 0x09, 0x38, 0x36, 0x2E, 0x26, 0x34, 0x32, 0x23, 0x28, 0x23, 0x20, 
	0x33, 0x26, 0x1D, 0x2D, 0x34, 0x4C, 0x32, 0x10, 0x54, 0xA6, 0x4F, 0x33, 0x00, 0x00, 0x37, 0xBD, 
	0xCA, 0xAC, 0x97, 0x88, 0xC4, 0xAE, 0xA1, 0x8C, 0x5A, 0x36, 0x3B, 0x43, 0x2E, 0x3C, 0x31, 0x39, 
	0x32, 0x3D, 0x53, 0x59, 0x5D, 0x68, 0x49, 0x3A, 0x55, 0x35, 0x53, 0x90, 0xC5, 0x44, 0x33, 0x00, 
	0x00, 0x5E, 0xAF, 0xC0, 0xC9, 0x9B, 0x9C, 0xCD, 0xEF, 0xE1, 0xC2, 0xBC, 0x9E, 0x9B, 0x2D, 0x29, 
	0x39, 0x3E, 0x4D, 0x4F, 0x5D, 0x67, 0x6A, 0x7D, 0x75, 0x5B, 0x3D, 0x4D, 0x3F, 0x4F, 0x57, 0x2C, 
	0x08, 0x1E, 0x33, 0x00, 0x78, 0xB9, 0xA9, 0xC1, 0x97, 0xBA, 0xDD, 0xE1, 0xDE, 0xE1, 0xC9, 0xCD, 
	0xD0, 0x9F, 0x4D, 0x10, 0x2A, 0x48, 0x3F, 0x35, 0x40, 0x28, 0x2A, 0x62, 0x53, 0x35, 0x63, 0x63, 
	0x5E, 0x48, 0x1F, 0x3B, 0x23, 0x37, 0x00, 0x76, 0xDD, 0xC5, 0xA8, 0xBE, 0xCC, 0xDF, 0xE4, 0xD5, 
	0xDA, 0xE2, 0xDF, 0xC3, 0xBD, 0x84, 0x95, 0x41, 0x29, 0x47, 0x3D, 0x3D, 0x4F, 0xAB, 0xE7, 0x87, 
	0x38, 0x33, 0x45, 0x86, 0x46, 0x6C, 0xD8, 0x94, 0x28, 0x35, 0x8F, 0xD3, 0xB6, 0xB2, 0xDA, 0xE6, 
	0xDC, 0xDF, 0xDE, 0xDD, 0xDD, 0xED, 0xE0, 0xB9, 0x9B, 0x94, 0x63, 0x7B, 0x64, 0x4F, 0x68, 0xCD, 
	0xD1, 0x8D, 0x8F, 0x5D, 0x37, 0x49, 0x3F, 0x57, 0xD7, 0xBB, 0x90, 0x4A, 0x44, 0x4B, 0xC0, 0xB8, 
	0xDD, 0xE3, 0xDB, 0xDC, 0xE0, 0xDE, 0xE1, 0xEA, 0xBD, 0xA4, 0xDD, 0xC7, 0xA2, 0x81, 0xA3, 0xA0, 
	0x72, 0xD9, 0xEF, 0x82, 0x4E, 0x62, 0x60, 0x51, 0x43, 0x25, 0x6F, 0xBD, 0x90, 0x92, 0x45, 0x3F, 
	0x00, 0x46, 0x95, 0xBA, 0xD7, 0xDE, 0xDF, 0xDF, 0xDB, 0xE2, 0xEA, 0xCC, 0xBE, 0xEF, 0xD0, 0x6F, 
	0x79, 0x61, 0x5B, 0x86, 0xE8, 0xB1, 0x99, 0x9C, 0xA2, 0x71, 0x4D, 0x4E, 0x3F, 0x27, 0x70, 0x5C, 
	0x1E, 0x35, 0x4F, 0x00, 0x00, 0x55, 0x09, 0xAF, 0xEF, 0xD5, 0xDE, 0xE3, 0xDD, 0xE1, 0xEA, 0xEF, 
	0xD2, 0x9B, 0x57, 0x35, 0x28, 0x25, 0x53, 0xC3, 0xE5, 0xDF, 0xEF, 0xE9, 0x75, 0x4A, 0xA2, 0xD2, 
	0x4F, 0x0B, 0x15, 0x33, 0x76, 0x5C, 0x00, 0x00, 0x00, 0x84, 0xEE, 0xCE, 0xE1, 0xC3, 0xC3, 0xE7, 
	0xE3, 0xE6, 0xEB, 0x64, 0x51, 0x42, 0x1F, 0x2F, 0x30, 0x24, 0x4D, 0xA5, 0xEF, 0xEF, 0xD6, 0x76, 
	0x51, 0xAB, 0xAA, 0xA7, 0x4F, 0x05, 0x4A, 0x89, 0x5E, 0x00, 0x00, 0x23, 0xA2, 0xE9, 0xB2, 0xE5, 
	0x9C, 0xC2, 0xE8, 0xE0, 0xE1, 0xCC, 0x72, 0x4C, 0x4C, 0x3A, 0x21, 0x2B, 0x47, 0x49, 0x40, 0x70, 
	0x97, 0x67, 0x53, 0x5A, 0x46, 0x1C, 0x9F, 0xB2, 0x7A, 0xA1, 0x80, 0x66, 0x00, 0x00, 0x2F, 0x7F, 
	0xC9, 0x8A, 0x74, 0x7C, 0xE2, 0xEA, 0xD7, 0xD3, 0x8C, 0x9C, 0xAF, 0xB0, 0x61, 0x1D, 0x45, 0x54, 
	0x52, 0x43, 0x13, 0x25, 0x36, 0x47, 0x2E, 0x53, 0x6F, 0x37, 0xB5, 0xEF, 0x8F, 0x5E, 0x69, 0x00, 
	0x00, 0x3A, 0x19, 0x37, 0x26, 0x68, 0xBC, 0xBF, 0x84, 0xA7, 0xEC, 0xA6, 0x63, 0x59, 0x4E, 0x81, 
	0x8B, 0x77, 0xD9, 0xA2, 0x9E, 0x64, 0x20, 0x46, 0x2A, 0x3F, 0x63, 0x3F, 0x57, 0xD0, 0xB8, 0x7F, 
	0x63, 0x48, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x95, 0xC5, 0xC6, 0x7C, 0x61, 0xAC, 0xD3, 0x7D, 0x6A, 
	0x6E, 0x5A, 0x68, 0xA9, 0xBB, 0xCC, 0xBE, 0x8C, 0xB8, 0x6E, 0x4C, 0x53, 0x51, 0x38, 0x16, 0x8B, 
	0xCD, 0x3A, 0x37, 0x7D, 0x00, 0x00, 0x00, 0x00, 0x27, 0x3C, 0xDA, 0xC7, 0x8F, 0x99, 0x7B, 0xA9, 
	0x7E, 0x68, 0x95, 0xA7, 0x4A, 0x0E, 0x2A, 0x54, 0x28, 0x35, 0x06, 0x7B, 0xE3, 0x65, 0x50, 0x6E, 
	0x61, 0x47, 0x4B, 0x7B, 0x5F, 0x7A, 0x63, 0x00, 0x00, 0x00, 0x00, 0x35, 0x19, 0x8F, 0xD4, 0xA8, 
	0x5F, 0x72, 0xA2, 0x71, 0x3E, 0x83, 0x57, 0x16, 0x42, 0xA6, 0x97, 0xA4, 0xA8, 0x94, 0x3A, 0x67, 
	0x87, 0x37, 0x39, 0x63, 0x84, 0x72, 0x2D, 0x81, 0xB0, 0x35, 0x00, 0x00, 0x00, 0x00, 0x38, 0x1D, 
	0x01, 0x22, 0x47, 0x6A, 0x63, 0x36, 0x3D, 0x55, 0x4B, 0x37, 0x73, 0xAB, 0xE4, 0xEF, 0xE4, 0xEC, 
	0xEF, 0xD2, 0x91, 0x44, 0x12, 0x25, 0x58, 0x58, 0x70, 0x78, 0x9F, 0x8A, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x52, 0x80, 0x85, 0x8E, 0x96, 0x22, 0x3F, 0x22, 0x39, 0x42, 0xA2, 0xD8, 0xB7, 0xA9, 
	0x9F, 0xBE, 0xCB, 0xE0, 0xEF, 0x7F, 0x11, 0x08, 0x31, 0x28, 0x68, 0x63, 0x14, 0x4C, 0x49, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0xA9, 0xAD, 0xB1, 0x7B, 0x23, 0x54, 0x41, 0x32, 0x6A, 
	0x95, 0xB0, 0xCF, 0xC8, 0xDD, 0xEB, 0xD2, 0x95, 0x6C, 0x2C, 0x32, 0x32, 0x33, 0xA4, 0x38, 0x53, 
	0x7D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x21, 0x40, 0x72, 0xA6, 0x65, 0x2C, 
	0x2B, 0x34, 0x7A, 0x79, 0x5E, 0x58, 0x4A, 0x76, 0x99, 0x7C, 0x4D, 0x38, 0x13, 0x69, 0xB7, 0x7E, 
	0x2A, 0x75, 0xE0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x01, 0x18, 
	0x56, 0x4D, 0x01, 0x29, 0x22, 0x12, 0x30, 0x52, 0x51, 0x54, 0x4F, 0x26, 0x17, 0x0C, 0x0B, 0x2C, 
	0x73, 0x97, 0x8C, 0x43, 0xAF, 0x5B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3E, 0x07, 0x08, 0x1B, 0x2B, 0x2D, 0x50, 0x47, 0x59, 0x56, 0x6C, 0xDD, 0xD1, 0x76, 0x40, 
	0x50, 0x28, 0x32, 0x6A, 0x6D, 0x49, 0x9A, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0x14, 0x2F, 0x61, 0x57, 0x31, 0x33, 0x60, 0x8C, 0x4F, 0x5D, 
	0x52, 0x54, 0x60, 0x58, 0x4B, 0xA5, 0xA8, 0x8C, 0x8E, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x2A, 0x5D, 0x9D, 0x8E, 0x54, 0x36, 
	0x34, 0x13, 0x1A, 0x36, 0x45, 0x27, 0x06, 0x81, 0xEB, 0xD6, 0x9E, 0x3D, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x5A, 0x6B, 
	0x64, 0x53, 0x5A, 0x1F, 0x4F, 0x86, 0x56, 0x92, 0x98, 0xA5, 0xDF, 0xC8, 0x71, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x39, 0x57, 0x71, 0x9C, 0x85, 0xBE, 0x9F, 0xAA, 0xBD, 0xC2, 0xC2, 0x93, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x65, 0x6F, 0x6F, 0x94, 0x6C, 0x77, 0x95, 0x8F, 0x72, 
	0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};
