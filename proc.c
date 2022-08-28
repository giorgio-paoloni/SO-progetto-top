#include "proc.h"

void print_proc(WINDOW* window, int current_index, int start_row){

  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;
  dirent* proc_iter;

  char* pid_path;
  char* pid_cmdline;

  FILE* file_cmdline;
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  int i = 3, j = 0;//i indica la riga (della finestra) dove stampare, j il processo da stampare

  mvwprintw(window, 1, 2, "| PID | pid_path | cmdline | \n");

  while((proc_iter = readdir(proc_dir)) != NULL){
    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){

      pid_path = (char*) malloc((PROC_PATH_STRLEN + 1 + strlen(proc_iter->d_name)) * sizeof(char));
      strcpy(pid_path, PROC_PATH);
      strcat(pid_path, "/");
      strcat(pid_path, proc_iter->d_name);

      pid_cmdline = (char*) malloc((sizeof(pid_path) + 1 + CMD_LINE_LENGHT) * sizeof(char));

      strcpy(pid_cmdline, pid_path);
      strcat(pid_cmdline, "/");
      strcat(pid_cmdline, "cmdline");

      file_cmdline = fopen(pid_cmdline, "r");

      if(file_cmdline == NULL){//err
        free(pid_path);
        free(pid_cmdline);
        continue;
      }

      fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);

      fclose(file_cmdline);

      if(strcmp(buffer_cmdline,"\0") == 0){
        free(pid_path);
        free(pid_cmdline);
        continue;
      }

      strcpy(buffer_cmdline+strlen(buffer_cmdline), "\0");

      if(j >= current_index){
        mvwprintw(window, i, 2, "%s  %s  %s\n", proc_iter->d_name, pid_path, buffer_cmdline);
        wrefresh(window);
        i++;
      }

      memset(buffer_cmdline,0,BUFFER_CMDLINE_LENGHT);
      free(pid_path);
      free(pid_cmdline);

      j++;
    }
  }

  closedir(proc_dir);
}

void print_proc_advanced(WINDOW* window, int current_index, int start_row){

  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;
  dirent* proc_iter;

  char* pid_path;
  char* pid_cmdline;
  char* pid_stat;

  FILE* file_cmdline;
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  int i = 3, j = 0;//i indica la riga (della finestra) dove stampare, j il processo da stampare

  wclear(window);
  wrefresh(window);
  box(window, (int) '|', (int) '-');

  mvwprintw(window, 1, 2, "| PID | command | state | priority | total time | user time | s.user time | CPU% | \n");

  while((proc_iter = readdir(proc_dir)) != NULL){
    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){

      pid_path = (char*) malloc((PROC_PATH_STRLEN + 1 + strlen(proc_iter->d_name)) * sizeof(char));
      strcpy(pid_path, PROC_PATH);
      strcat(pid_path, "/");
      strcat(pid_path, proc_iter->d_name);

      pid_cmdline = (char*) malloc((sizeof(pid_path) + 1 + CMD_LINE_LENGHT) * sizeof(char));

      strcpy(pid_cmdline, pid_path);
      strcat(pid_cmdline, "/");
      strcat(pid_cmdline, "cmdline");

      file_cmdline = fopen(pid_cmdline, "r");

      if(file_cmdline == NULL){//err
        free(pid_path);
        free(pid_cmdline);
        continue;
      }

      fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);

      fclose(file_cmdline);

      if(strcmp(buffer_cmdline,"\0") == 0){
        free(pid_path);
        free(pid_cmdline);
        continue;
      }

      strcpy(buffer_cmdline+strlen(buffer_cmdline), "\0");

      if(j >= current_index){

        pid_stat = (char*) malloc((sizeof(pid_path) + 1 + strlen("stat")) * sizeof(char));
        strcpy(pid_stat, pid_path);
        strcat(pid_stat, "/");
        strcat(pid_stat, "stat");

        mvwprintw(window, i, 2, "%s | %s\n", proc_iter->d_name, print_PID_stats(pid_stat));
        wrefresh(window);

        free(pid_stat);
        i++;
      }

      memset(buffer_cmdline,0,BUFFER_CMDLINE_LENGHT);
      free(pid_path);
      free(pid_cmdline);

      j++;
    }
  }

  closedir(proc_dir);
}

char* print_PID_stats(char* path){

  FILE* file_stat;
  char* buffer_stat = (char*) malloc(BUFFER_STAT_LENGHT*sizeof(char));
  char* token;
  char* ret = (char*) malloc(RET_LENGHT*sizeof(char));
  char* command;

  memset(ret,0,RET_LENGHT);

  long int frequency = sysconf(_SC_CLK_TCK);//dal man proc

  long unsigned int system_uptime_sec = 1;
  long unsigned int total_time_sec = 0;
  long unsigned int user_time_clock = 0, user_time_sec = 0;
  long unsigned int superuser_time_clock = 0, superuser_time_sec = 0;
  long long unsigned int start_time_clock = 0, start_time_sec = 0;
  long unsigned int elapsed_time_sec = 1;
  double cpu_percentage_used_time_sec = 0;
  char state;
  long int priority = 0;
  int i = 1 ;


  if((file_stat = fopen(path, "r")) == NULL){//err
    fclose(file_stat);
    free(buffer_stat);
    return ret;
  }

  if(fgets(buffer_stat, BUFFER_STAT_LENGHT,file_stat) == NULL){//err
    fclose(file_stat);
    free(buffer_stat);
    return ret;
  }

  fclose(file_stat);

  token = strtok(buffer_stat, SEPARATOR1);

  while(token != NULL && i < MAX_TOKEN1){//strtok
    //credits. https://man7.org/linux/man-pages/man5/procfs.5.html
    //credits. https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat
    //credits. https://www.baeldung.com/linux/total-process-cpu-usage (altro linguaggio ma comprensibile e riscribile in C, nella guida dice starttime e' 21, invece e' 22 da man proc)

    //i campi sono oltre 50, non tutti ci servono
    //ricordo che sono espressi in Hertz (1/T), quindi devo ottenere la durata di T dal sistema

    if(i == 2){//comm  %s : comando, lo ho gia', pero' voglio la versione corta solo comando
    //da sistemare
      if(token[0] == '('){ //tipo (cmd)

        int p = 1;
        command = (char*) malloc((strlen(token)-2)*sizeof(char));
        while(token[p] != ')'){
          command[p-1] = token[p];
          p++;
        }
        command[p] = (char) 0;
      }else{//tipo ./cmd
        command = (char*) malloc((strlen(token))*sizeof(char));
        strcpy(command, token);
      }

    }else if(i == 3){ //state  %c
      state = (char) token[0];
    }else if(i == 14){//utime  %lu : tempo speso dal processo in user
      user_time_clock = (unsigned) atol(token);
    }else if(i == 15){//stime  %lu : tempo speso dal processo in superuser (kernel)
      superuser_time_clock = (unsigned) atol(token);
    }else if(i == 16){//cutime  %ld : tempo speso dai figli del processo in user
      //nella metrica del tempo totale ho deciso di non includerlo, ma lo inserisco per eventuali future modifiche
    }else if(i == 17){//cstime  %ld : tempo speso dai figli del processo in superuser
      //nella metrica del tempo totale ho deciso di non includerlo, ma lo inserisco per eventuali future modifiche
    }else if(i == 18){//priority  %ld : priorita' processo
      priority = atol(token);
    }else if(i == 22){//starttime  %llu : tempo di avvio del processo a partire dal boot
      start_time_clock = (unsigned) atoll(token);
      /*strcpy(ret, token);
      return ret;*/
    }

    //free(token);

    token = strtok(NULL, SEPARATOR1);
    i++;
  }

  //lavora su freq == 0 o total time == 0, per ora ho messo 1 di default per evitare divisioni per 0, pero' e' una soluzione ingenua

  if(frequency == 0) frequency = 1; //err, divisione per 0

  //tempi utili
  user_time_sec = user_time_clock / frequency;
  superuser_time_sec = superuser_time_clock / frequency;
  start_time_sec = start_time_clock / frequency;
  total_time_sec = user_time_sec + superuser_time_sec;

  if(total_time_sec == 0) total_time_sec = 1; //err, questo processo non ha tempo?


  system_uptime_sec = get_system_uptime();
  /*mvwprintw(window, 1, 2, "tkn:%lu\n", system_uptime_sec);
  wrefresh(window);
  return;*/

  elapsed_time_sec = system_uptime_sec - start_time_sec;

  if(elapsed_time_sec != 0){
    cpu_percentage_used_time_sec = (total_time_sec*100)/elapsed_time_sec;
  }else{//il processo e' partito al boot, quindi dividerei per 0 (floating point ex)
    cpu_percentage_used_time_sec = 1;
  }

  //sprintf(ret, "%s", path);
  //sprintf(ret, "%s ", command);

  /*strcpy(ret, command);
  return ret;*/

  sprintf(ret, "%s | %c | %ld | %lu | %lu | %lu | %f", command, state, priority, total_time_sec, user_time_sec, superuser_time_sec, cpu_percentage_used_time_sec );

  /*mvwprintw(window, y, 10, "Tot:%lu Usr:%lu Ker:%lu CPUper:%lu\n", total_time_sec, user_time_sec, superuser_time_sec, cpu_percentage_used_time_sec);
  wrefresh(window);*/
  free(command);
  free(buffer_stat);
  return ret;
}

long unsigned int get_system_uptime(){
  //credits.https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/deployment_guide/s2-proc-uptime
  //credits. man proc

  FILE* file_proc_uptime;
  char* buffer = (char*) malloc(256*sizeof(char));
  file_proc_uptime = fopen(PROC_UPTIME_PATH, "r");
  char* token;

  if(file_proc_uptime == NULL){//err
    fclose(file_proc_uptime);
    free(buffer);
    return 11;
  }

  if(fgets(buffer, 256,file_proc_uptime) == NULL){//err
    fclose(file_proc_uptime);
    free(buffer);
    return 11;
  }

  token = strtok(buffer, SEPARATOR1);

  fclose(file_proc_uptime);
  free(buffer);

  return (unsigned) atol(token);

}

int current_number_of_processes(){

  int count = 1;

  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return count;

  dirent* proc_iter;
  int proc_strlen = strlen(PROC_PATH);

  char* pid_path;
  char* pid_cmdline;

  FILE* file_cmdline;

  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];
  memset(buffer_cmdline,0,BUFFER_CMDLINE_LENGHT);



  while((proc_iter = readdir(proc_dir)) != NULL){
    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){

      pid_path = (char*) malloc((proc_strlen + 1 + strlen(proc_iter->d_name)) * sizeof(char));
      strcpy(pid_path, PROC_PATH);
      strcat(pid_path, "/");
      strcat(pid_path, proc_iter->d_name);
      //strcat(pid_path, "\0");

      pid_cmdline = (char*) malloc((sizeof(pid_path) + 1 + CMD_LINE_LENGHT) * sizeof(char));

      strcpy(pid_cmdline, pid_path);
      strcat(pid_cmdline, "/");
      strcat(pid_cmdline, "cmdline");

      file_cmdline = fopen(pid_cmdline, "r");

      if(file_cmdline == NULL){
        fclose(file_cmdline);
        free(pid_path);
        free(pid_cmdline);
        continue;
      }

      fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);

      fclose(file_cmdline);
      free(pid_path);
      free(pid_cmdline);

      if(strcmp(buffer_cmdline,"\0") == 0){
        continue;
      }else{
        count++;
        memset(buffer_cmdline,0,BUFFER_CMDLINE_LENGHT);
      }
    }
  }
  return count;
}

int is_pid(char* name){
  //Sfrutto questo metodo perche' leggendo la documentazione (*), in /proc dir, non ci sono solo cartelle PID
  //*https://man7.org/linux/man-pages/man5/proc.5.html oppure man proc
  int i = 0;
  while(name[i] != '\0'){
    if(!(name[i] >= '0' && name[i] <= '9')) return 0;
    i++;
  }
  return 1;
}


//"fortunatamente" non essendo un programma che deve girare su un sistema con poca ram posso permettermi questi sprechi di memoria.
