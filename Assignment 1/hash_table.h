#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "linked_list.h"

// NOTE: don't forget to add comments

// NOTE: no hash_remove functions (removes single key value pairs) since there is no need for it in this program (maybe add later if needed?)
typedef struct hash_table hash_table_t;

struct hash_table {
	linked_list_t *buckets;
	int bucket_count;
};

// allocates memory for the initial size of the heap (with empty linked lists)
int hash_table_init(hash_table_t *hash_table);

// returns NULL if nothing was found
void *hash_table_get(hash_table_t *hash_table, char *key);

// calls rehash if the rehash condition is met
int hash_table_put(hash_table_t *hash_table, char *key, void *value);

// free's all memory associated with this hashmap (calls linked_list_free)
void hash_table_free(hash_table_t *hash_table);

#endif