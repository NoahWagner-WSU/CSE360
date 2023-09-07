#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <stdio.h>

static void add_null_terminator(char *word_end) {
	int i = 1;
	while(*(word_end - i) == ' ')
		i++;
	*(word_end - i + 1) = '\0';
}

int lineNum(char *dictionaryName, char *word, int dictWidth)
{
	int dictionary = open(dictionaryName, O_RDONLY);
	int low_bound = 0;
	// NOTE: check with teacher if I should error check lseek
	int high_bound = lseek(dictionary, 0, SEEK_END) / dictWidth;
	int line_num = 0;
	int found = 0;

	char *buffer = malloc(sizeof(char) * dictWidth);

	// NOTE: check with teacher if this is good error checking
	if (dictionary == -1) {
		line_num = errno;
		perror("Error opening file: ");
		return line_num;
	}

	lseek(dictionary, high_bound * dictWidth / 2, SEEK_SET);

	while (low_bound <= high_bound) {
		line_num = (high_bound + low_bound) / 2;

		lseek(dictionary, line_num * dictWidth, SEEK_SET);

		int bytes_read = read(dictionary, buffer, dictWidth);

		if (bytes_read == -1) {
			// reverses opperations done on line_num at return statement to perserve error code
			line_num = -errno - 1;
			perror("Error reading file: ");
			break;
		}

		add_null_terminator(buffer + dictWidth - 1);

		int compare_result = strcmp(word, buffer);

		if (compare_result == 0) {
			found = 1;
			break;
		} else if (compare_result > 0) {
			low_bound = line_num + 1;
		} else {
			high_bound = line_num - 1;
		}
	}

	close(dictionary);
	free(buffer);
	// ask teacher if first line should be considered 1 or 0
	return found ? (line_num + 1) : -(line_num + 1);
}