#include <ncurses.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUMWEEK 7
#define SQ_HEIGHT 12
#define SQ_WIDTH 30
#define L_MARGIN 2
#define S_MARGIN 1.5

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

struct panel {
	WINDOW *main;
	WINDOW *nest;
} typedef PANEL;

const char *temp_path = "../txt/temperature.txt";
const char *fc_path = "../txt/forecast.txt";
const char *chance_path = "../txt/chance.txt";
const char *maxmin_path = "../txt/maxmin.txt";

WINDOW *GRID[3];

int temperature[3];
char forecast[3][64];
int chance[3];
int max[3];
int min[3];

/* TODO: Should check for user's terminal size before running.
 * fill_fc should use malloc to dynamically deal with strncmp. 
 * Also for forecasts (longer strings) you should have a smaller window inside
 * so the text can wrap properly. Window within window.
 * Use watch() command to check on txt files?
 * Better practice with threads. Might need a lock on the files. Research into
 * this.
 *
 * Implement ASCII art, somehow. Look at wego's source code for ref.
 * Add wind direction, color, ...
 */

int retrieve_day(time_t result);
void create_grid(void);
void destroy_grid(void);
void fill_temp(FILE *f);
void fill_fc(FILE *f);
void fill_chance(FILE *f);
void fill_maxmin(FILE *f);
float convert_to_cel(int x);

void *pthread_init_weather(void *vargp) {
	FILE *f1, *f2, *f3, *f4;
	f1 = fopen(temp_path, "r");
	f2 = fopen(fc_path, "r");
	f3 = fopen(chance_path, "r");
	f4 = fopen(maxmin_path, "r");

	for(;;)	{
		fill_temp(f1);
		fill_fc(f2);
		fill_chance(f3);
		fill_maxmin(f4);
		rewind(f1);
		rewind(f2);
		rewind(f3);
		rewind(f4);
		sleep(1);
	}
	
	fclose(f1);
	fclose(f2);
	fclose(f3);
	fclose(f4);
	
	return NULL;
}

int main()
{		
	pthread_t thread_id;
	pthread_create(&thread_id, NULL, pthread_init_weather, NULL);

	int num_day;
	time_t res = time(NULL);

	num_day = retrieve_day(res);
	if (res == -1) {
		perror("Failure");
		return -1;
	}

	initscr();
	noecho();
	cbreak(); // Line buffering disabled, reads one char at a time w/o newline
	
	keypad(stdscr, TRUE); // Enables function keys (e.g. arrows)

	printw("Today is ");
	attron(A_BOLD);
	printw("%s.\n", dict[num_day]); // Prints current day based off of dict
	attroff(A_BOLD);
	printw("The weather for the next 3 days in ");
	attron(A_REVERSE);
	printw("DURHAM, NC");
	attroff(A_REVERSE);

	refresh();
	create_grid();
	
	for (int i = 0; i < 3; num_day++, i++) {
		if (i == 0) { // for first square on grid aka current day
			mvwprintw(GRID[i], SQ_HEIGHT/6, SQ_WIDTH/6, "Today");
			mvwprintw(GRID[i], (SQ_HEIGHT/6) + L_MARGIN, SQ_WIDTH/6, "%dF/%.1fC",
					temperature[i], convert_to_cel(temperature[i]));
			mvwprintw(GRID[i], (SQ_HEIGHT/6) + (S_MARGIN * 2), SQ_WIDTH/6,
					"Chance of rain: %d%%", chance[i]);
			mvwprintw(GRID[i], (SQ_HEIGHT/6) + (S_MARGIN * 3), SQ_WIDTH/6,
					"%s", forecast[i]);
			mvwprintw(GRID[i], (SQ_HEIGHT/6) + (S_MARGIN * 5), SQ_WIDTH/6,
					"High: %d, Low: %d", max[i], min[i]);
		} else {
			mvwprintw(GRID[i], SQ_HEIGHT/6, SQ_WIDTH / 6, "%s",
					dict[num_day % (NUMWEEK-1)]);
			mvwprintw(GRID[i], (SQ_HEIGHT/6) + L_MARGIN, SQ_WIDTH/6, "%dF/%.1fC",
					temperature[i], convert_to_cel(temperature[i]));
			mvwprintw(GRID[i], (SQ_HEIGHT/6) + (S_MARGIN * 2), SQ_WIDTH/6,
					"Chance of rain: %d%%", chance[i]);
			mvwprintw(GRID[i], (SQ_HEIGHT/6) + (S_MARGIN * 3), SQ_WIDTH/6,
					"%s", forecast[i]);
			mvwprintw(GRID[i], (SQ_HEIGHT/6) + (S_MARGIN * 5), SQ_WIDTH/6,
					"High: %d, Low: %d", max[i], min[i]);
		}
		wrefresh(GRID[i]);
	}

	
	refresh();
	getch(); // program exits when any key is pressed
	destroy_grid();
	endwin();
	return 0;
}

int retrieve_day(time_t result) {
	char *day;
	if(result != (time_t)(-1)) {
		day = strtok(asctime(gmtime(&result)), " ");
		for (int i = 0; i < NUMWEEK-1; i++) {
			if (strncmp(day, dict[i].name, 2) == 0) {
				return i;
			}
		}
	} else {
		perror("Time retrieval issue.");
		return -1;
	}
	return -1;
}

void create_grid(void)
{
	int i;
	int starty = 3, startx;
	int smally = 4, smallx;

	for (i = 0; i < 3; i++) {
		startx = i * SQ_WIDTH;
		GRID[i] = newwin(SQ_HEIGHT, SQ_WIDTH, starty, startx);
	}

	for (i = 0; i < 3; i++) {
		// box(GRID[i], 0, 0);
		wrefresh(GRID[i]);
	}
}

void destroy_grid(void) {
	for (int i = 0; i < 3; i++) {
		wborder(GRID[i], ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		// delwin does not delete borders, need to use wborder to handle cleanly
		delwin(GRID[i]);
	}
}

void fill_temp(FILE *f){
	char buf[4];
	int temp, i = 0;
	while (fgets(buf, sizeof(buf), f) && i < 3) {
		temp = atoi(buf);
		temperature[i] = temp;
		i++;
			}
}

void fill_fc(FILE *f){
	char buf[60];
	int i = 0;
	while (fgets(buf, sizeof(buf), f) && i < 3) {
		strtok(buf, "\n");
		strncpy(forecast[i], buf, 60);
		i++;
	}
}

void fill_chance(FILE *f) {
	char buf[4];
	int temp, i = 0;
	while (fgets(buf, sizeof(buf), f) && i < 3) {
		temp = atoi(buf);
		chance[i] = temp;
		i++;	
			}
}

void fill_maxmin(FILE *f) {
	char buf[6];
	int temp, i = 0;
	while (fgets(buf, sizeof(buf), f) && i < 6) {
		temp = atoi(buf);
		if (i >= 3) {
			min[i-3] = temp;
		} else {
			max[i] = temp;
		}
		i++;
	}
}

float convert_to_cel(int x) {
	return (((float) x - 32) / 1.8);
}
