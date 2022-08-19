//#define _POSIX_C_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>//richiesto da man 2 kill
#include <signal.h> //idem di sopra

int kill_PID(int process_pid);
int sleep_PID(int process_pid);
