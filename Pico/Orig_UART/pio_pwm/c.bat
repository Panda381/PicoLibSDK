@echo off
rem Compilation...

set TARGET=pio_pwm
set GRPDIR=Orig_UART
set MEMMAP=

if exist pwm.pio.h del pwm.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\pwm.pio pwm.pio.h
if exist pwm.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
