#define NDEBUG

#include <ncurses.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUMWEEK 7
#define SQ_HEIGHT 18
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

struct square {
	WINDOW* main;
	WINDOW* sub;
};

const char *temp_path = "../txt/temperature.txt";
const char *fc_path = "../txt/forecast.txt";
const char *chance_path = "../txt/chance.txt";
const char *maxmin_path = "../txt/maxmin.txt";

// WINDOW *GRID[3];
struct square grid[3];

int temperature[3];
char forecast[3][64];
int chance[3];
int max[3];
int min[3];
int flag = 0;

/* TODO: Should check for user's terminal size before running.
 * Fix formatting/placement of text...
 * fill_fc should use malloc to dynamically deal with strncmp. 
 * Do I even need threads? Lol probably not
 * 
 * Not this-file-specific, but write some kind of build.sh/install.sh that
 * handles installing/making this program so hypothetically people could just
 * download the git repo and do a "make install"
 *
 * Implement ASCII art, somehow. Look at wego's source code for ref.
 * Add wind direction, color, ...
 * Toggle on/off for fahrenheit and celsius
 * Some kind of interactivity for cursor? (i.e. highlight square we are on)
 * Distant goal: make the location customizable with user input
 */

int check_file(FILE *f);
int retrieve_day(time_t result);
void create_grid(void);
void destroy_grid(void);
void fill_temp(FILE *f);
void fill_fc(FILE *f);
void fill_chance(FILE *f);
void fill_maxmin(FILE *f);
int convert_to_cel(int x);
int convert_to_fah(int x);
int scriptinit();

void *pthread_init_weather(void *vargp)
{
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

	FILE *f1 = fopen(temp_path, "r");
	check_file(f1);

	FILE *f2 = fopen(fc_path, "r");
	check_file(f2);

	FILE *f3 = fopen(chance_path, "r");
	check_file(f3);

	FILE *f4 = fopen(maxmin_path, "r");
	check_file(f4);

	while(1) {
		pthread_mutex_lock(&lock);
		fill_temp(f1);
		fill_fc(f2);
		fill_chance(f3);
		fill_maxmin(f4);
		rewind(f1);
		rewind(f2);
		rewind(f3);
		rewind(f4);
		pthread_mutex_unlock(&lock);
		sleep(10);
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

	int num_day, rc, x, y;
	time_t res = time(NULL);

	num_day = retrieve_day(res);
	if (num_day == -1) {
		perror("Failure in parsing date.");
		exit(-1);
	}

	rc = scriptinit();	
	if (rc != 0)
		exit(-1);

	getmaxyx(stdscr, x, y);

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
	printw("DURHAM, NC"); // TODO: Allow user to choose city/state they're in? 
						  // Country?
	attroff(A_REVERSE);

	refresh();
	create_grid();
	
	for (int i = 0; i < 3; num_day++, i++) {
		if (i == 0) { // for first square on grid aka current day
			mvwprintw(grid[i].sub, x/6, y/6, "Today");
			mvwprintw(grid[i].sub, (x/6) + L_MARGIN, y/6, "%dF/%dC",
					temperature[i], convert_to_cel(temperature[i]));
			mvwprintw(grid[i].sub, (x/6.) + (S_MARGIN * 2) + 3, y/6,
					"Chance of rain: %d%%", chance[i]);
			mvwprintw(grid[i].sub, (x/6.) + (S_MARGIN * 3) + 3, y/6,
					"%s", forecast[i]);
			mvwprintw(grid[i].sub, (x/6.) + (S_MARGIN * 5) + 3, y/6,
					"High: %d, Low: %d", max[i], min[i]);
		} else {
			mvwprintw(grid[i].sub, x/6, y/ 6, "%s",
					dict[num_day % NUMWEEK]);
			mvwprintw(grid[i].sub, (x/6) + L_MARGIN, y/6, "%dF/%dC",
					temperature[i], convert_to_cel(temperature[i]));
			mvwprintw(grid[i].sub, (x/6.) + (S_MARGIN * 2) + 3, y/6,
					"Chance of rain: %d%%", chance[i]);
			mvwprintw(grid[i].sub, (x/6.) + (S_MARGIN * 3) + 3, y/6,
					"%s", forecast[i]);
			mvwprintw(grid[i].sub, (x/6.) + (S_MARGIN * 5) + 3, y/6,
					"High: %d, Low: %d", max[i], min[i]);
		}
		wrefresh(grid[i].main);
		wrefresh(grid[i].sub);
	}

	refresh();
	getch(); // program exits when any key is pressed
	pthread_cancel(thread_id);
	// pthread_join(thread_id, NULL);
	destroy_grid();
	endwin();
}

int check_file(FILE *stream)
{
	if (!stream) {
		perror("Error: Unable to open file.");
		exit(1);
		// return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int retrieve_day(time_t result)
{
	char *day;
	if(result != (time_t)(-1)) {
		day = strtok(asctime(gmtime(&result)), " ");
		for (int i = 0; i < NUMWEEK; i++) {
			if (strncmp(day, dict[i].name, 2) == 0) {
				return i;
			}
		}
	} else {
		fprintf(stderr, "Time retrieval issue.");
		exit(-1);
	}
	return -1;
}

void create_grid(void)
{
	int i;
	int starty = 3, startx;

	for (i = 0; i < 3; i++) {
		startx = i * SQ_WIDTH;

		grid[i].main = newwin(SQ_HEIGHT, SQ_WIDTH, starty, startx);
		grid[i].sub = newwin(SQ_HEIGHT-2, SQ_WIDTH-4, starty+1, startx+2);

		wborder(grid[i].main, '|', '|', '-', '-', '+', '+', '+', '+');
		// wborder(grid[i].sub, '|', '|', '-', '-', '+', '+', '+', '+');

		// Questionable things are happening!?
		refresh();
	}

	for (i = 0; i < 3; i++) {
		// box(GRID[i], 0, 0);
		wrefresh(grid[i].main);
		wrefresh(grid[i].sub);
	}
}

void destroy_grid(void)
{
	for (int i = 0; i < 3; i++) {
		wborder(grid[i].main, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		wborder(grid[i].sub, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		// delwin does not delete borders, need to use wborder for cleanliness
		delwin(grid[i].main);
		delwin(grid[i].sub);
	}
}

void fill_temp(FILE *f)
{
	char buf[4];
	int temp, i = 0;
	while (fgets(buf, sizeof(buf), f) && i < 3) {
		temp = atoi(buf);
		temperature[i] = temp;
		i++;
	}
}

void fill_fc(FILE *f)
{
	char buf[60];
	int i = 0;
	while (fgets(buf, sizeof(buf), f) && i < 3) {
		strtok(buf, "\n");
		strncpy(forecast[i], buf, 60);
		i++;
	}
}

void fill_chance(FILE *f)
{
	char buf[4];
	int temp, i = 0;
	while (fgets(buf, sizeof(buf), f) && i < 3) {
		temp = atoi(buf);
		chance[i] = temp;
		i++;	
	}
}

void fill_maxmin(FILE *f)
{
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

// Inaccurate. But I don't want floats to display temperatures, so...?
int convert_to_cel(int x)
{
	return ((x - 32) / 1.8);
}

int convert_to_fah(int x)
{
	return ((x * 1.8) + 32);
}

int scriptinit(void)
{
	int rc;
	rc = system("./weather.sh");
	if (rc != 0) {
		fprintf(stderr, "Could not run shell script.\n");
		return -1;
	}
	return 0;
}
