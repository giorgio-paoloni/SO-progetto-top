OBJS	= bot.o proc.o
SOURCE	= bot.c proc.c
HEADER	= bot.h proc.h
OUT	= bot
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

bot.o: bot.c
	$(CC) $(FLAGS) bot.c -std=c99

proc.o: proc.c
	$(CC) $(FLAGS) proc.c -std=c99


clean:
	rm -f $(OBJS) $(OUT)
