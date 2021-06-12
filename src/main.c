#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <ftw.h>
#include <sys/stat.h>
#include <sys/resource.h>

// ./main THING_TO_WATCH -c ARGS

struct map_pair {
	char* key;
	time_t value;
};

struct map {
	struct map_pair* cells;
	size_t map_length;
};

bool map_init(struct map* m);
ssize_t map_has_key(struct map* m, const char* key);
bool map_set_key(struct map* m, const char* key, time_t value);
bool map_remove_key(struct map* m, const char* key);
time_t map_get_key(struct map* m, const char* key);
void map_free(struct map* m);

bool map_init(struct map* m) {
	m->map_length = 0;
	m->cells = calloc(1, sizeof(struct map_pair));
	if(!m->cells) {
		return false;
	} else {
		return true;
	}
}

void map_free(struct map* m) {
	for(size_t i = 0; i < m->map_length; i++) {
		if(m->cells[i].key != NULL) {
			free(m->cells[i].key);
		}
	}
	free(m->cells);
}

ssize_t map_has_key(struct map* m, const char* key) {
	for(size_t i = 0; i < m->map_length; i++) {
		if(m->cells[i].key != NULL) {
			if(strcmp(m->cells[i].key, key) == 0) {
				return i;
			}
		}
	}

	return -1;
}

bool map_set_key(struct map* m, const char* key, time_t value) {
	if(map_has_key(m, key) > -1) {
		// Overwrite the key
		size_t position = (size_t)map_has_key(m, key);
		m->cells[position].value = value;
		return true;
	} else {
		// New key

		// Check if there is a space:
		for(size_t i = 0; i < m->map_length; i++) {
			if(m->cells[i].key == NULL) {
				m->cells[i].key = strdup(key);
				m->cells[i].value = value;
				return true;
			}
		}

		// Reallocation necessary:
		struct map_pair* tmp = realloc(m->cells, (m->map_length + 2) * sizeof(struct map_pair));
		if(!tmp) {
			return false;
		}
		m->cells = tmp;
		m->map_length++;
		m->cells[m->map_length - 1].key = strdup(key);
		m->cells[m->map_length - 1].value = value;
		return true;
	}

	return false;
}

bool map_remove_key(struct map* m, const char* key) {
	if(map_has_key(m, key) > -1) {
		size_t position = (size_t)map_has_key(m, key);

		free(m->cells[position].key);
		m->cells[position].key = NULL;

		m->cells[position].value = 0;
		return true;
	}

	return false;
}

time_t map_get_key(struct map* m, const char* key) {
	size_t position = (size_t)map_has_key(m, key);
	return m->cells[position].value;
}

struct map files_map;

bool files_modified = false;

int
file_check(fpath, sb, tflag, ftwbuf)
	const char* fpath;
	const struct stat* sb;
	int tflag;
	struct FTW* ftwbuf;
{
	// Only care about regular files...
	if(tflag == FTW_F) {
		// Is this a new file?
		if(map_has_key(&files_map, fpath) == -1) {
			files_modified = true;
		} else {
			// Has the file been modified?
			size_t position = (size_t)map_has_key(&files_map, fpath);
			if(difftime(map_get_key(&files_map, fpath), sb->st_mtime) < 0) {
				files_modified = true;
			}
		}

		// Update the file listing
		map_set_key(&files_map, fpath, sb->st_mtime);
	}

	return 0;
}

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

	// Can we even access it?
	if(stat(filename, &sfile) != 0) {
		return 1;
	}

	pid_t pid;
	int status;

	// Check if filename is a directory...
	if(S_ISDIR(sfile.st_mode)) {
		// Get a reasonable file descriptor limit
		// for when we're walking our tree.
		struct rlimit rlp = {0};
		size_t limit = 0;
		if(getrlimit(RLIMIT_NOFILE, &rlp) == 0) {
			// Set to soft limit
			limit = rlp.rlim_cur / 4;
			if(limit >= rlp.rlim_max) {
				// Set to reasonable guess
				limit = 20;
			}
		} else {
			// Set to reasonable guess
			limit = 20;
		}

		// Initialise the file map
		if(!map_init(&files_map)) {
			return 1;
		}

		// Fill the initial data
		files_modified = false;
		if(nftw(filename, file_check, limit, 0) == -1) {
			return 1;
		}

		// Loop...
		while(true) {
			// Check for modifications...
			files_modified = false;
			if(nftw(filename, file_check, limit, 0) == -1) {
				return 1;
			}

			// Check for file deletions...
			for(size_t i = 0; i < files_map.map_length; i++) {
				if(files_map.cells[i].key != NULL) {
					if(stat(files_map.cells[i].key, &sfile) != 0) {
						map_remove_key(&files_map, files_map.cells[i].key);
						files_modified = true;
					}
				}
			}

			// If files were modified, run command & wait
			if(files_modified) {
				files_modified = false;

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
						files_modified = false;
					}
				}
			}

			// Calling stat in an infinite loop will peg the CPU.
			sleep(1);
		}
	} else {
		time_t old_time;
		time_t current_time;
		time(&old_time);
		time(&current_time);

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
}
