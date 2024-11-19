#!/bin/bash

# Compilation...

export TARGET="interp"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
