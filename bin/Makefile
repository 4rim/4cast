CC=gcc
LIBS=-lncurses -pthread
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb

printscreen: printscreen.o
	${CC} -o $@ $< ${LIBS} ${CFLAGS}

printscreen.o: printscreen.c
	${CC} -o $@ -c $<

clean:
	rm -rf *.o
