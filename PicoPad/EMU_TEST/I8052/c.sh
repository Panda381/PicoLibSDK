#!/bin/bash

# Compilation...

export TARGET="I8052"
export GRPDIR="EMU_TEST"
export MEMMAP=""

../../../_c1.sh "$1"
