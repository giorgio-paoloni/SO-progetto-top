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
#define PROC_PATH_STRLEN 5
#define PROC_UPTIME_PATH "/proc/uptime"
#define BUFFER_CMDLINE_LENGHT 256
#define SEPARATOR1 " \t\n"
#define MAX_TOKEN1 23
#define BUFFER_STAT_LENGHT 256
#define RET_LENGHT 256
#define CMD_LINE_LENGHT 7

typedef struct dirent dirent; //usato per non scrivere ogni volta "struct dirent", sono sfaticato

void print_proc(WINDOW* window, int current_index, int start_row);
void print_proc_advanced(WINDOW* window, int current_index, int start_row);
//utili
long unsigned int get_system_uptime();
char* print_PID_stats(char* path);
int current_number_of_processes();
int is_pid(char* name);

//cosa mostra top? https://www.booleanworld.com/guide-linux-top-command/
