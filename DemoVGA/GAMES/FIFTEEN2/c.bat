@echo off
rem Compilation...

set TARGET=FIFTEEN2
set GRPDIR=GAME

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat demovga
