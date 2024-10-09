rem @echo off
rem Compilation...

call d.bat

set TARGET=boot2_w25q080_RP2350
set MCU=RP2350A
set CPUTYPE=cortex-m33

_c1.bat 2
