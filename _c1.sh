#!/bin/bash

# Compilation... Compile one project (one output UF2 file)

# First command-line parameter (%1) of this batch contains device name (e.g. picopad10).
# This batch is called from 2nd-level subdirectory, base directory of the project
# (e.g. from the DEMO\HELLOW\c.bat).

# ------------------------------------------------
# Edit this line to setup path to GCC-ARM compiler
# ------------------------------------------------
# set PATH=..\..\_tools;C:\ARM10\bin;%PATH%

# ----------------------------------------------------------------------
# Use following commands before running this batch, to setup compilation
# ----------------------------------------------------------------------

# Command to setup target project name (i.e. name of output file).
# This command is located in c.bat file. Name "LOADER" is reserved for boot loader.
#    set TARGET=HELLOW

# Command to setup target group sub-directory (directory to which the program
# will be copied on the target SD card). This command is located in c.bat file.
#    set GRPDIR=DEMO

# Command to select target device (use names from the _setup.bat file).
# This command is located in _setup.bat file. Parameter for the command is
# passed as command-line parameter of c.bat, c_all.bat, _setup.bat and _c1.bat.
#    set DEVICE=picopad10

# Command to select class of target device (to use common compilation setup).
# This command is located in _setup.bat file.
#    set DEVCLASS=picopad

# Command to setup target device directory (directory containing image of SD card).
# This command is located in _setup.bat file.
#    set DEVDIR=!PicoPad

# Setup parameters DEVICE, DEVCLASS and DEVDIR (%1 = device name)

source ../../../_setup.sh $1

# Check if target name "LOADER" is used in other directory than root.
# We need it to detect compilation of boot loader in makefile.

if [[ "${TARGET}" == "LOADER" ]]; then
  if [[ "${GRPDIR}" != "." ]]; then 
    echo "---- Application cannot have name LOADER ----"
    exit 1
  fi
fi

if [[ -e "${TARGET}".uf2 ]]; then rm "${TARGET}".uf2; fi
if [[ -e "${TARGET}".bin ]]; then rm "${TARGET}".bin; fi

# Compile
echo "Device: $DEVICE"
make -j$(nproc) all

# If you want to see all error messages, compile using this command:
# make all > err.txt

# Check compilation result
if [[ $? > 0 ]]; then echo "ERROR!"; exit 1; fi
if [[ ! -e $TARGET.uf2 ]]; then echo "ERROR!"; exit 1; fi
if [[ ! -e $TARGET.bin ]]; then echo "ERROR!"; exit 1; fi

# Compilation OK, list memory size
echo
cat $TARGET.siz

# Calculate CRC to check by boot loader (skip if compiling boot loader)
if [[ "${TARGET}" == "LOADER" ]]; then
  # Copy UF2 file to destination folder with image of SD card
  if [[ ! -e ../../../"${DEVDIR}"/"${GRPDIR}"/*.UF2 ]]; then mkdir ../../../"${DEVDIR}"/"${GRPDIR}"; fi
  cp "${TARGET}".uf2 ../../../"${DEVDIR}"/"${GRPDIR}"/"${TARGET}".UF2 > null
  exit 0
fi

if [[ "${DEVCLASS}" == "pico" ]]; then
  # Copy UF2 file to destination folder with image of SD card
  if [[ ! -e ../../../"${DEVDIR}"/"${GRPDIR}"/*.UF2 ]]; then mkdir ../../../"${DEVDIR}"/"${GRPDIR}"; fi
  cp "${TARGET}".uf2 ../../../"${DEVDIR}"/"${GRPDIR}"/"${TARGET}".UF2 > null
  exit 0
fi

if [[ "${MEMMAP}" == "noflash" ]]; then
  # Copy UF2 file to destination folder with image of SD card
  if [[ ! -e ../../../"${DEVDIR}"/"${GRPDIR}"/*.UF2 ]]; then mkdir ../../../"${DEVDIR}"/"${GRPDIR}"; fi
  cp "${TARGET}".uf2 ../../../"${DEVDIR}"/"${GRPDIR}"/"${TARGET}".UF2 > null
  exit 0
fi

../../../_tools/PicoPadLoaderCrc/LoaderCrc "${TARGET}".bin "${TARGET}".uf2

if [[ $? > 0 ]]; then echo "ERROR!"; exit 1; fi

