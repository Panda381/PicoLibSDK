@echo off
rem Compilation...

set TARGET=LOADER
set GRPDIR=.

call ..\..\..\_c1.bat %1

rem Export to ASM
exe\LoaderBin LOADER.bin ..\..\loader_%DEVICE%.S
