#!/bin/sh

#TEST0
make clean && make && valgrind --leak-check=yes --log-file="./valgrind_output" ./bot

#TEST1
#make clean && make && valgrind  --leak-check=yes --show-leak-kinds=all --log-file="./valgrind_output" ./bot

#Giorgio Paoloni, 1883570
