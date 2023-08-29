This folder contains PicoVGA driver with 8-bit output to VGA.
It works, but not correctly - unlike the original PicoVGA version
(with the original SDK library), you cannot use video modes
with higher resolution than 800x600. Reason unknown - either
wrong (slow) flash memory access settings or wrong timing.

The library can be connected to the project according to
the MONOSCP2 sample. Extract sample into folder Picoino\TEST.


To include PicoVGA8 into your project, add following lines to project files.

To config.h add:
#define USE_PICOVGA8		1		// use PicoVGA 8-bit display
#define USE_MINIVGA		0		// use mini-VGA display with simple frame buffer
#define USE_DRAW		0		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#define USE_DRAW_STDIO		0		// use DRAW stdio (DrawPrint function)
#include "../../../_display/picovga8/_config.h"
#include "../../../config_def.h"	// default configuration

To include.h add:
#include "../../../includes.h"	// all includes
#include "../../../_display/picovga8/_include.h"

To Makefile add:
include ../../../_display/picovga8/_makefile.inc
include ../../../Makefile.inc
