#include "TUI.h"

void TUI_default_interface(){

  //clear();
  int char_input;

  initscr(); //ncurses, inizializza cose...
  raw(); //ncurses, disabilita il line buffering dello stdin e segnali come CTRL-C
  noecho();//ncurses, disabilita il print su STDOUT di getch
  curs_set(0); //https://stackoverflow.com/questions/19614156/c-curses-remove-blinking-cursor-from-game, sempre funzione di curses / ncurses
  keypad(stdscr, true); //abilita tasti particolari come FNn o backspace ecc
  //credits. https://stackoverflow.com/questions/27200597/c-ncurses-key-backspace-not-working

  //int y, x;
  int max_y, max_x;

  //getyx(stdscr, y, x);//stdscr e' di ncurses, la funzione ottiene la posizione del cursore nella finestra (0,0)
  getmaxyx(stdscr, max_y, max_x);//ottiene la dimensione massima attuale della finestra di terminale

  //questi 2 valori possono essere utili per avere delle posizioni e dimensioni relative alla finestra di terminale, "responsive design",...

  refresh();

  WINDOW* window1 = newwin(3, max_x, 0, 0); //info & commands window
  WINDOW* window2 = NULL;// newwin(8, max_x, 3, 0); //stats window, alla fine ho deciso di implementarla su un'altra schermata, rimane perche' altrimenti dovrei shiftare le finestre di -1 e non voglio creare errori.
  WINDOW* window3 = newwin(max_y-3, max_x, 3, 0);//process list window
  WINDOW* window4 = NULL;//finestra input testuale visibile, non sempre utilizzata

  nodelay(stdscr, true);//per non blocking getch, credits. https://gist.github.com/mfcworks/3a32513f26bdc58fd3bd, devo rileggermi bene il man

  //keypad(window3, TRUE);
  //scrollok(window3, TRUE);

  //le lascio per controllare se sforo nel terminale, non so se le terro'...
  box(window1, (int) '|', (int) '-');
  //box(window2, (int) '|', (int) '-');
  box(window3, (int) '|', (int) '-');

  mvwprintw(window1, 1, 2, "(h)help, (q)quit, (k)kill, (l)list, (s)stats");
  //mvwprintw(window1, 2, 2, "R = %d C = %d", max_y, max_x);

  //scrollok(window3, true);

  wrefresh(window1);
  //wrefresh(window2);
  wrefresh(window3);

  nodelay(stdscr, false);
  keypad(stdscr, true);

  int i = 2;

  print_proc2(window3);

  while(1){

    //avrei potuto usare switch-case, ma non mi piace
    char_input = getch();
    if(char_input == (int) 'q' || char_input == (int) 'Q') break; //l'utente ha inserito q, cioe' QUIT



    if(char_input == (int) 'h' || char_input == (int) 'H'){//l'utente ha inserito h, cioe' HELP
      TUI_help_interface(window1, window2, window3, window4, max_y, max_x);
      reset_to_default_interface(window1, window2, window3, window4, max_y, max_x);

    }else if(char_input == (int) 'k' || char_input == (int) 'K'){//l'utente ha inserito k, cioe' kill

      TUI_kill_interface(window1, window2, window3, window4, max_y, max_x);
      reset_to_default_interface(window1, window2, window3, window4, max_y, max_x);

    }else if(char_input == (int) 'l' || char_input == (int) 'L'){

      TUI_list_interface(window1, window2, window3, window4, max_y, max_x);
      reset_to_default_interface(window1, window2, window3, window4, max_y, max_x);

    }else if(char_input == (int) 's' || char_input == (int) 's'){
      TUI_stats_interface(window1, window2, window3, window4, max_y, max_x);
      reset_to_default_interface(window1, window2, window3, window4, max_y, max_x);

    }else if (char_input == KEY_UP){
      if(i > 0) i = (i-1)%max_y;
      if(i <= 0) i = max_y;

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');

      print_proc3(window3, i);

    }else if(char_input == KEY_DOWN){
      i = (i+1)%max_y;
      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');

      print_proc3(window3, i);
    }
  }

  nodelay(stdscr, true);
  keypad(stdscr, false);

  endwin();//ncurses, dealloca le finestre
  clear();
  return;
}

void TUI_kill_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x){

  int q = 2;

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "(b)back");

  //wclear(window2);
  //wrefresh(window2);

  //printw("test1\n");

  wclear(window3);
  wrefresh(window3);//applicare il clear prima di spostarla, altrimenti rimangono dei caratteri sotto

  wresize(window3, max_y-6, max_x);//IMPORTANTE: ho perso 1 ora a capire il problema, dal man se il mvwin sfora le dimensioni di stdscr (es scorri in basso come questo caso) NON viene applicato, quindi se scorri in basso PRIMA devi ridimensionare delle dimensioni che scorri la finestra!
  //credits. mvwin Calling mvwin moves the window so that the upper left-hand corner is at position (x, y).  If the move would cause the window to be off the screen, it is an error and the window is not moved.  Moving subwindows is allowed, but should be avoided.

  mvwin(window3, 6, 0);
  wrefresh(window3);

  box(window3, (int) '|', (int) '-');
  wrefresh(window3);

  mvwprintw(window3, 1, 2, "test2 in\n");

  window4 = newwin(3, max_x, 3, 0);
  box(window4, (int) '|', (int) '-');

  print_proc2(window3);

  wrefresh(window1);
  //wrefresh(window2);
  wrefresh(window3);
  wrefresh(window4);

  refresh();

  char window_input[32]; //PID lungo  massimo 32 caratteri

  for(int j = 0;  j < 32; j++){
    window_input[j] = '\0';
  }

  int  i = 0;

  mvwprintw(window4, 1, 2, "PID: (Digita il PID da uccidere, invio per confermare)");
  wrefresh(window4);

  //questo meccanismo mi permette di stampare IRT nella window4 i caratteri digitati, nodelay(.., false) mi rende la getch() bloccante

  nodelay(stdscr, false);

  while((window_input[i] = (char) getch()) != '\n' && i < 32){

    if(window_input[i] == 'b' || window_input[i] == 'B'){//l'utente puo' premere b in ogni momento e annulla l'inserimento del pid
      window_input[0] = 'b';
      break;
    }

    if(i>0 && (window_input[i] == (char) KEY_BACKSPACE || window_input[i] == (char) 127 || window_input[i] == (char) 8 || window_input[i] == (char) '\b')){//l'utente puo' cancellare il testo credits https://stackoverflow.com/questions/44943249/detecting-key-backspace-in-ncurses, per ora non mi rileva il carattere per cancellare... con altri caratteri sembra funzionare o comunque funziona non perfettamente

      window_input[i] = '\0'; //evita caratteri sporchi
      window_input[i-1] = '\0';//cancella il carattere precedente

      wclear(window4);
      box(window4, (int) '|', (int) '-');
      wrefresh(window4);
      mvwprintw(window4, 1, 2, "PID: ");
      mvwprintw(window4, 1, 7, window_input);
      wrefresh(window4);
      i--;
      continue;
    }

    if (window_input[i] == (char) KEY_UP){ //cast a char importante
      if(q > 0) q = (q-1)%max_y;
      if(q <= 0) q = max_y;

      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');

      print_proc3(window3, q);
      continue;

    }else if(window_input[i] == (char) KEY_DOWN){ //cast a char importante
      q = (q+1)%max_y;
      wclear(window3);
      wrefresh(window3);
      box(window3, (int) '|', (int) '-');

      print_proc3(window3, q);
      continue;
    }

    if(window_input[i] < '0' || window_input[i] > '9') continue; //controllo PID, e' SOLO numerico

    wclear(window4);
    box(window4, (int) '|', (int) '-');
    wrefresh(window4);
    mvwprintw(window4, 1, 2, "PID: ");
    mvwprintw(window4, 1, 7, window_input);
    wrefresh(window4);
    i++;
  }

  nodelay(stdscr, true);

  if(!(window_input[0] == '\n' || window_input[0] == 'b' || window_input[0] == 'B')){
    if(kill_PID(atoi(window_input)) == -1){//err
      mvwprintw(window4, 1, i+8, "non ucciso");
    }else{
      mvwprintw(window4, 1, i+8, "ucciso");
    }
    wrefresh(window4);

    window_input[0] = getch();
  }

  while(!(window_input[0] == '\n' || window_input[0] == 'b'|| window_input[0] == 'B')){
    window_input[0] = getch();
  }

  return;
}

void TUI_help_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x){

  wclear(window1);
  wrefresh(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "(b)back");
  wrefresh(window1);

  //wclear(window2);
  //wrefresh(window2);

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

void TUI_list_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x){

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "(b)back");
  wrefresh(window1);

  wclear(window2);
  wrefresh(window2);

  mvwin(window3, 3, 0);
  wresize(window3, max_y-3, max_x);
  wclear(window3);
  wrefresh(window3);
  box(window3, (int) '|', (int) '-');
  print_proc3(window3, 0);
  wrefresh(window3);

  nodelay(stdscr, false);
  //keypad(stdscr, true);

  int char_input = getch();


  int i = 0;

  while(!(char_input == (int) 'b' || char_input == (int) 'B')){

    //ciclica
    if (char_input == KEY_UP){
      if(i > 0) i = (i-1)%max_y;
      if(i <= 0) i = max_y;

    }else if(char_input == KEY_DOWN){
      i = (i+1)%max_y;
    }

    wclear(window3);
    wrefresh(window3);
    box(window3, (int) '|', (int) '-');

    print_proc3(window3, i);

    char_input = getch();
  }

  nodelay(stdscr, true);
  //keypad(stdscr, false);

  return;
}

void TUI_stats_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x){

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "(b)back");

  //wclear(window2);
  //wrefresh(window2);

  wclear(window3);
  wrefresh(window3);

  mvwin(window3, 6, 0);
  wresize(window3, max_y-6, max_x);
  box(window3, (int) '|', (int) '-');

  wrefresh(window1);
  //wrefresh(window2);
  wrefresh(window3);
  wrefresh(window4);

  refresh();

  int char_input = getch();

  while(!(char_input == (int) 'b' || char_input == (int) 'B') ){
    char_input = getch();
  }

  return;
}


void reset_to_default_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x){
  mvwprintw(window1, 1, 2, "(h)help, (q)quit, (k)kill, (l)list, (s)stats");
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

  print_proc2(window3);
  wrefresh(window3);

}
