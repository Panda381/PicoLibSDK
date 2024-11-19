#!/bin/bash

# Compilation...

export TARGET="system_board"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
