#include "proc.h"

long number_of_processors = -1;

long page_size = -1 ;// = sysconf(_SC_PAGESIZE);//https://man7.org/linux/man-pages/man2/getpagesize.2.html
//Portable applications should employ sysconf(_Ssysinfo(system_information);sysinfo(system_information);C_PAGESIZE) instead of getpagesize():

//regex_t regex_var;

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
      vm_size = strtoul(token, NULL, 10);// / 1000 ; //e' segnato in bytes, converto in KB
    }else if(i == 24){//(24) rss  %ld 
      rss = strtol(token, NULL, 10);
      used_physical_memory = rss * page_size; //è in BYTES (ex//KB RES )
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

  //credits. https://stackoverflow.com/questions/2674312/how-to-append-strings-using-sprintf
  //imposto gli spazi

  int length = 0;

  length += snprintf(ret + length, RET_LENGHT - length, "%-20s  %1c  %2ld  %8.2f  %8.2f  %8.2f  %6.2f%%  %6.2f%% ", command, state, priority, total_time_sec, user_time_sec, superuser_time_sec, cpu_percentage_used_time_sec, used_physical_memory_percentage);

  if (used_physical_memory < KB_SIZE){
    length += snprintf(ret + length, RET_LENGHT - length, "  %6lldB", used_physical_memory);
  }else if (used_physical_memory >= KB_SIZE && used_physical_memory < MB_SIZE){
    length += snprintf(ret + length, RET_LENGHT - length, "  %6.2lfK", ((double)used_physical_memory )/ ((double)KB_SIZE));
  }else if (used_physical_memory >= MB_SIZE && used_physical_memory < GB_SIZE){
    length += snprintf(ret + length, RET_LENGHT - length, "  %6.2lfM", ((double)used_physical_memory )/ ((double)MB_SIZE));
  }else if(used_physical_memory >= GB_SIZE){
    length += snprintf(ret + length, RET_LENGHT - length, "  %6.2lfG", ((double)used_physical_memory) / ((double)GB_SIZE));
  }

  if (vm_size < KB_SIZE){
    length += snprintf(ret + length, RET_LENGHT - length, "  %6ldB%c", vm_size, '\0');
  }else if (vm_size >= KB_SIZE && vm_size < MB_SIZE){
    length += snprintf(ret + length, RET_LENGHT - length, "  %6.2fK%c", ((double) vm_size )/ ((double)KB_SIZE) , '\0');
  }else if (vm_size >= MB_SIZE && vm_size < GB_SIZE){
    length += snprintf(ret + length, RET_LENGHT - length, "  %6.2fM%c", ((double)vm_size) / ((double)MB_SIZE), '\0');
  }else if(vm_size >= GB_SIZE){
    length += snprintf(ret + length, RET_LENGHT - length, "  %6.2fG%c",((double)vm_size) / ((double)GB_SIZE), '\0');
  }

  
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
    mvwprintw(window, 1, 2, "%s %c", "| PID | pid_path | cmdline |", '\0'); 
    //https://stackoverflow.com/questions/23924497/how-to-fix-gcc-wall-embedded-0-in-format-warning
    //https://pubs.opengroup.org/onlinepubs/009695399/functions/fprintf.html
  }else{
    //%-20s  %1c  %2ld  %6.2f  %6.2f  %6.2f  %6.2f%%  %3.2f%%  %-8lld  %-8ld%c
    //"PID", "CMD", "S", "PR", "TT-s", "UT-s", "SU-s", "%CPU", "%MEM", "RES-KB", "VIRT-KB", '\0');
    //CPU% +1 perché c'è %
    //CPU% +1 perché c'è K/M/G
    mvwprintw(window, 1, 2, "%-6s  %-20s  %-1s  %-2s  %8s  %8s  %8s  %7s  %7s   %7s  %7s%c", "PID", "CMD", "S", "PR", "TT-s", "UT-s", "SU-s", "%CPU", "%MEM", "RES", "VIRT", '\0');
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
          if(strlen(proc_iter->d_name) < 7){
            //https://stackoverflow.com/questions/23776824/what-is-the-meaning-of-s-in-a-printf-format-string
            mvwprintw(window, i, 2, "%-6s  %s %c", proc_iter->d_name, ret_pid_stats, '\0');
          }else{
            //PID troppo lungo, creo deallineamento del print, ma riesco a vedere il pid per intero senza tagliarlo
            mvwprintw(window, i, 2, "%s  %s %c", proc_iter->d_name, ret_pid_stats, '\0');
          }
          
          //wrefresh(window);
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
  //si occupa di impaginare bene le colonne ecc
  mem_usage(window, ROW_POS2, COL_POS2); //sta sotto, però lo carico prima perché è un'info disponibile subito (a diff di cpu_usage)

  mvwprintw(window, 1, 2, "CPU(TOT): %c", '\0');
  percentage_bar(window, 1, 12, cpu_usage_var->cpu_percentage[0]);

  for (int k = 1; k < (NUM_PROCESSOR + 1); k++){
    // wclear(window3);
    //mvwprintw(window, k + 1, 2, "CPU: %d Usage: %0.2f %c", (k - 1), cpu_usage_var->cpu_percentage[k], '\0');
    mvwprintw(window, ROW_POS0, COL_POS0, "CPU(%d): %c", (k - 1), '\0');
    percentage_bar(window, ROW_POS1, COL_POS1, cpu_usage_var->cpu_percentage[k]);
    
  }

  wrefresh(window);

  return;
}

void percentage_bar(WINDOW *window, int starting_row, int starting_col, double percentage){

  if(/*percentage >= 0 && */percentage < 10){ // [########] 100%
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[..........]", percentage, '\0');  
  }else if(percentage >= 10 && percentage < 20){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[#.........]", percentage, '\0');
  }else if(percentage >= 20 && percentage < 30){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[##........]", percentage, '\0');
  }else if(percentage >= 30 && percentage < 40){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[###.......]", percentage, '\0');
  }else if(percentage >= 40 && percentage < 50){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[####......]", percentage, '\0');
  }else if(percentage >= 50 && percentage < 60){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[#####.....]", percentage, '\0');
  }else if(percentage >= 60 && percentage < 70){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[######....]", percentage, '\0');
  }else if(percentage >= 70 && percentage < 80){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[#######...]", percentage, '\0');
  }else if(percentage >= 80 && percentage < 90){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[########..]", percentage, '\0');
  }else if(percentage >= 90 && percentage < 100){
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[#########.]", percentage, '\0');
  }else{
    mvwprintw(window, starting_row, starting_col, "%s %0.2f%% %c", "[##########]", percentage, '\0');
  }

  wrefresh(window);

  return;
}

void cpu_usage(){
  //questo metodo verrà lanciato come thread

  //credits.https://stackoverflow.com/questions/7684359/how-to-use-nanosleep-in-c-what-are-tim-tv-sec-and-tim-tv-nsec
  
  cpu_snapshot(0);

  if(nanosleep(&sleep_value, NULL) == -1 && ( errno == EINTR || errno == EINVAL)) exit(EXIT_FAILURE); //errore se interrupt o valore non valido

  cpu_snapshot(1);

  for (int k = 0; k < (NUM_PROCESSOR + 1); k++){
    cpu_usage_var->idle_time_diff_sec[k] = (double) cpu_snapshot_t1->idle_time_sec[k] - cpu_snapshot_t0->idle_time_sec[k];
    cpu_usage_var->total_time_diff_sec[k] = (double) cpu_snapshot_t1->total_time_sec[k] - cpu_snapshot_t0->total_time_sec[k];

    if (cpu_usage_var->total_time_diff_sec[k] != 0){
      cpu_usage_var->cpu_percentage[k] = (double)100 - ((double)cpu_usage_var->idle_time_diff_sec[k] * 100 / (double)cpu_usage_var->total_time_diff_sec[k]);
    }else{
      cpu_usage_var->cpu_percentage[k] = (double)0;
    }
  }
  
  return;
}

void* cpu_usage_thread_wrapper(void* arg){

  // l'idea è creare dei thread (detached), costantemente che creano un'istantanea al file /proc/stat allocandolo nelle apposite strutture e che poi viene aggiornato su cpu_usage_t (la differnza)
  // nel mentre il programma principale continua a girare e ogni refresh stampa il contenuto della struttura
  // devo usare i thread perché c'è bisogno di un delta-t (Dt) di tempo(calcolato in maniera del tutto euristica) per calcolare la differenza.
  // se effettuassi lo snapshot/screenshot senza Dt (anche minimo) avrei gli stessi risultati nei due tempi di istantanea, inoltre Dt deve non essere eccessivamente piccolo per 2 motivi:
  //il refresh-rete della UI, non ha senso calcolarlo ogni 1ns se la UI si aggiorna ogni s (deve -> al refresh-r): calcolerei delle info non mostrate a causa del RR
  //e inoltre se troppo piccolo i calcoli verrebbero veramente piccoli come valore e questo potrebbe creare over/under flow oppure calcoli imprecisi perché "non si riesce a cogliere la differenza di utilizzo proporzionata al Dt bene"

  //il Dt bloccherebbe il flusso di esecuzione del programma main, quindi tramite thread non viene bloccato e l'utente può continuare ad usare il programma

  //i thread effettuano side-effect sulle strutture allocate

  //non so se serve realmente bloccare dei segnali...
  sigset_t set1;
  sigemptyset(&set1);
  sigaddset(&set1, SIGALRM);
  pthread_sigmask(SIG_SETMASK, &set1, NULL);

  sem_wait(&sem1);
  cpu_usage(); //CS, 1 solo thread alla volta in writing (producer)
  sem_post(&sem1);

  return NULL;
}

void cpu_snapshot(int time){

  cpu_snapshot_t *cpu_snap;

  if(time == 0){
    cpu_snap = cpu_snapshot_t0;
  }else if(time == 1){
    cpu_snap = cpu_snapshot_t1;
  }else{
    return;
  }

  int p = -1; 
  int i;
  FILE* fp;
  char* buffer_line = NULL;
  size_t lenght = 0;
  char* token;
  long int frequency = sysconf(_SC_CLK_TCK);

  if ((fp = fopen(PROC_STAT_PATH, "r")) == NULL) exit(EXIT_FAILURE);


  while (getline(&buffer_line, &lenght, fp) != -1){

    i = 0;

    token = strtok(buffer_line, SEPARATOR1);

    if (i == 0 && buffer_line[0] == 'c' && buffer_line[1] == 'p' && buffer_line[2] == 'u')
    { // cpuN

      // NB: a causa di cpu prima di tutti sono traslati di 1 in avanti...
      // p = 0 => cpu
      // p = 1 => cpu0
      // etc..

      p++; // vale -1, quindi inizia da 0

      while (i < MAX_TOKEN3 && token != NULL){

        token = strtok(NULL, SEPARATOR1);
        i++;

        if (i == 1){ // user
          cpu_snap->user_time_sec[p] = (double)strtoul(token, NULL, 10) / (double)frequency;
        }else if (i == 2){ // nice
          //
        }else if (i == 3){ // system
          cpu_snap->superuser_time_sec[p] = (double)strtoul(token, NULL, 10) / (double)frequency;
        }else if (i == 4){ // idle
          cpu_snap->idle_time_sec[p] = (double)strtoul(token, NULL, 10) / (double)frequency;
        }else if (i == 5){
          cpu_snap->iowait_time_sec[p] = (double)strtoul(token, NULL, 10) / (double)frequency;
        }else if (i == 6){
          cpu_snap->irq_time_sec[p] = (double)strtoul(token, NULL, 10) / (double)frequency;
        }else if (i == 7){
          cpu_snap->softirq_time_sec[p] = (double)strtoul(token, NULL, 10) / (double)frequency;
        }else if (i == 8){
          cpu_snap->steal_time_sec[p] = (double)strtoul(token, NULL, 10) / (double)frequency;
        }else if (i == 9){
          cpu_snap->guest_time_sec[p] = (double)strtoul(token, NULL, 10) / (double)frequency;
        }else if (i == 10){
          cpu_snap->guest_nice_time_sec[p] = (double)strtoul(token, NULL, 10) / (double)frequency;
        }
      }
    }
  }

  for(int k = 0 ; k < NUM_PROCESSOR + 1; k++){
    cpu_snap->total_time_sec[k] = cpu_snap->user_time_sec[k] + cpu_snap->superuser_time_sec[k] + cpu_snap->idle_time_sec[k] + cpu_snap->iowait_time_sec[k] + cpu_snap->irq_time_sec[k] + cpu_snap->softirq_time_sec[k] + cpu_snap->steal_time_sec[k] + cpu_snap->guest_time_sec[k] + cpu_snap->guest_nice_time_sec[k];
  }

  fclose(fp);
  free(buffer_line);

  return;
}

void* cpu_snapshot_alloc(int time){

  cpu_snapshot_t* ret = (cpu_snapshot_t*) malloc(sizeof(cpu_snapshot_t));

  ret->time = time;

  ret->total_time_sec = (double*) malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->user_time_sec = (double*) malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->superuser_time_sec = (double *)malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->idle_time_sec = (double *)malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->iowait_time_sec = (double *)malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->irq_time_sec = (double *)malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->softirq_time_sec = (double *)malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->steal_time_sec = (double *)malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->guest_time_sec = (double *)malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->guest_nice_time_sec = (double *)malloc((1 + NUM_PROCESSOR) * sizeof(double));

  return (void*) ret;
}

void cpu_snapshot_free(cpu_snapshot_t* struct_ptr){

  free(struct_ptr->total_time_sec);
  free(struct_ptr->user_time_sec);
  free(struct_ptr->superuser_time_sec);
  free(struct_ptr->idle_time_sec);
  free(struct_ptr->iowait_time_sec);
  free(struct_ptr->irq_time_sec);
  free(struct_ptr->softirq_time_sec);
  free(struct_ptr->steal_time_sec);
  free(struct_ptr->guest_time_sec);
  free(struct_ptr->guest_nice_time_sec);

  free(struct_ptr);

  return;
}

void* cpu_usage_alloc(){

  cpu_usage_t *ret = (cpu_usage_t*) malloc(sizeof(cpu_usage_t));

  //equival calloc

  ret->idle_time_diff_sec = (double*) malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->total_time_diff_sec = (double*) malloc((1 + NUM_PROCESSOR) * sizeof(double));
  ret->cpu_percentage = (double*) malloc((1 + NUM_PROCESSOR) * sizeof(double));

  memset(ret->idle_time_diff_sec, 0, (1 + NUM_PROCESSOR) * sizeof(double));
  memset(ret->total_time_diff_sec, 0, (1 + NUM_PROCESSOR) * sizeof(double));
  memset(ret->cpu_percentage, 0, (1 + NUM_PROCESSOR) * sizeof(double));

  return (void*) ret;
}

void cpu_usage_free(cpu_usage_t* struct_ptr){

  free(struct_ptr->idle_time_diff_sec);
  free(struct_ptr->total_time_diff_sec);
  free(struct_ptr->cpu_percentage);
  
  free(struct_ptr);

  return;
}

void mem_usage(WINDOW *window, int starting_row, int starting_col){
  FILE *fp;
  char *buffer_line = NULL;
  size_t lenght = 0;
  char *cmd_token;
  char *token;
  long unsigned mem_total, mem_available;
  double mem_usage_percentage = 0; 
  //mem_available:mem_total = x : 100
  //1 - sopra
  long unsigned swap_total, swap_free;
  double swap_usage_percentage = 0;

  if ((fp = fopen(PROC_MEMINFO_PATH, "r")) == NULL) exit(EXIT_FAILURE);

  while (getline(&buffer_line, &lenght, fp) != -1){
    
    cmd_token = strtok(buffer_line, SEPARATOR2);
    token = strtok(NULL, SEPARATOR1);

    if (!strcmp(cmd_token, "MemTotal")){
      mem_total = strtoul(token, NULL, 10);
    }else if (!strcmp(cmd_token, "MemAvailable")){
      mem_available = strtoul(token, NULL, 10);
    }else if (!strcmp(cmd_token, "SwapTotal")){
      swap_total = strtoul(token, NULL, 10);
    }else if (!strcmp(cmd_token, "SwapFree")){
      swap_free = strtoul(token, NULL, 10);
      break; //evito altri cicli perché tanto non mi servono le righe successive
    }

  }

  if (mem_total != 0) mem_usage_percentage = 100 - ((double) (mem_available*100) / (double) mem_total);
  if (swap_total != 0) swap_usage_percentage = 100 - ((double) (swap_free*100) / (double) swap_total);

  mvwprintw(window, starting_row, starting_col, "MEM%%: %c", '\0');
  percentage_bar(window, starting_row, starting_col + 10, mem_usage_percentage);
  mvwprintw(window, starting_row + 1, starting_col, "MEM-total:%0.2fGB MEM-used:%0.2fGB MEM-available:%0.2fGB %c", ((double)mem_total / (double)KB_TO_GB), ((double)(mem_total - mem_available) / (double)KB_TO_GB), ((double) mem_available / (double)KB_TO_GB), '\0');

  mvwprintw(window, starting_row + 3, starting_col, "SWAP%%: %c", '\0');
  percentage_bar(window, starting_row + 3, starting_col + 10, swap_usage_percentage);
  mvwprintw(window, starting_row + 4, starting_col, "SWAP-total:%0.2fGB SWAP-used:%0.2fGB SWAP-free:%0.2fGB %c", ((double)swap_total / (double)KB_TO_GB), ((double)(swap_total - swap_free) / (double)KB_TO_GB), ((double) swap_free / (double)KB_TO_GB), '\0');

  free(buffer_line);
  fclose(fp);

  return;
}

void find_process(WINDOW* window, int starting_index, char* string_to_compare){
  if(string_to_compare == NULL)return;
  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;

  regex_t regex_var;
  memset(&regex_var, 0, sizeof(regex_t));
  if(regcomp(&regex_var, string_to_compare, 0) != 0) return;
  
  int max_y = getmaxy(window);

  dirent* proc_iter;
  FILE* file_cmdline;

  char pid_cmdline[PID_CMDLINE_LENGHT];
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  int i = 3, j = 0;//i indica la riga (della finestra) dove stampare, j il processo da stampare
  mvwprintw(window, 1, 2, "| PID | cmdline | %c", '\0');
  box(window, (int)'|', (int)'-');
  wrefresh(window);

  while((proc_iter = readdir(proc_dir)) != NULL && i < (max_y - 1) ){

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

      if(file_cmdline == NULL) continue;

      fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);
      fclose(file_cmdline);
      
      if(strcmp(buffer_cmdline,"\0") == 0) continue; //cmdline vuoto, pid di un processo senza cmdline

      if(j >= starting_index){

        if(regexec(&regex_var, buffer_cmdline,  0, NULL, 0) == 0){//match con la regex (per nome)
          
          mvwprintw(window, i, 2, "%s %s %c", proc_iter->d_name, buffer_cmdline, '\0');
          wrefresh(window);
          i++;
        }

        if (string_to_compare[0] != '\0' && regexec(&regex_var, proc_iter->d_name, 0, NULL, 0) == 0){ ////match con la regex (per PID)
          // match con la regex
          mvwprintw(window, i, 2, "%s %s %c", proc_iter->d_name, buffer_cmdline, '\0');
          wrefresh(window);
          i++;
        }
        
      }
      
      j++;
    }
  }

  regfree(&regex_var);
  closedir(proc_dir);
}

int number_of_regex_matches(char* string_to_compare){ //versione separata

  regex_t regex_var;
  memset(&regex_var, 0, sizeof(regex_t));

  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return 0;
  if(regcomp(&regex_var, string_to_compare, 0) != 0) return 0;

  dirent* proc_iter;
  FILE* file_cmdline;

  char pid_cmdline[PID_CMDLINE_LENGHT];
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  int count = 0;

  while((proc_iter = readdir(proc_dir)) != NULL ){

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

      if(file_cmdline == NULL)continue;
  
      fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);
      fclose(file_cmdline);
      
      if(strcmp(buffer_cmdline,"\0") == 0) continue; //cmdline vuoto, pid di un processo senza cmdline
      
      if(regexec(&regex_var, buffer_cmdline,  0, NULL, 0) == 0){//match con la regex (per nome)
        count++;
      }

      if (string_to_compare[0] != '\0' && regexec(&regex_var, proc_iter->d_name, 0, NULL, 0) == 0){ ////match con la regex (per PID)
        count++;
      }
        
    }
  }

  closedir(proc_dir);
  if(count == 0) count = 1; //perché faccio / o % count, quindi è un errore di divisione per zero, da evitare, approssimato ad 1
  return count;
}

void* pid_order_alloc(){
  //primo nodo (-1)
  pid_order_t* ret = (pid_order_t *)malloc(sizeof(pid_order_t));
  ret->ordering_method = ORDERBY_PID_C;
  ret->num_proc = -1;
  ret->PID = NULL;
  ret->cmdline = NULL;
  //...

  pid_order(ret, ORDERBY_PID_C);
  return (void*) ret;
}

void pid_order_print(pid_order_t *ret, WINDOW *window, int starting_index){

  int i = starting_index, j = 0; // salto diversi incrementi inutili
  //i indica la cella dell'array da cui stampare, j la riga della finestra in cui stampare
  int local_max_y = getmaxy(window);
  local_max_y -= 2; //questione di grafica

  wclear(window);
  box(window, (int)'|', (int)'-');

  while(i < ret->num_proc && j < local_max_y  ){
    mvwprintw(window, 1 + j, 2, "PID: %d %s %c", ret->PID[i], ret->cmdline[i], '\0');
    i++;
    j++;
  }

  wrefresh(window);
  return;
}

void pid_order_free(pid_order_t* ret){

  for (int i = 0; i < ret->max_size; i++){
    free(ret->cmdline[i]);
  }

  free(ret->PID);
  free(ret->cmdline);

  free(ret);

  return;
}

void pid_order_resize(pid_order_t* ret, int new_number_of_processes){

  int new_max_size;

  if(ret->max_size < new_number_of_processes){
    new_max_size = INCREASE_FACTOR * new_number_of_processes;
  }else{
    new_max_size = DECREASE_FACTOR * new_number_of_processes;
  }
  
  for (int i = 0; i < ret->max_size; i++){
    free(ret->cmdline[i]);
  }

  ret->PID = (int*) realloc(ret->PID, new_max_size * sizeof(int));
  ret->cmdline = (char**) realloc(ret->cmdline, new_max_size * sizeof(char*));

  for(int i = 0; i < new_max_size; i++){
    //ret->cmdline[i] = (char*) realloc(ret->cmdline[i], CMD_LINE_LENGHT * sizeof(char));
    ret->cmdline[i] = (char*) malloc( CMD_LINE_LENGHT * sizeof(char));
  }

  ret->max_size = new_max_size;
  ret->num_proc = 0;

  return;
}

void pid_order(pid_order_t *ret, int orderby){

  if(ret == NULL)return;
  
  ret->ordering_method = orderby;
  int cnp = current_number_of_processes();
  ret->num_proc = cnp;

  //resize se troppo piccolo o troppo grande...
  if(ret->max_size <= cnp) pid_order_resize(ret, cnp);
  //if(ret->max_size >= cnp*4 ) pid_order_resize(ret, cnp);

  get_info_of_processes(ret);
  
  if(orderby == ORDERBY_PID_C){
    //già ordinati dal kernel per PID
    return;
  }else if(orderby == ORDERBY_PID_D){
    array_reverse_custom(ret);
    return;
  }else if(orderby == ORDERBY_CMDLINE_C || orderby == ORDERBY_CMDLINE_D){
    string_sort(ret);
    return;
  }
  
  return;
}

void get_info_of_processes2(pid_order_t *ret){

  if(ret == NULL) return;
  
  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;

  int i = 0, q = 0, check = 1;

  dirent* proc_iter;
  FILE* file_cmdline;

  char pid_cmdline[PID_CMDLINE_LENGHT];
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  char *token = NULL;
  char *prev_token = NULL;

  while( i < ret->num_proc && (proc_iter = readdir(proc_dir)) != NULL){

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
        //memset(pid_cmdline, 0, PID_CMDLINE_LENGHT);
        continue;
      }

      fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);

      fclose(file_cmdline);

      if(strcmp(buffer_cmdline,"\0") == 0) continue;

      ret->PID[i] = atoi(proc_iter->d_name);

      //pulisco cmdline da caratteri "sporchi" che impedirebbero un corretto ordinamento corretto
      //da sistemare...
      token = strtok(buffer_cmdline, SEPARATOR3);
      check = 1;
      
      while(check && token != NULL){
        q = 0;

        while(token[q]){
          // ci sono dei cmdline veramente strani es: /../../spotify --params/120120a 90 spot/
          //questo evita di tokenizzare i token se ci sono / oltre la cmdline di interesse
          if(token[q] == ' ') check = 0;
          q++;
        }

        prev_token = token;
        token = strtok(NULL, SEPARATOR3);
        //rimuovo gli spazi dal token
      }

      token = strtok(prev_token, SEPARATOR4);

      if (strcpy(ret->cmdline[i], token) == NULL) return;

      i++;
    }
  }

  closedir(proc_dir);
  return;
}

void string_sort(pid_order_t *ret){
  //PER ORA USO UN SEMPLICE BUBBLE-SORT: O(n^2)
  //credits. https://pencilprogrammer.com/c-programs/sort-names-in-alphabetical-order/
  //preso e modificato

  //NB:sostiture strcpy con strncpy (overflow safety)

  char temp_str[CMD_LINE_LENGHT];
  int temp_int;

  int size_of_array = ret->num_proc;

  for(int i=0; i < size_of_array; i++){
    for(int j=0; j < size_of_array-1-i; j++){

      
      if( ret->ordering_method == ORDERBY_CMDLINE_C && strcmp(ret->cmdline[j], ret->cmdline[j+1]) > 0 ){
        //swap array[j] and array[j+1]

        strncpy(temp_str, ret->cmdline[j], CMD_LINE_LENGHT);
        strncpy(ret->cmdline[j], ret->cmdline[j+1], CMD_LINE_LENGHT);
        strncpy(ret->cmdline[j+1], temp_str, CMD_LINE_LENGHT);

        temp_int = ret->PID[j];
        ret->PID[j] = ret->PID[j+1];
        ret->PID[j+1] = temp_int;

      }else if( ret->ordering_method == ORDERBY_CMDLINE_D && strcmp(ret->cmdline[j], ret->cmdline[j+1]) < 0 ){
        
        strncpy(temp_str, ret->cmdline[j+1], CMD_LINE_LENGHT);
        strncpy(ret->cmdline[j+1], ret->cmdline[j], CMD_LINE_LENGHT);
        strncpy(ret->cmdline[j], temp_str, CMD_LINE_LENGHT);

        temp_int = ret->PID[j+1];
        ret->PID[j+1] = ret->PID[j];
        ret->PID[j] = temp_int;

      }
    }
  }



  return;
}

void array_reverse_custom(pid_order_t *ret){

  int temp_int;
  char temp_str[CMD_LINE_LENGHT];
  int size_of_array = ret->num_proc;

  for(int i = 0; i < size_of_array/2 ; i++){

    strncpy(temp_str, ret->cmdline[i], CMD_LINE_LENGHT);
    strncpy(ret->cmdline[i], ret->cmdline[size_of_array - i - 1], CMD_LINE_LENGHT);
    strncpy(ret->cmdline[size_of_array - i - 1], temp_str, CMD_LINE_LENGHT);

    temp_int = ret->PID[i];
    ret->PID[i] = ret->PID[size_of_array - i - 1];
    ret->PID[size_of_array - i - 1] = temp_int;

  }
}

void get_info_of_processes(pid_order_t *ret){

  if(ret == NULL) return;
  
  DIR* proc_dir;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;

  int i = 0, q = 0, check = 1;

  dirent* proc_iter;
  FILE* file_cmdline;
  FILE* file_stat;

  char pid_cmdline[PID_CMDLINE_LENGHT];
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  char pid_stat[PID_STAT_LENGHT];
  char buffer_stat[BUFFER_STAT_LENGHT];

  char *token = NULL;
  char *prev_token = NULL;

  while( i < ret->num_proc && (proc_iter = readdir(proc_dir)) != NULL){

    memset(buffer_cmdline, 0, BUFFER_CMDLINE_LENGHT);
    memset(pid_cmdline, 0, PID_CMDLINE_LENGHT);
    memset(buffer_stat, 0, BUFFER_STAT_LENGHT);
    memset(pid_stat, 0, PID_STAT_LENGHT);

    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){

      strcpy(pid_cmdline, PROC_PATH);
      strcat(pid_cmdline, "/");
      strcat(pid_cmdline, proc_iter->d_name);
      strcat(pid_cmdline, "/");
      strcat(pid_cmdline, "cmdline");
      strcat(pid_cmdline, "\0");

      strcpy(pid_stat, PROC_PATH);
      strcat(pid_stat, "/");
      strcat(pid_stat, proc_iter->d_name);
      strcat(pid_stat, "/");
      strcat(pid_stat, "stat");
      strcat(pid_stat, "\0");

      if((file_cmdline = fopen(pid_cmdline, "r")) == NULL ){
        continue;
      }

      if ((file_stat = fopen(pid_stat, "r")) == NULL){
        fclose(file_cmdline);
        continue;
      }


      //entrambi i file aperti
      if (fgets(buffer_cmdline, BUFFER_CMDLINE_LENGHT, file_cmdline) == NULL || fgets(buffer_stat, BUFFER_STAT_LENGHT, file_stat) == NULL){ // err
        fclose(file_cmdline);
        fclose(file_stat);
        continue;
      }

      //entrambi i file "letti"

      fclose(file_cmdline);
      fclose(file_stat);

      ret->PID[i] = atoi(proc_iter->d_name);

      //pulisco cmdline da caratteri "sporchi" che impedirebbero un corretto ordinamento corretto
      //da sistemare...
      token = strtok(buffer_cmdline, SEPARATOR3);
      check = 1;
      
      while(check && token != NULL){
        q = 0;

        while(token[q]){
          // ci sono dei cmdline veramente strani es: /../../spotify --params/120120a 90 spot/
          //questo evita di tokenizzare i token se ci sono / oltre la cmdline di interesse
          if(token[q] == ' ') check = 0;
          q++;
        }

        prev_token = token;
        token = strtok(NULL, SEPARATOR3);
        //rimuovo gli spazi dal token
      }

      token = strtok(prev_token, SEPARATOR4);

      if (strcpy(ret->cmdline[i], token) == NULL) return;


      i++;
    }
  }

  closedir(proc_dir);
  return;
}