@echo off
rem Compilation...

set TARGET=SDBOOT
set GRPDIR=.
set MEMMAP=noflash

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
rem call ..\..\..\_c1.bat picopad08
rem call ..\..\..\_c1.bat picopadvga
call ..\..\..\_c1.bat picopad10

set MEMMAP=
