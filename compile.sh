#!/bin/bash

# This script is used to compile src into bin

# Compile src into bin

rm -rf build

mkdir build

cd build

#gcc -o alix_station ../src/challenge1.c -Wall -Wextra -Werror -pedantic -std=c99 -g
gcc -c ../src/function_optimized.c -o function_optimized.o -no-pie -static
gcc -o challenges ../src/challenges.c -Wall -Wextra -pedantic -std=gnu11 -g
gcc -o prog_to_run ../src/prog.c -Wall -Wextra -no-pie -static