#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h> //guida seguita https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/, curses o ncurses lib
//TUI = text-based user interface, sfrutto le potenzialita' di ncurses

#define HEGHT1 50
#define WIDTH1 40
#define STARTY1 0
#define STARTX1 0

void TUI_default_interface();
void TUI_help_interface();
void TUI_kill_interface();
