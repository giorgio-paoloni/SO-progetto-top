#include "process_monitor.h"

int kill_PID(int process_pid){
  if(process_pid <= 1) return -1; //per ora gestisco segnali con pid >1. dal man (LPM man 2 kill) risulta che init e' 1, mentre 0 e negativi sono riferiti per gruppi marticolari di segnali
  return kill(process_pid, SIGKILL);
}

int sleep_PID(int process_pid){
  //TODO
  return -1;
}
