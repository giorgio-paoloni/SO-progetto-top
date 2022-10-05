#include "proc.h"
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
void TUI_sleep_interface(WINDOW* window1,WINDOW* window2,WINDOW* window3,WINDOW* window4,int max_y, int max_x);
void TUI_resume_interface(WINDOW* window1,WINDOW* window2,WINDOW* window3,WINDOW* window4,int max_y, int max_x);
void TUI_find_interface(WINDOW* window1,WINDOW* window2,WINDOW* window3,WINDOW* window4,int max_y, int max_x);

//utili
void reset_to_default_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x);
void resize_term_custom(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int old_max_y, int old_max_x, int calling_interface);

//INTERFACE (IF)
#define DEFAULT_IF 0
#define KILL_IF 1

#define WINDOW_INPUT_LENGHT 32
#define HELP_PRINT "Questo programma e' stato scritto da Giorgio Paoloni 1883570, replica di Top, non a caso si chiama Bot. \nOgni schermata di puo' scorrere con le frecce su e giu' I comandi implementati sono: \n 1)kill per uccidere un processo tramite PID\n2)sleep per addormentare un processo tramite PID \n3)resume per risvegliare un processo tramite PID \n2)list per ottenere una list completa e avanzata di tutti i processi \n3)stats info cumulative dei PID\n "
