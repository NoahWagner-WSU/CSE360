/*
NAME: Noah Wagner
DATE: 8/29/23
ClASS: CSE360
ASSIGNMENT: Assignment 1
DESC: counts repeating word pairs from user specified files from the command
	line arguments. Uses a hashmap for fast performance, and prints results to
	stdout.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "program.h"

int main(int argc, char **argv) {
	int max_print = -1;
	int num_of_files;
	FILE **files;

	int file_read_fail = parse_args(argc, argv, &max_print, &num_of_files, &files);

	if (file_read_fail) {
		for (int i = 0; i < num_of_files; i++) {
			fclose(files[i]);
		}
		free(files);
		exit(1);
	}

	hash_table_t hash_table;

	int error = hash_table_init(&hash_table);

	if (error) {
		printf("Error: failed to initialize hash table\n");
		exit(1);
	}

	for (int i = 0; i < num_of_files; i++) {
		count_word_pairs(files[i], &hash_table);
		fclose(files[i]);
	}

	free(files);

	print_results(&hash_table, max_print);

	hash_table_free(&hash_table);

	return 0;
}