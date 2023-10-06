@echo off
rem Compilation...

set TARGET=SDCARD
set GRPDIR=TEST
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat picopad08
rem ..\..\..\_c1.bat picopadvga
..\..\..\_c1.bat picopad10
