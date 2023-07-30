@echo off
rem Compilation...
if exist qvga.pio.h del qvga.pio.h
..\..\_tools\pioasm.exe -o c-sdk qvga.pio qvga.pio.h
if exist qvga.pio.h goto stop
pause
:stop
