@echo off

REM Common Settings
REM set CFLAGS=-Wall -g3
set CFLAGS=-Wall
set LINKER=-lm -lSDL2 -lSDL2main -lglew32 -lopengl32 -lglu32 -lgdi32
SET PFLAGS=-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE

REM Main Game
SET SOURCES=src\shadertoy.c
gcc -o shadertoy.exe %CFLAGS% %PFLAGS% %SOURCES% %LINKER%
SET SOURCES=

REM END

