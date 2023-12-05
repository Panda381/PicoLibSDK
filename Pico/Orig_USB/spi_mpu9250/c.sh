#!/bin/bash

# Compilation...

export TARGET="spi_mpu9250"
export GRPDIR="Orig_USB"
export MEMMAP=""

../../../_c1.sh "$1"
