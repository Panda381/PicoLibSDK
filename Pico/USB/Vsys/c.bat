@echo off
rem Compilation...

set TARGET=Vsys
set GRPDIR=USB
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
