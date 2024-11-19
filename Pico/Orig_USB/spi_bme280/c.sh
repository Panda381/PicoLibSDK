#!/bin/bash

# Compilation...

export TARGET="spi_bme280"
export GRPDIR="Orig_USB"
export MEMMAP=""

../../../_c1.sh "$1"
