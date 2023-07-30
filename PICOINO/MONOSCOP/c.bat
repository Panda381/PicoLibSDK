@echo off
rem Compilation...

set TARGET=MONOSCOP
set GRPDIR=TEST

rem Select one of the following rows for device-specific compilation,
rem or leave compilation with parameter %1, using default device selected
rem in the _setup.bat file.

..\..\_c1.bat picoino10
rem ..\..\_c1.bat picopad08
rem ..\..\_c1.bat picopad10
..\..\_c1.bat %1
