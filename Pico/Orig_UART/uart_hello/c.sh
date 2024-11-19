#!/bin/bash

# Compilation...

export TARGET="uart_hello"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
