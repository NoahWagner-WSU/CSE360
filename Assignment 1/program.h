#include "hash_table.h"

int parse_args(int argc, char **argv, int *max_print, int *num_of_files, FILE ***files);

void count_word_pairs(FILE *file, hash_table_t *hash_table);

// prints the final hash map, max_print would be -count
void print_results(hash_table_t *hash_table, int max_print);