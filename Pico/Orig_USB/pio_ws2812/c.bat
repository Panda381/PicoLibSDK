@echo off
rem Compilation...

set TARGET=pio_ws2812
set GRPDIR=Orig_USB
set MEMMAP=

if exist ws2812.pio.h del ws2812.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\ws2812.pio ws2812.pio.h
if exist ws2812.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
