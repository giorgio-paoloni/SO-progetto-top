#!/bin/sh

#TEST0
#make clean && make && valgrind --log-file="./valgrind_output" ./bot

#TEST 0.1
#make clean && make && valgrind --track-origins=yes --log-file="./valgrind_output" ./bot


#TEST1
#make clean && make && valgrind --leak-check=yes --show-leak-kinds=all --log-file="./valgrind_output" ./bot

#TEST1.1
#make clean && make && valgrind --verbose --leak-check=yes --show-leak-kinds=all --log-file="./valgrind_output" ./bot

#TEST1.2
make clean && make && valgrind --leak-check=yes --log-file="./valgrind_output" ./bot

#TEST2
#make clean && make && valgrind --leak-check=yes --show-possibly-lost=no --show-leak-kinds=all --log-file="./valgrind_output" ./bot

#TEST3
#make clean && make && valgrind --leak-check=yes --show-possibly-lost=yes --show-reachable=yes --log-file="./valgrind_output" ./bot



#Giorgio Paoloni, 1883570
