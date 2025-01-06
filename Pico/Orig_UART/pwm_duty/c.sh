#!/bin/bash

# Compilation...

export TARGET="pwm_duty"
export GRPDIR="Orig_UART"
export MEMMAP=""

../../../_c1.sh "$1"
