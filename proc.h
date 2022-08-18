#define _GNU_SOURCE //DT_DIR not included in posix, source: https://stackoverflow.com/questions/46694350/cant-compare-dirent-d-type-to-dt-dir
#define _POSIX_C_SOURCE //fopen? https://man7.org/linux/man-pages/man3/fopen.3.html

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h> //https://man7.org/linux/man-pages/man3/opendir.3.html, useful for manipulating directory in C
#include <string.h>
#include <fcntl.h>
#include <ncurses.h>


// the path "/proc" is used only in gnu-linux OSes, is not portable throught different OSes (ex. windows or BSD or MacOS)
#define PROC_PATH "/proc"

typedef struct dirent dirent; //used to don't type "struct dirent" everytime, cause I'm lazy

//DIR* get_proc();
void print_proc();
int is_pid(char* name);
void print_proc2();
