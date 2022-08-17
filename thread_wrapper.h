
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define SEM1 "sem1"

void threadify();
