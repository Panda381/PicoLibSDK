#!/bin/bash

# Compilation...

export TARGET="M6502"
export GRPDIR="EMU_TEST"
export MEMMAP=""

../../../_c1.sh "$1"
