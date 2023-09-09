#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <stdio.h>

/* Description
 *	adds a null terminator to the end of a string
 * Params
 *	word_end: the end of the string to add the null terminator to
 *		  the string is formatted like so "abcd....      \n"
 * Return
 *	nothing
 */
static void add_null_terminator(char *word_end) {
	int i = 1;
	while(*(word_end - i) == ' ')
		i++;
	*(word_end - i + 1) = '\0';
}

/* Description
 *	finds the line number that the passed word is on within the dictionary.
 *	assumes the dictionary (file of words) is sorted alphabetically, has
 *	one word per line, and each line is the same length (spaces are filled
 *	empty space untill new line character is reached). This function uses
 *	binary search for fast performance.
 * Params
 *	dictionaryName: the path to the file containing all the sorted words
 *			to search
 *	word: the word to try and find
 *	dictWidth: the length of each line of the dictionary
 * Return
 *	the line the word was found on, if the word wasn't found it return the
 *	negative of last line searched, and if there was an error it returns
 *	the errno for the specific error
 */
int lineNum(char *dictionaryName, char *word, int dictWidth)
{
	int dictionary = open(dictionaryName, O_RDONLY);
	int low_bound = 0;
	int high_bound = 0;
	int line_num = 0;
	int found = 0;

	if (dictionary == -1) {
		line_num = errno;
		perror("Error opening file: ");
		return line_num;
	}

	int file_size = lseek(dictionary, 0, SEEK_END);

	if(file_size == -1) {
		line_num = errno;
		perror("Error reading file size: ");
		return line_num;
	}

	high_bound = file_size / dictWidth - 1;

	char *buffer = malloc(sizeof(char) * dictWidth);
	char *search_word = malloc(sizeof(char) * dictWidth);

	// truncate the word to be a max of dictWidth - 1 characters
	memcpy(search_word, word, dictWidth);
	search_word[dictWidth - 1] = '\0';

	// keep searching the file if our search space is larger than 0
	while (low_bound <= high_bound) {
		line_num = (high_bound + low_bound) / 2;

		// round up if line_num is of the form x.5
		if((high_bound + low_bound) % 2 == 1) 
			line_num++;

		// shift the read pointer to the current line number
		if (lseek(dictionary, line_num * dictWidth, SEEK_SET) == -1) {
			line_num = -errno - 1;
			perror("Error calling lseek: ");
			break;
		}

		// read the line
		if (read(dictionary, buffer, dictWidth) == -1) {
			/* reverses opperations done on line_num
			 * at return statement to perserve error code
			 */
			line_num = -errno - 1;
			perror("Error reading file: ");
			break;
		}

		add_null_terminator(buffer + dictWidth - 1);

		int compare_result = strcmp(search_word, buffer);

		/*
		 * compare the word found on the line to the passed word, 
		 * adjust search space accordingly
		 */
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
	free(search_word);
	// add 1 to the line number since files start with line number 1
	return found ? (line_num + 1) : -(line_num + 1);
}