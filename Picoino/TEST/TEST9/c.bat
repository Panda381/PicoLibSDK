@echo off
rem Compilation...

set TARGET=TEST9
set GRPDIR=TEST
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat picoino10
rem ..\..\..\_c1.bat picoinomini
