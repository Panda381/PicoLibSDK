
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// screen resolution

#define IMGWIDTH 320	// image width
#define IMGHEIGHT 208	// image height
//#define IMGWIDTHBYTE WIDTH // image width in bytes
#define BARWIDTH 12	// bar width
#define ROWLEN	38	// text row length
#define TXTHEIGHT (HEIGHT - IMGHEIGHT) // height of text field
#define TEXTROWS 4	// number of text rows

#define PAGES 22	// number of pages

// format: 8-bit pixel graphics
// image width: 320 pixels
// image height: 208 lines
// image pitch: 320 bytes
extern const u8 Img01[66560] __attribute__ ((aligned(4)));
extern const u8 Img02[66560] __attribute__ ((aligned(4)));
extern const u8 Img03[66560] __attribute__ ((aligned(4)));
extern const u8 Img04[66560] __attribute__ ((aligned(4)));
extern const u8 Img05[66560] __attribute__ ((aligned(4)));
extern const u8 Img06[66560] __attribute__ ((aligned(4)));
extern const u8 Img07[66560] __attribute__ ((aligned(4)));
extern const u8 Img08[66560] __attribute__ ((aligned(4)));
extern const u8 Img09[66560] __attribute__ ((aligned(4)));
extern const u8 Img10[66560] __attribute__ ((aligned(4)));
extern const u8 Img11[66560] __attribute__ ((aligned(4)));
extern const u8 Img12[66560] __attribute__ ((aligned(4)));
extern const u8 Img13[66560] __attribute__ ((aligned(4)));
extern const u8 Img14[66560] __attribute__ ((aligned(4)));
extern const u8 Img15[66560] __attribute__ ((aligned(4)));
extern const u8 Img16[66560] __attribute__ ((aligned(4)));
extern const u8 Img17[66560] __attribute__ ((aligned(4)));
extern const u8 Img18[66560] __attribute__ ((aligned(4)));
extern const u8 Img19[66560] __attribute__ ((aligned(4)));
extern const u8 Img20[66560] __attribute__ ((aligned(4)));
extern const u8 Img22[66560] __attribute__ ((aligned(4)));

#endif // _MAIN_H
