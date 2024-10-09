@echo off
rem Compilation...

call d.bat

set TARGET=boot2_w25q080_RP2040
set MCU=RP2040
set CPUTYPE=cortex-m0plus

_c1.bat 1
