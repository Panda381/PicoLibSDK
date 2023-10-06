#!/bin/bash

# Compilation...

export TARGET="USBHOST"
export GRPDIR="USB"
export MEMMAP=""

../../../_c1.sh "$1"
