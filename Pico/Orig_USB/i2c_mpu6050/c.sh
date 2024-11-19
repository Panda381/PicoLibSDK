#!/bin/bash

# Compilation...

export TARGET="i2c_mpu6050"
export GRPDIR="Orig_USB"
export MEMMAP=""

../../../_c1.sh "$1"
