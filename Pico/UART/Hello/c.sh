#!/bin/bash

# Compilation...

export TARGET="Hello"
export GRPDIR="UART"
export MEMMAP=""

../../../_c1.sh "$1"
