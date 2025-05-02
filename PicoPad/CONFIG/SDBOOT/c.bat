@echo off
rem Compilation...

set TARGET=SDBOOT
set GRPDIR=.
set MEMMAP=noflash

if "%1"=="" goto default
call ..\..\..\_c1.bat %1
goto stop

:default
rem call ..\..\..\_c1.bat picopad08
rem call ..\..\..\_c1.bat picopadvga
rem call ..\..\..\_c1.bat picopad10
call ..\..\..\_c1.bat picopad20
rem call ..\..\..\_c1.bat picopad20riscv
rem call ..\..\..\_c1.bat picopadhstx
rem call ..\..\..\_c1.bat picopadhstxriscv

:stop
set MEMMAP=
