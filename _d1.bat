@echo off
rem Delete temporary files of one project

rem Command to setup target project name (i.e. name of output file).
rem This command is located in d.bat file.
rem    set TARGET=HELLOW

if exist build\%TARGET%.elf del build\%TARGET%.elf
if exist build\*.elf goto nobuild
if not exist build\*.o goto nobuild
del build\*.o
rd /q build

:nobuild
if exist %TARGET%.hex del %TARGET%.hex
if exist %TARGET%.lst del %TARGET%.lst
if exist %TARGET%.map del %TARGET%.map
if exist %TARGET%.siz del %TARGET%.siz
if exist %TARGET%.sym del %TARGET%.sym
if exist %TARGET%.bin del %TARGET%.bin
if exist %TARGET%.uf2 del %TARGET%.uf2

:end
