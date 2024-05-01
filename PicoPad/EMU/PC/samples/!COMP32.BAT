@echo off
call _EXE\ASMCOM.BAT ALPHABET
if not exist ALPHABET.BIN goto stop

call _EXE\ASMCOM.BAT EGGS
if not exist EGGS.BIN goto stop

call _EXE\ASMCOM.BAT CHESS
if not exist CHESS.BIN goto stop

call _EXE\ASMCOM.BAT LINES
if not exist LINES.BIN goto stop

call _EXE\ASMCOM.BAT MANDELB
if not exist MANDELB.BIN goto stop

call _EXE\ASMCOM.BAT MINIMAZE
if not exist MINIMAZE.BIN goto stop

call _EXE\ASMCOM.BAT SPOTS
if not exist SPOTS.BIN goto stop

call _EXE\ASMEXE.BAT TETRIX
if not exist TETRIX.BIN goto stop

call _EXE\ASMCOM.BAT TRAIN
if not exist TRAIN.BIN goto stop

call _EXE\ASMCOM.BAT VEGAS
if not exist VEGAS.BIN goto stop

call _EXE\ASMCOM.BAT ZOOM
if not exist ZOOM.BIN goto stop




call _EXE\ASMEXE.BAT CENTIPED
if not exist CENTIPED.BIN goto stop

call _EXE\ASMEXE.BAT CONTAX
if not exist CONTAX.BIN goto stop

call _EXE\ASMCOM.BAT FLOPBIRD
if not exist FLOPBIRD.BIN goto stop

call _EXE\ASMCOM.BAT GOODY
if not exist GOODY.BIN goto stop

call _EXE\ASMCOM.BAT MISION
if not exist MISION.BIN goto stop

call _EXE\ASMEXE.BAT PANGO
if not exist PANGO.BIN goto stop

call _EXE\ASMEXE.BAT PATROL
if not exist PATROL.BIN goto stop

call _EXE\ASMCOM.BAT PCMAN
if not exist PCMAN.BIN goto stop

call _EXE\ASMEXE.BAT SPACEWAR
if not exist SPACEWAR.BIN goto stop

call _EXE\ASMEXE.BAT TENNIS
if not exist TENNIS.BIN goto stop

call _EXE\ASMEXE.BAT TOWER
if not exist TOWER.BIN goto stop

call _EXE\ASMEXE.BAT VLEAPER
if not exist VLEAPER.BIN goto stop


:stop
