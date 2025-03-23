PicoPadImg version 2.0, (c) 2025 Miroslav Nemecek
Image converter to PicoPad formats.

Input image must be in Windows BMP format, in 24-bit, 15-bit (save as "16-bit" format),
8-bit, 4-bit or 1-bit color format. Switch OFF compression, switch ON "flip row order".

Syntax: input.bmp output.cpp name format [test.bmp]
  'input.bmp' input image in BMP format
  'output.cpp' output file as C++ source
  'name' name of data array
  'format' specifies required output format:
       1 ... 1-bit Y1 (8 pixels per byte), requires 1-bit input image
       2 ... 2-bit Y2 (4 pixels per byte), requires 4-bit input image with 4 palettes
       3 ... 3-bit RGB111 (10 pixels per 32-bit), requires 4-bit input image with 8 palettes
       4 ... 4-bit YRGB1111 (2 pixels per byte), requires 4-bit input image
       6 ... 6-bit RGB222 (5 pixels per 32-bit), requires 8-bit input image
       8 ... 8-bit RGB332 (1 pixel per byte), requires 8-bit input image
       12 ... 12-bit RGB444 (8 pixels per three 32-bit), requires 15-bit input image
       12d ... 12-bit RGB444 with dithering, requires 15-bit input image
       15 ... 15-bit RGB555 (1 pixel per 2 bytes), requires 15-bit input image
       16 ... 16-bit RGB565 (1 pixel per 2 bytes), requires 24-bit input image
       r4 ... 4-bit compression RLE4, requires 4-bit input image
       r8 ... 8-bit compression RLE8, requires 8-bit input image
       x15 ... 15-bit HSTX compression, requires 15-bit input image
       x16 ... 16-bit HSTX compression, requires 24-bit input image
       a1 ... attribute compression 1, cell 8x8, 2 colors, requires 4-bit input image
       a2 ... attribute compression 2, cell 4x4, 2 colors, requires 4-bit input image
       a3 ... attribute compression 3, cell 8x8, 4 colors, requires 4-bit input image
       a4 ... attribute compression 4, cell 4x4, 4 colors, requires 4-bit input image
       a5 ... attribute compression 5, cell 8x8, 2 colors, requires 8-bit input image
       a6 ... attribute compression 6, cell 4x4, 2 colors, requires 8-bit input image
       a7 ... attribute compression 7, cell 8x8, 4 colors, requires 8-bit input image
       a8 ... attribute compression 8, cell 4x4, 4 colors, requires 8-bit input image
  'test.bmp' output test image of attribute compression
