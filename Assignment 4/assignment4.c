/*
 * name: Noah Wagner
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

void my_exec(char *prog_path, char **argv) {
	execvp(prog_path, argv);
	fprintf(stderr, "%s\n", strerror(errno));
	exit(errno);
}

/*
 * ./assignment4 <arg1> : <arg2>
 *	Where: <arg1> and <arg2> are optional parameters that specify the programs
 *	to be run. If <arg1> is specified but <arg2> is not, then <arg1> should be
 *	run as though there was not a colon. Same for if <arg2> is specified but
 *	<arg1> is not.
 */
int main(int argc, char **argv)
{
	char **left_args = argv + 1;
	char **right_args = NULL;

	int i = 1;
	while(i < argc) {
		if(!strcmp(argv[i], ":")) {
			argv[i] = NULL;
			break;
		}
		i++;
	}

	if(++i < argc)
		right_args = argv + i;

	if(!left_args[0] && !right_args) {
		fprintf(stderr, "Usage: ./assignment4 <arg1> : <arg2>\n");
		return EXIT_FAILURE;
	}

	if(!right_args)
		my_exec(left_args[0], left_args);
	else if(!left_args[0])
		my_exec(right_args[0], right_args);

	int fd[2];
	int rdr, wtr;
	pipe(fd);
	rdr = fd[0];
	wtr = fd[1];

	switch(fork()) {
		case -1:
			fprintf(stderr, "%s\n", strerror(errno));
			return errno;
		case 0:
			close(rdr);
			close(1); // close stdout
			dup(wtr);
			close(wtr);
			my_exec(left_args[0], left_args);
		default:
			close(wtr);
			close(0); // close stdin
			dup(rdr);
			close(rdr);
			my_exec(right_args[0], right_args);
	}
	return 0;
}