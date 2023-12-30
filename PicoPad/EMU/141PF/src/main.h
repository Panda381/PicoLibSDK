
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// format: 1-bit pixel graphics
// image width: 2048 pixels
// image height: 16/14/13/12 lines
// image pitch: 256 bytes
extern const u8 Font141PF8x16[4096] __attribute__ ((aligned(4)));
extern const u8 Font141PF8x14[3584] __attribute__ ((aligned(4)));
extern const u8 Font141PF8x13[3328] __attribute__ ((aligned(4)));
extern const u8 Font141PF8x12[3072] __attribute__ ((aligned(4)));

extern const u8 busicom_Prog[];
extern const int busicom_ProgLen; // = 1195;

// === Keyboard
// format: 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 128 lines
// image pitch: 320 bytes
extern const u16 KeybUpImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KeybUpImg[40960] __attribute__ ((aligned(4)));
extern const u16 KeybDnImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 KeybDnImg[40960] __attribute__ ((aligned(4)));

#define KEYB_W		32	// width of one key
#define KEYB_H		32	// height of one key
#define KEYB_WALL	320	// width of keyboard
#define KEYB_HALL	128	// height of keyboard
#define KEYB_X		0	// keyboard X coordinate
#define KEYB_Y		(HEIGHT-KEYB_HALL) // keyboard Y coordinate (= 112)
#define KEYB_FRAMECOL	COL_GREEN // key frame color
#define KEYB_HELP	COL_GREEN // key help on dark
#define KEYB_HELP2	COL_RED // key help on light

// === Light panel
// format: 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 48 lines
// image pitch: 320 bytes
extern const u16 LightOffImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 LightOffImg[15360] __attribute__ ((aligned(4)));
extern const u16 LightOnImg_Pal[255] __attribute__ ((aligned(4)));
extern const u8 LightOnImg[15360] __attribute__ ((aligned(4)));

#define LIGHT_WALL	320	// width of light panel
#define LIGHT_HALL	48	// height of light panel
#define LIGHT_X		0	// light X coordinate
#define LIGHT_Y		(KEYB_Y - LIGHT_HALL) // light Y coordinate (= 64)

#define LAMP_Y		(LIGHT_Y+4) // Y coordinate of lamps
#define LAMP_W		16	// width of lamp
#define LAMP_H		28	// height of lamp
#define LAMPOVF_X	232	// X coordinate of OVF lamp
#define LAMPNEG_X	256	// X coordinate of NEG lamp
#define LAMPM_X		280	// X coordinate of M lamp

#define ADV_W		34	// advance button width
#define ADV_H		31	// advance button height
#define ADV_X		0	// advance button X coordinate
#define ADV_Y		LIGHT_Y	// advance button Y coordinate

#define DP_W		88	// width of decimal point switch
#define DP_H		16	// height of decimal point switch
#define DP_X		48	// X coordinate of decimal point switch
#define DP_Y		(LIGHT_Y+4) // Y coordinate of decimal point switch

#define SW_W		64	// width of rounding switch
#define SW_H		16	// height of rounding switch
#define SW_X		152	// X coordinate of rounding switch
#define SW_Y		(LIGHT_Y+4) // Y coordinate of rounding switch

#define SLIDE_W		16	// slider width
#define SLIDE_H		12	// slider height
#define SLIDE_X		56	// slider source X coordinate
#define SLIDE_Y		4	// slider source Y coordinate

// === Paper

#define DIG_FONTW	8		// width of digit
#define DIG_FONTH	13		// height of digit
#define DIG_NUM		18		// number of printed digits
#define PRINT_WIDTH	(DIG_NUM*DIG_FONTW) // printed width (18*8 = 144)
#define PRINT_X		((WIDTH - PRINT_WIDTH)/2) // printed start X coordinate (= 88)

#define PAPER_MARGIN	16		// paper margin
#define PAPER_WIDTH	(PRINT_WIDTH + 2*PAPER_MARGIN) // paper width (= 176)
#define PAPER_HEIGHT	LIGHT_Y		// paper height (= 64)
#define PAPER_Y		0		// paper Y position
#define PAPER_X		(PRINT_X - PAPER_MARGIN) // paper X position (= 72)

#define PRINT_Y		(PAPER_Y+PAPER_HEIGHT-DIG_FONTH) // printed start Y coordinate

#define PAPER_COLOR	COL_WHITE	// paper color
#define PRINT_COLOR	COL_BLACK	// print color
#define PRINT_COLOR2	COL_RED		// print negative color

// === Logo 1
// format: 4-bit paletted pixel graphics
// image width: 64 pixels
// image height: 64 lines
// image pitch: 32 bytes
extern const u16 Logo1Img_Pal[16] __attribute__ ((aligned(4)));
extern const u8 Logo1Img[2048] __attribute__ ((aligned(4)));

#define LOGO1_W		64		// logo 1 width
#define LOGO1_H		64		// logo 1 height
#define LOGO1_X		0		// logo 1 X coordinate
#define LOGO1_Y		0		// logo 1 Y coordinate

// === Logo 2
// format: 4-bit paletted pixel graphics
// image width: 64 pixels
// image height: 64 lines
// image pitch: 32 bytes
extern const u16 Logo2Img_Pal[16] __attribute__ ((aligned(4)));
extern const u8 Logo2Img[2048] __attribute__ ((aligned(4)));

#define LOGO2_W		64		// logo 2 width
#define LOGO2_H		64		// logo 2 height
#define LOGO2_X		(WIDTH - LOGO2_W) // logo 2 X coordinate
#define LOGO2_Y		0		// logo 2 Y coordinate

// sound format: PCM mono 8-bit 22050Hz
extern const u8 PrintSnd[4794];

// sound format: PCM mono 8-bit 22050Hz
extern const u8 IdleSnd[69785];

// sound format: PCM mono 8-bit 22050Hz
extern const u8 ClickSnd[882];

#endif // _MAIN_H
