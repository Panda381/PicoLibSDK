#!/bin/bash

# Compilation...

export TARGET="flash_program"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
