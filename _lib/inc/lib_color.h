
// ****************************************************************************
//
//                                  Color
//                             RGBA Color Vector
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#if USE_COLOR	// use color vector (lib_color.c, lib_color.h)

#ifndef _LIB_COLOR_H
#define _LIB_COLOR_H

typedef float color_float;	// color element (use float or double)

// color vector (color values are typically in range 0..1)
typedef struct {
	color_float r;		// red (or HUE)
	color_float g;		// green (or SAT)
	color_float b;		// blue (or LUM or VAL)
	color_float a;		// alpha (0 = transparent, 1 = opaque)
} sColor;

// get vales as u8 (0..255)
INLINE u8 ColorR(const sColor* col) { int v = (int)(col->r*255+0.5); return (u8)CLAMPU8(v); }
INLINE u8 ColorG(const sColor* col) { int v = (int)(col->g*255+0.5); return (u8)CLAMPU8(v); }
INLINE u8 ColorB(const sColor* col) { int v = (int)(col->b*255+0.5); return (u8)CLAMPU8(v); }
INLINE u8 ColorA(const sColor* col) { int v = (int)(col->a*255+0.5); return (u8)CLAMPU8(v); }

// set color values from u8
INLINE void ColorSet(sColor* col, u8 r, u8 g, u8 b, u8 a)
	{ col->r = r/255.0; col->g = g/255.0; col->b = b/255.0; col->a = a/255.0; }

// set color to black
INLINE void ColorBlack(sColor* col) { col->r = 0; col->g = 0; col->b = 0; col->a = 1; }

// set color to white
INLINE void ColorWhite(sColor* col) { col->r = 1; col->g = 1; col->b = 1; col->a = 1; }

// set grey value (val = grey value 0 to 1)
INLINE void ColorGrey(sColor* col, color_float val) { col->r = val; col->g = val; col->b = val; col->a = 1; }

// clamp color value to valid range 0..1
void ColorClamp(sColor* col);

// absolute value of color
void ColorAbs(sColor* col);

// convert color to grey value
color_float ColorGetGrey(const sColor* col);
u8 ColorGetGreyByte(const sColor* col);

// get linear grey value (average value)
color_float ColorGetLinGrey(const sColor* col);

// decolorize (set to average grey value)
void ColorDecolor(sColor* col);

// transposition between 2 colors (k: 0=src1 .. 1=src2)
void ColorTrans(sColor* dst, const sColor* src1, const sColor* src2, color_float k);

// blend by src alpha (a: 0=dst,...1=src)
void ColorBlend(sColor* dst, const sColor* src);

// blend by dst alpha (a: 0=src,...1=dst)
void ColorBlendInv(sColor* dst, const sColor* src);

// invert color
INLINE void ColorInv(sColor* col) { col->r = 1 - col->r; col->g = 1 - col->g; col->b = 1 - col->b; }

// negate color
INLINE void ColorNeg(sColor* col) { col->r = - col->r; col->g = - col->g; col->b = - col->b; }

// add constant to color
INLINE void ColorAdd(sColor* col, color_float k) { col->r += k; col->g += k; col->b += k; }

// sum two colors
INLINE void ColorSum(sColor* dst, const sColor* src)  { dst->r += src->r; dst->g += src->g; dst->b += src->b; }

// multiply color by constant
INLINE void ColorMul(sColor* col, color_float k) { col->r *= k; col->g *= k; col->b *= k; }

// divide color by constant
INLINE void ColorDiv(sColor* col, color_float k) { k = 1/k; col->r *= k; col->g *= k; col->b *= k; }

// square color
INLINE void ColorSqr(sColor* col) { col->r *= col->r; col->g *= col->g; col->b *= col->b; }

// square root color
INLINE void ColorSqrt(sColor* col) { col->r = sqrt(col->r); col->g = sqrt(col->g); col->b = sqrt(col->b); }

// resize color interval (from 0..1 to min..max)
void ColorRange(sColor* col, color_float min, color_float max);

// average colors
void ColorAvg(sColor* col, const sColor* src);

// minimize color
void ColorMin(sColor* col, const sColor* src);

// maximize color
void ColorMax(sColor* col, const sColor* src);

// adjust brightness (0.5 = no change, >0.5 rise, <0.5 lower brightness)
void ColorBright(sColor* col, color_float bright);

// adjust contrast (0.5 = no change)
void ColorContrast(sColor* col, color_float contrast);

// adjust brightness level (controls are in range 0..1, 0.5=linear)
void ColorLevel(sColor* col, color_float bright, color_float contrast, color_float gamma);

// convert color to RGB
u32 ColorRGB(const sColor* col);

// convert color to RGBA
u32 ColorRGBA(const sColor* col);

// convert color to BGR
u32 ColorBGR(const sColor* col);

// convert color to BGRA
u32 ColorBGRA(const sColor* col);

// convert color to 16-bit RGB
u16 ColorRGB16(const sColor* col);

// convert color to 15-bit RGB
u16 ColorRGB15(const sColor* col);

// convert RGB to HSL
void ColorRGBToHSL(sColor* col);

// convert HSL to RGB
void ColorHSLToRGB(sColor* col);

// convert RGB to HSV
void ColorRGBToHSV(sColor* col);

// convert HSV to RGB
void ColorHSVToRGB(sColor* col);

#endif // _LIB_COLOR_H

#endif // USE_COLOR	// use color vector (lib_color.c, lib_color.h)
