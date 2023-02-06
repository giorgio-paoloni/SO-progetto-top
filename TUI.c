#include "TUI.h"

struct winsize size;

WINDOW* window1;
WINDOW* window2;
WINDOW* window3;
WINDOW* window4;

sem_t sem1;
int sem1_val;
pthread_t t1;

cpu_usage_t* cpu_usage_var; //= NULL;
cpu_snapshot_t* cpu_snapshot_t0; // = NULL;
cpu_snapshot_t* cpu_snapshot_t1;// = NULL;

struct timespec sleep_value = {0};

int has_colors_bool = 0;

struct sigaction signal_handler_struct, signal_handler_struct_old;

int starting_row = 2, starting_process = 0;
int find_starting_process = 0; // NB:diverso da starting_process e find_starting_process
int orderby_starting_process = 0; // NB:diverso da starting_process e find_starting_process

pid_order_t *pid_order_v;// = (pid_order_t *)pid_order_alloc();

int max_y, max_x;

int current_if =  DEFAULT_IF;

char window_input[WINDOW_INPUT_LENGHT]; // PID lungo  massimo WINDOW_INPUT_LENGHT caratteri

void TUI_default_interface(){
  
  cpu_snapshot_t0 = cpu_snapshot_alloc(0);
  cpu_snapshot_t1 = cpu_snapshot_alloc(1);

  pid_order_v = (pid_order_t *) pid_order_alloc(); //free

  sleep_value.tv_nsec = INTERVAL_MS;
  sleep_value.tv_sec = INTERVAL_S;

  sem_init(&sem1, 0, 1);
  cpu_usage_var = (cpu_usage_t *) cpu_usage_alloc();
  
  pthread_create(&t1, NULL, cpu_usage_thread_wrapper, NULL);
  pthread_detach(t1); // la inzializzo, non è aggiornata all'ultimo istante ma da l'impressione all'utente di averlo istantaneamente

  memset(&signal_handler_struct, 0, sizeof(struct sigaction));
  memset(&signal_handler_struct_old, 0, sizeof(struct sigaction));
  signal_handler_struct.sa_handler = &signal_handler;
  signal_handler_struct.sa_flags = 0;
  sigemptyset(&signal_handler_struct.sa_mask);
  

  if(sigaction(SIGALRM, &signal_handler_struct, &signal_handler_struct_old) == -1 ) return; 
  if(sigaction(SIGWINCH, &signal_handler_struct, &signal_handler_struct_old) == -1 ) return;

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

  mvwprintw(window1, 1, 2, "%s %c", "(h)help, (q)quit, (k)kill, (z)sleep, (r)resume, (l)list, (f)find, (o)orderby, (s)stats", '\0');
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
    }else if(char_input == (int) 'e' || char_input == (int) 'E'){
      TUI_easteregg_inferface();
      reset_to_default_interface();
    }else if(char_input == (int) 'f' || char_input == (int) 'F'){
      TUI_find_interface();
      reset_to_default_interface();
    }else if(char_input == (int) 'o' || char_input == (int) 'O'){
      TUI_orderby_interface();
      reset_to_default_interface();
    }else if (char_input == KEY_UP){

      if(starting_process > 0){
        starting_process--;
      }else{
        starting_process = current_number_of_processes()-1;
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

  // utile per memleak
  _nc_free_and_exit();
  // --disable-leaks, per i warning memleak di valgrind
  // https://stackoverflow.com/questions/32410125/valgrind-shows-memory-leaks-from-ncurses-commands-after-using-appropriate-free
  // https://invisible-island.net/ncurses/ncurses.faq.html#config_leaks (Testing for Memory Leaks)

  //tutti still reachable sono liberati qui
  sem_destroy(&sem1);
  cpu_usage_free(cpu_usage_var);

  cpu_snapshot_free(cpu_snapshot_t0);
  cpu_snapshot_free(cpu_snapshot_t1);

  /*cpu_usage_var = NULL;
  cpu_snapshot_t0 = NULL;
  cpu_snapshot_t1 = NULL;*/

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
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
  wrefresh(window1);

  wclear(window3);
  mvwprintw(window3, 1, 2, HELP_PRINT);
  box(window3, (int)'|', (int)'-');
  wrefresh(window3);

  int char_input = getch();

  while(!(char_input == (int) 'b' || char_input == (int) 'B') ){
    char_input = getch();
  }

  return;
}

void TUI_list_interface(){

  current_if = LIST_IF;

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
  wrefresh(window1);


  wclear(window3);
  box(window3, (int) '|', (int) '-'); 
  print_proc_advanced(window3, starting_process, starting_row);
  wrefresh(window3);

  int char_input = getch();

  while(!(char_input == (int) 'b' || char_input == (int) 'B')){
    
    //ciclica
    if (char_input == KEY_UP){

      if(starting_process > 0){
        starting_process--;
      }else{
        starting_process = current_number_of_processes()-1;
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

  return;
}

void TUI_stats_interface(){

  current_if = STATS_IF;

  pthread_create(&t1, NULL, cpu_usage_thread_wrapper, NULL);
  pthread_detach(t1);

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
  wrefresh(window1);

  wclear(window3);

  box(window3, (int) '|', (int) '-');
  wrefresh(window3);

  //non amo il do..while 
  int char_input = (int) ' '; //valore a caso... ma diverso da B o b

  while( !(char_input == (int) 'b' || char_input == (int) 'B') ){
    print_stats(window3, 0, 0);

    if(sem_getvalue(&sem1, &sem1_val) == -1) exit(EXIT_FAILURE);

    if(sem1_val){ //evito di generare troppi thread se tanto il semaforo è bloccato
      pthread_create(&t1, NULL, cpu_usage_thread_wrapper, NULL);
      pthread_detach(t1);
    }

    char_input = getch();
  
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

    if(has_colors_bool) wattroff(window3, COLOR_PAIR(1));
    box(window3, (int)'|', (int)'-');
    wrefresh(window3);

    char_input = getch();//a getch è bloccante, ma andando a vedere sul man se riceve un segnale timer è come se avessi inserito qualcosa (ERR) e va alla riga successiva di esecuzione, ricordo che c'è un timer che ogni 1s lancia il segnale quindi ogni 1s la getch restituisce ERR se non è inserito niente

    if (char_input == (int)'b' || char_input == (int)'B') break;

  }

  if(has_colors_bool) wattroff(window3, COLOR_PAIR(1));

  return;
}

void TUI_find_interface(){

  current_if = FIND_IF;

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "%s %c", "premi INVIO per terminare la ricerca...", '\0');
  wrefresh(window1);

  wclear(window3);
  
  wresize(window3, max_y-6, max_x);
  mvwin(window3, 6, 0);
  box(window3, (int) '|', (int) '-');
  wrefresh(window3);

  wclear(window4);
  box(window4, (int) '|', (int) '-');
  mvwprintw(window4, 1, 2, "Ricerca: (Digita il processo o il PID da cercare, invio per terminare la ricerca)");
  wrefresh(window4);

  int j = 0;
  memset(window_input,0,WINDOW_INPUT_LENGHT);

  find_process(window3, starting_process, window_input);
  char get_input;

  while((get_input = (char) getch()) != '\n' && j < WINDOW_INPUT_LENGHT){

    if((get_input == (char) KEY_BACKSPACE || get_input == (char) 127 || get_input == (char) 8 || get_input == (char) '\b')){//l'utente puo' cancellare il testo credits https://stackoverflow.com/questions/44943249/detecting-key-backspace-in-ncurses
      
      find_starting_process = 0;

      if(j == 0){
        window_input[0] = '\0'; // evita caratteri sporchi
        continue;
      }

      window_input[j] = '\0'; //evita caratteri sporchi
      window_input[j-1] = '\0';//cancella il carattere precedente
      wclear(window4);
      box(window4, (int)'|', (int)'-');

      if(j == 1){
        mvwprintw(window4, 1, 2, "Ricerca: (Digita il processo o il PID da cercare, invio per terminare la ricerca)");
      }else{
        mvwprintw(window4, 1, 2, "Ricerca: ");
        mvwprintw(window4, 1, 12, "%s", window_input);
      }

      wrefresh(window4);
      j--;

      wclear(window3);
      wrefresh(window3);
      find_process(window3, starting_process, window_input);
      continue;
    }

    if (get_input == (char) KEY_UP){

      //window_input[j] = '\0';

      if(find_starting_process > 0){
        find_starting_process--; 
      }else{
        find_starting_process = number_of_regex_matches(window_input)-1;
      }

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');
      find_process(window3, find_starting_process, window_input);

      //mvwprintw(window3, 10, 1, "%d %c", find_starting_process, '\0');
      //wrefresh(window3);
      continue;

    }else if(get_input == (char) KEY_DOWN){ //cast a char importante
      //window_input[j] = '\0';

      find_starting_process = (find_starting_process + 1) % number_of_regex_matches(window_input);

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');

      find_process(window3, find_starting_process, window_input);

      continue;
    }else if(!( (get_input >= '0' && get_input <= '9') ||( (get_input >= 'A' && get_input <= 'Z') ) ||  (get_input >= 'a' && get_input <= 'z') ) ){
      continue; //non è alpha-numerico
    }

    //prima non viene salvato in window_input
    window_input[j] = get_input;

    find_starting_process = 0;

    wclear(window3);
    wrefresh(window3);
    find_process(window3, find_starting_process, window_input);

    wclear(window4);
    box(window4, (int) '|', (int) '-');
    wrefresh(window4);
    mvwprintw(window4, 1, 2, "Ricerca: ");
    mvwprintw(window4, 1, 12, "%s %c", window_input, '\0');
    wrefresh(window4);
    j++;
  }

  memset(window_input, 0, WINDOW_INPUT_LENGHT);//leak dati?

  return;
}

void reset_to_default_interface(){

  mvwprintw(window1, 1, 2, "%s %c", "(h)help, (q)quit, (k)kill, (z)sleep, (r)resume, (l)list, (f)find, (o)orderby, (s)stats", '\0');
  wrefresh(window1);

  //box(window2, (int) '|', (int) '-');
  //wrefresh(window2);

  wclear(window4); //prima di window3 per una questione di UI
  wrefresh(window4);

  wresize(window3, max_y-3, max_x);
  mvwin(window3, 3, 0);
  wclear(window3);
  box(window3, (int) '|', (int) '-');
  print_proc(window3, starting_process, starting_row);
  wrefresh(window3);

  memset(window_input, 0, WINDOW_INPUT_LENGHT); //questione data leak
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
        starting_process = current_number_of_processes() -1 ;
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

      if(kill_PID(atoi(window_input)) < 0){//err
        mvwprintw(window4, 1, j+8, "non ucciso");
      }else{
        mvwprintw(window4, 1, j+8, "ucciso");
      }

    }else if(current_if == SLEEP_IF){

      if(sleep_PID(atoi(window_input)) < 0){//err
        mvwprintw(window4, 1, j+8, "non addormentato");
      }else{
        mvwprintw(window4, 1, j+8, "addormentato");
      }
      
    }else{

      if(resume_PID(atoi(window_input)) < 0){//err
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

  memset(window_input, 0, WINDOW_INPUT_LENGHT); // leak dati?
  return;

}

void TUI_orderby_interface(){

  //sintassi:
  //C/D: crescente-decrescente
  //0/1/2/3/4: PID/cmdline/...

  current_if = ORDERBY_IF;

  orderby_starting_process = 0;
  wclear(window1);
  box(window1, (int)'|', (int)'-');
  mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
  wrefresh(window1);

  wclear(window3);
  wrefresh(window3);                  
  wresize(window3, max_y - 6, max_x); 
  mvwin(window3, 6, 0);
  box(window3, (int)'|', (int)'-');
  wrefresh(window3);

  box(window4, (int)'|', (int)'-');
  mvwprintw(window4, 1, 2,"%s %c", "Ordina per: (Inserisci il tipo di ordinamento, invio per confermare)", '\0');
  wrefresh(window4);

  mvwprintw(window3, 1, 2, "%s %c", "Attesa inserimento metodo di ordinamento... (invio per confermare)", '\0');
  mvwprintw(window3, 3, 2, "%s %c", "Sintassi: <ORDINE><CAMPO> ", '\0');
  mvwprintw(window3, 4, 2, "%s %c", "<ORDINE>: (c)crescente (d)decrescente", '\0');
  mvwprintw(window3, 5, 2, "%s %c", "<CAMPO>: (0)PID (1)cmdline (2)RES (3)VIRT (4)CPU% (5)MEM%", '\0');
  mvwprintw(window3, 6, 2, "%s %c", "Es: c1 <=> ordinamento <crescente> tramite il campo di <cmdline> ", '\0');
  wrefresh(window3);

  memset(window_input, 0, WINDOW_INPUT_LENGHT);

  int j = 0;// j = indica le celle occupate dell'array window_input
  char get_input;
  // WINDOW_INPUT_LENGHT2 ridotto a 3 caratteri
  while (j < WINDOW_INPUT_LENGHT2){

    get_input = getch();

    if (get_input == '\n'|| get_input == 'b' || get_input == 'B'){
      if(j == 2 && get_input == '\n') break;

      memset(window_input, 0, WINDOW_INPUT_LENGHT); // leak dati?
      return;
      
    }

    if (get_input == (char)KEY_BACKSPACE || get_input == (char)127 ||get_input == (char)8 || get_input == (char)'\b'){

      wclear(window4);
      box(window4, (int)'|', (int)'-');

      if(j == 0){
        window_input[0] = '\0'; // evita caratteri sporchi
        continue;
      }

      window_input[j] = '\0'; //evita caratteri sporchi
      window_input[j-1] = '\0';//cancella il carattere precedente

      if(j == 1){
        mvwprintw(window4, 1, 2,"Ordina per: (Inserisci il tipo di ordinamento, invio per confermare) %c", '\0');
        //mvwprintw(window4, 1, 2, "Ricerca: (Digita il processo o il PID da cercare, invio per terminare la ricerca)");
      }else{
        mvwprintw(window4, 1, 2, "Ordina per: %s %c", window_input, '\0');
      }

      wrefresh(window4);
      j--;
      continue;
    }
    
    if (!((get_input >= '0' && get_input <= '9') || ((get_input >= 'A' && get_input <= 'Z')) || (get_input >= 'a' && get_input <= 'z'))){
      continue; // non è alpha-numerico
    }
    
    if(j != 2){
      //stampo solo 2 su 3 caratteri, l'ultimo serve solo per il canc ecc
      window_input[j] = get_input;
      wclear(window4);
      box(window4, (int)'|', (int)'-');
      mvwprintw(window4, 1, 2, "Ordina per: %s", window_input);
      wrefresh(window4);
      j++;
    }
  }

  window_input[5] = 'X'; 
  //window_input viene inizializzato a fine esecuzione, e usato solo per le prime 3 celle, di cui le successive '\0' (fine stringa)
  //sfrutto questa cosa per inizializzare una cella superiore a '\0' con un valore a caso per farmi capire in che fase sono da altre funzioni
  //es. c1\t\0\0X
  //es. 01 2 3 45
  if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '0'){ //C0
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "PID CRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_PID_C);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '0'){//D0
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "PID DECRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_PID_D);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '1'){//C1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CMDLINE CRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_CMDLINE_C);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '1'){//D1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CMDLINE DECRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_CMDLINE_D);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '2'){//D1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "RES CRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_RES_C);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '2'){//D1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "RES DECRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_RES_D);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '3'){//D1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "VIRT CRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_VIRT_C);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '3'){//D1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "VIRT DECRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_VIRT_D);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '4'){//D1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CPU PERCENTAGE CRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_CPUP_C);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '4'){//D1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CPU PERCENTAGE DECRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_CPUP_D);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '5'){//D1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "MEM PERCENTAGE CRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_MEMP_C);
    pid_order_print(pid_order_v, window3, 0);
  }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '5'){//D1
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "MEM PERCENTAGE DECRESCENTE", '\0');
    wrefresh(window4);
    pid_order(pid_order_v, ORDERBY_MEMP_D);
    pid_order_print(pid_order_v, window3, 0);
  }else{
    wclear(window4);
    box(window4, (int)'|', (int)'-');
    mvwprintw(window4, 1, 2, "%s %c", "METODO DI ORDINAMENTO NON PRESENTE!" , '\0');
    wrefresh(window4);

    get_input = 'R'; //casuale

    //get_input = getch();
    while (!( get_input == '\n' || get_input == 'b' || get_input == 'B')) get_input = getch();

    memset(window_input, 0, WINDOW_INPUT_LENGHT); // leak dati?
    return;
  }

  get_input = getch();

  while (!( get_input == '\n' || get_input == 'b' || get_input == 'B')){
    if (get_input == (char)KEY_UP){


      if (orderby_starting_process > 0){
        orderby_starting_process--;
      }else{
        orderby_starting_process = pid_order_v->num_proc - 1;
      }

    }else if (get_input == (char)KEY_DOWN){

      orderby_starting_process = (orderby_starting_process + 1) % pid_order_v->num_proc;
      
    }

    //pid_order(pid_order_v, ORDERBY_CMDLINE_D);
    mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "???", '\0');
    pid_order_print(pid_order_v, window3, orderby_starting_process);

    get_input = getch();
  }

  memset(window_input, 0, WINDOW_INPUT_LENGHT); // leak dati?
  
  return;
}

void resize_term_custom(){ 
  //c'è resizeterm, ma viene consigliato in caso di layout complicati di ridimensionare e muovere manualemente, credits. https://invisible-island.net/ncurses/man/resizeterm.3x.html
  //non funziona perfettamente, ma è accettabile

  //getmaxyx(stdscr, max_y, max_x);
  //credits. https://stackoverflow.com/questions/1811955/ncurses-terminal-size
  //getmaxyx ottiene dei valori non aggiornati IRT durante il resize, invece ioctl riesce ad aggiornare immediatamente i valori
  //questo crea altri problemi che i resize non funzionano bene per dimensioni maggiorate (crash) da capire come fare...
  
  if (ioctl(0, TIOCGWINSZ, (char *)&size) < 0) printf("TIOCGWINSZ error");

  //wresize(stdscr, size.ws_row, size.ws_col); //non funziona bene
  // problemi ncurses e resize asyncroni 
  //credits.https://stackoverflow.com/questions/13707137/resizing-glitch-with-ncurses?rq=1
  // credits.https://linux.die.net/man/3/resize_term
  //leggendo dal man praticamente il getmaxyx ecc vengono applicati quando il getch ottiene KEY_RESIZE come valore, ma questo non può accadere nella nostra versione che funziona tramite interrupt (SIGWINCH) asincrono
  //quindi un metodo è ottenere i val da ioctl e usare le righe qua sotto
  resizeterm(size.ws_row, size.ws_col);
  getmaxyx(stdscr, max_y, max_x);
  refresh();//importante

  wclear(window1);
  wrefresh(window1);

  wclear(window3);
  wrefresh(window3);

  wclear(window4);
  wrefresh(window4);

  /*//DEBUG
  mvwprintw(window1, 1, 1, "LINES = %d, COLS = %d", max_y, max_x);
  wrefresh(window1);
  return;*/

  //resize crea crash se le dimensioni sono troppo piccole es 2, poiché win3 è y 2-3 = -1 e crasha il resize
  //https://linux.die.net/man/3/wresize (>= 0)
  //da sistemare, caso verametne degenere

  if(max_y <= 6 || max_x <= 0){ //per ora semplicemente chiudo il programma e dealloco le risorse deallocabili...
    keypad(stdscr, false);

    endwin(); // ncurses, dealloca le finestre
    clear();

    sem_destroy(&sem1);
    cpu_usage_free(cpu_usage_var);

    cpu_snapshot_free(cpu_snapshot_t0);
    cpu_snapshot_free(cpu_snapshot_t1);
    exit(EXIT_FAILURE);
  } 

  if(current_if == DEFAULT_IF){

    wresize(window1, WINDOW1_Y, max_x);
    box(window1, (int) '|', (int) '-');
    mvwprintw(window1, 1, 2, "%s %c", "(h)help, (q)quit, (k)kill, (z)sleep, (r)resume, (l)list, (f)find, (o)orderby, (s)stats", '\0');
    wrefresh(window1);

    wresize(window3, max_y-3, max_x);
    box(window3, (int) '|', (int) '-');
    wrefresh(window3);

    print_proc(window3, starting_process, starting_row);

    return;

  }else if(current_if == KILL_IF || current_if == SLEEP_IF || current_if == RESUME_IF ){
    
    wresize(window1, WINDOW1_Y, max_x);
    box(window1, (int) '|', (int) '-');
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    wrefresh(window1);

    wresize(window3, max_y-6, max_x);
    box(window3, (int) '|', (int) '-');
    wrefresh(window3);

    print_proc(window3, starting_process, starting_row);

    wresize(window4, WINDOW4_Y, max_x);
    box(window4, (int) '|', (int) '-');

    //RICONTROLLA!!
    if(window_input[0] == (char) KEY_RESIZE){

      if(current_if == KILL_IF){
        mvwprintw(window4, 1, 2, "PID: (Digita il PID da uccidere, invio per confermare)");
      }else if(current_if == SLEEP_IF){
        mvwprintw(window4, 1, 2, "PID: (Digita il PID da addormentare, invio per confermare)");
      }else{
        mvwprintw(window4, 1, 2, "PID: (Digita il PID da risvegliare, invio per confermare)");
      }

    }else{
      window_input[strlen(window_input) - 1] = '\0'; // evita caratteri sporchi, controlla bene 
      mvwprintw(window4, 1, 2, "PID: ");
      mvwprintw(window4, 1, 7, "%s", window_input);
    }

    wrefresh(window4);

    return;
    
  }else if(current_if == EASTEREGG_IF){

    wresize(window1, WINDOW1_Y, max_x);
    box(window1, (int)'|', (int)'-');
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    wrefresh(window1);

    wresize(window3, max_y - 3, max_x);
    box(window3, (int)'|', (int)'-');
    wrefresh(window3);
    
    return;

  }else if(current_if == LIST_IF){
    wresize(window1, WINDOW1_Y, max_x);
    box(window1, (int) '|', (int) '-');
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    wrefresh(window1);

    wresize(window3, max_y-3, max_x);
    box(window3, (int) '|', (int) '-');
    wrefresh(window3);

    print_proc_advanced(window3, starting_process, starting_row);

    return;

  }else if(current_if == STATS_IF){

    wresize(window1, WINDOW1_Y, max_x);
    box(window1, (int)'|', (int)'-');
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    wrefresh(window1);

    wresize(window3, max_y - 3, max_x);
    box(window3, (int)'|', (int)'-');
    wrefresh(window3);

    print_stats(window3, starting_process, starting_row);

    return;

  }else if(current_if == FIND_IF){

    //if(strlen(window_input) > 0) window_input[strlen(window_input) - 1] = '\0'; // rimuovo il KEY_RESIZE salvato in windows_input

    box(window1, (int)'|', (int)'-');
    mvwprintw(window1, 1, 2, "%s %c", "premi INVIO per terminare la ricerca...", '\0');
    wrefresh(window1);

    wresize(window3, max_y - 6, max_x);
    mvwin(window3, 6, 0);
    box(window3, (int)'|', (int)'-');
    find_process(window3, starting_process, window_input);
    //mvwprintw(window3, 1, 2, "%s %c", window_input, '\0');
    wrefresh(window3);

    wclear(window4);
    box(window4, (int)'|', (int)'-');
    if(window_input[0] != '\0'){
      mvwprintw(window4, 1, 2, "Ricerca:  %s %c", window_input, '\0');
    }else{
      mvwprintw(window4, 1, 2, "Ricerca: (Digita il processo o il PID da cercare, invio per terminare la ricerca)");
    }
    wrefresh(window4);

    return;

  }else if(current_if == ORDERBY_IF){
    

    wresize(window1, WINDOW1_Y, max_x);
    box(window1, (int)'|', (int)'-');
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    wrefresh(window1);

    wresize(window3, max_y - 6, max_x);
    box(window3, (int)'|', (int)'-');
    wrefresh(window3);
    
    wresize(window4, WINDOW4_Y, max_x);
    box(window4, (int)'|', (int)'-');
  
    if(window_input[0] == '\0'){
      mvwprintw(window4, 1, 2, "%s %c", "Ordina per: (Inserisci il tipo di ordinamento, invio per confermare)", '\0');
      mvwprintw(window3, 1, 2, "%s %c", "Attesa inserimento metodo di ordinamento... (invio per confermare)", '\0');
      mvwprintw(window3, 3, 2, "%s %c", "Sintassi: <ORDINE><CAMPO> ", '\0');
      mvwprintw(window3, 4, 2, "%s %c", "<ORDINE>: (c)crescente (d)decrescente", '\0');
      mvwprintw(window3, 5, 2, "%s %c", "<CAMPO>: (0)PID (1)cmdline (2)RES (3)VIRT (4)CPU% (5)MEM%", '\0');
      mvwprintw(window3, 6, 2, "%s %c", "Es: c1 <=> ordinamento <crescente> tramite il campo di <cmdline> ", '\0');
    }else{
      if (window_input[5] == 'X'){

        int cc1 = 1;

        if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '0'){ //C0
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "PID CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '0'){//D0
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "PID DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '1'){//C1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CMDLINE CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '1'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CMDLINE DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '2'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "RES CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '2'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "RES DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '3'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "VIRT CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '3'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "VIRT DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '4'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CPU PERCENTAGE CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '4'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CPU PERCENTAGE DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '5'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "MEM PERCENTAGE CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '5'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "MEM PERCENTAGE DECRESCENTE", '\0');
        }else{
          mvwprintw(window4, 1, 2, "%s %c", "METODO DI ORDINAMENTO NON PRESENTE!" , '\0');
          mvwprintw(window3, 1, 2, "%s %c", "Attesa inserimento metodo di ordinamento... (invio per confermare)", '\0');
          mvwprintw(window3, 3, 2, "%s %c", "Sintassi: <ORDINE><CAMPO> ", '\0');
          mvwprintw(window3, 4, 2, "%s %c", "<ORDINE>: (c)crescente (d)decrescente", '\0');
          mvwprintw(window3, 5, 2, "%s %c", "<CAMPO>: (0)PID (1)cmdline (2)RES (3)VIRT (4)CPU% (5)MEM%", '\0');
          mvwprintw(window3, 6, 2, "%s %c", "Es: c1 <=> ordinamento <crescente> tramite il campo di <cmdline> ", '\0');
          cc1 = 0;
        }

        if(cc1){
          pid_order(pid_order_v, pid_order_v->ordering_method);
          pid_order_print(pid_order_v, window3, 0);
        }

      }else{
        mvwprintw(window4, 1, 2, "Ordina per: %s %c", window_input, '\0');
        mvwprintw(window3, 1, 2, "%s %c", "Attesa inserimento metodo di ordinamento... (invio per confermare)", '\0');
        mvwprintw(window3, 3, 2, "%s %c", "Sintassi: <ORDINE><CAMPO> ", '\0');
        mvwprintw(window3, 4, 2, "%s %c", "<ORDINE>: (c)crescente (d)decrescente", '\0');
        mvwprintw(window3, 5, 2, "%s %c", "<CAMPO>: (0)PID (1)cmdline (2)RES (3)VIRT (4)CPU% (5)MEM%", '\0');
        mvwprintw(window3, 6, 2, "%s %c", "Es: c1 <=> ordinamento <crescente> tramite il campo di <cmdline> ", '\0');
      }
    }

    wrefresh(window3);
    wrefresh(window4);

    return;

  }else if(current_if == HELP_IF){

    wresize(window1, WINDOW1_Y, max_x);
    box(window1, (int)'|', (int)'-');
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    wrefresh(window1);

    wresize(window3, max_y - 3, max_x);
    mvwprintw(window3, 1, 2, HELP_PRINT);
    box(window3, (int)'|', (int)'-');
    wrefresh(window3);
    
    return;

  }
  return;
}

void refresh_UI(){

  if(current_if == HELP_IF || current_if == EASTEREGG_IF ) return; //schermate statiche
  
  wclear(window3);
  box(window3, (int) '|', (int) '-');
  wrefresh(window3);

  if(current_if == STATS_IF){
    print_stats(window3, starting_process, starting_row);
    return;
  }else if(current_if == LIST_IF){
    print_proc_advanced(window3, starting_process, starting_row);
    return;
  }else if(current_if == DEFAULT_IF || current_if == KILL_IF || current_if == SLEEP_IF || current_if == RESUME_IF){
    print_proc(window3, starting_process, starting_row);
    return;
  }else if(current_if == FIND_IF){
    //if(strlen(window_input) > 0) window_input[strlen(window_input)] = '\0'; // rimuovo il KEY_RESIZE salvato in windows_input
    box(window1, (int)'|', (int)'-');
    mvwprintw(window1, 1, 2, "%s %c", "premi INVIO per terminare la ricerca...", '\0');
    wrefresh(window1);

    wresize(window3, max_y - 6, max_x);
    mvwin(window3, 6, 0);
    box(window3, (int)'|', (int)'-');
    find_process(window3, find_starting_process, window_input);
    //mvwprintw(window3, 1, 2, "%s %c", window_input, '\0');
    wrefresh(window3);

    wclear(window4);
    box(window4, (int)'|', (int)'-');
    if(window_input[0] != '\0'){
      mvwprintw(window4, 1, 2, "Ricerca:  %s %c", window_input, '\0');
    }else{
      mvwprintw(window4, 1, 2, "%s %c", "Ricerca: (Digita il processo o il PID da cercare, invio per terminare la ricerca)", '\0');
    }
    wrefresh(window4);
    return;
  }else if(current_if == ORDERBY_IF){

    box(window1, (int)'|', (int)'-');
    mvwprintw(window1, 1, 2, "%s %c", "(b)back", '\0');
    wrefresh(window1);

    wresize(window3, max_y - 6, max_x);
    mvwin(window3, 6, 0);
    box(window3, (int)'|', (int)'-');

    wclear(window4);
    box(window4, (int)'|', (int)'-');

    if(window_input[0] == '\0'){
      mvwprintw(window4, 1, 2, "%s %c", "Ordina per: (Inserisci il tipo di ordinamento, invio per confermare)", '\0');
      mvwprintw(window3, 1, 2, "%s %c", "Attesa inserimento metodo di ordinamento... (invio per confermare)", '\0');
      mvwprintw(window3, 3, 2, "%s %c", "Sintassi: <ORDINE><CAMPO> ", '\0');
      mvwprintw(window3, 4, 2, "%s %c", "<ORDINE>: (c)crescente (d)decrescente", '\0');
      mvwprintw(window3, 5, 2, "%s %c", "<CAMPO>: (0)PID (1)cmdline (2)RES (3)VIRT (4)CPU% (5)MEM%", '\0');
      mvwprintw(window3, 6, 2, "%s %c", "Es: c1 <=> ordinamento <crescente> tramite il campo di <cmdline> ", '\0');
    }else{
      if (window_input[5] == 'X'){

        int cc1 = 1;

        if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '0'){ //C0
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "PID CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '0'){//D0
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "PID DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '1'){//C1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CMDLINE CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '1'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CMDLINE DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '2'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "RES CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '2'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "RES DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '3'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "VIRT CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '3'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "VIRT DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '4'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CPU PERCENTAGE CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '4'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "CPU PERCENTAGE DECRESCENTE", '\0');
        }else if( (window_input[0] == 'c' || window_input[0] == 'C') && window_input[1] == '5'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "MEM PERCENTAGE CRESCENTE", '\0');
        }else if( (window_input[0] == 'd' || window_input[0] == 'D') && window_input[1] == '5'){//D1
          mvwprintw(window4, 1, 2, "Ordina per: %s (%s) %c", window_input, "MEM PERCENTAGE DECRESCENTE", '\0');
        }else{
          mvwprintw(window4, 1, 2, "%s %c", "METODO DI ORDINAMENTO NON PRESENTE!" , '\0');
          mvwprintw(window3, 1, 2, "%s %c", "Attesa inserimento metodo di ordinamento... (invio per confermare)", '\0');
          mvwprintw(window3, 3, 2, "%s %c", "Sintassi: <ORDINE><CAMPO> ", '\0');
          mvwprintw(window3, 4, 2, "%s %c", "<ORDINE>: (c)crescente (d)decrescente", '\0');
          mvwprintw(window3, 5, 2, "%s %c", "<CAMPO>: (0)PID (1)cmdline (2)RES (3)VIRT (4)CPU% (5)MEM%", '\0');
          mvwprintw(window3, 6, 2, "%s %c", "Es: c1 <=> ordinamento <crescente> tramite il campo di <cmdline> ", '\0');
          cc1 = 0;
        }

        if(cc1){
          pid_order(pid_order_v, pid_order_v->ordering_method);
          pid_order_print(pid_order_v, window3, 0);
        }

      }else{
        mvwprintw(window4, 1, 2, "Ordina per: %s %c", window_input, '\0');
        mvwprintw(window3, 1, 2, "%s %c", "Attesa inserimento metodo di ordinamento... (invio per confermare)", '\0');
        mvwprintw(window3, 3, 2, "%s %c", "Sintassi: <ORDINE><CAMPO> ", '\0');
        mvwprintw(window3, 4, 2, "%s %c", "<ORDINE>: (c)crescente (d)decrescente", '\0');
        mvwprintw(window3, 5, 2, "%s %c", "<CAMPO>: (0)PID (1)cmdline (2)RES (3)VIRT (4)CPU% (5)MEM%", '\0');
        mvwprintw(window3, 6, 2, "%s %c", "Es: c1 <=> ordinamento <crescente> tramite il campo di <cmdline> ", '\0');
      }
    }
    wrefresh(window3);
    wrefresh(window4);
    return;
  }
  return;
}

void signal_handler(int sig){
  //TBD
  //stavo leggendo che non è una buona pratica installare un allarme così (a causa del context switch), informati... per ora lo implemento così per vedere se funziona
  
  if(sig == SIGALRM){
    refresh_UI();
    alarm(REFRESH_RATE);
  }else if(sig == SIGWINCH){
    resize_term_custom();
  }
  
  //tecnicamente dovrei rilanciarlo solo ogni SIGALRM ma ho paura che i segnali essendo asincroni possano accavallarsi e non venire richiamato
  //es.mi arrivano istantaneamente SIGWINCH e SIGALARM e SIGALARM venisse scartato, non verrebbe richiamato il refresh
  return;
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

  file = fopen(EE_BOT_full_path, "r");
  if(!file) return;
  size_t nread = 0;

  //grandezze finestra w3
  /*int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);
  max_y -= 3;

  #define COORD_Y max_y/2
  #define COORD_X max_x/2*/

  if((nread = fread(buf, 1, sizeof(buf), file)) > 0){
    //mvwprintw(window3, COORD_Y, COORD_X, "%ld %c", nread, '\0');
    //mvwprintw(window3, COORD_Y, COORD_X, "bot. %c", '\0');
    mvwprintw(window3, 1, 1, "%s%c", buf, '\0');
  }

  fclose(file);
  return;
}

