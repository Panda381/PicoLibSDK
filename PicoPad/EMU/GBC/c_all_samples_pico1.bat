@echo off

rem MAG ROM size: Picopad1 with RP2040
set GBMAXROM=1980000

echo GBMAXROM=%GBMAXROM%
for %%a in (.\samples\*.gb) do call _c_1_gb.bat %%~na
for %%a in (.\samples\*.gbc) do call _c_1_gbc.bat %%~na
