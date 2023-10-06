#!/bin/bash

# Compilation...

export TARGET="Pi"
export GRPDIR="UART"
export MEMMAP=""

../../../_c1.sh "$1"
