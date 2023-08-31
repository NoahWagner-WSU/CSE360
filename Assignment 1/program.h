#include "hash_table.h"

// calls getWord until reaches the end of the file, adds the consecutive word pair info to the hash table, 
// basically just adds every consecutive word pair to the hashmap (if hashmap index already found that pair, increase the count)
void count_word_pairs(FILE *file, hash_table_t *hash_table);

// prints the final hash map, max_print would be -count
void print_results(hash_table_t *hash_table, int max_print);