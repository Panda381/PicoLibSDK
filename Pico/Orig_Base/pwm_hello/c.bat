@echo off
rem Compilation...

set TARGET=pwm_hello
set GRPDIR=Orig_Base
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
