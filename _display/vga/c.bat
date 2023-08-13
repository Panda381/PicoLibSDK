@echo off
rem Compilation...
if exist vga.pio.h del vga.pio.h
..\..\_tools\pioasm.exe -o c-sdk vga.pio vga.pio.h
if exist vga.pio.h goto stop
pause
:stop
