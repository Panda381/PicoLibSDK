#!/bin/bash

# Compilation...

export TARGET="pio_spiflash"
export GRPDIR="Orig_USB"
export MEMMAP=""

../../../_c1.sh "$1"
