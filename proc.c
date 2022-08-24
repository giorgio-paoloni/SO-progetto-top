#include "proc.h"

#define CMD_LINE_LENGHT 7

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

/*void print_proc(){

  DIR* proc_dir;

  dirent* proc_iter;
  int proc_strlen = strlen(PROC_PATH);

  char* pid_path;
  char* pid_cmdline;

  FILE* file_cmdline;
  #define BUFFER_CMDLINE_LENGHT 256
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  int bytes_read;
  int char_input;

  //initscr(); //ncurses
  //raw(); //ncurses, disabilita il line buffering dello stdin
  //noecho();//ncurses, disabilita il print su STDOUT di getch

  //while(1){
    if((proc_dir = opendir(PROC_PATH)) == NULL) return;

    while((proc_iter = readdir(proc_dir)) != NULL){
      if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){ //it's a PID directory
        //navigate nested directory in /PROC

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
          printf("err\n");
          continue;
        }

        bytes_read = fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);
        //buffer_cmdline[bytes_read] = (char) "\0";
        //printf("bytes r: %d ", bytes_read);

        fclose(file_cmdline);

        //if(strcmp(buffer_cmdline,"\0") == 0) continue;

        printf("PID: %s, pid_path: %s, cmdline: %s\n", proc_iter->d_name, pid_path, buffer_cmdline);

        strcpy(buffer_cmdline, "\0");

        free(pid_path);
      }
    }

    closedir(proc_dir);


    //char_input = getch();
    //if(char_input == (int) 'q') break; //l'utente ha inserito q, cioe' QUIT
    //if(char_input == (int) 'h'){//l'utente ha inserito h, cioe' HELP


    //};
    //sleep(3);

    //system("clear"); //shell command "clear" or CTRL+A+L
  //}

  //endwin();//ncurses
}*/ //print LEGACY

void print_proc2(WINDOW* window){ //variante 2 usata in certe funzioni

  DIR* proc_dir;

  dirent* proc_iter;
  int proc_strlen = strlen(PROC_PATH);

  char* pid_path;
  char* pid_cmdline;

  FILE* file_cmdline;
  #define BUFFER_CMDLINE_LENGHT 256
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  int bytes_read;
  int char_input;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;

  int i = 3;

  mvwprintw(window, 1, 2, "| PID | pid_path | cmdline |\n");

  while((proc_iter = readdir(proc_dir)) != NULL){
    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){ //E' una cartella PID
      //navigo le cartelle annidate in /PROC

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
        //printf("err\n");
        continue;
      }

      bytes_read = fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);
      //buffer_cmdline[bytes_read] = (char) "\0";
      //printf("bytes r: %d ", bytes_read);

      fclose(file_cmdline);

      if(strcmp(buffer_cmdline,"\0") == 0) continue;

      //printf("PID: %s, pid_path: %s, cmdline: %s\n", proc_iter->d_name, pid_path, buffer_cmdline);

      strcpy(buffer_cmdline+strlen(buffer_cmdline), "\0");
      //if(i >10) continue;
      /*mvwprintw(window, i, 2, "PID: %s, pid_path: %s, cmdline: %s\n", proc_iter->d_name, pid_path, buffer_cmdline);*/

      mvwprintw(window, i, 2, "%s  %s  %s\n", proc_iter->d_name, pid_path, buffer_cmdline);

      i++;

      wrefresh(window);

      strcpy(buffer_cmdline, "\0");

      free(pid_path);
    }
  }

  closedir(proc_dir);
}

void print_proc3(WINDOW* window, int start_row){//variante 3

  DIR* proc_dir;

  dirent* proc_iter;
  int proc_strlen = strlen(PROC_PATH);

  char* pid_path;
  char* pid_cmdline;

  FILE* file_cmdline;
  #define BUFFER_CMDLINE_LENGHT 256
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  int bytes_read;
  int char_input;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;

  int i = 3;

  mvwprintw(window, 1, 2, "| PID | pid_path | cmdline |\n");

  int j = 0;

  while((proc_iter = readdir(proc_dir)) != NULL){
    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){ //it's a PID directory
      //navigate nested directory in /PROC

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
        printf("err\n");
        continue;
      }

      bytes_read = fread(&buffer_cmdline, sizeof(char), BUFFER_CMDLINE_LENGHT, file_cmdline);
      //buffer_cmdline[bytes_read] = (char) "\0";
      //printf("bytes r: %d ", bytes_read);

      fclose(file_cmdline);

      if(strcmp(buffer_cmdline,"\0") == 0) continue;

      //printf("PID: %s, pid_path: %s, cmdline: %s\n", proc_iter->d_name, pid_path, buffer_cmdline);

      strcpy(buffer_cmdline+strlen(buffer_cmdline), "\0");
      //if(i >10) continue;
      if(j >= start_row){

        mvwprintw(window, i, 2, "%s  %s  %s\n", proc_iter->d_name, pid_path, buffer_cmdline);
        i++;
        wrefresh(window);
      }

      strcpy(buffer_cmdline, "\0");

      free(pid_path);

      j++;
    }
  }

  closedir(proc_dir);
}

void print_proc_advanced(WINDOW* window, int start_row){//variante 3

  DIR* proc_dir;

  dirent* proc_iter;
  int proc_strlen = strlen(PROC_PATH);

  char* pid_path;
  char* pid_cmdline;

  FILE* file_cmdline;

  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];
  char* pid_stat;

  int bytes_read;
  int char_input;
  if((proc_dir = opendir(PROC_PATH)) == NULL) return;

  int i = 3;

  mvwprintw(window, 1, 2, "| PID | pid_path | cmdline |\n");

  int j = 0;

  while((proc_iter = readdir(proc_dir)) != NULL){
    if(is_pid(proc_iter->d_name) && proc_iter->d_type == DT_DIR){ //it's a PID directory
      //navigate nested directory in /PROC

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
      if(file_cmdline == NULL) continue;

      fclose(file_cmdline);

      //ok
      //mvwprintw(window, 1, 2,"ok");
      //wrefresh(window);

      //if(strcmp(buffer_cmdline,"\0") == 0) continue;

      strcpy(buffer_cmdline+strlen(buffer_cmdline), "\0");
      //if(i >10) continue;
      if(j >= start_row){

        //mvwprintw(window, i, 2, "%s  %s  %s\n", proc_iter->d_name, pid_path, buffer_cmdline);

        pid_stat = (char*) malloc((proc_strlen + 1 + strlen("stat")) * sizeof(char));
        strcpy(pid_stat, pid_path);
        strcat(pid_stat, "/");
        strcat(pid_stat, "stat");

        print_PID_stats(window, i, pid_stat);//il problema e' qui, credo

        free(pid_stat);
        i++;
        wrefresh(window);

      }

      strcpy(buffer_cmdline, "\0");

      free(pid_path);

      j++;
    }
  }

  closedir(proc_dir);
}

void print_PID_stats(WINDOW* window, int y, char* path){

  #define BUFFER_STAT_LENGHT 128


  FILE* file_stat;
  char* buffer_stat = (char*) malloc(BUFFER_STAT_LENGHT*sizeof(char));
  char* token = (char*) malloc(32*sizeof(char));//?

  //const char* separator = " \t\n";

  long int frequency = sysconf(_SC_CLK_TCK);//dal man proc

  long unsigned int system_uptime_sec = 1;
  long unsigned int total_time_sec = 0;
  long unsigned int user_time_clock = 0, user_time_sec = 0;
  long unsigned int superuser_time_clock = 0, superuser_time_sec = 0;
  long long unsigned int start_time_clock = 0, start_time_sec = 0;
  long unsigned int elapsed_time_sec;
  double cpu_percentage_used_time_sec;
  char state;
  long int priority = 0;
  int i = 1 ;

  /*mvwprintw(window, 1, 2, "%s\n", path);
  wrefresh(window);*/

  if((file_stat = fopen(path, "r")) == NULL){//err
    fclose(file_stat);
    free(buffer_stat);
    free(token);
    return;
  }

  /*if(fgets(buffer_stat, BUFFER_STAT_LENGHT,file_stat) == NULL){
    fclose(file_stat);
    free(buffer_stat);
    free(token);
    return;
  }*/
  /*fgets(buffer_stat, BUFFER_STAT_LENGHT,file_stat);
  mvwprintw(window, 1, 2, "%s\n", buffer_stat);
  wrefresh(window);
  return;*/
  /*mvwprintw(window, 1, 2, "ok5");
  wrefresh(window);
  return;*/

  token = strtok(buffer_stat, SEPARATOR1);

  while(token != NULL && i < MAX_TOKEN1){//strtok
    //credits. https://man7.org/linux/man-pages/man5/procfs.5.html
    //credits. https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat
    //credits. https://www.baeldung.com/linux/total-process-cpu-usage (altro linguaggio ma comprensibile e riscribile in C, nella guida dice starttime e' 21, invece e' 22 da man proc)

    //i campi sono oltre 50, non tutti ci servono
    //ricordo che sono espressi in Hertz (1/T), quindi devo ottenere la durata di T dal sistema

    if(i == 3){ //state  %c
      state = token;
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
    }

    i++;
    token = strtok(NULL, SEPARATOR1);
  }

  //tempi utili
  user_time_sec = user_time_clock / frequency;
  superuser_time_sec = superuser_time_clock / frequency;
  start_time_sec = start_time_clock / frequency;
  total_time_sec = user_time_sec + superuser_time_sec;
  system_uptime_sec = get_system_uptime();
  elapsed_time_sec = system_uptime_sec - start_time_sec;  
  cpu_percentage_used_time_sec = (total_time_sec*100)/elapsed_time_sec;

  mvwprintw(window, y, 10, "%lu  %lu  %lf\n", user_time_sec, superuser_time_sec, cpu_percentage_used_time_sec);
  wrefresh(window);

  fclose(file_stat);
  free(buffer_stat);
  free(token);
  return;
}

long unsigned int get_system_uptime(){
  //credits.https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/deployment_guide/s2-proc-uptime
  //credits. man proc

  FILE* file_proc_uptime;
  char* buffer = (char*) malloc(256*sizeof(char));
  file_proc_uptime = fopen(PROC_UPTIME_PATH, "r");
  char* token = (char*) malloc(32*sizeof(char));

  if(file_proc_uptime == NULL){//err
    fclose(file_proc_uptime);
    free(buffer);
    free(token);
    return 11;
  }

  fgets(buffer, 256,file_proc_uptime);
  /*if(fgets(buffer, 256,file_proc_uptime) == NULL ){
    fclose(file_proc_uptime);
    free(buffer);
    free(token);
    return 11;
  }*/

  token = strtok(buffer, SEPARATOR1);

  fclose(file_proc_uptime);
  free(buffer);
  //free(token);

  return atol(token);

}


//"fortunatamente" non essendo un programma che deve girare su un sistema con poca ram posso permettermi questi sprechi di memoria.
