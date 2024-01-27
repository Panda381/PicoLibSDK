if "%1" == "" goto err
if exist %1.BIN del %1.BIN
if exist %1.P del %1.P
ASL.EXE %1.ASM -o %1.P -q
if not exist %1.P goto err
P2BIN.EXE %1.P %1.BIN
del %1.P
if not exist %1.BIN goto err

goto ok

:err
pause
:ok
