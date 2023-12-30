@echo off
rem Compilation...

if exist dvivga.pio.h del dvivga.pio.h
..\..\_tools\pioasm.exe -o c-sdk dvivga.pio dvivga.pio.h
if not exist dvivga.pio.h goto err

goto stop

:err
pause
:stop
