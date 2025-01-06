#!/bin/bash

# Compilation...

export TARGET="flash_xip_stream"
export GRPDIR="Orig_USB"
export MEMMAP=""

../../../_c1.sh "$1"
