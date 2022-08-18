#include "TUI.h"

void TUI_default_interface(){

  //clear();
  int char_input;

  initscr(); //ncurses, inizializza cose...
  raw(); //ncurses, disabilita il line buffering dello stdin e segnali come CTRL-C
  noecho();//ncurses, disabilita il print su STDOUT di getch
  curs_set(0); //https://stackoverflow.com/questions/19614156/c-curses-remove-blinking-cursor-from-game, sempre funzione di curses / ncurses

  //int y, x;
  int max_y, max_x;

  //getyx(stdscr, y, x);//stdscr e' di ncurses, la funzione ottiene la posizione del cursore nella finestra (0,0)
  getmaxyx(stdscr, max_y, max_x);//ottiene la dimensione massima attuale della finestra di terminale

  //questi 4 valori possono essere utili per avere delle posizioni e dimensioni relative alla finestra di terminale, "responsive design",...


  //WINDOW* window1 = newwin(HEGHT1, WIDTH1, STARTY1, STARTX1);

  refresh();

  WINDOW* window1 = newwin(3, max_x, 0, 0); //info & commands window
  WINDOW* window2 = newwin(8, max_x, 3, 0); //stats window
  WINDOW* window3 = newwin(max_y-11, max_x, 11, 0);//process list window
  WINDOW* window4 = NULL; //input win, es kill PID

  nodelay(window1, TRUE);//per non blocking getch, credits. https://gist.github.com/mfcworks/3a32513f26bdc58fd3bd
  nodelay(window2, TRUE);//devo rileggermi bene il man
  nodelay(window3, TRUE);
  //nodelay(window4, TRUE);

  //keypad(window3, TRUE);
  //scrollok(window3, TRUE);

  //le lascio per controllare se sforo nel terminale, non so se le terro'...
  box(window1, (int) '|', (int) '-');
  box(window2, (int) '|', (int) '-');
  box(window3, (int) '|', (int) '-');

  mvwprintw(window1, 1, 2, "(h)help, (q)quit, (k)kill, (l)list,");
  //mvwprintw(window1, 2, 2, "R = %d C = %d", max_y, max_x);


  wrefresh(window1);
  wrefresh(window2);
  wrefresh(window3);


  while(1){

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

    print_proc2(window3);

    //avrei potuto usare switch-case, ma non mi piace
    char_input = getch();
    if(char_input == (int) 'q' || char_input == (int) 'Q') break; //l'utente ha inserito q, cioe' QUIT

    if(char_input == (int) 'h' || char_input == (int) 'H'){//l'utente ha inserito h, cioe' HELP
      TUI_help_interface();
    }else if(char_input == (int) 'k' || char_input == (int) 'K'){//l'utente ha inserito k, cioe' kill
    TUI_kill_interface(window1, window2, window3, window4, max_y, max_x);

    }
  }

  endwin();//ncurses, dealloca le finestre
  return;
}

void TUI_kill_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x){

  window4 = newwin(3, max_x, 3, 0);
  box(window4, (int) '|', (int) '-');

  wclear(window1);
  box(window1, (int) '|', (int) '-');
  wclear(window2);
  mvwprintw(window1, 1, 2, "(h)help, (b)back");

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

  while(getch() != 'b'){

  }

  return;
}

void TUI_help_interface(){
  refresh();
  return;
}
