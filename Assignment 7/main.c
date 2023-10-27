#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "assignment7.h"

#include <string.h>
#include <assert.h>
#include <ctype.h>

#define MAX_WORDS 1000000
#define DICT_MAX_WORD_LEN	256

/* Reads characters from fd until a single word is assembled */
/* and returns a copy of the word allocated from the heap.   */
/* NULL is returned at EOF.									 */
/* Words are defined to be separated by whitespace and start */
/* with an alphabetic character.  All alphabetic characters  */
/* translated to lower case and punctuation is removed.      */
char* getNextWord(FILE* fd) {
	char ch;								/* holds current character */
	char wordBuffer[DICT_MAX_WORD_LEN];		/* buffer for build a word */
	int putChar = 0;						/* current pos in buffer   */

	assert(fd != NULL);		/* the file descriptor better not be NULL */

	/* read characters until we find an alphabetic one (or an EOF) */
	while ((ch = fgetc(fd)) != EOF) {
		if (isalpha(ch)) break;
	}
	if (ch == EOF) return NULL;		/* if we hit an EOF, we're done */

	/* otherwise, we have found the first character of the next word */
	wordBuffer[putChar++] = tolower(ch);

	/* loop, getting more characters (unless there's an EOF) */
	while ((ch = fgetc(fd)) != EOF) {
		/* the word is ended if we encounter whitespace */
		/* or if we run out of room in the buffer       */
		if (isspace(ch) || putChar >= DICT_MAX_WORD_LEN - 1) break;

		/* otherwise, copy the (lowercase) character into the word   */
		/* but only if it is alphanumeric, thus dropping punctuation */
		if (isalnum(ch)) {
			wordBuffer[putChar++] = tolower(ch);
		}
	}

	wordBuffer[putChar] = '\0';		/* terminate the word          */
	return strdup(wordBuffer);		/* re-allocate it off the heap */
}


int main(int argc, char **argv) 
{
	FILE *file = fopen("lorem", "r");
	char **words = malloc(sizeof(*words) * MAX_WORDS);
	char *word = NULL;

	int word_count = 0;
	while((word = getNextWord(file)) != NULL && word_count < MAX_WORDS) {
		words[word_count] = word;
		word_count++;
	}

	clock_t before_time = clock();

	setSortThreads(4);
	sortThreaded(words, word_count);

	printf("Total time: %f seconds\n", (double)(clock() - before_time) / CLOCKS_PER_SEC);
	fflush(stdout);

	for (int i = 0; i < word_count; ++i) {
		free(words[i]);
	}
	free(words);
	return 0;
}