#include "proc.h"

#define CMD_LINE_LENGHT 7

int is_pid(char* name){
  //I'm doing that because if you read the documentation (*), in /proc dir, there are not only PID folders
  //*https://man7.org/linux/man-pages/man5/proc.5.html
  int i = 0;
  while(name[i] != '\0'){
    if(!(name[i] >= '0' && name[i] <= '9')) return 0;
    i++;
  }
  return 1;
}

void print_proc(){

  DIR* proc_dir;

  dirent* proc_iter;
  int proc_strlen = strlen(PROC_PATH);

  char* pid_path;
  char* pid_cmdline;

  FILE* file_cmdline;
  #define BUFFER_CMDLINE_LENGHT 256
  char buffer_cmdline[BUFFER_CMDLINE_LENGHT];

  int bytes_read;

  while(1){
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

        printf("PID: %s, pid_path: %s, cmdline: %s\n", proc_iter->d_name, pid_path, buffer_cmdline);

        strcpy(buffer_cmdline, "\0");

        free(pid_path);
      }
    }
    closedir(proc_dir);

    sleep(3);
  }

}
