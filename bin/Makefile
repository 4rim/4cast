CC=gcc
LIBS=-lncurses

printscreen: printscreen.o
	${CC} -o $@ $< ${LIBS}

printscreen.o: printscreen.c
	${CC} -o $@ -c $<
