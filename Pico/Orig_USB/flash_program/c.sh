#!/bin/bash

# Compilation...

export TARGET="flash_program"
export GRPDIR="Orig_USB"
export MEMMAP=""

../../../_c1.sh "$1"
