#!/bin/bash

# Compilation...

export TARGET="spi_flash"
export GRPDIR="Orig_USB"
export MEMMAP=""

../../../_c1.sh "$1"
