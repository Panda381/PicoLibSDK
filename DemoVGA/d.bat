@echo off
rem Delete temporary files of all projects in all sub-directories

rem Loop to find all sub-directories
for /D %%d in (*) do call :del1 %%d
exit /b

rem Sub-batch to delete temporary files of one project in %1 subdirectory.
:del1
if not exist %1\d.bat goto stop
cd %1
echo Deleting %1
call d.bat
cd ..
:stop
