
// ****************************************************************************
//
//                                 Tables
//
// ****************************************************************************

// current palette (format RGB565)
extern u16 gbPalette[64];

// Title checksums that are treated specially by the CGB boot ROM
// 14 indexes above 0x40 (64) have to be disambiguated.
extern const u8 gbColorizationChecksums[79];

// For titles with colliding checksums, the fourth character of the game title
// for disambiguation.
extern const u8 gbColorizationDisambigChars[29];

// Palette ID | (Flags << 5)
extern const u8 gbColorizationPaletteInfo[94];

// Uncompressed palette data from the CGB boot ROM (format BGR555)
#define PALDATA_NUM 44
extern const u16 gbColorizationPaletteData[PALDATA_NUM][3][4];

// palette manual selection
//  flag 5 = normal state, flag 7 = use index 0x20..
#define PAL_NUM 60
extern const u8 gbColorPalSel[PAL_NUM];

// extra palette setup
extern const u16 gbPalExt[];
