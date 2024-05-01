@echo off

echo Compiling %1.COM
if exist %1.BIN del %1.BIN
rem /z  Display source line with error message
rem /m5 Allow 5 multiple passes to resolve forward references
rem /t  Suppress messages if successful assembly
rem /n  Suppress symbol tables in listing
rem /l,/la Generate listing: l=normal listing, la=expanded listing
_EXE\TASM.EXE %1.ASM %1.OBJ /z/m5/t/n
rem _EXE\TASM.EXE %1.ASM %1.OBJ %1.LST /z/m5/t/n/l
if errorlevel == 1 goto err

rem /t  Create COM file (same as /Tdc)
rem /x  No map file at all
rem /n  No default libraries
_EXE\TLINK.EXE %1.OBJ, %1.BIN /t/x/n
if errorlevel == 1 goto err
goto end

:err
pause

:end
del %1.OBJ
echo.
