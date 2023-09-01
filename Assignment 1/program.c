/*
NAME: Noah Wagner
DATE: 8/29/23
ClASS: CSE360
ASSIGNMENT: Assignment 1
DESC: Contains the main functions of the program to load files, read repeated
	word pairs and put them in the hash table, and print out the program's
	results.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "program.h"
#include "getWord.h"
#include "hash_table.h"

/*
Description
	checks if the count argument is valid, exits to program if the
	count is invalid

Params
	arg1: the second string in the argv variable from main

Return
	returns 1 if the user provided the argument,
	and 0 if no count argument was specified
*/
static int parse_count(char *arg1, int *max_print) {
	int failed = 0;
	char garbage[100];

	// check for optional flag
	int count_result = sscanf(arg1, "-%d%s", max_print, garbage);

	// all sscanf conditions that means we recieved an invalid count
	if (count_result == 2)
		failed = 1;
	else if (sscanf(arg1, "-%s", garbage) && count_result != 1)
		failed = 1;
	else if (*max_print < 0)
		failed = 1;

	// exit if count is invalid
	if (failed) {
		printf("Error: recieved invalid -count argument\n");
		exit(1);
	}

	// return whether we got count arg or not
	return (count_result == 1);
}

/*
Description
	opens all the files specified from the rest of the arguments
	in argv. Sets num_of_files to the number of files opened, exits
	the program if a file could not be read

Params
	file_start_index: the index where to start reading files from argv
	argc: total number of command line arguments
	argv: array of input strings from command arguments
	num_of_files: value this function fills to the number of files opened
	files: a pointer to an array of file pointers

Return
	nothing
*/
static void parse_files(int file_start_index, int argc, char **argv, int *num_of_files, FILE ***files) {
	// calculate the number of files to open
	*num_of_files = argc - file_start_index;

	// if there aren't any files, exit
	if (*num_of_files == 0) {
		printf("Error: must recieve at least 1 file\n");
		exit(1);
	}

	// allocate space for files
	*files = malloc(sizeof(FILE *) * *num_of_files);

	// check if malloc failed, if so exit()
	if(files == NULL) {
		printf("Error: failed to open files\n");
		exit(1);
	}

	// if we failed to read a file
	int file_read_fail = 0;

	// start reading all the provided files
	for (int i = file_start_index; i < argc; i++) {
		FILE *file = fopen(argv[i], "r");

		// if the file didn't open...
		if (file == NULL) {
			printf("Error: unable to read file named \"%s\"\n", argv[i]);
			// set re-calculate num of files so we know how many to free
			*num_of_files = i - file_start_index;
			file_read_fail = 1;
			break;
		}

		(*files)[i - file_start_index] = file;
	}

	// if we failed to read a file, close the previously opened files
	if (file_read_fail) {
		for (int i = 0; i < *num_of_files; i++) {
			fclose((*files)[i]);
		}
		free(*files);
		exit(1);
	}
}

void parse_args(int argc, char **argv, int *max_print, int *num_of_files, FILE ***files) {
	// if no arguments were provided, print a usage statement
	if (argc == 1) {
		printf("Usage: ./pairsofwords <-count> fileName1 <fileName2> ... \n");
		exit(1);
	}

	// calculate where the files start (either at index 1 or 2)
	int file_start_index = 1 + parse_count(argv[1], max_print);

	// open all the files
	parse_files(file_start_index, argc, argv, num_of_files, files);
}

/*
Description
	concatenates two strings together with a space between them 

Params
	s1_length: the length of the first string s1
	s1: the first string, will be the first word in the final string
	s2_length: the length of the second string s2
	s2: the second string, will be the second word in the final string
	
Return
	the concatenated string, allocated on the heap, if malloc failed
	return NULL
*/
static char *create_word_pair(int s1_length, char *s1, int s2_length, char *s2) {
	// allocate enough space for s1 and s2, a space between them, and a null terminator
	char *word_pair = (char *)malloc(s1_length + s2_length + 2);

	if (word_pair == NULL) return NULL;

	int i = 0;
	int j = 0;

	// add s1 to the final word pair
	while (s1[i] != '\0') {
		word_pair[i] = s1[i];
		i++;
	}

	// add the space between the words
	word_pair[i] = ' ';
	i++;

	// add s2 to the final word pair
	while (s2[j] != '\0') {
		word_pair[i + j] = s2[j];
		j++;
	}

	// don't forget to append a null terminator
	word_pair[i + j] = '\0';

	// return the concatenated string
	return word_pair;
}

// the qsort compare function, orders from greatest -> smallest
static int compar (const void *l, const void*r) {
	int count_left = *((int *)(((hash_table_kv_t *)l)->value));
	int count_right = *((int *)(((hash_table_kv_t *)r)->value));

	return count_right - count_left;
}

void count_word_pairs(FILE *file, hash_table_t *hash_table) {
	char *prev_word = getNextWord(file);

	// if the file is empty, stop reading word pairs
	if (prev_word == NULL) return;

	char *curr_word = getNextWord(file);

	int prev_length = strlen(prev_word);
	int curr_length = 0;

	// read word pairs while we haven't reached the end of the file
	while (curr_word != NULL) {
		// update the current length
		curr_length = strlen(curr_word);

		// append the previous and current words together
		char *word_pair = create_word_pair(prev_length, prev_word, curr_length, curr_word);

		// try to index the hash table to get the amount of times word_pair has been seen already
		int *pair_count = (int *) hash_table_get(hash_table, word_pair);

		// if pair_count has been found, increment the count
		if (pair_count != NULL) {
			(*pair_count)++;
			free(word_pair);
		} else {
			// if the pair has not been encountered, put an integer at that hash index
			int *initial_count = (int *)malloc(sizeof(int));
			*initial_count = 1;
			hash_table_set(hash_table, word_pair, (void *)initial_count);
		}

		// free the previous word
		free(prev_word);

		// update the previous word
		prev_word = curr_word;
		prev_length = curr_length;

		// update the current word
		curr_word = getNextWord(file);
	}

	// don't forget to free the final word
	free(prev_word);
}

void print_results(hash_table_t *hash_table, int max_print) {
	// convert the hash table to an array of key value pairs
	hash_table_kv_t *key_values = hash_table_to_array(hash_table);

	// sort the array
	qsort((void *)key_values, hash_table->kv_count, sizeof(hash_table_kv_t), compar);

	// if max_print is invalid, set it to the amount of key value pairs in hash_table
	if (max_print == 0 || max_print > hash_table->kv_count)
		max_print = hash_table->kv_count;

	// print all key values
	for (int i = 0; i < max_print; i++) {
		printf("%10d %s\n", *((int *)key_values[i].value), key_values[i].key);
	}

	// free the array
	free(key_values);
}
