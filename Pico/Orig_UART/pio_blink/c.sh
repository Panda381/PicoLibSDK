#!/bin/bash

# Compilation...

export TARGET="pio_blink"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
