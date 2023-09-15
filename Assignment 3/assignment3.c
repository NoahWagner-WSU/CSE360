#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>


int readable(char *inputPath) {
	// default to cwd if no path was given
	if(inputPath == NULL) {
		char buffer[PATH_MAX];
		if(getcwd(buffer, PATH_MAX) != NULL) {
			return readable(buffer);
		} else {
			fprintf(stderr, "%s\n", strerror(errno));
			return errno;
		}
	}

	struct stat area;
	struct stat *s = &area;

	// base case 1: something went wrong trying to lstat file or directory
	// NOTE: does this return if the last directory in the path is not readable?
	// if so, then base case 4 sub-case 1 will never be reached, as this will catch it
	if(lstat(inputPath, s) == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -errno;
	}

	// base case 2: reached a readable regular file
	if(S_ISREG(S_ISREG(s->st_mode)) && s->st_mode & S_IRUSR) {
		return 1;
	} else if(!S_ISDIR(s->st_mode)){
		/*
		 * base case 3: reached special file (including symbolic links)
		 * or non-readable regular file
		 */
		return 0;
	}

	DIR *dir = opendir(inputPath);

	// base case 4: failed to open the directory
	if(dir == NULL) {
		// no read permission sub-case
		if(errno == EACCES)
			return 0;
		// something else went wrong sub-case
		fprintf(stderr, "%s\n", strerror(errno));
		return -errno;
	}

	struct dirent *entry;

	// recursion: if we reached a searchable directory dir, readable(dir/file)
	while((entry = readdir(dir)) != NULL) {
		// readable(dir + "/" entry->d_name)
	}

	closedir(dir);
}