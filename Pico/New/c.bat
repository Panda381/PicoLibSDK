@echo off
rem Compilation... Compile all projects in all sub-directories

rem First command-line parameter (%1) of this batch can contain device name (e.g. pico).

rem Only Pico
if "%1"=="pico" goto start
if not "%1"=="" goto stop

:start
rem Compile projects from all sub-directories
for /D %%d in (*) do call :comp1 %%d %%1
exit /b

rem Sub-batch to compile one project in %1 subdirectory, as %2 device.
:comp1
if not exist %1\c.bat goto stop
cd %1
echo.
echo ======== Compiling %1 ========
call c.bat %2
cd ..
:stop
