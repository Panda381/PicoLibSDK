@echo off
rem Compilation...

set TARGET=system_dbltap
set GRPDIR=Orig_Base
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
