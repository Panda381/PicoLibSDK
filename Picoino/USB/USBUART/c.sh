#!/bin/bash

# Compilation...

export TARGET="USBUART"
export GRPDIR="USB"
export MEMMAP=""

../../../_c1.sh "$1"
