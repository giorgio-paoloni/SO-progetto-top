#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h> //guida seguita https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/, curses o ncurses lib
//TUI = text-based user interface, sfrutto le potenzialita' di ncurses

void TUI_default_interface();
void TUI_kill_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x);
void TUI_help_interface(WINDOW* window1, WINDOW* window2, WINDOW* window3, WINDOW* window4, int max_y, int max_x);
