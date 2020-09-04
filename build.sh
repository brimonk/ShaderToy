#!/usr/bin/env bash

CFLAGS="-Wall -g3"
LINKER="-lm -lSDL2 -lSDL2main -lGL -lGLU -lGLEW"

# Main Game
SOURCES=src/shadertoy.c
gcc -o shadertoy ${CFLAGS} ${SOURCES} ${LINKER}
SOURCES=

