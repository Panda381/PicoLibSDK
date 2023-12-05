@echo off
rem Compilation...

set TARGET=system_board
set GRPDIR=Orig_USB
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
