#!/bin/bash

# Compilation... Compile one project (one output UF2 file)

# rem In order to compile from any directory, the $(dirname "$0")/ parameter
# rem is used instead of the relative path ../../../ .

# First command-line parameter ($1) of this script contains device name (e.g. picopad10).
# This script is called from 2nd-level subdirectory, base directory of the project
# (e.g. from the DEMO/HELLO/c.sh).

# -----------------------------------------------------------------------
# Use following commands before running this script, to setup compilation
# -----------------------------------------------------------------------

# Command to setup target project name (i.e. name of output file).
# This command is located in c.sh file. Name "LOADER" is reserved for boot loader.
#    export TARGET="HELLO"

# Command to setup target group sub-directory (directory to which the program
# will be copied on the target SD card). This command is located in c.sh file.
#    export GRPDIR="DEMO"

# Command to select target device (use names from the _setup.sh file).
# This command is located in _setup.sh file. Parameter for the command is
# passed as command-line parameter of c.sh, c_all.sh, _setup.sh and _c1.sh.
#    export DEVICE="picopad10"

# Command to select class of target device (to use common compilation setup).
# This command is located in _setup.sh file.
#    export DEVCLASS="picopad"

# Command to setup target device directory (directory containing image of SD card).
# This command is located in _setup.sh file.
#    export DEVDIR="!PicoPad"

# Setup parameters DEVICE, DEVCLASS and DEVDIR (%1 = device name)

source $(dirname "$0")/_setup.sh $1

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
  if [[ ! -e $(dirname "$0")/"${DEVDIR}"/"${GRPDIR}"/*.UF2 ]]; then mkdir $(dirname "$0")/"${DEVDIR}"/"${GRPDIR}"; fi
  cp "${TARGET}".uf2 $(dirname "$0")/"${DEVDIR}"/"${GRPDIR}"/"${TARGET}".UF2 > null
  exit 0
fi

if [[ "${DEVCLASS}" == "pico" ]]; then
  # Copy UF2 file to destination folder with image of SD card
  if [[ ! -e $(dirname "$0")/"${DEVDIR}"/"${GRPDIR}"/*.UF2 ]]; then mkdir $(dirname "$0")/"${DEVDIR}"/"${GRPDIR}"; fi
  cp "${TARGET}".uf2 $(dirname "$0")/"${DEVDIR}"/"${GRPDIR}"/"${TARGET}".UF2 > null
  exit 0
fi

if [[ "${MEMMAP}" == "noflash" ]]; then
  # Copy UF2 file to destination folder with image of SD card
  if [[ ! -e $(dirname "$0")/"${DEVDIR}"/"${GRPDIR}"/*.UF2 ]]; then mkdir $(dirname "$0")/"${DEVDIR}"/"${GRPDIR}"; fi
  cp "${TARGET}".uf2 $(dirname "$0")/"${DEVDIR}"/"${GRPDIR}"/"${TARGET}".UF2 > null
  exit 0
fi

$(dirname "$0")/_tools/PicoPadLoaderCrc/LoaderCrc "${TARGET}".bin "${TARGET}".uf2

if [[ $? > 0 ]]; then echo "ERROR!"; exit 1; fi

