@echo off
rem Compilation...

set TARGET=SOKOBAN
set GRPDIR=GAME
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat demovga
