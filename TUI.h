#include "proc.h"
#include "process_monitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> //sigaction..
#include <ncurses.h> //guida seguita https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/, curses o ncurses lib
//TUI = text-based user interface, sfrutto le potenzialita' di ncurses

//base
void TUI_default_interface();
void TUI_kill_interface();
void TUI_help_interface();
void TUI_list_interface();
void TUI_stats_interface();
void TUI_sleep_interface();
void TUI_resume_interface();
void TUI_easteregg_inferface();
void TUI_find_interface();
void TUI_kill_sleep_resume_interface();
void TUI_orderby_interface();

//utili
void reset_to_default_interface();
void resize_term_custom();
void refresh_UI();
void print_easteregg(int i);

//signal
void signal_handler(int sig);

//INTERFACE (IF)
#define DEFAULT_IF 0
#define KILL_IF 1
#define SLEEP_IF 2
#define RESUME_IF 3
#define LIST_IF 4
#define FIND_IF 5
#define STATS_IF 6
#define HELP_IF 7
#define EASTEREGG_IF 8
#define ORDERBY_IF 9

//altro 
#define WINDOW_INPUT_LENGHT 32
#define WINDOW_INPUT_LENGHT2 3
#define REFRESH_RATE 1
#define BUF_LENGHT1 2048
//altro pt2
#define EE_BOT_PATH "./ee_bot"
#define CUSTOM_LENGHT1 32
#define VAL_LENGHT 4
#define MAX_TXT 13
#define MAX_COLORS 256

#define HELP_PRINT "Questo programma e' stato scritto da Giorgio Paoloni 1883570, replica di Top, non a caso si chiama Bot. \nOgni schermata di puo' scorrere con le frecce su e giu' I comandi implementati sono: \n 1)kill per uccidere un processo tramite PID\n2)sleep per addormentare un processo tramite PID \n3)resume per risvegliare un processo tramite PID \n2)list per ottenere una list completa e avanzata di tutti i processi \n3)stats info cumulative dei PID\n "

//DIM WINDOW, non utilizzate particolarmente...
#define WINDOW1_Y 3
#define WINDOW1_X 3

#define WINDOW3_Y 3
#define WINDOW3_X 3

#define WINDOW4_Y 3
#define WINDOW4_X 3
