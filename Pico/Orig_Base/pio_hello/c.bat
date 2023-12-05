@echo off
rem Compilation...

set TARGET=pio_hello
set GRPDIR=Orig_Base
set MEMMAP=

if exist hello.pio.h del hello.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\hello.pio hello.pio.h
if exist hello.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
