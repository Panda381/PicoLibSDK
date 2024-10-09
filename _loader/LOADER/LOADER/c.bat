@echo off
rem Compilation...
if "%2" == "" goto err

set TARGET=LOADER
set GRPDIR=.
set MEMMAP=

call ..\..\..\_c1.bat %1

rem Export to ASM
exe\LoaderBin LOADER.bin ..\..\loader_%DEVICE%.S %2
if not errorlevel 1 goto err
pause

:err
