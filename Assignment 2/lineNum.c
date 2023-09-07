#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <stdio.h>

static int find_word(char *buffer, int *line_num, int dictWidth)
{
	int i = 0;
	while (buffer[i] != '\n')
		i++;
	if(i == dictWidth - 1) {
		i = 0;
	} else {
		i++;
		(*line_num)++;
	}
	return i;
}

static void add_null_terminator(char *word) {
	int i = 0;
	while(word[i] != '\n')
		i++;
	i--;
	while(word[i] == ' ')
		i--;
	word[++i] = '\0';
}

int lineNum(char *dictionaryName, char *word, int dictWidth)
{
	int dictionary = open(dictionaryName, O_RDONLY);
	// NOTE: check with teacher if I should error check lseek
	int binary_step = lseek(dictionary, 0, SEEK_END) / 2;
	int line_num = 0;

	int read_size = dictWidth * 2;
	char *buffer = malloc(sizeof(char) * read_size);

	// NOTE: check with teacher if this is good error checking
	if (dictionary == -1) {
		line_num = errno;
		printf("Error opening file: %s\n", strerror(line_num));
		return line_num;
	}

	int offset = lseek(dictionary, binary_step, SEEK_SET);

	while (1) {
		line_num = offset / dictWidth + 1;
		binary_step /= 2;

		if(line_num == 1) {
			lseek(dictionary, 0, SEEK_SET);
		}

		int bytes_read = read(dictionary, buffer, read_size);

		if (bytes_read == -1) {
			line_num = errno;
			printf("Error reading file: %s\n", strerror(line_num));
			break;
		}

		int word_index = find_word(buffer, &line_num, dictWidth);

		add_null_terminator(buffer + word_index);

		int compare_result = strcmp(word, buffer + word_index);

		if (compare_result == 0) {
			break;
		} else if (compare_result > 0) {
			offset = lseek(
			                 dictionary,
			                 binary_step - bytes_read,
			                 SEEK_CUR
			         );
		} else {
			offset = lseek(
			                 dictionary,
			                 -1 * (binary_step + bytes_read),
			                 SEEK_CUR
			         );
		}

		if (binary_step < 1) {
			line_num *= -1;
			break;
		}

		// forces binary step be negative at EOF
		memset(buffer, 255, read_size);
	}

	close(dictionary);
	free(buffer);
	return line_num;
}