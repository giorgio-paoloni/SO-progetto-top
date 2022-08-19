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

  //questi 4 valori possono essere utili per avere delle posizioni e dimensioni relative alla finestra di terminale, "responsive design",...

  refresh();

  WINDOW* window1 = newwin(3, max_x, 0, 0); //info & commands window
  WINDOW* window2 = newwin(8, max_x, 3, 0); //stats window
  WINDOW* window3 = newwin(max_y-11, max_x, 11, 0);//process list window
  WINDOW* window4 = NULL; //input win, es kill PID

  //nodelay(window1, TRUE);
  //nodelay(window2, TRUE);
  //nodelay(window3, TRUE);
  //nodelay(window4, TRUE);

  nodelay(stdscr, true);//per non blocking getch, credits. https://gist.github.com/mfcworks/3a32513f26bdc58fd3bd, devo rileggermi bene il man

  //keypad(window3, TRUE);
  //scrollok(window3, TRUE);

  //le lascio per controllare se sforo nel terminale, non so se le terro'...
  box(window1, (int) '|', (int) '-');
  box(window2, (int) '|', (int) '-');
  box(window3, (int) '|', (int) '-');

  mvwprintw(window1, 1, 2, "(h)help, (q)quit, (k)kill, (l)list");
  //mvwprintw(window1, 2, 2, "R = %d C = %d", max_y, max_x);


  wrefresh(window1);
  wrefresh(window2);
  wrefresh(window3);


  while(1){

    print_proc2(window3);

    //avrei potuto usare switch-case, ma non mi piace
    char_input = getch();
    if(char_input == (int) 'q' || char_input == (int) 'Q') break; //l'utente ha inserito q, cioe' QUIT

    if(char_input == (int) 'h' || char_input == (int) 'H'){//l'utente ha inserito h, cioe' HELP
      TUI_help_interface(window1, window2, window3, window4, max_y, max_x);
    }else if(char_input == (int) 'k' || char_input == (int) 'K'){//l'utente ha inserito k, cioe' kill
    TUI_kill_interface(window1, window2, window3, window4, max_y, max_x);

    mvwprintw(window1, 1, 2, "(h)help, (q)quit, (k)kill, (l)list");
    wrefresh(window1);

    mvwin(window3, 11, 0);
    wclear(window3);
    wrefresh(window3);

    wresize(window3, max_y-11, max_x);
    wrefresh(window3);

    box(window2, (int) '|', (int) '-');
    wrefresh(window2);

    box(window3, (int) '|', (int) '-');

    }
  }

  endwin();//ncurses, dealloca le finestre
  clear();
  return;
}

void TUI_kill_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x){

  window4 = newwin(3, max_x, 3, 0);
  box(window4, (int) '|', (int) '-');

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "(h)help, (b)back");

  wclear(window2);
  wrefresh(window2);

  wclear(window3);
  wrefresh(window3);//applicare il clear prima di spostarla, altrimenti rimangono dei caratteri sotto

  mvwin(window3, 6, 0);
  wresize(window3, max_y-6, max_x);
  box(window3, (int) '|', (int) '-');

  //print_proc2(window3);

  wrefresh(window1);
  wrefresh(window2);
  wrefresh(window3);
  wrefresh(window4);

  refresh();

  char window_input[32];

  for(int j = 0;  j < 32; j++){
    window_input[j] = '\0';
  }

  int  i = 0;

  mvwprintw(window4, 1, 2, "PID: (Digita il PID da uccidere, invio per confermare)");
  wrefresh(window4);

  //questo meccanismo mi permette di stampare IRT nella window4 i caratteri digitati, nodelay(.., false) mi rende la getch() bloccante

  nodelay(stdscr, false);

  while((window_input[i] = (char) getch()) != '\n' && i < 32){

    if(window_input[i] == 'b' || window_input[i] == 'B' || window_input[i] == 'q' || window_input[i] == 'Q'){//l'utente puo' premere b o q in ogni momento e annulla l'inserimento del pid
      window_input[0] = 'b';
      break;
    }

    if(i>0 && (window_input[i] == KEY_BACKSPACE || window_input[i] == 127 || window_input[i] == 8 || window_input[i] == '\b')){//l'utente puo' cancellare il testo credits https://stackoverflow.com/questions/44943249/detecting-key-backspace-in-ncurses, per ora non mi rileva il cancellare...

      window_input[i-1] = '\0';
      wclear(window4);
      box(window4, (int) '|', (int) '-');
      wrefresh(window4);
      mvwprintw(window4, 1, 2, "PID: ");
      mvwprintw(window4, 1, 7, window_input);
      wrefresh(window4);
      i--;
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

  if(!(window_input[0] == 'b' || window_input[0] == 'B' || window_input[0] == 'q' || window_input[0] == 'Q')){
    if(kill_PID(atoi(window_input)) == -1){//err
      mvwprintw(window4, 1, i+8, "non ucciso");
    }else{
      mvwprintw(window4, 1, i+8, "ucciso");
    }
    wrefresh(window4);

    window_input[0] = getch();
  }

  while(!(window_input[0] == 'b' || window_input[0] == 'B' || window_input[0] == 'q' || window_input[0] == 'Q')){
    window_input[0] = getch();
  }

  return;
}

void TUI_help_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x){

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  mvwprintw(window1, 1, 2, "(h)help, (b)back");

  wclear(window2);
  wrefresh(window2);
  
  wclear(window3);
  wrefresh(window3);//applicare il clear prima di spostarla, altrimenti rimangono dei caratteri sotto

  mvwin(window3, 6, 0);
  wresize(window3, max_y-6, max_x);
  box(window3, (int) '|', (int) '-');

  while(getch() != 'q');//busywait temporaneo per vedere se funziona

  refresh();
  return;
}
