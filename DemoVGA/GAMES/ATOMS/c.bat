@echo off
rem Compilation...

set TARGET=ATOMS
set GRPDIR=GAME
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat demovga
