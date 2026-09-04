#!/bin/bash
cd build
cmake ..
cmake --build .
cp test ../test
cd ..