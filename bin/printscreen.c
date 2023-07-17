#include <ncurses.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define NUMWEEK 7

int main()
{	
	int row, col, maxcol;
	int num_day;
	char *day;
	time_t res = time(NULL);

	struct entry {
		char *name;
		int num;
	};

	struct entry dict[NUMWEEK] = {
		{ "Sun", 0 }, 
		{ "Mon", 1 }, 
		{ "Tue", 2 }, 
		{ "Wed", 3 },
		{ "Thu", 4 },
		{ "Fri", 5 },
		{ "Sat", 6 },
	};

	if(res != (time_t)(-1)) {
		day = strtok(asctime(gmtime(&res)), " ");
		for (int i = 0; i < NUMWEEK-1; i++) {
			if (strncmp(day, dict[i].name, 2) == 0) {
				num_day = i;
			}
		}
	} else {
		exit(1);
	}

	initscr();
	noecho();
	keypad(stdscr, TRUE);
	getmaxyx(stdscr,row,col);
	maxcol = col;
	printw("The weather for the next 3 days:");
	
	for (; (num_day % NUMWEEK-1) < 3; num_day++) {
		move(row/4,col/8);
		attron(A_BOLD);
		
		if (num_day == 0) printw("Today");	
		else {
			printw("%s", dict[(num_day) % NUMWEEK-1]);
		}
	
		attroff(A_BOLD);
		col += (maxcol*1.3);
	}
	
	refresh();

	getch();
	endwin();

	return 0;
}
