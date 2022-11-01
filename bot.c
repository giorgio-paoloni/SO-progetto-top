#include "bot.h"

int main(){
  TUI_default_interface();
  //exit_curses(0);
  return 0;
}

//si consiglia di tenere il terminale in 16:9 o 4:3 con una grandezza approssimativa di 100x30px o maggiore, per poter leggere bene tutto e non avere glitch grafici

// https://invisible-island.net/ncurses/ncurses.faq.html#config_leaks
//(possibili) memleak di ncurses sono spiegati qui, Testing for Memory Leaks