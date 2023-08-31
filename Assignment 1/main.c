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

typedef struct { int x, y; } point_t;

int main(int argc, char **argv) {
	// allocate an array of file pointers to be the size of sizeof(FILE *) * (argc - 1) <- if count is specified, else just argc
	// create the main hash_table
	// for every file pointer...
		// run the count word pairs function
	// print output
	// return success exit code

	hash_table_t hash_table;

	hash_table_init(&hash_table);

	FILE *f = fopen("mobydick.txt", "r");

	count_word_pairs(f, &hash_table);

	print_results(&hash_table, 10);

	hash_table_free(&hash_table);

	fclose(f);

	return 0;
}