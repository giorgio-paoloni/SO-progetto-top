#define _GNU_SOURCE //serve per DT_DIR non incluso in posix, credits. https://stackoverflow.com/questions/46694350/cant-compare-dirent-d-type-to-dt-dir
#define _POSIX_C_SOURCE //fopen? https://man7.org/linux/man-pages/man3/fopen.3.html

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/sysinfo.h>//https://man7.org/linux/man-pages/man2/sysinfo.2.html, per info sulla ram
#include <dirent.h> //https://man7.org/linux/man-pages/man3/opendir.3.html, utile per manipolare cartelle in C
#include <sys/stat.h>//non credo verrà usata alla fine...
#include <string.h>
#include <fcntl.h>
#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

#undef _POSIX_C_SOURCE //https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-closedir-close-directory

//il percorso "/proc" e' usato solamente in OS Gnu-Linux, non e' portabile attraverso differenti OS come windows ecc.. quindi lo dichiaro qua in modo da poter cambiare agilmente il percorso di proc, se presente
#define PROC_PATH "/proc"
#define PROC_PATH_STRLEN 5
#define PROC_UPTIME_PATH "/proc/uptime"
#define BUFFER_CMDLINE_LENGHT 128
#define SEPARATOR1 " \t\n"
#define SEPARATOR2 " \t\n:"
#define MAX_TOKEN1 25
#define MAX_TOKEN2 5
#define MAX_TOKEN3 11
#define BUFFER_STAT_LENGHT 256
#define BUFFER_STATM_LENGHT 256
#define RET_LENGHT 256
#define CMD_LINE_LENGHT 7
#define REFRESH_RATE 1
//
#define PROC_STAT_PATH "/proc/stat"
#define PROC_MEMINFO_PATH "/proc/meminfo"

#define PID_PATH_LENGHT 64
#define PID_CMDLINE_LENGHT 128
#define PID_STAT_LENGHT 128
#define PID_STATM_LENGHT 128

#define COMMAND_LENGHT 64

#define PRINT_PROC 0
#define PRINT_PROC_ADVANCED 1

#define NUM_PROCESSOR sysconf(_SC_NPROCESSORS_ONLN)

typedef struct dirent dirent; //usato per non scrivere ogni volta "struct dirent", sono sfaticato

void print_proc(WINDOW* window, int starting_index, int starting_row);
void print_proc_advanced(WINDOW* window, int starting_index, int starting_row);

//utili
long unsigned int get_system_uptime();
char* print_PID_stats(char* path);
int current_number_of_processes();
int is_pid(char* name);
void cumulative_print_proc(WINDOW* window, int starting_index, int starting_row, int calling_function);
void print_stats(WINDOW *window, int starting_index, int starting_row);
void percentage_bar(WINDOW *window, int starting_row, int starting_col, double percentage);

//var
extern sem_t sem1; //dichiarata in TUI.C

//struct

typedef struct cpu_snapshot_t{
    int time;

    double* total_time_sec;
    double* user_time_sec;
    double* superuser_time_sec;
    double* idle_time_sec;
    double* iowait_time_sec;
    double* irq_time_sec;
    double* softirq_time_sec;
    double* steal_time_sec;
    double* guest_time_sec;
    double* guest_nice_time_sec;

}cpu_snapshot_t;

typedef struct cpu_usage_t{
    double* idle_time_diff_sec;
    double* total_time_diff_sec;
    double* cpu_percentage;
}cpu_usage_t;

typedef struct thread_arg_t{
    void *win1;
    void *cpu_us;
} thread_arg_t;

void cpu_usage(WINDOW* window, void* arg);
void* cpu_usage_alloc();
void cpu_usage_free(cpu_usage_t *free);
void* cpu_usage_thread_wrapper(void *arg);

void* cpu_snapshot(int time);
void* cpu_snapshot_alloc(int time);
void cpu_snapshot_free(cpu_snapshot_t* free);


//cosa mostra top? https://www.booleanworld.com/guide-linux-top-command/

//man top: https://man7.org/linux/man-pages/man1/top.1.html

//void delay(int milliseconds);