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
	// main variables used in program
	int max_print = 0;
	int num_of_files;
	FILE **files;
	hash_table_t hash_table;

	// check if all arguments passed are valid
	parse_args(argc, argv, &max_print, &num_of_files, &files);

	// if we reached this point, all arguments are valid, so initialize the hash table
	int error = hash_table_init(&hash_table);

	// if allocation failed, return from program (note, we don't close any files)
	if (error) {
		printf("Error: failed to initialize hash table\n");
		exit(1);
	}

	// count the word pairs for each provided file, then close it
	for (int i = 0; i < num_of_files; i++) {
		count_word_pairs(files[i], &hash_table);
		fclose(files[i]);
	}

	// free the files array
	free(files);

	// show the results to the user
	print_results(&hash_table, max_print);

	// free the hash table
	hash_table_free(&hash_table);

	return 0;
}