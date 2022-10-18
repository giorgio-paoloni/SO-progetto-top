#include "TUI.h"

WINDOW* window1;
WINDOW* window2;
WINDOW* window3;
WINDOW* window4;

int has_colors_bool = 0;

struct sigaction signal_handler_struct, signal_handler_struct_old;

int starting_row = 2, starting_process = 0;

int max_y, max_x;

int current_if =  DEFAULT_IF;

void TUI_default_interface(){

  memset(&signal_handler_struct, 0, sizeof(struct sigaction));
  memset(&signal_handler_struct_old, 0, sizeof(struct sigaction));
  signal_handler_struct.sa_handler = &signal_handler;
  signal_handler_struct.sa_flags = 0;
  sigemptyset(&signal_handler_struct.sa_mask);

  if(sigaction(SIGALRM, &signal_handler_struct, &signal_handler_struct_old) == -1 ) perror("errore installazione sigaction!");

  int char_input;
  

  initscr(); //ncurses, inizializza cose...
  if ((has_colors_bool = has_colors())) start_color(); // https://linux.die.net/man/3/start_color
  raw(); //ncurses, disabilita il line buffering dello stdin e segnali come CTRL-C
  noecho();//ncurses, disabilita il print su STDOUT di getch
  curs_set(0); //https://stackoverflow.com/questions/19614156/c-curses-remove-blinking-cursor-from-game, sempre funzione di curses / ncurses
  keypad(stdscr, true); //abilita tasti particolari come FNn o backspace ecc
  //credits. https://stackoverflow.com/questions/27200597/c-ncurses-key-backspace-not-working
  //getyx(stdscr, y, x);//stdscr e' di ncurses, la funzione ottiene la posizione del cursore nella finestra (0,0)
  use_default_colors();// https://invisible-island.net/ncurses/man/default_colors.3x.html
  getmaxyx(stdscr, max_y, max_x);//ottiene la dimensione massima attuale della finestra di terminale

  //questi 2 valori possono essere utili per avere delle posizioni e dimensioni relative alla finestra di terminale, "responsive design",...

  refresh();

  window1 = newwin(3, max_x, 0, 0); //info & commands window
  window2 = NULL;// newwin(8, max_x, 3, 0); //stats window, alla fine ho deciso di implementarla su un'altra schermata, rimane perche' altrimenti dovrei shiftare le finestre di -1 e non voglio creare errori.
  window3 = newwin(max_y-3, max_x, 3, 0);//process list window
  window4 = newwin(3, max_x, 3, 0);;//finestra input testuale visibile, non sempre utilizzata

  //le lascio per controllare se sforo nel terminale, non so se le terro'...
  box(window1, (int) '|', (int) '-');
  //box(window2, (int) '|', (int) '-');%d
  box(window3, (int) '|', (int) '-');

  mvwprintw(window1, 1, 2, "%s %c", "(h)help, (q)quit, (k)kill, (z)sleep, (r)resume, (l)list, (f)find, (s)stats", '\0');
  //mvwprintw(window1, 2, 2, "R = %d C = %d", max_y, max_x);

  wclear(window4);
  wrefresh(window4);

  wrefresh(window1);
  wrefresh(window3);

  //nodelay(stdscr, true);//per non blocking getch, credits. https://gist.github.com/mfcworks/3a32513f26bdc58fd3bd, devo rileggermi bene il man
  nodelay(stdscr, false);
  keypad(stdscr, true);
  
  starting_row = 2, starting_process = 0;
  //i = 2, w = 0; //w tiene il conto del numero dei processi letti, mentre i tiene conto della riga dove stampare nella finestra

  print_proc(window3, 0, 0);

  getmaxyx(stdscr, max_y, max_x);

  alarm(REFRESH_RATE);

  while(1){

    current_if = DEFAULT_IF;

    if(is_term_resized(max_y, max_x)){
      resize_term_custom();
      print_proc(window3, starting_process, starting_row);
      getmaxyx(stdscr, max_y, max_x);
    }

    //avrei potuto usare switch-case, ma non mi piace
    char_input = getch();
    if(char_input == (int) 'q' || char_input == (int) 'Q') break; //l'utente ha inserito q, cioe' QUIT

    if(char_input == (int) 'h' || char_input == (int) 'H'){//l'utente ha inserito h, cioe' HELP
      TUI_help_interface();
      reset_to_default_interface();

    }else if(char_input == (int) 'k' || char_input == (int) 'K'){//l'utente ha inserito k, cioe' kill

      TUI_kill_interface();
      reset_to_default_interface();

    }else if(char_input == (int) 'l' || char_input == (int) 'L'){

      TUI_list_interface();
      reset_to_default_interface();

    }else if(char_input == (int) 's' || char_input == (int) 's'){
      TUI_stats_interface();
      reset_to_default_interface();

    }else if(char_input == (int) 'z' || char_input == (int) 'Z'){
      TUI_sleep_interface();
      reset_to_default_interface();

    }else if(char_input == (int) 'r' || char_input == (int) 'R'){
      TUI_resume_interface();
      reset_to_default_interface();

    }else if(char_input == (int) 'e' || char_input == (int) 'E'){//easter-egg, vorrei implementare un qualcosa alla sl https://github.com/mtoyoda/sl
      //TBD
      TUI_easteregg_inferface();
      //sleep(5);
      reset_to_default_interface();
    }else if(char_input == (int) 'f' || char_input == (int) 'F'){
      //TUI_find_interface();
      reset_to_default_interface();
    }else if (char_input == KEY_UP){

      if(starting_process > 0){
        starting_process--;
      }else{
        starting_process = current_number_of_processes();
      }

      if(starting_row > 0){
        starting_row--;
      }else{
        starting_row = max_y;
      }

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');
      wrefresh(window3);

      print_proc(window3, starting_process, starting_process);

    }else if(char_input == KEY_DOWN){

      starting_process = (starting_process+1)%current_number_of_processes();//current_number_of_process problemi
      starting_row = (starting_row+1)%max_y;

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');
      wrefresh(window3);

      print_proc(window3, starting_process, starting_row);
    }

  }

  //nodelay(stdscr, false);
  keypad(stdscr, false);

  endwin();//ncurses, dealloca le finestre
  clear();

  return;
}

void TUI_kill_interface(){
  current_if = KILL_IF;
  TUI_kill_sleep_resume_interface();
}

void TUI_sleep_interface(){
  current_if = SLEEP_IF;
  TUI_kill_sleep_resume_interface();
}

void TUI_resume_interface(){
  current_if = RESUME_IF;
  TUI_kill_sleep_resume_interface();
}

void TUI_help_interface(){

  current_if = HELP_IF;

  wclear(window1);
  wrefresh(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
  wrefresh(window1);

  wclear(window3);
  wrefresh(window3);//applicare il clear prima di spostarla, altrimenti rimangono dei caratteri sotto

  mvwin(window3, 3, 0);
  wresize(window3, max_y-3, max_x);
  box(window3, (int) '|', (int) '-');
  mvwprintw(window3, 1, 2, HELP_PRINT);
  wrefresh(window3);

  int char_input = getch();

  while(!(char_input == (int) 'b' || char_input == (int) 'B') ){
    char_input = getch();
  }

  refresh();
  return;
}

void TUI_list_interface(){

  current_if = LIST_IF;

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
  wrefresh(window1);

  wclear(window2);
  wrefresh(window2);

  mvwin(window3, 3, 0);
  wresize(window3, max_y-3, max_x);
  wclear(window3);
  wrefresh(window3);
  box(window3, (int) '|', (int) '-'); 

  print_proc_advanced(window3, starting_process, starting_row);

  wrefresh(window3);

  //nodelay(stdscr, false);
  //keypad(stdscr, true);

  int char_input = getch();

  while(!(char_input == (int) 'b' || char_input == (int) 'B')){
    

    if(is_term_resized(max_y, max_x)){
      resize_term_custom();
      print_proc_advanced(window3, starting_process, starting_row);
      getmaxyx(stdscr, max_y, max_x);
    }

    //ciclica
    if (char_input == KEY_UP){

      if(starting_process > 0){
        starting_process--;
      }else{
        starting_process = current_number_of_processes();
      }

      if(starting_row > 0){
        starting_row--;
      }else{
        starting_row = max_y;
      }

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');

      print_proc_advanced(window3, starting_process, starting_row);

    }else if(char_input == KEY_DOWN){

      starting_process = (starting_process+1)%current_number_of_processes();
      starting_row = (starting_row+1)%max_y;

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');

      print_proc_advanced(window3, starting_process, starting_row);

    }

    char_input = getch();
  }

  //nodelay(stdscr, true);
  //keypad(stdscr, false);
  return;
}

void TUI_stats_interface(){

  current_if = STATS_IF;

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');

  wclear(window3);
  wrefresh(window3);

  mvwin(window3, 3, 0);
  wresize(window3, max_y-3, max_x);
  box(window3, (int) '|', (int) '-');

  wrefresh(window1);
  wrefresh(window3);
  wrefresh(window4);

  refresh();

  int char_input = getch();

  while(!(char_input == (int) 'b' || char_input == (int) 'B') ){
    char_input = getch();
    /*if(char_input != ERR)*/ print_stats(window3, starting_process, starting_row);

  }

  return;
}

void TUI_easteregg_inferface(){
  //credits. https://patorjk.com/software/taag/#p=display&f=Graffiti&t=Type%20Something%20
  //informati sul range dei colori permessi dal terminale... per ora è 256
  current_if = EASTEREGG_IF;

  wclear(window1);
  box(window1, (int)'|', (int)'-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
  wrefresh(window1);

  int char_input;

  int i ;

  while(1){

    i = rand() % MAX_TXT;
  
    if(is_term_resized(max_y, max_x)){
      resize_term_custom();
      getmaxyx(stdscr, max_y, max_x);
    }

    wclear(window3);
    wrefresh(window3);

    if (has_colors_bool){
      init_pair(1, (rand() % MAX_COLORS), -1); //-1,-1 indica i colori default del terminale
      wattron(window3, COLOR_PAIR(1));
    }

    print_easteregg(i);

    char_input = getch();//a getch è bloccante, ma andando a vedere sul man se riceve un segnale timer è come se avessi inserito qualcosa (ERR) e va alla riga successiva di esecuzione, ricordo che c'è un timer che ogni 1s lancia il segnale quindi ogni 1s la getch restituisce ERR se non è inserito niente
    

    if (char_input == (int)'b' || char_input == (int)'B') break;

  }


  if(has_colors_bool) wattroff(window3, COLOR_PAIR(1));

  return;
}

void TUI_find_interface(){

  current_if = FIND_IF;

  //NOTA: devo disabilitare b, perché potrebbe essere conenuto nel nome del processo, ora quando premo b in ogni momento torna indietro

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');

  wclear(window3);
  wrefresh(window3);//applicare il clear prima di spostarla, altrimenti rimangono dei caratteri sotto

  wresize(window3, max_y-6, max_x);//IMPORTANTE: ho perso 1 ora a capire il problema, dal man se il mvwin sfora le dimensioni di stdscr (es scorri in basso come questo caso) NON viene applicato, quindi se scorri in basso PRIMA devi ridimensionare delle dimensioni che scorri la finestra!
  //credits. mvwin Calling mvwin moves the window so that the upper left-hand corner is at position (x, y).  If the move would cause the window to be off the screen, it is an error and the window is not moved.  Moving subwindows is allowed, but should be avoided.

  mvwin(window3, 6, 0);
  wrefresh(window3);

  box(window3, (int) '|', (int) '-');
  wrefresh(window3);

  mvwprintw(window3, 1, 2, "test2 in\n");

  box(window4, (int) '|', (int) '-');

  print_proc(window3, 0, 0);

  wrefresh(window1);
  wrefresh(window3);
  wrefresh(window4);

  refresh();

  char window_input[WINDOW_INPUT_LENGHT]; //PID lungo  massimo WINDOW_INPUT_LENGHT caratteri

  int j = 0;
  
  memset(window_input,0,WINDOW_INPUT_LENGHT);

  mvwprintw(window4, 1, 2, "PID/Processo: (Digita il PID o il nome del processo da cercare, invio per confermare)");
  wrefresh(window4);


  while((window_input[j] = (char) getch()) != '\n' && j < WINDOW_INPUT_LENGHT){

    if(window_input[j] == 'b' || window_input[j] == 'B'){//l'utente puo' premere b in ogni momento e annulla l'inserimento del pid
      window_input[0] = 'b';
      break;
    }

    if(j>0 && (window_input[j] == (char) KEY_BACKSPACE || window_input[j] == (char) 127 || window_input[j] == (char) 8 || window_input[j] == (char) '\b')){//l'utente puo' cancellare il testo credits https://stackoverflow.com/questions/44943249/detecting-key-backspace-in-ncurses

      window_input[j] = '\0'; //evita caratteri sporchi
      window_input[j-1] = '\0';//cancella il carattere precedente

      wclear(window4);
      box(window4, (int) '|', (int) '-');
      wrefresh(window4);
      mvwprintw(window4, 1, 2, "PID/Processo: ");
      mvwprintw(window4, 1, 7, "%s", window_input);
      wrefresh(window4);
      j--;
      continue;
    }

    if (window_input[j] == (char) KEY_UP){ //cast a char importante

      if(starting_process > 0){
        starting_process--;
      }else{
        starting_process = current_number_of_processes();
      }

      if(starting_row > 0){
        starting_row--;
      }else{
        starting_row = max_y;
      }

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');
      print_proc(window3, starting_process, starting_row);
      continue;

    }else if(window_input[j] == (char) KEY_DOWN){ //cast a char importante

      starting_process = (starting_process+1)%current_number_of_processes();
      starting_row = (starting_row+1)%max_y;

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');

      print_proc(window3, starting_process, starting_row);
      continue;
    }

    //if(window_input[j] < '0' || window_input[j] > '9') continue; //controllo PID, e' SOLO numerico

    wclear(window4);
    box(window4, (int) '|', (int) '-');
    wrefresh(window4);
    mvwprintw(window4, 1, 2, "PID/Processo: ");
    mvwprintw(window4, 1, 15, "%s", window_input);
    wrefresh(window4);
    j++;
  }

  //nodelay(stdscr, true);

  if(!(window_input[0] == '\n' || window_input[0] == 'b' || window_input[0] == 'B')){
    /*if(kill_PID(atoi(window_input)) == -1){//err
      mvwprintw(window4, 1, j+8, "non ucciso");
    }else{
      mvwprintw(window4, 1, j+8, "ucciso");
    }
    wrefresh(window4);*/

    mvwprintw(window4, 1, j+8, "Risultati ricerca:");

    window_input[0] = getch();
  }

  while(!(window_input[0] == '\n' || window_input[0] == 'b'|| window_input[0] == 'B')){
    window_input[0] = getch();
  }
  return;
}

void reset_to_default_interface(){

  //current_if = DEFAULT_IF;//per le chiamate asincrone di timer interrupt ecc

  mvwprintw(window1, 1, 2, "%s %c", "(h)help, (q)quit, (k)kill, (z)sleep, (r)resume, (l)list, (f)find, (s)stats", '\0');
  wrefresh(window1);

  wclear(window4);
  wrefresh(window4);

  //box(window2, (int) '|', (int) '-');
  //wrefresh(window2);

  wresize(window3, max_y-3, max_x);
  wrefresh(window3);

  mvwin(window3, 3, 0);
  wclear(window3);
  wrefresh(window3);

  box(window3, (int) '|', (int) '-');

  print_proc(window3, starting_process, starting_row);
  wrefresh(window3);

}

void TUI_kill_sleep_resume_interface(){
  
  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');

  wclear(window3);
  wrefresh(window3);//applicare il clear prima di spostarla, altrimenti rimangono dei caratteri sotto
  wresize(window3, max_y-6, max_x);//IMPORTANTE: ho perso 1 ora a capire il problema, dal man se il mvwin sfora le dimensioni di stdscr (es scorri in basso come questo caso) NON viene applicato, quindi se scorri in basso PRIMA devi ridimensionare delle dimensioni che scorri la finestra!
  //credits. mvwin Calling mvwin moves the window so that the upper left-hand corner is at position (x, y).  If the move would cause the window to be off the screen, it is an error and the window is not moved.  Moving subwindows is allowed, but should be avoided.

  mvwin(window3, 6, 0);
  wrefresh(window3);

  box(window3, (int) '|', (int) '-');
  wrefresh(window3);

  //mvwprintw(window3, 1, 2, "test2 in\n");

  box(window4, (int) '|', (int) '-');

  print_proc(window3, starting_process, starting_row);

  wrefresh(window1);
  wrefresh(window3);
  wrefresh(window4);

  refresh();

  char window_input[WINDOW_INPUT_LENGHT]; //PID lungo  massimo WINDOW_INPUT_LENGHT caratteri
  memset(window_input,0,WINDOW_INPUT_LENGHT);

  int  j = 0;
  //j = indica le celle occupate dell'array window_input

  if(current_if == KILL_IF){
    mvwprintw(window4, 1, 2, "PID: (Digita il PID da uccidere, invio per confermare)");
  }else if(current_if == SLEEP_IF){
    mvwprintw(window4, 1, 2, "PID: (Digita il PID da addormentare, invio per confermare)");
  }else{
    mvwprintw(window4, 1, 2, "PID: (Digita il PID da risvegliare, invio per confermare)");
  }
  
  wrefresh(window4);

  while((window_input[j] = (char) getch()) != '\n' && j < WINDOW_INPUT_LENGHT){

    if(is_term_resized(max_y, max_x)){
      resize_term_custom(window1, window2, window3, window4, max_y, max_x, current_if);

      if(j == 0){

        if(current_if == KILL_IF){
          mvwprintw(window4, 1, 2, "PID: (Digita il PID da uccidere, invio per confermare)");
        }else if(current_if == SLEEP_IF){
          mvwprintw(window4, 1, 2, "PID: (Digita il PID da addormentare, invio per confermare)");
        }else{
          mvwprintw(window4, 1, 2, "PID: (Digita il PID da risvegliare, invio per confermare)");
        }

      }else{
        window_input[j] = '\0'; //evita caratteri sporchi
        mvwprintw(window4, 1, 2, "PID: ");
        mvwprintw(window4, 1, 7, "%s", window_input);
      }
      wrefresh(window4);

      print_proc(window3, starting_process, starting_row);

      getmaxyx(stdscr, max_y, max_x);
      continue;
    }

    if(window_input[j] == 'b' || window_input[j] == 'B'){//l'utente puo' premere b in ogni momento e annulla l'inserimento del pid
      window_input[0] = 'b';
      break;
    }

    if(j>0 && (window_input[j] == (char) KEY_BACKSPACE || window_input[j] == (char) 127 || window_input[j] == (char) 8 || window_input[j] == (char) '\b')){//l'utente puo' cancellare il testo credits https://stackoverflow.com/questions/44943249/detecting-key-backspace-in-ncurses

      window_input[j] = '\0'; //evita caratteri sporchi
      window_input[j-1] = '\0';//cancella il carattere precedente

      wclear(window4);
      box(window4, (int) '|', (int) '-');
      wrefresh(window4);
      mvwprintw(window4, 1, 2, "PID: ");
      mvwprintw(window4, 1, 7, "%s", window_input);
      wrefresh(window4);
      j--;
      continue;
    }

    if (window_input[j] == (char) KEY_UP){ //cast a char importante

      if(starting_process > 0){
        starting_process--;
      }else{
        starting_process = current_number_of_processes();
      }

      if(starting_row > 0){
        starting_row--;
      }else{
        starting_row = max_y;
      }

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');
      print_proc(window3, starting_process, starting_row);
      continue;

    }else if(window_input[j] == (char) KEY_DOWN){ //cast a char importante

      starting_process = (starting_process+1)%current_number_of_processes();
      starting_row = (starting_row+1)%max_y;

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');

      print_proc(window3, starting_process, starting_process);
      continue;
    }

    if(window_input[j] < '0' || window_input[j] > '9') continue; //controllo PID, e' SOLO numerico

    wclear(window4);
    box(window4, (int) '|', (int) '-');
    wrefresh(window4);
    mvwprintw(window4, 1, 2, "PID: ");
    mvwprintw(window4, 1, 7, "%s", window_input);
    wrefresh(window4);
    j++;
  }

  if(!(window_input[0] == '\n' || window_input[0] == 'b' || window_input[0] == 'B')){
    if(current_if == KILL_IF){

      if(kill_PID(atoi(window_input)) == -1){//err
        mvwprintw(window4, 1, j+8, "non ucciso");
      }else{
        mvwprintw(window4, 1, j+8, "ucciso");
      }

    }else if(current_if == SLEEP_IF){

      if(sleep_PID(atoi(window_input)) == -1){//err
        mvwprintw(window4, 1, j+8, "non addormentato");
      }else{
        mvwprintw(window4, 1, j+8, "addormentato");
      }
      
    }else{

      if(resume_PID(atoi(window_input)) == -1){//err
        mvwprintw(window4, 1, j+8, "non risvegliato");
      }else{
        mvwprintw(window4, 1, j+8, "risvegliato");
      }

    }

    wrefresh(window4);

    window_input[0] = getch();
  }

  while(!(window_input[0] == '\n' || window_input[0] == 'b'|| window_input[0] == 'B')){
    window_input[0] = getch();
  }

  return;

}

void resize_term_custom(){ 
  //c'è resizeterm, ma viene consigliato in caso di layout complicati di ridimensionare e muovere manualemente, credits. https://invisible-island.net/ncurses/man/resizeterm.3x.html
  
  int new_max_y, new_max_x;
  getmaxyx(stdscr, new_max_y, new_max_x);

  wclear(window1);
  //wclear(window2);
  wclear(window3);
  wclear(window4);

  wrefresh(window1);
  //wrefresh(window2);
  wrefresh(window3);
  wrefresh(window4);

  if(current_if == DEFAULT_IF){
    wresize(window1, 3, new_max_x);
    mvwin(window1, 0, 0);
    wrefresh(window1);
    box(window1, (int) '|', (int) '-');

    mvwprintw(window1, 1, 2, "%s %c", "(h)help, (q)quit, (k)kill, (z)sleep, (r)resume, (l)list, (f)find, (s)stats", '\0');

    //mvwprintw(window1, 1, 2, "RESIZED"); //TEST
    wrefresh(window1);

    wresize(window3, new_max_y-3, new_max_x);
    mvwin(window3, 3, 0);
    wrefresh(window3);
    box(window3, (int) '|', (int) '-');
    wrefresh(window3);

  }else if(current_if == KILL_IF || current_if == SLEEP_IF || current_if == RESUME_IF ){
    wresize(window1, 3, new_max_x);
    mvwin(window1, 0, 0);
    wrefresh(window1);
    box(window1, (int) '|', (int) '-');
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    wrefresh(window1);

    wresize(window3, new_max_y-6, new_max_x);
    mvwin(window3, 6, 0);
    wrefresh(window3);
    box(window3, (int) '|', (int) '-');
    wrefresh(window3);

    wresize(window4, 3, new_max_x);
    mvwin(window4, 3, 0);
    wrefresh(window4);
    box(window4, (int) '|', (int) '-');
    //mvwprintw(window4, 1, 2, "PID: (Digita il PID da uccidere, invio per confermare)");
    wrefresh(window4);
  }else if(current_if == EASTEREGG_IF){

    box(window1, (int)'|', (int)'-');
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    wrefresh(window1);

  }else if(current_if == LIST_IF){
    //da sistemare
    wresize(window1, 3, new_max_x);
    mvwin(window1, 0, 0);
    wrefresh(window1);
    box(window1, (int) '|', (int) '-');
    wrefresh(window1);
    
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    
    wrefresh(window1);

    wresize(window3, new_max_y-3, new_max_x);
    mvwin(window3, 3, 0);
    wrefresh(window3);
    box(window3, (int) '|', (int) '-');
    wrefresh(window3);
  }//etc...

  refresh();
  
}

void refresh_UI(){ 
  if(current_if == HELP_IF || current_if == EASTEREGG_IF ) return;
  //devo differenziare tra le le UI chiamanti
  wclear(window3);
  wrefresh(window3);
  box(window3, (int) '|', (int) '-');

  if(current_if == STATS_IF){
    print_stats(window3, starting_process, starting_row);
  }else if(current_if != LIST_IF){
    print_proc(window3, starting_process, starting_row);
  }else{
    print_proc_advanced(window3, starting_process, starting_row);
  }

  return;
}

void signal_handler(int sig){
  //TBD
  //stavo leggendo che non è una buona pratica installare un allarme così (a causa del context switch), informati... per ora lo implemento così per vedere se funziona
  refresh_UI();
  alarm(REFRESH_RATE);
  //return;
}

void print_easteregg(int i){
  //snippet credits. https://stackoverflow.com/questions/3463426/in-c-how-should-i-read-a-text-file-and-print-all-strings
  //copiato e modificato
  //devo farlo con dei file perché gli ASCII-TEXT-ART stampati sono multilinea non riesco a definirli come macro define
  //si è uno spreco, ma questa parte di codice non verrà chiamata sempre

  char val[VAL_LENGHT];
  char buf[BUF_LENGHT1];
  char EE_BOT_full_path[CUSTOM_LENGHT1];

  memset(EE_BOT_full_path, 0, CUSTOM_LENGHT1);
  memset(buf, 0, BUF_LENGHT1);
  memset(val, 0, VAL_LENGHT);

  sprintf(val, "%d", i);

  strcat(EE_BOT_full_path, EE_BOT_PATH);
  strcat(EE_BOT_full_path, "/");
  strcat(EE_BOT_full_path, "bot");
  strcat(EE_BOT_full_path, val);
  strcat(EE_BOT_full_path, ".txt");
  strcat(EE_BOT_full_path, "\0");
  
  FILE *file = NULL;
  size_t nread = 0;

  file = fopen(EE_BOT_full_path, "r");
  if(!file) return;

  while ((nread = fread(buf, 1, sizeof(buf), file)) > 0){
    mvwprintw(window3, 1, 1, "%s %c", buf, '\0');
    wrefresh(window3);
  }

  fclose(file);

}