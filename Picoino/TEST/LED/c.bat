@echo off
rem Compilation...

set TARGET=LED
set GRPDIR=TEST
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat picoino10
rem ..\..\..\_c1.bat picoinomini
