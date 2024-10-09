@echo off
rem Delete...

set TARGET=boot2_w25q080_RP2040
call ..\_d1.bat

set TARGET=boot2_w25q080_RP2350
call ..\_d1.bat

set TARGET=boot2_w25q080_RP2350RISCV
call ..\_d1.bat
