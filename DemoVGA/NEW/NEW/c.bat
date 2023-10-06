@echo off
rem Compilation...

set TARGET=NEW
set GRPDIR=NEW
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat demovga
