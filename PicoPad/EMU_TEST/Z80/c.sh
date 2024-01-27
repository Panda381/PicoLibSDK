#!/bin/bash

# Compilation...

export TARGET="Z80"
export GRPDIR="EMU_TEST"
export MEMMAP=""

../../../_c1.sh "$1"
