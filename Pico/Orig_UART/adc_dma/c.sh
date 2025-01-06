#!/bin/bash

# Compilation...

export TARGET="adc_dma"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
