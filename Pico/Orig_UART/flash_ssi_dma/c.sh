#!/bin/bash

# Compilation...

export TARGET="flash_ssi_dma"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
