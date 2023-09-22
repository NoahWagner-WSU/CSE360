/*
 * name: Noah Wagner
 * assignment:
 *	assignment4 -- acts as a pipe using ":" to seperate programs.
 * description:
 *	See CS 360 Processes and Exec/Pipes lecture for helpful tips.
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

/* Description:
 *	error check wrapper for execvp
 *
 * Params:
 *	prog_path: cooresponds to the first input of execvp
 *	argv: cooresponds to the second input of execvp
 *
 * Return:
 * 	returns nothing, exits the program with errno status if execvp fails
 *	also prints cooresponding errno message to stderr
 */
void my_exec(char *prog_path, char **argv) {
	execvp(prog_path, argv);
	fprintf(stderr, "%s\n", strerror(errno));
	exit(errno);
}

/* ./assignment4 <arg1> : <arg2>
 *	Where: <arg1> and <arg2> are optional parameters that specify 
 *	the programs to be run. If <arg1> is specified but <arg2> is 
 *	not, then <arg1> should be run as though there was not a colon. 
 *	Same for if <arg2> is specified but <arg1> is not.
 */
int main(int argc, char **argv)
{
	char **left_args = argv + 1;
	char **right_args = NULL;

	// find the colon
	int i = 1;
	while(i < argc) {
		if(!strcmp(argv[i], ":")) {
			argv[i] = NULL;
			break;
		}
		i++;
	}

	// find the right_arg
	if(++i < argc)
		right_args = argv + i;

	// print usage statement if no args or just ":" was passed
	if(!left_args[0] && !right_args) {
		fprintf(stderr, "Usage: ./assignment4 <arg1> : <arg2>\n");
		return EXIT_FAILURE;
	}

	// run only the left/right program if the other wasn't provided
	if(!right_args)
		my_exec(left_args[0], left_args);
	else if(!left_args[0])
		my_exec(right_args[0], right_args);

	// initialize the pipe (both left/right args were specified)
	int fd[2];
	int rdr, wtr;
	pipe(fd);
	rdr = fd[0];
	wtr = fd[1];

	// spawn child and pipe its stdout to parents stdin
	switch(fork()) {
		case -1:
			fprintf(stderr, "%s\n", strerror(errno));
			return errno;
		case 0:
			close(rdr);
			// close stdout
			close(1);
			dup(wtr);
			close(wtr);
			my_exec(left_args[0], left_args);
		default:
			close(wtr);
			// close stdin
			close(0);
			dup(rdr);
			close(rdr);
			my_exec(right_args[0], right_args);
	}
	return 0;
}