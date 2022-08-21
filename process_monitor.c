#include "process_monitor.h"

int kill_PID(int process_pid){
  if(process_pid <= 1) return -1; //per ora gestisco segnali con pid >1. dal man (LPM man 2 kill) risulta che init e' 1, mentre 0 e negativi sono riferiti per gruppi particolari di segnali
  return kill(process_pid, SIGKILL);
}

int sleep_PID(int process_pid){
  if(process_pid <= 1) return -1;
  return kill(process_pid, SIGSTOP);//SIGSTOP?
}

int resume_PID(int process_pid){
  if(process_pid <= 1) return -1;
  return kill(process_pid, SIGCONT);//SIGCONT?
}

int terminate_PID(int process_pid){
  if(process_pid <= 1) return -1;
  return kill(process_pid, SIGTERM);
  return -1;
}
