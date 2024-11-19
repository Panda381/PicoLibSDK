#!/bin/bash

# Compilation...

export TARGET="flash_nuke"
export GRPDIR="Orig_Base"
export MEMMAP="noflash"

../../../_c1.sh "$1"
