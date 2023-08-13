@echo off
rem Compilation...

set TARGET=MANDEL
set GRPDIR=PROG

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat demovga
