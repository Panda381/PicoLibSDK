#!/bin/bash

# Compilation...

export TARGET="SDBOOT"
export GRPDIR="."
export MEMMAP="noflash"

../../../_c1.sh "$1"
