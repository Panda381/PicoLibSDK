@echo off
echo.
if "%1" == "" goto stop
if exist src\program.cpp del src\program.cpp
if exist build\program.o del build\program.o
echo %1.GBC
GBprep\GBprep samples\%1.GBC src\program.cpp %1 GBC %GBMAXROM%
if errorlevel 1 goto err
call _c.bat
copy /b GBC.uf2 samples\%1.UF2
if not exist samples\%1.BMP copy GBC.BMP samples\%1.BMP
if not exist samples\%1.PNG copy GBC.PNG samples\%1.PNG
goto stop
:err
pause
:stop
