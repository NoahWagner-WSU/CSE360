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

// not sure what "Growth of the table will be transparent to the code using your hash table module." means, ask prof
typedef struct hash_table_kv hash_table_kv_t;

struct hash_table_kv {
	char *key;
	void *value;
};

/*
Description
	initializes the heap, allocates memory for initial size

Params
	hash_table: a pointer to the table to initialize

Return
	0 on success, 1 on failure to allocate
*/
int hash_table_init(hash_table_t *hash_table);

/*
Description
	counts and returns all the key-value pairs in the hash map

Params
	hash_table: the hash table to count key value pairs from

Return
	the number of key value pairs in the hash table
*/
int hash_table_count_kv(hash_table_t *hash_table);

/*
Description
	gets the value from the corresponding key

Params
	hash_table: a pointer to the table to get the value from
	key: the key to index the hash by

Return
	NULL if nothing was found, value of key (void *) if found
*/
void *hash_table_get(hash_table_t *hash_table, char *key);

/*
Description
	if the passed key exists, return the value of the key, else add the 
	key value pair to the hash table

Params
	hash_table: a pointer to the table to run this function on
	key: the key to "get or set"
	value: the value to set the key to if the key isn't already 
		in the hash table (isn't used in the case where the key
		is already in the hash table)

Return
	NULL if the key isn't in the hash table, or the current value of the
	key if it is already in the hash table
*/
void *hash_table_get_or_set(hash_table_t *hash_table, char *key, void *value);

/*
Description
	puts all key-value pairs into an array for easy access. Calling 
	hash_table_free(hash_table) will free all the keys and values in
	the array, so only call the hash free function once you're done 
	using array. The array is allocated on heap, you must free it 
	when you are done using it.

Params
	hash_table: the table reformat into an array

Return
	NULL if failed to allocate the array, else a pointer to the 
	beginning of the new key-value array
*/
hash_table_kv_t *hash_table_to_array(hash_table_t *hash_table);

/*
Description
	free's all memory associated with this hash table

Params
	hash_table: a pointer to the table to free

Return
	nothing
*/
void hash_table_free(hash_table_t *hash_table);

#endif