#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	FILE *fp;
	char *path = "~/weather.sh";
	char *args[] = {NULL};
	execv(path, args);

	fp = fopen(path, "r");
	return 0;
}
