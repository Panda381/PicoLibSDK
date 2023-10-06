@echo off
rem Compilation...

set TARGET=RAYTRACE
set GRPDIR=DEMO
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat picotron
