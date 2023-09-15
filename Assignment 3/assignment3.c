#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

int find_readables(char *inputPath)
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

	// base case 2: reached a readable regular file
	if (S_ISREG(s->st_mode) && (s->st_mode & S_IRUSR)) {
		return 1;
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
		// something else went wrong sub-case
		fprintf(stderr, "%s\n", strerror(errno));
		return -errno;
	}

	if (chdir(inputPath) == -1) {
		// no read permission sub-case
		int tmp_error = errno;
		closedir(dir);
		if (tmp_error == EACCES)
			return 0;
		fprintf(stderr, "%s\n", strerror(tmp_error));
		return -tmp_error;
	}

	struct dirent *entry;

	int readables = 0;

	// recursion: if we reached a searchable directory dir, readable(dir/file)
	while ((entry = readdir(dir)) != NULL) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;

		int tmp = find_readables(entry->d_name);
		if (tmp < 0) {
			readables = tmp;
			break;
		} else {
			readables += tmp;
		}
	}

	chdir("..");
	closedir(dir);
	return readables;
}

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

	if (lstat(inputPath, s) == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -errno;
	}

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