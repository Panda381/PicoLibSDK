#!/bin/bash

# exportup device to compile

# First command-line parameter ($1) of this script contains device name (e.g. picopad10).
# This script is called from the _c1.sh file.

# Use configuration name as parameter of the script, to select target device:
#   pico .......... base Raspberry Pico module
#   pico2 ......... base Raspberry Pico 2 module in ARM mode
#   pico2riscv .... base Raspberry Pico 2 module in RISC-V mode
#   picotron ...... Picotron with VGA YRGB1111 output
#   picoinomini ... Picoino mini version with VGA RGB332 output
#   picoino10 ..... Picoino version 1.0 with VGA RGB332 output
#   demovga ....... DemoVGA board with VGA RGB565 output
#   picopad08 ..... PicoPad beta version 0.8
#   picopad10 ..... PicoPad version 1.0 with TFT RGB565 output
#   picopad20 ..... PicoPad version 2.0 (ARM) with TFT RGB565 output
#   picopad20riscv  PicoPad version 2.0 (RISC-V) with TFT RGB565 output
#   picopadvga .... PicoPad with VGA RGB565 output
#   (empty) ....... default compilation

case "$1" in
    "pico")
	export DEVICE="pico"
	export DEVCLASS="pico"
	export DEVDIR="!Pico"
	;;
    "pico2")
	export DEVICE="pico2"
	export DEVCLASS="pico"
	export DEVDIR="!Pico2"
	;;
    "pico2riscv")
	export DEVICE="pico2riscv"
	export DEVCLASS="pico"
	export DEVDIR="!Pico2riscv"
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
    "picopad20")
	export DEVICE="picopad20"
	export DEVCLASS="picopad"
	export DEVDIR="!PicoPad20"
	;;
    "picopad20riscv")
	export DEVICE="picopad20riscv"
	export DEVCLASS="picopad"
	export DEVDIR="!PicoPad20riscv"
	;;
    "picopadvga")
	export DEVICE="picopadvga"
	export DEVCLASS="picopad"
	export DEVDIR="!PicoPadVGA"
	;;
     *) 
	export DEVICE="picopad10"
	export DEVCLASS="picopad"
	export DEVDIR="!PicoPad20"
	;;
esac

return
