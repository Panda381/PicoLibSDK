#!/bin/bash

# Compilation...

export TARGET="clocks_48MHz"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
