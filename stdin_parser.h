#define STDIN 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/select.h> //https://man7.org/linux/man-pages/man2/select.2.html mi serve per input asincroni su stdin
//credits. https://stackoverflow.com/questions/10219340/using-stdin-with-select-in-c

void keypressed();
