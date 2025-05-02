
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

ALIGNED u16 FrameBuf[320*240];

int main()
{
	// initialize videomode 320x240/16-bit (sys_clock=126 MHz)
	DispVMode320x240x16(0, FrameBuf);

	// copy image to the screen
	memcpy(FrameBuf, MonoscopeImg, sizeof(FrameBuf));
	while (True) {}
}
