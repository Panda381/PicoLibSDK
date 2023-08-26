@echo off
rem Compilation...

set TARGET=NEW
set GRPDIR=NEW

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat picotron
