#include "hash_table.h"

/*
Description
	makes sure all command line arguments are correct, exits program
	if there is any argument that is invalid, else it fills all the
	parameters to the command line arguments

Params
	argc: total number of command line arguments
	argv: array of input strings from command arguments
	max_print: the parsed count argument
	num_of_files: value this function fills to the number of files opened
	files: a pointer to an array of file pointers
	
Return
	nothing
*/
void parse_args(int argc, char **argv, int *max_print, int *num_of_files, FILE ***files);

/*
Description
	counts all consecutive word pairs from the given file pointer
	puts the results in the hash table

Params
	file: the file to count word pairs from
	hash_table: the hash table to put the results to
	
Return
	nothing
*/
void count_word_pairs(FILE *file, hash_table_t *hash_table);

/*
Description
	prints the results of the program, prints the "max_print" most counted
	word pairs found. Prints all found consecutive word pairs if max_print is
	0

Params
	hash_table: the hash table to print
	max_print: the max word pair counts to print (from greatest down)
	
Return
	nothing
*/
void print_results(hash_table_t *hash_table, int max_print);