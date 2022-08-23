#define _GNU_SOURCE //serve per DT_DIR non incluso in posix, credits. https://stackoverflow.com/questions/46694350/cant-compare-dirent-d-type-to-dt-dir
#define _POSIX_C_SOURCE //fopen? https://man7.org/linux/man-pages/man3/fopen.3.html

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h> //https://man7.org/linux/man-pages/man3/opendir.3.html, utile per manipolare cartelle in C
#include <string.h>
#include <fcntl.h>
#include <ncurses.h>

//il percorso "/proc" e' usato solamente in OS Gnu-Linux, non e' portabile attraverso differenti OS come windows ecc.. quindi lo dichiaro qua in modo da poter cambiare agilmente il percorso di proc, se presente
#define PROC_PATH "/proc"
#define BUFFER_CMDLINE_LENGHT 256

typedef struct dirent dirent; //usato per non scrivere ogni volta "struct dirent", sono sfaticato

//DIR* get_proc();
void print_proc();//prima implementazione, non utilizzata, ma lasciata commentata come Vanilla
int is_pid(char* name);
void print_proc2(WINDOW* window);
void print_proc3(WINDOW* window, int start_row);
void print_proc_advanced(WINDOW* window, int start_row);

//cosa mostra top? https://www.booleanworld.com/guide-linux-top-command/
