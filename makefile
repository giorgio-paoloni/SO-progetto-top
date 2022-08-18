OBJS	= bot.o proc.o TUI.o
SOURCE	= bot.c proc.c TUI.c
HEADER	= bot.h proc.h TUI.h
OUT	= bot
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS = -lncurses #https://stackoverflow.com/questions/16192087/undefined-reference-to-initscr-ncurses

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

bot.o: bot.c
	$(CC) $(FLAGS) bot.c -std=c99

proc.o: proc.c
	$(CC) $(FLAGS) proc.c -std=c99

TUI.o: TUI.c
	$(CC) $(FLAGS) TUI.c -std=c99

clean:
	rm -f $(OBJS) $(OUT)
