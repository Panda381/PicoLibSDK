@echo off
rem Compilation...

set TARGET=TVTENNIS
set GRPDIR=GAME

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat demovga
