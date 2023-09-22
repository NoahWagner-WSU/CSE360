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

/*
 * ./assignment4 <arg1> : <arg2>
 *	Where: <arg1> and <arg2> are optional parameters that specify the programs
 *	to be run. If <arg1> is specified but <arg2> is not, then <arg1> should be
 *	run as though there was not a colon. Same for if <arg2> is specified but
 *	<arg1> is not.
 */


int main(int argc, char **argv, char **envp)
{
	char **left_args = argv + 1;
	char **right_args;

	int i = 1;
	while(strcmp(argv[i], ":") && (i < argc - 1))
		i++;

	argv[i] = NULL;

	if(i + 1 < argc)
		right_args = argv + i + 1;

	int fd[2];
	int rdr, wtr;
	pipe(fd);
	rdr = fd[0];
	wtr = fd[1];

	if(fork()) {
		close(rdr);
		close(1); // close stdout
		dup(wtr);
		close(wtr);
		// for some reason this execve call isn't working
		execve(argv[1], left_args, envp);
		// code can execute here, idk why
	} else {
		close(wtr);
		close(0); // close stdin
		dup(rdr);
		close(rdr);
		execve(argv[i + 1], right_args, envp);
	}
	return 0;
}