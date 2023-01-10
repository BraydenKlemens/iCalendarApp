CC = gcc
CFLAGS = -g -Wall -std=c11

SRC = src/

all: library

LinkedListAPI.o: $(SRC)LinkedListAPI.c
    $(CC) $(CFLAGS) -fpic -Iinclude -c $(SRC)LinkedListAPI.c -o bin/LinkedListAPI.o

CalendarParser.o: $(SRC)CalendarParser.c
    $(CC) $(CFLAGS) -fpic -Iinclude -c $(SRC)CalendarParser.c -o bin/CalendarParser.o

helper.o: $(SRC)helper.c
    $(CC) $(CFLAGS) -fpic -Iinclude -c $(SRC)helper.c -o bin/helper.o

library: LinkedListAPI.o CalendarParser.o helper.o
    gcc -shared -o ../sharedLib.so bin/helper.o bin/CalendarParser.o bin/LinkedListAPI.o

clean:
    rm bin/helper.o bin/CalendarParser.o bin/LinkedListAPI.o ../sharedLib.so
