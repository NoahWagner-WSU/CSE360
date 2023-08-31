#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "linked_list.h"

// NOTE: don't forget to add comments

// NOTE: no hash_remove functions (removes single key value pairs) since there is no need for it in this program (maybe add later if needed?)
typedef struct hash_table hash_table_t;

struct hash_table {
	linked_list_t *buckets;
	int bucket_count, kv_count;
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
	add a key value pair to the hash table, expands the hash table
	if neccessary

Params
	hash_table: a pointer to the hash table to add the key value pair to
	key: the key to add
	value: the value of the key

Return
	returns 0 on success and 1 on failure to allocate space for the key value pair
*/
int hash_table_set(hash_table_t *hash_table, char *key, void *value);

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