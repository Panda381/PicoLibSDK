
// ****************************************************************************
//
//                             Display Commons
//
// ****************************************************************************

// blend 2 colors together
u16 Col16Blend2(u16 col1, u16 col2);

// blend 4 colors together
u16 Col16Blend4(u16 col1, u16 col2, u16 col3, u16 col4);

// Draw 8-bit paletted image with half X and Y resolution
//  src ... image data
//  pal ... palettes
//  xs ... source X*2 (can be odd)
//  ys ... source Y*2 (cannot be odd)
//  xd ... destination X
//  yd ... destination Y
//  w .... width*2
//  h .... height*2
//  ws ... source total width*2 (in pixels)
void DrawImgPalDbl(const u8* src, const u16* pal, int xs, int ys, int xd, int yd, int w, int h, int ws);

// shadow screen
void DispShadow();
