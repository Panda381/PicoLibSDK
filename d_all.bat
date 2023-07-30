@echo off
rem Delete temporary files of all projects in all sub-directories

rem Delete boot2 stage
cd _boot2
call d.bat
cd ..

rem Delete boot loader
cd _loader\LOADER
call d.bat
cd ..\..

rem Loop to find all sub-directories
for /D %%d in (*) do call :del1 %%d
exit /b

rem Sub-batch to delete temporary files of one group of projects in %1 subdirectory.
:del1
if not exist %1\d.bat goto stop
cd %1
echo Deleting group %1
call d.bat
cd ..
:stop
