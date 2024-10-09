@echo off
rem Compilation...

set TARGET=pio_blink
set GRPDIR=Orig_USB
set MEMMAP=

if exist blink.pio.h del blink.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\blink.pio blink.pio.h
if exist blink.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
