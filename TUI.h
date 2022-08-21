#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h> //guida seguita https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/, curses o ncurses lib
//TUI = text-based user interface, sfrutto le potenzialita' di ncurses

//base
void TUI_default_interface();
void TUI_kill_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x);
void TUI_help_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x);
void TUI_list_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x);
void TUI_stats_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x);

//utili
void reset_to_default_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x);


#define HELP_PRINT "Questo programma e' stato scritto da Giorgio Paoloni 1883570, replica di Top, non a caso si chiama Bot. \n I comandi implementati sono: \n 1)kill per ... \n 2)list per ottenere una list completa di tutti i processi con info singole scrollable con le freccia su e giu' \n 3)stats info cumulative dei PID\n "
