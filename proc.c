#include "proc.h"

void print_proc(WINDOW* window, int starting_index, int starting_row){
  //ottimizzazione funzione, viene terminata prima e allocata sullo stack... TBF
  //conto come processi "effettivi" solo quelli con cmdline presente
  
  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;
  
  dirent* proc_iter;
  FILE* file_cmdline;

  char pid_cmdline[PID_CMDLINE_LENGHT];
  char pid_path[PID_PATH_LENGHT];
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];
  //char* token;

  int i = 3, j = 0;//i indica la riga (della finestra) dove stampare, j il processo da stampare

  int max_y = getmaxy(window);

  mvwprintw(window, 1, 2, "%s %c", "| PID | pid_path | cmdline |", '\0'); //https://stackoverflow.com/questions/23924497/how-to-fix-gcc-wall-embedded-0-in-format-warning

  while((proc_iter = readdir(proc_dir)) != NULL && i < (max_y - 1) ){

    memset(buffer_cmdline, 0, BUFFER_CMDLINE_LENGHT);
    memset(pid_cmdline, 0, PID_CMDLINE_LENGHT);
    memset(pid_path, 0, PID_PATH_LENGHT);

    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){

      strcpy(pid_cmdline, PROC_PATH);
      strcat(pid_cmdline, "/");
      strcat(pid_cmdline, proc_iter->d_name);
      strcat(pid_cmdline, "/");

      strcat(pid_path, pid_cmdline);

      strcat(pid_cmdline, "cmdline");
      strcat(pid_cmdline, "\0");

      file_cmdline = fopen(pid_cmdline, "r");

      if(file_cmdline == NULL){//err file
        fclose(file_cmdline);
        continue;
      }
      
      fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);

      fclose(file_cmdline);
      

      if(strcmp(buffer_cmdline,"\0") == 0) continue; //cmdline vuoto, pid di un processo senza cmdline

      if(j >= starting_index){

        strtok(buffer_cmdline, SEPARATOR2); //evita altri caratteri scomodi di parametri nel cmdline ecc...(es: --no-sandbox --enable-crashpad)

        mvwprintw(window, i, 2, "%s  %s  %s %c", proc_iter->d_name, pid_path, buffer_cmdline, '\0');
        wrefresh(window);
        i++;
      }
      
      j++;
    }
  }

  closedir(proc_dir);
}

void print_proc_advanced(WINDOW* window, int starting_index, int starting_row){

  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;
  
  dirent* proc_iter;
  FILE* file_cmdline;

  char pid_cmdline[PID_CMDLINE_LENGHT];
  char pid_path[PID_PATH_LENGHT];
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  char* ret_pid_stats;

  int i = 3, j = 0;

  int max_y = getmaxy(window);

  mvwprintw(window, 1, 2, "%s %c", "| PID | command | state | priority | total time | user time | s.user time | CPU% |", '\0');
  
  while((proc_iter = readdir(proc_dir)) != NULL && i < (max_y - 1) ){

    memset(buffer_cmdline, 0, BUFFER_CMDLINE_LENGHT);
    memset(pid_cmdline, 0, PID_CMDLINE_LENGHT);
    memset(pid_path, 0, PID_PATH_LENGHT);

    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){

      strcpy(pid_cmdline, PROC_PATH);
      strcat(pid_cmdline, "/");
      strcat(pid_cmdline, proc_iter->d_name);
      strcat(pid_cmdline, "/");

      strcat(pid_path, pid_cmdline);

      strcat(pid_cmdline, "cmdline");
      strcat(pid_cmdline, "\0");

      file_cmdline = fopen(pid_cmdline, "r");

      if(file_cmdline == NULL){//err file
        fclose(file_cmdline);
        continue;
      }
      
      fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);

      fclose(file_cmdline);
      

      if(strcmp(buffer_cmdline,"\0") == 0) continue;

      if(j >= starting_index){

        ret_pid_stats = print_PID_stats(pid_path);

        mvwprintw(window, i, 2, "%s | %s %c", proc_iter->d_name, ret_pid_stats, '\0');
        wrefresh(window);

        free(ret_pid_stats);

        i++;
      }
      
      j++;
    }
  }

  closedir(proc_dir);
}

char* print_PID_stats(char* path){

  char* pid_stat = (char*) malloc((sizeof(path) + 1 + strlen("stat")) * sizeof(char));
  strcpy(pid_stat, path);
  strcat(pid_stat, "/");
  strcat(pid_stat, "stat");

  char* pid_statm = (char*) malloc((sizeof(path) + 1 + strlen("statm")) * sizeof(char));
  strcpy(pid_statm, path);
  strcat(pid_statm, "/");
  strcat(pid_statm, "statm");


  FILE* file_stat;
  char* buffer_stat = (char*) malloc(BUFFER_STAT_LENGHT*sizeof(char));
  char* token;
  char* ret = (char*) malloc(RET_LENGHT*sizeof(char));
  char* command;

  memset(ret, 0, RET_LENGHT);
  memset(buffer_stat, 0, BUFFER_STAT_LENGHT);

  long int frequency = sysconf(_SC_CLK_TCK);//dal man proc
  long unsigned int system_uptime_sec = 1;
  long long unsigned int total_time_sec = 0;
  long unsigned int user_time_clock = 0, user_time_sec = 0;
  long unsigned int superuser_time_clock = 0, superuser_time_sec = 0;
  long long unsigned int start_time_clock = 0, start_time_sec = 0;
  long unsigned int elapsed_time_sec = 1;
  double cpu_percentage_used_time_sec = 0;
  char state;
  long int priority = 0;
  int i = 1 ;


  if((file_stat = fopen(pid_stat, "r")) == NULL){//err
    fclose(file_stat);
    free(buffer_stat);
    free(pid_stat);
    free(pid_statm);
    return ret;
  }

  if(fgets(buffer_stat, BUFFER_STAT_LENGHT,file_stat) == NULL){//err
    fclose(file_stat);
    free(buffer_stat);
    free(pid_stat);
    free(pid_statm);
    return ret;
  }

  fclose(file_stat);

  token = strtok(buffer_stat, SEPARATOR1);

  while(token != NULL && i < MAX_TOKEN1){//strtok
    //credits. https://man7.org/linux/man-pages/man5/procfs.5.html
    //credits. https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat
    //credits. https://www.baeldung.com/linux/total-process-cpu-usage (altro linguaggio ma comprensibile e riscribile in C, nella guida dice starttime e' 21, invece e' 22 da man proc)

    ///proc/[pid]/stat
    ///proc/[pid]/statm

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
        command[p] = '\0'; //(char) 0;
      }else{//tipo ./cmd
        command = (char*) malloc((strlen(token))*sizeof(char));
        strcpy(command, token);
      }

    }else if(i == 3){ //state  %c
      state = (char) token[0]; //?
    }else if(i == 14){//utime  %lu : tempo speso dal processo in user
      user_time_clock = strtoul(token, NULL, 10); // https://pubs.opengroup.org/onlinepubs/9699919799/functions/strtoul.html
    }else if(i == 15){//stime  %lu : tempo speso dal processo in superuser (kernel)
      superuser_time_clock = strtoul(token, NULL, 10);
    }else if(i == 18){//priority  %ld : priorita' processo
      priority = atol(token);
    }else if(i == 22){//starttime  %llu : tempo di avvio del processo a partire dal boot
      start_time_clock = strtoull(token, NULL, 10);
      /*strcpy(ret, token);
      return ret;*/
    }

    token = strtok(NULL, SEPARATOR1);
    i++;
  }

  memset(buffer_stat, 0, BUFFER_STAT_LENGHT);
  i = 0;
  //https://linux.die.net/man/1/top
  //https://www.google.com/search?q=virt+res+memory&oq=virt+res&aqs=edge.2.0i512j69i57j0i512l5j0i10i457i512j0i390.3928j0j1&sourceid=chrome&ie=UTF-8
  //https://serverfault.com/questions/138427/what-does-virtual-memory-size-in-top-mean
  //https://phoenixnap.com/kb/linux-commands-check-memory-usage

  if((file_stat = fopen(pid_statm, "r")) == NULL){//err
    fclose(file_stat);
    free(buffer_stat);
    free(pid_stat);
    free(pid_statm);
    return ret;
  }

  if(fgets(buffer_stat, BUFFER_STAT_LENGHT,file_stat) == NULL){//err
    fclose(file_stat);
    free(buffer_stat);
    free(pid_stat);
    free(pid_statm);
    return ret;
  }

  fclose(file_stat);

  token = strtok(buffer_stat, SEPARATOR1);

  while(token != NULL && i < MAX_TOKEN2){

    if(i == 2){

    }

    token = strtok(NULL, SEPARATOR1);
    i++;
  }



  //lavora su freq == 0 o total time == 0, per ora ho messo 1 di default per evitare divisioni per 0, pero' e' una soluzione ingenua

  if(frequency == 0) frequency = 1; //err, divisione per 0

  //tempi utili
  //F = 1/T => TF = 1 => T = 1/F

  //https://en.wikipedia.org/wiki/C_data_types

  //rappresentati come %lu le divisioni, perdo la virgola, è importante?
  //per ora ho deciso di troncare le divisioni con la virgola.


  user_time_sec = user_time_clock / frequency;
  superuser_time_sec = superuser_time_clock / frequency;
  start_time_sec = start_time_clock / frequency;
  total_time_sec = (long long unsigned) (user_time_sec + superuser_time_sec); //overflow somma?

  //if(total_time_sec == 0) total_time_sec = 1; //err, questo processo non ha tempo?

  system_uptime_sec = get_system_uptime();
  /*mvwprintw(window1, 1, 2, "tkn:%lu\n", system_uptime_sec);
  wrefresh(window1);
  return;*/
  
  /*if(system_uptime_sec == 0){

  }*/

  //elapsed_time_sec = system_uptime_sec;
  //elapsed_time_sec = start_time_sec ;

  elapsed_time_sec = system_uptime_sec - start_time_sec;

  if(elapsed_time_sec != 0){
    //cpu_percentage_used_time_sec = elapsed_time_sec;
    //cpu_percentage_used_time_sec = total_time_sec;
    cpu_percentage_used_time_sec = (long double) (total_time_sec*100) / (long double) elapsed_time_sec;
  }else{//il processo e' partito al boot, quindi dividerei per 0 (floating point ex)
    cpu_percentage_used_time_sec = 0;
    //cpu_percentage_used_time_sec = 999;
  }

  //sprintf(ret, "%s", path);
  //sprintf(ret, "%s ", command);

  /*strcpy(ret, command);
  return ret;*/

  sprintf(ret, "%s | %c | %ld | %llu | %lu | %lu | %0.3f%s", command, state, priority, total_time_sec, user_time_sec, superuser_time_sec, cpu_percentage_used_time_sec, "%" );

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
    return 0;
  }

  if(fgets(buffer, 256,file_proc_uptime) == NULL){//err
    fclose(file_proc_uptime);
    free(buffer);
    return 0;
  }

  fgets(buffer, 256, file_proc_uptime);

  token = strtok(buffer, SEPARATOR1);

  //NB: errore
  //non alloca memoria, ma tokenizza il buffer, 
  //ciao\tcome\tstai\t?
  //diventa
  //0123456789...
  //ciao\0come\0stai\0?
  //ad ogni ciclo di token cambia il puntatore
  //buffer[0], buffer[1], buffer[10]
  //se libero prima il buffer (free), sarà sempre 0 il token

  int value = (long unsigned int) strtod(token, NULL);

  fclose(file_proc_uptime);
  free(buffer);

  return value;

}


int current_number_of_processes(){ //testa memory leak ecc
  
  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return 0;

  dirent* proc_iter;
  FILE* file_cmdline;

  int count = 0;

  //essendo chiamato tante volte, preferisco allocare le cose sullo stack per evitare di allocare e deallocare memoria costantemente

  char pid_cmdline[PID_CMDLINE_LENGHT];
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT]; 

  while((proc_iter = readdir(proc_dir)) != NULL){

    memset(buffer_cmdline, 0, BUFFER_CMDLINE_LENGHT);
    memset(pid_cmdline, 0, PID_CMDLINE_LENGHT);

    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){

      strcpy(pid_cmdline, PROC_PATH);
      strcat(pid_cmdline, "/");
      strcat(pid_cmdline, proc_iter->d_name);
      strcat(pid_cmdline, "/");
      strcat(pid_cmdline, "cmdline");
      strcat(pid_cmdline, "\0");

      file_cmdline = fopen(pid_cmdline, "r");

      if(file_cmdline == NULL){
        fclose(file_cmdline);
        //memset(pid_cmdline, 0, PID_CMDLINE_LENGHT);
        continue;
      }

      fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);

      fclose(file_cmdline);

      if(strcmp(buffer_cmdline,"\0") == 0) continue;
      count++;
    }
  }

  closedir(proc_dir);
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
