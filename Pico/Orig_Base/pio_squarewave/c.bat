@echo off
rem Compilation...

set TARGET=pio_squarewave
set GRPDIR=Orig_Base
set MEMMAP=

if exist squarewave.pio.h del squarewave.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\squarewave.pio squarewave.pio.h
if exist squarewave.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
