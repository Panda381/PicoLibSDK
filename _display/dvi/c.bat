@echo off
rem Compilation...

if exist dvi.pio.h del dvi.pio.h
..\..\_tools\pioasm.exe -o c-sdk dvi.pio dvi.pio.h
if not exist dvi.pio.h goto err

goto stop

:err
pause
:stop
