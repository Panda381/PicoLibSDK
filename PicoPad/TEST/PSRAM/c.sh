#!/bin/bash

# Compilation...

export TARGET="PSRAM"
export GRPDIR="TEST"
export MEMMAP="psram"

../../../_c1.sh "$1"
