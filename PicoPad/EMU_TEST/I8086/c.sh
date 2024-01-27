#!/bin/bash

# Compilation...

export TARGET="I8086"
export GRPDIR="EMU_TEST"
export MEMMAP=""

../../../_c1.sh "$1"
