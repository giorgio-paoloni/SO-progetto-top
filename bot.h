//#include "proc.h"
#include "TUI.h"
#include "process_monitor.h"
#include <ncurses.h> //libreria che e' molto utile per input asincroni su STDIN, per avere un STDIN senza essere bufferizzato, fornire una TUI (text-based user interface),...
//STDSCR, in ncurses

//L'obiettivo e' fornire una TUI MINIMA, non ho intenzione di curare particolarmente la grafica, ma comunque fornire un modo abbastanza famigliare e reattivo per gestire questo programma Top-like.
