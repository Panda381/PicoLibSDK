@echo off
rem Export result file to the hardware...
rem   Edit name of the destination disk in the "copy" command,
rem   or change disk name of the Pico image in task manager.

if not exist %TARGET%.uf2 goto end
copy /b %TARGET%.uf2 R:

:end
