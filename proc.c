#include "proc.h"

long number_of_processors = -1;

long page_size = -1 ;// = sysconf(_SC_PAGESIZE);//https://man7.org/linux/man-pages/man2/getpagesize.2.html
//Portable applications should employ sysconf(_Ssysinfo(system_information);sysinfo(system_information);C_PAGESIZE) instead of getpagesize():

void print_proc(WINDOW* window, int starting_index, int starting_row){
  cumulative_print_proc(window, starting_index, starting_row, PRINT_PROC);
}

void print_proc_advanced(WINDOW* window, int starting_index, int starting_row){
  cumulative_print_proc(window, starting_index, starting_row, PRINT_PROC_ADVANCED);
}

char* print_PID_stats(char* path){
  //da sistemare le varie metriche...

  // PID USER PR  NI VIRT RES SHR S %CPU %MEM TIME+ COMMAND

  FILE* file_stat;
  //FILE* file_statm;

  char buffer_stat[BUFFER_STAT_LENGHT];
  //char buffer_statm[BUFFER_STATM_LENGHT];

  char pid_stat[PID_STAT_LENGHT];
  //char pid_statm[PID_STATM_LENGHT];

  char command[COMMAND_LENGHT];

  char* token;
  char* ret;

  int i = 1;

  //INFO
  char state;
  long int priority;

  //CPU
  long int frequency = sysconf(_SC_CLK_TCK);//dal man proc, frequenza variabile?
  //clock
  long unsigned int user_time_clock;
  long unsigned int superuser_time_clock;
  long long unsigned int start_time_clock;
  //sec
  double elapsed_time_sec;
  double system_uptime_sec;
  double cpu_percentage_used_time_sec;
  double superuser_time_sec;
  double user_time_sec;
  double total_time_sec;

  double start_time_sec;

  //MEM
  struct sysinfo system_information;
  if(sysinfo(&system_information) == -1) return NULL;

  unsigned long total_physical_memory = system_information.totalram / 1024; //ritorna in bytes, converto in KB
  long rss;
  long long used_physical_memory;
  double used_physical_memory_percentage;
  long unsigned vm_size;

  if(page_size == -1){
    page_size = sysconf(_SC_PAGESIZE)/1024; //mostrata in KB
  }

  strcpy(pid_stat, path);
  strcat(pid_stat, "/");
  strcat(pid_stat, "stat");
  strcat(pid_stat, "\0");

  //strcpy(pid_statm, path);
  //strcat(pid_statm, "/");
  //strcat(pid_statm, "statm");
  //strcat(pid_statm, "\0");
  
  memset(buffer_stat, 0, BUFFER_STAT_LENGHT);
  //memset(buffer_statm, 0, BUFFER_STATM_LENGHT);
  
  if((file_stat = fopen(pid_stat, "r")) == NULL){//err
    return NULL;
  }

  /*if((file_statm = fopen(pid_statm, "r")) == NULL){//err
    return NULL;
  }*/

  if(fgets(buffer_stat, BUFFER_STAT_LENGHT,file_stat) == NULL){//err
    fclose(file_stat);
    //fclose(file_statm);
    return NULL;
  }

  /*if(fgets(buffer_statm, BUFFER_STATM_LENGHT,file_statm) == NULL){//err
    fclose(file_stat);
    fclose(file_statm);
    return NULL;
  }*/

  fclose(file_stat);
  //fclose(file_statm);

  ret = (char*) malloc(RET_LENGHT*sizeof(char));

  memset(ret, 0, RET_LENGHT);
  memset(command, 0, COMMAND_LENGHT);

  token = strtok(buffer_stat, SEPARATOR1);

  // cosa mostra top: PID USER PR NI VIRT RES SHR S %CPU %MEM TIME+ COMMAND
  //nb: le metriche possono scostare leggermente tra di loro perché ci sono vari modi di leggere i campi, più o meno precisi e dipende anche come vengono calcolate le operazioni


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
        while(token[p] != ')'){
          command[p-1] = token[p];
          p++;
        }
        command[p] = '\0'; //(char) 0;
      }else{//tipo ./cmd
        strcpy(command, token);
        strcpy(command, "\0");
      }

    }else if(i == 3){ //state  %c
      state = (char) token[0]; //?
    }else if(i == 14){//utime  %lu : tempo speso dal processo in user
      user_time_clock = strtoul(token, NULL, 10); // https://pubs.opengroup.org/onlinepubs/9699919799/functions/strtoul.html
    }else if(i == 15){//stime  %lu : tempo speso dal processo in superuser (kernel)
      superuser_time_clock = strtoul(token, NULL, 10);
    }else if(i == 18){//priority  %ld : priorita' processo
      priority = strtol(token, NULL, 10);
    }else if(i == 22){//starttime  %llu : tempo di avvio del processo a partire dal boot
      start_time_clock = strtoull(token, NULL, 10);
    }else if(i == 23){//vsize  %lu
      vm_size = strtoul(token, NULL, 10) / 1000 ; //e' segnato in bytes, converto in KB
    }else if(i == 24){//(24) rss  %ld 
      rss = strtol(token, NULL, 10);
      used_physical_memory = rss * page_size; //KB RES 
    }

    token = strtok(NULL, SEPARATOR1);
    i++;
  }
  
  used_physical_memory_percentage = ((double) used_physical_memory * 100) / (double) total_physical_memory;

  i = 0;
  //https://linux.die.net/man/1/top
  //https://www.google.com/search?q=virt+res+memory&oq=virt+res&aqs=edge.2.0i512j69i57j0i512l5j0i10i457i512j0i390.3928j0j1&sourceid=chrome&ie=UTF-8
  //https://serverfault.com/questions/138427/what-does-virtual-memory-size-in-top-mean
  //https://phoenixnap.com/kb/linux-commands-check-memory-usage
  //https://www.ibm.com/docs/en/aix/7.2?topic=usage-memory-determination-ps-command
  //https://stackoverflow.com/questions/1558402/memory-usage-of-current-process-in-c

  //token = NULL;// strtok(buffer_statm, SEPARATOR1);

  /*while(token != NULL && i < MAX_TOKEN2){
    token = strtok(NULL, SEPARATOR1);
    if(i == 2){
      
    }
    i++;
  }*/

  //https://en.wikipedia.org/wiki/C_data_types

  //rappresentati come %lu le divisioni, perdo la virgola, è importante?
  //per ora ho deciso di troncare le divisioni con la virgola.

  //lavora su freq == 0 o total time == 0, per ora ho messo 1 di default per evitare divisioni per 0, pero' e' una soluzione ingenua
  if(frequency == 0) return NULL; //err, divisione per 0

  user_time_sec = (double) user_time_clock / (double) frequency;
  superuser_time_sec = (double) superuser_time_clock / (double) frequency;
  start_time_sec = (double) start_time_clock / (double) frequency;
  total_time_sec = (double) (user_time_sec + superuser_time_sec); // overflow somma?

  //if(total_time_sec == 0) total_time_sec = 1; //err, questo processo non ha tempo(?)

  system_uptime_sec = (double)get_system_uptime();
  if(system_uptime_sec == 0)return NULL; //è una casistica impossibile(?)

  elapsed_time_sec = (double) system_uptime_sec - (double) start_time_sec;
  //il tempo attuale-tempo di avvio

  //? tot : elaps = x : 100

  //if(elapsed_time_sec != 0){
    cpu_percentage_used_time_sec = (double) (total_time_sec*100) / (double) elapsed_time_sec;
    if(cpu_percentage_used_time_sec > 100) cpu_percentage_used_time_sec = 100;//test
  //}else{//il processo e' partito ESATTAMENTE al boot, quindi dividerei per 0 (floating point execpt)
    //cpu_percentage_used_time_sec = 0;
  //}

  sprintf(ret, "%s %c %ld %0.2f %0.2f %0.2f %0.2f%% %0.2f%% %lld %ld %c", command, state, priority, total_time_sec, user_time_sec, superuser_time_sec, cpu_percentage_used_time_sec, used_physical_memory_percentage, used_physical_memory, vm_size, '\0');

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

void cumulative_print_proc(WINDOW* window, int starting_index, int starting_row, int calling_function){
  //ottimizzazione funzione, viene terminata prima e allocata sullo stack... TBF
  //conto come processi "effettivi" solo quelli con cmdline presente
  
  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;
  
  dirent* proc_iter;
  FILE* file_cmdline;

  char pid_cmdline[PID_CMDLINE_LENGHT];
  char pid_path[PID_PATH_LENGHT];
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];
  char* ret_pid_stats;

  int i = 3, j = 0;//i indica la riga (della finestra) dove stampare, j il processo da stampare

  int max_y = getmaxy(window);

  if(calling_function == PRINT_PROC){
    mvwprintw(window, 1, 2, "%s %c", "| PID | pid_path | cmdline |", '\0'); //https://stackoverflow.com/questions/23924497/how-to-fix-gcc-wall-embedded-0-in-format-warning
  }else{

    mvwprintw(window, 1, 2, "%s %c", "PID CMD S PR TT-s UT-s SU-s %CPU %MEM RES-KB VIRT-KB", '\0');
  }

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
        if(calling_function == PRINT_PROC){

          strtok(buffer_cmdline, SEPARATOR2); //evita altri caratteri scomodi di parametri nel cmdline ecc...(es: --no-sandbox --enable-crashpad)
          mvwprintw(window, i, 2, "%s  %s  %s %c", proc_iter->d_name, pid_path, buffer_cmdline, '\0');

        }else{
          
          ret_pid_stats = print_PID_stats(pid_path);
          mvwprintw(window, i, 2, "%s %s %c", proc_iter->d_name, ret_pid_stats, '\0');
          wrefresh(window);
          free(ret_pid_stats);
          
        }
        wrefresh(window);
        i++;
      }
      
      j++;
    }
  }

  closedir(proc_dir);
}

void print_stats(WINDOW *window, int starting_index, int starting_row){

  //mem
  long unsigned mem_total, mem_free, mem_available;

  // clock
  long unsigned int user_time_clock;
  long unsigned int superuser_time_clock;
  long unsigned int idle_time_clock;
  long unsigned int iowait_time_clock;
  long unsigned int irq_time_clock;
  long unsigned int softirq_time_clock;
  long unsigned int steal_time_clock;
  long unsigned int guest_time_clock;
  long unsigned int guest_nice_time_clock;

  //sec
  double total_time_sec;

  double user_time_sec;
  double superuser_time_sec;
  double idle_time_sec;
  double iowait_time_sec;
  double irq_time_sec;
  double softirq_time_sec;
  double steal_time_sec;
  double guest_time_sec;
  double guest_nice_time_sec;

  //cpu

  double cpu_percentage;
  

  if (number_of_processors == -1) number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
  int n = 0;
  long int frequency = sysconf(_SC_CLK_TCK); // dal man proc, frequenza variabile?
  char* token;
  int i = 0;

  // https://man7.org/linux/man-pages/man5/proc.5.html
  // cartella: /proc/stat
  // credits. snippet https://stackoverflow.com/questions/3501338/c-read-file-line-by-line
  // credits. snippet https://linux.die.net/man/3/getline
  //snippet leggermente modificato
  //https://www.baeldung.com/linux/get-cpu-usage

  wclear(window);
  box(window, (int)'|', (int)'-');
  mvwprintw(window, 1, 2, "STATS CUMULATIVE DI SISTEMA: %c", '\0');
  wrefresh(window);

  FILE *fp = NULL;
  char *buffer_line = NULL;//dal man dice che la alloca lui, e se troppo piccola la realloca, va deallocata. Userei lo stack ma lo snippet del man consigliato è questo...
  size_t lenght = 0;

  if ((fp = fopen(PROC_STAT_PATH, "r")) == NULL) exit(EXIT_FAILURE);

  while (getline(&buffer_line, &lenght, fp) != -1){
    i = 0;
    //wclear(window);
    //mvwprintw(window, 2, 2, "%s %c", buffer_line, '\0');
    //wrefresh(window);
    //sleep(1);

    token = strtok(buffer_line, SEPARATOR1);

    if(i == 0){
      if (buffer_line[0] == 'c' && buffer_line[1] == 'p' && buffer_line[2] == 'u'){ // cpuN

        while (token != NULL && i < MAX_TOKEN3){

          token = strtok(NULL, SEPARATOR1);
          i++;

          if(i == 1){//user
            user_time_clock = strtoul(token, NULL, 10);
          }else if(i == 2){//nice
            //
          }else if(i == 3){//system
            superuser_time_clock = strtoul(token, NULL, 10);
          }else if(i == 4){//idle
            idle_time_clock = strtoul(token, NULL, 10);
          }else if(i == 5){
            iowait_time_clock = strtoul(token, NULL, 10);
          }else if(i == 6){
            irq_time_clock = strtoul(token, NULL, 10);
          }else if(i == 7){
            softirq_time_clock = strtoul(token, NULL, 10);
          }else if(i == 8){
            steal_time_clock = strtoul(token, NULL, 10);
          }else if(i == 9){
            guest_time_clock = strtoul(token, NULL, 10);
          }else if(i == 10){
            guest_nice_time_clock = strtoul(token, NULL, 10);
          }
        }

        user_time_sec = (double) user_time_clock / (double) frequency;
        superuser_time_sec = (double) superuser_time_clock / (double) frequency;
        idle_time_sec = (double) idle_time_clock / (double) frequency;
        iowait_time_sec = (double) iowait_time_clock / (double) frequency;
        irq_time_sec = (double) irq_time_clock / (double) frequency;
        softirq_time_sec = (double) softirq_time_clock / (double) frequency;
        steal_time_sec = (double) steal_time_clock / (double) frequency;
        guest_time_sec = (double) guest_time_clock / (double) frequency;
        guest_nice_time_sec = (double) guest_nice_time_clock / (double) frequency;

        total_time_sec = user_time_sec + superuser_time_sec + idle_time_sec + iowait_time_sec + irq_time_sec + softirq_time_sec + steal_time_sec + guest_time_sec + guest_nice_time_sec;

        cpu_percentage = 100 - ((double)idle_time_sec * 100 / (double) total_time_sec);

        if (buffer_line[3] != ' ' && buffer_line[3] != '\0'){
          // wclear(window);
          //mvwprintw(window, n + 2, 2, "CPU%d %s %c", n, buffer_line, '\0');
          //mvwprintw(window, n + 2, 2, "%s %c", buffer_line, '\0');
          mvwprintw(window, n + 2, 2, "CPU%d %c", n, '\0');
          percentage_bar(window, n + 2, 9, cpu_percentage);
          //wrefresh(window);
          n = (n + 1) % (number_of_processors+1);
        }else{}

      }
    }  
  }

  fclose(fp);

  if ((fp = fopen(PROC_MEMINFO_PATH, "r")) == NULL) exit(EXIT_FAILURE);
  token = NULL;
  i = 0;

  while (getline(&buffer_line, &lenght, fp) != -1){
    i = 0;
    token = strtok(buffer_line, SEPARATOR2);

    if (!strcmp(token, "MemTotal")){//MemTotal %lu
      token = strtok(NULL, SEPARATOR1);
      mem_total = strtoul(token, NULL, 10);
    }else if (!strcmp(token, "MemFree")){
      token = strtok(NULL, SEPARATOR1);//
      mem_free = strtoul(token, NULL, 10);
    }else if (!strcmp(token, "MemAvailable")){//
      token = strtok(NULL, SEPARATOR1);
      mem_available = strtoul(token, NULL, 10);
    }
  }

  fclose(fp);
  return;
}

void percentage_bar(WINDOW *window, int starting_row, int starting_col, double percentage){

  if(percentage >= 0 && percentage < 10){ // [########] 100%
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[..........]", percentage, '\0');  
  }else if(percentage >= 10 && percentage < 20){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[#.........]", percentage, '\0');
  }else if(percentage >= 20 && percentage < 30){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[##........]", percentage, '\0');
  }else if(percentage >= 30 && percentage < 40){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[###.......]", percentage, '\0');
  }else if(percentage >= 40 && percentage < 50){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[####......]", percentage, '\0');
  }else if(percentage >= 50 && percentage < 60){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[#####.....]", percentage, '\0');
  }else if(percentage >= 60 && percentage < 70){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[######....]", percentage, '\0');
  }else if(percentage >= 70 && percentage < 80){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[#######...]", percentage, '\0');
  }else if(percentage >= 80 && percentage < 90){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[########..]", percentage, '\0');
  }else if(percentage >= 90 && percentage < 100){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[#########.]", percentage, '\0');
  }else{
    mvwprintw(window, starting_row, starting_col, "%s %0.2f %% %c", "[##########]", percentage, '\0');
  }

  wrefresh(window);

  return;
}