#!/bin/bash

# exportup device to compile

# First command-line parameter ($1) of this script contains device name (e.g. picopad10).
# This script is called from the _c1.sh file.

# Use configuration name as parameter of the script, to select target device:
#   pico .......... base Raspberry Pico module, optionally with UART on pins GPIO0 and GPIO1
#   picotron ...... Picotron with VGA YRGB1111 output
#   picoinomini ... Picoino mini version with VGA RGB332 output
#   picoino10 ..... Picoino version 1.0 with VGA RGB332 output
#   demovga ....... DemoVGA board with VGA RGB565 output
#   picopad08 ..... PicoPad beta version 0.8
#   picopad10 ..... PicoPad version 1.0 with TFT RGB565 output
#   (empty) ....... default compilation

case "$1" in
    "pico")
	export DEVICE="pico"
	export DEVCLASS="pico"
	export DEVDIR="!Pico"
	;;
    "picotron")
	export DEVICE="picotron"
	export DEVCLASS="picotron"
	export DEVDIR="!Picotron"
	;;
    "picoinomini")
	export DEVICE="picoinomini"
	export DEVCLASS="picoino"
	export DEVDIR="!PicoinoMini"
	;;
    "picoino10")
	export DEVICE="picoino10"
	export DEVCLASS="picoino"
	export DEVDIR="!Picoino10"
	;;
    "demovga")
	export DEVICE="demovga"
	export DEVCLASS="demovga"
	export DEVDIR="!DemoVGA"
	;;
    "picopad08")
	export DEVICE="picopad08"
	export DEVCLASS="picopad"
	export DEVDIR="!PicoPad08"
	;;
    "picopad10")
	export DEVICE="picopad10"
	export DEVCLASS="picopad"
	export DEVDIR="!PicoPad10"
	;;
     *) 
	export DEVICE="picopad10"
	export DEVCLASS="picopad"
	export DEVDIR="!PicoPad10"
	;;
esac

return
