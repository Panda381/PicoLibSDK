#!/bin/bash

# Compilation...

export TARGET="picoboard_blinky"
export GRPDIR="Orig_Base"
export MEMMAP=""

../../../_c1.sh "$1"
