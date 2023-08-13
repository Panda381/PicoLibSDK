@echo off
rem Compilation...

set TARGET=FLAG2
set GRPDIR=DEMO

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat demovga
