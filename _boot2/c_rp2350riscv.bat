rem @echo off
rem Compilation...

call d.bat

set TARGET=boot2_w25q080_RP2350RISCV
set MCU=RP2350A
set CPUTYPE=risc-v

_c1.bat 3
