#include "bot.h"

int main(){
  TUI_default_interface();
  return 0;
}

//si consiglia di tenere il terminale in 16:9 o 4:3 con una grandezza approssimativa di 100x30px o maggiore, per poter leggere bene tutto e non avere glitch grafici

// https://invisible-island.net/ncurses/ncurses.faq.html#config_leaks
//(possibili) memleak di ncurses sono spiegati qui, Testing for Memory Leaks
//https://stackoverflow.com/questions/3840582/still-reachable-leak-detected-by-valgrind (global var)
//https: // stackoverflow.com/questions/2144531/valgrind-claims-there-is-unfreed-memory-is-this-bad