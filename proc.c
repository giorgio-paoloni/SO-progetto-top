#include "proc.h"

#define CMD_LINE_LENGHT 7

int is_pid(char* name){
  //Sfrutto questo metodo perche' leggendo la documentazione (*), in /proc dir, non ci sono solo cartelle PID
  //*https://man7.org/linux/man-pages/man5/proc.5.html
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

//"fortunatamente" non essendo un programma che deve girare su un sistema con poca ram posso permettermi questi sprechi di memoria.
