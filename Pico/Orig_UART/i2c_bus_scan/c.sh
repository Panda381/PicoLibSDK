#!/bin/bash

# Compilation...

export TARGET="i2c_bus_scan"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
