#!/bin/bash

# Delete temporary files of one project

# Command to setup TARGET project name (i.e. name of output file).
# This command is located in d.bat file.
#    TARGET=HELLOW

rm -f build/"${TARGET}".elf
rm -f "${TARGET}".hex
rm -f "${TARGET}".lst
rm -f "${TARGET}".map
rm -f "${TARGET}".siz
rm -f "${TARGET}".sym
rm -f "${TARGET}".bin
rm -f "${TARGET}".uf2
rm -f build/*.o
rmdir build
