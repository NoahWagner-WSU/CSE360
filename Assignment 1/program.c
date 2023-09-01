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

int parse_args(int argc, char **argv, int *max_print, int *num_of_files, FILE ***files) {
	if(argc == 1) {
		printf("Usage: ./pairsofwords <-count> fileName1 <fileName2> <filename3> ... \n");
		exit(1);
	}

	int failed = 0;
	char garbage[100];
	int file_start_index = 1;

	// check for optional flag
	int count_result = sscanf(argv[1], "-%d%s", max_print, garbage);

	if(count_result == 2) 
		failed = 1;
	else if (sscanf(argv[1], "-%s", garbage) && count_result != 1)
		failed = 1;

	if(failed) {
		printf("Error: recieved invalid -count argument\n");
		exit(1);
	}

	if(count_result == 1)
		file_start_index = 2;

	*num_of_files = argc - file_start_index;

	if(*num_of_files == 0) {
		printf("Error: must recieve at least 1 file\n");
		exit(1);
	}

	*files = malloc(sizeof(FILE *) * *num_of_files);

	for (int i = file_start_index; i < argc; i++) {
		FILE *file = fopen(argv[i], "r");

		if(file == NULL) {
			printf("Error: unable to read file named \"%s\"\n", argv[i]);
			*num_of_files = i - file_start_index;
			return 1;
		}

		(*files)[i - file_start_index] = file;
	}

	return 0;
}

static char *create_word_pair(int s1_length, char *s1, int s2_length, char *s2) {
	char *word_pair = (char *)malloc(s1_length + s2_length + 2);

	if(word_pair == NULL) return NULL;

	int i = 0;
	int j = 0;

	while(s1[i] != '\0') {
		word_pair[i] = s1[i];
		i++;
	}

	word_pair[i] = ' ';
	i++;

	while(s2[j] != '\0') {
		word_pair[i + j] = s2[j];
		j++;
	}

	word_pair[i + j] = '\0';

	return word_pair;
}

static int compar (const void *l, const void*r) {
	int count_left = *((int *)(((hash_table_kv_t *)l)->value));
	int count_right = *((int *)(((hash_table_kv_t *)r)->value));

	return count_right - count_left;
}

void count_word_pairs(FILE *file, hash_table_t *hash_table) {
	char *prev_word = getNextWord(file);
	char *curr_word = getNextWord(file);

	int prev_length = strlen(prev_word);
	int curr_length = 0;

	while(curr_word != NULL) {
		curr_length = strlen(curr_word);

		char *word_pair = create_word_pair(prev_length, prev_word, curr_length, curr_word);

		int *pair_count = (int *) hash_table_get(hash_table, word_pair);

		if(pair_count != NULL) {
			(*pair_count)++;
			free(word_pair);
		} else {
			int *initial_count = (int *)malloc(sizeof(int));
			*initial_count = 1;
			hash_table_set(hash_table, word_pair, (void *)initial_count);
		}

		free(prev_word);

		prev_word = curr_word;
		prev_length = curr_length;

		curr_word = getNextWord(file);
	}

	free(prev_word);
}

void print_results(hash_table_t *hash_table, int max_print) {
	hash_table_kv_t *key_values = hash_table_to_array(hash_table);

	qsort((void *)key_values, hash_table->kv_count, sizeof(hash_table_kv_t), compar);

	if(max_print == -1) 
		max_print = hash_table->kv_count;

	for (int i = 0; i < max_print; ++i) {
		printf("%10d %s\n", *((int *)key_values[i].value), key_values[i].key);
	}

	free(key_values);
}
