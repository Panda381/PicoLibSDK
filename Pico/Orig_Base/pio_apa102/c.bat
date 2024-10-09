@echo off
rem Compilation...

set TARGET=pio_apa102
set GRPDIR=Orig_Base
set MEMMAP=

if exist apa102.pio.h del apa102.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\apa102.pio apa102.pio.h
if exist apa102.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
