@echo off
rem Compilation...

set TARGET=pio_hub75
set GRPDIR=Orig_Base
set MEMMAP=

if exist hub75.pio.h del hub75.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\hub75.pio hub75.pio.h
if exist hub75.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
