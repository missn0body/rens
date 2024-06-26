// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// POSIX libraries
#include <sys/stat.h>
#include <dirent.h>

/////////////////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////////////////

static const char *VERSION = "1.0.0";

typedef char flag_t;
static flag_t status = 0x00;
enum : char
{
	VERBOSE  = (1 << 0),
	PREVIEW  = (1 << 1),
	NOBAIL   = (1 << 2),
	NOPAT    = (1 << 3),
	QUIET    = (1 << 4),
	FIRSTSUF = (1 << 5)
};

static const char *defdir = ".";
static constexpr short bufsize  = 256;
static constexpr short defwidth = 3;

/////////////////////////////////////////////////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////////////////////////////////////////////////

inline bool check_if_dir(const struct stat *input) { return (S_ISDIR(input->st_mode)); }

static inline void   set(flag_t *in, char what) { (*in) |=   what;  }
static inline void unset(flag_t *in, char what) { (*in) &= (~what); }
static inline bool  test(flag_t *in, char what) { return (((*in) & what) != 0); }

static inline const char *boolstr(const bool what) 	 { return (what) ? "true" : "false"; }
static inline const char  *bitstr(flag_t *in, char what) { return boolstr(test(in, what));   }

void usage(void)
{
	printf("REName and Suffix (%s): big brother renames your files\n", VERSION);
	printf("created by anson <thesearethethingswesaw@gmail.com>\n\n");
	printf("usage:\n\trens (-h / --help)\n\trens --version\n");
	printf("\trens [-npqsv] [<pattern>] [<directory>]\n");
	printf("\trens [-npqsv] [-w <number>] [<pattern>] [<directory>]\n\n");

	printf("options:\n\t%18s\t%s\n","-n, --no-bail","if buffer overflow/truncation detected, do not stop on precaution, only on error");
	printf("\t%18s\t%s\n", "-p, --preview",		"previews the operation verbosely, but does not actually operate. activates verbose");
	printf("\t%18s\t%s\n", "-q, --quiet",		"does not print any status, verbose, or error messages. counters verbose");
	printf("\t%18s\t%s\n", "-s, --suffix-first",	"signals RENS to add a suffix to the initial file in directory");
	printf("\t%18s\t%s\n", "-v, --verbose",		"prints verbose diagnostic information");
	printf("\t%18s\t%s\n", "-w, --width",		"choose the number of digits to append to new filename, default is 3");
	printf("\t%18s\t%s\n", "<number>",		"a decimal, non-negative number");
	printf("\t%18s\t%s\n", "<pattern>",		"a non-REGEX string to rename all files in directory to, defaults to original filename");
	printf("\t%18s\t%s\n", "<directory>",		"a directory to search through, defaults to \".\"\n");

	printf("copyright (c) 2024, see LICENSE for related details\n");
	return;
}

void version(void)
{
	printf("REName and Suffix (%s): big brother renames your files\n", VERSION);
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////
// main() function
/////////////////////////////////////////////////////////////////////////////////////////

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
	char dirname[bufsize] = {0}, pattern[bufsize] = {0}, newname[bufsize] = {0};
	char fnindex[bufsize - 1] = {0};

	// A number that can be set by arguments
	short suffix_width = 0;

	// Command-line arguments parsing here...
	int c;
	while(--argc > 0 && (*++argv)[0] != '\0')
	{
		// I didn't want to use another header for isdigit(), so this ugly thing
		// will have to do
		if((*argv)[0] != '-' && ((**argv - '0') >= 1 || (**argv - '0') <= 9))
		{
			if(dirname[0] != '\0')
			{
				fprintf(stderr, "%s: discarded program input -- \"%s\"\n", programname, *argv);
				continue;
			}

			snprintf((pattern[0] == '\0') ? pattern : dirname, bufsize, "%s", *argv);
		}

		if((*argv)[0] == '-')
		{
			// If there's another dash, then it's a long option.
			// Move the pointer up 2 places and compare the word itself.
			if((*argv)[1] == '-')
			{
				// Using continue statements here so that the user
				// can use both single character and long options
				// simultaniously, and the loop can test both.
				if(strcmp((*argv) + 2, "help")    == 0) { usage();   exit(EXIT_SUCCESS); }
				if(strcmp((*argv) + 2, "version") == 0) { version(); exit(EXIT_SUCCESS); }
				if(strcmp((*argv) + 2, "width")   == 0)
				{
					if(argv[1] == nullptr)
					{
						fprintf(stderr, "%s: no argument for width\n", programname);
						exit(EXIT_FAILURE);
					}

					suffix_width = atoi(argv[1]);
					continue;
				}
				if(strcmp((*argv) + 2, "no-bail") == 0) { set(&status, NOBAIL); continue; }
				if(strcmp((*argv) + 2, "preview") == 0)
				{
					set(&status, PREVIEW);
					set(&status, VERBOSE);
					continue;
				}
				if(strcmp((*argv) + 2, "verbose") == 0) { set(&status, VERBOSE); continue; }
				if(strcmp((*argv) + 2, "suffix-first") == 0) { set(&status, FIRSTSUF); continue; }
				if(strcmp((*argv) + 2, "quiet") == 0)
				{
					set(&status, QUIET);
					if(test(&status, VERBOSE))
					{
						fprintf(stderr, "%s: quiet flag counters verbose flag\n", programname);
						unset(&status, VERBOSE);
					}

					continue;
				}
			}
			while((c = *++argv[0]))
			{
				// Single character option testing here.
				switch(c)
				{
					case 'h': usage(); exit(EXIT_SUCCESS);
					case 'w':
						  if(argv[1] == nullptr)
						  {
							fprintf(stderr, "%s: no argument for width\n", programname);
							exit(EXIT_FAILURE);
						  }

						  suffix_width = atoi(argv[1]);
						  break;

					case 'n': set(&status, NOBAIL);  break;
					case 'p':
						  set(&status, PREVIEW);
						  set(&status, VERBOSE);
						  break;

					case 'v': set(&status, VERBOSE); break;
					case 's': set(&status, FIRSTSUF); break;
					case 'q':
						  set(&status, QUIET);
						  if(test(&status, VERBOSE))
						  {
							fprintf(stderr, "%s: quiet flag counters verbose flag\n", programname);
							unset(&status, VERBOSE);
						  }

						  break;
					// This error flag can either be set by a
					// completely unrelated character inputted,
					// or you managed to put -option instead of
					// --option.
					default : fprintf(stderr, "%s: unknown option -- \"%s\", try \"--help\"\n", programname, *argv);
						  exit(EXIT_FAILURE);
				}
			}

			continue;
		}
	}

	// If we did not get a directory name, just look inside current working directory
	if(dirname[0] == '\0') 			     snprintf(dirname, bufsize, "%s", defdir);
	if(pattern[0] == '\0')			     set(&status, NOPAT);
	if(suffix_width <= 0)			     suffix_width = defwidth;
	if((dirobj = opendir(dirname)) == nullptr) { perror(programname); exit(EXIT_FAILURE); }
	// Plus one for forwards slash
	size_t dirlen = strlen(dirname) + 1, ret = 0;

	// There's two checks for bailout, with two uses each, so it's more efficient
	// to just refer to a single bool instead of complicating inline ternary ops
	bool no_bail = test(&status, NOBAIL);
	// Same with the quiet flag
	bool quiet = test(&status, QUIET);

	// Verbose output
	if(test(&status, VERBOSE))
	{
		printf("%s: dirname is %s\n", programname, dirname);
		printf("%s: pattern is %s\n", programname, pattern);
		printf("%s: suffix_width is %d\n", programname, suffix_width);
		printf("%s: no_bail is %s\n", programname, boolstr(no_bail));
		printf("%s: no_pat is %s\n", programname, bitstr(&status, NOPAT));
		printf("%s: preview is %s\n", programname, bitstr(&status, PREVIEW));
		printf("%s: first suffix is %s\n", programname, bitstr(&status, FIRSTSUF));
		putchar('\n');
	}

	// Setting full directory path ahead of time instead of overwriting the same
	// memory locations over and over again
	snprintf(fnindex, bufsize + 1, "%s/", dirname);
	// Iterating through files in the chosen directory
	for(size_t i = 0; (direntobj = readdir(dirobj)) != nullptr; i++)
	{
		// Attach filename that we got to end of path so we can stat
		ret = snprintf(fnindex + dirlen, bufsize, "%s", direntobj->d_name);
		if(ret >= bufsize - 1)
		{
			// If it seems that the full path is too big for our buffers, then either exit or warn
			if(!quiet) fprintf(stderr, "%s: fnindex (realpath of file) might be truncated, %s\n", programname, (no_bail) ?  "continuing..." :
																	"bailing out...");
			if(!no_bail) exit(EXIT_FAILURE);
		}

		// The index will increment regardless if its a directory or not,
		// so lets decrement to avoid gaps in numbers
		if(stat(fnindex, &statbuf) == -1) { i--; perror(programname); continue; }
		if(check_if_dir(&statbuf)) { i--; continue; }

		// If pattern not specified, then just grab current filename
		if(test(&status, NOPAT)) snprintf(pattern, bufsize, "%s", direntobj->d_name);

		// Assemble the new name
		if(i == 0 && !test(&status, FIRSTSUF))  ret = snprintf(newname, bufsize, "%s/%s", dirname, pattern);
		else					ret = snprintf(newname, bufsize, "%s/%s%0*ld", dirname, pattern, suffix_width, i);

		if(ret >= bufsize - 1)
		{
			// If it seems that the new name is too big for our buffers, then either exit or warn
			if(!quiet) fprintf(stderr, "%s: newname might be truncated, %s\n", programname, (no_bail) ? "continuing..." :
														    "bailing out...");
			if(!no_bail) exit(EXIT_FAILURE);
		}

		// A bunch of debugging prints
		if(test(&status, VERBOSE))
		{
			printf("realpath\t%s\n", fnindex);
			printf("basename\t%s\n", direntobj->d_name);
			printf("renamed to\t%s\n\n", newname);
		}

		if(!test(&status, PREVIEW)) rename(fnindex, newname);
	}

	return 0;
}

