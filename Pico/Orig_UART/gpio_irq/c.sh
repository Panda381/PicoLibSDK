#!/bin/bash

# Compilation...

export TARGET="gpio_irq"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
