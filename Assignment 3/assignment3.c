#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

static int find_readables(char *inputPath);
int readable(char *inputPath);

/* Description:
 * 	wrapper function for find_readables, checks for initial base cases
 *
 * Params:
 *	inputPath: the path to start counting regular readable files, defaults
 		   to cwd if NULL is passed
 *
 * Return:
 * 	on success, return the result from running find_readables(inputPath)
 *	on failure, print errno message stderror and return the negative of
 *	errno, fails if inputPath is a directory that cannot be read
 */
int readable(char *inputPath)
{
	char buffer[PATH_MAX] = {'\0'};

	// default to cwd if no path was given
	if (inputPath == NULL) {
		char buffer[PATH_MAX] = {'\0'};
		if (getcwd(buffer, PATH_MAX) != NULL) {
			return find_readables(buffer);
		} else {
			fprintf(stderr, "%s\n", strerror(errno));
			return errno;
		}
	}

	struct stat area;
	struct stat *s = &area;

	// try to get the state of the file or directory
	if (lstat(inputPath, s) == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -errno;
	}

	// if we are a directory, check if we can read from it
	if (S_ISDIR(s->st_mode)) {
		DIR *dir = opendir(inputPath);

		if (dir == NULL) {
			fprintf(stderr, "%s\n", strerror(errno));
			return -errno;
		}

		closedir(dir);
	}

	return find_readables(inputPath);
}

/* Description:
 * 	Recursively finds every regular readable file under inputPath
 *
 * Params:
 *	inputPath: the path to start counting regular readable files
 *
 * Return:
 * 	on success, return total number of regular readable files encountered
 *	on failure, print errno message stderror and return the negative of
 *	errno
 */
static int find_readables(char *inputPath)
{
	struct stat area;
	struct stat *s = &area;

	// base case 1: something went wrong trying to lstat file or directory
	if (lstat(inputPath, s) == -1) {
		// no read permission sub-case
		if (errno == EACCES)
			return 0;
		fprintf(stderr, "%s\n", strerror(errno));
		return -errno;
	}

	// base case 2: reached a regular file
	if (S_ISREG(s->st_mode)) {
		if(access(inputPath, R_OK) < 0) {
			if(errno == EACCES)
				return 0;
			fprintf(stderr, "%s\n", strerror(errno));
			return -errno;
		} else {
			return 1;
		}

	} else if (!S_ISDIR(s->st_mode)) {
		/*
		 * base case 3: reached special file (including symbolic links)
		 * or non-readable regular file
		 */
		return 0;
	}

	DIR *dir = opendir(inputPath);

	// base case 4: failed to open the directory
	if (dir == NULL) {
		// no read permission sub-case
		if (errno == EACCES)
			return 0;
		fprintf(stderr, "%s\n", strerror(errno));
		return -errno;
	}

	// base case 5: failed to change directory to dir
	if (chdir(inputPath) == -1) {
		int tmp_error = errno;
		closedir(dir);
		// no execute permission sub-case
		if (tmp_error == EACCES)
			return 0;
		fprintf(stderr, "%s\n", strerror(tmp_error));
		return -tmp_error;
	}

	struct dirent *entry;

	int readables = 0;

	// done to error check readdir()
	errno = 0;

	// recursion: if we reached a searchable directory...
	while ((entry = readdir(dir)) != NULL) {
		// don't step into . or .., this leads to infinite loops
		if (!strcmp(entry->d_name, ".") || 
		    !strcmp(entry->d_name, ".."))
			continue;

		int tmp = find_readables(entry->d_name);

		if (tmp < 0) {
			// error occured, return the error code
			readables = tmp;
			break;
		} else {
			readables += tmp;

			/* errno could still be equal to EACCES
			 * reset it for readdir error check since we
			 * don't care about permission denied errors
			 */
			errno = 0;
		}
	}

	// check if readdir failed
	if(readables > 0 && errno != 0) {
		readables = -errno;
		fprintf(stderr, "%s\n", strerror(errno));
	}

	chdir("..");
	closedir(dir);
	return readables;
}