#!/bin/bash

# Export result file to the hardware...
# Edit name of the destination disk in the "copy" command,
#   or change disk name of the Pico image in task manager.

if [[ -e "${TARGET}".uf2 ]]; then
  cp "${TARGET}".uf2 /media/        # Change this name of the mount point folder of your RPI-RP2 here.
fi


