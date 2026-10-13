#!/bin/bash
cd build
cmake ..
cmake --build .
cp opengl_test ../test
cd ..