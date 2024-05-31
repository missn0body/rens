// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// POSIX libraries
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

static const char *defdir = ".";
static constexpr short bufsize = 256;

inline bool check_if_dir(const struct stat *input) { return (S_ISDIR(input->st_mode)); }

int main(int argc, char *argv[])
{
	if(argc < 2 || !*argv[1])
	{
		fprintf(stderr, "%s: too few arguments, try \"--help\"\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Define the important structures
	struct stat    statbuf;
	struct dirent *direntobj = nullptr;
	DIR 	      *dirobj 	 = nullptr;

	// And some important buffers
	const char *programname = argv[0];
	char dirname[bufsize] = {0}, pattern[bufsize] = {0};
	char fnindex[bufsize - 1] = {0};

	// Flags that can be set with arguments
	bool verbose = false, preview = false;
	short suffix_width = 0;

	// Command-line arguments parsing here...

	// If we did not get a directory name, just look inside current working directory
	if(dirname[0] == '\0') 			     snprintf(dirname, bufsize, "%s", defdir);
	if((dirobj = opendir(dirname)) == nullptr) { perror(programname); exit(EXIT_FAILURE); }
	// Plus one for forwards slash
	size_t dirlen = strlen(dirname) + 1;

	// Setting full directory path ahead of time instead of overwriting the same
	// memory locations over and over again
	snprintf(fnindex, bufsize + 1, "%s/", dirname);
	// Iterating through files in the chosen directory
	for(size_t i = 0; (direntobj = readdir(dirobj)) != nullptr; i++)
	{
		// Attach filename that we got to end of path so we can stat
		snprintf(fnindex + dirlen, bufsize, "%s", direntobj->d_name);

		// The index will increment regardless if its a directory or not,
		// so lets decrement to avoid gaps in numbers
		if(stat(fnindex, &statbuf) == -1) { i--; perror(programname); continue; }
		if(check_if_dir(&statbuf)) { i--; continue; }

		// A bunch of debugging prints
		printf("realpath \"%s\", basename \"%s\", ", fnindex, direntobj->d_name);
		if(i == 0) printf("renamed \"%s\"\n", 	  (pattern[0] == '\0') ? fnindex : pattern);
		else	   printf("renamed \"%s%*ld\"\n", (pattern[0] == '\0') ? fnindex : pattern, suffix_width, i);
	}

	return 0;
}
