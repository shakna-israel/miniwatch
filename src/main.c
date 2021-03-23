#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>

// ./main THING_TO_WATCH -c ARGS

int main(int argc, char* argv[]) {
	if(argc < 2) {
		return 1;
	}

	char* filename = argv[1];

	char** array = calloc(argc + 2, sizeof(char*));
	int pos = 0;

	bool found = false;

	for(int i = 0; i < argc; i++) {
		if(found) {
			array[pos] = argv[i];
			pos++;
		}

		if(strcmp(argv[i], "-c") == 0) {
			found = true;
		}
	}

	if(found == false) {
		return 1;
	}

	struct stat sfile;
	time_t old_time;
	time_t current_time;
	time(&old_time);
	time(&current_time);

	pid_t pid;
	int status;

	while(true) {
		// Check if file/dir modified...
		stat(filename, &sfile);
		current_time = sfile.st_mtime;

		if(difftime(old_time, current_time) < 0) {
			old_time = current_time;

			if((pid = fork()) < 0) {
				// Fork fail.
				return 1;
			}
			else if(pid == 0) {
				// Run our command!
				execvp(*array, array);
			}
			else {
				// Wait until our child (execvp) terminates...
				while(wait(&status) != pid) {
					old_time = current_time;
				}
			}		
		}

		// Calling stat in an infinite loop will peg the CPU.
		sleep(1);
	}
}
