#include "thread_wrapper.h"


void threadify(){
  sem_t* sem;

  sem_unlink(SEM1);

  if((sem = sem_open (SEM1, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) printf("errore sem_open\n");

  while(1){
    /*if(sem_wait(sem) == -1) printf("err s\n");
      print_proc();
    if(sem_post(sem) == -1) printf("err s\n");*/

    //sleep(4);

    if(sem_wait(sem) == -1) printf("err s\n");
      keypressed();
    if(sem_post(sem) == -1) printf("err s\n");

    //sleep(1);
  }

  if(sem_close(sem) == -1) printf("errore sem_close\n");

  sem_unlink(SEM1);

  return;
}
