
// ****************************************************************************
//
//                               Title screen
//
// ****************************************************************************

// Title screen
// format: compressed 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 240 lines
// image pitch: 320 bytes
extern const u16 TitleImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 TitleImg[23560] __attribute__ ((aligned(4)));

// title screen (returns False to exit)
Bool TitleScreen();
