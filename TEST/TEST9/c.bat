@echo off
rem Compilation...

set TARGET=TEST9
set GRPDIR=TEST

rem Select one of the following rows for device-specific compilation,
rem or leave compilation with parameter %1, using default device selected
rem in the _setup.bat file.

rem ..\..\_c1.bat picopad08
rem ..\..\_c1.bat picopad10
..\..\_c1.bat %1
