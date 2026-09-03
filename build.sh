#!/bin/bash
# g++ -g -c engine/sound/miniaudio.c -o bin/miniaudio.o
g++ -g game.cpp bin/miniaudio.o -lGL -lGLEW -lglfw -o test
