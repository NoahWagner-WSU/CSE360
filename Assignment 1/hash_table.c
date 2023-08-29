/*
NAME:
DATE:
ClASS:
ASSIGNMENT:
DESC:
*/

#include <stdlib.h>
#include <string.h>
#include "hash_table.h"
#include "linked_list.h"
#include "crc64.h"

#define GROWTH_FACTOR 3
#define LOAD_FACTOR 0.75
#define INITIAL_SIZE 16

/*
Description
	initializes the heap, allocates memory for a specified size

Params
	hash_table: a pointer to the table to initialize
	size: the initial size of the heap

Return
	0 on success, 1 on failure to allocate
*/
static int init_with_size(hash_table_t *hash_table, int size) {
	// initialize the array of linked lists
	hash_table->buckets =
	    (linked_list_t *) malloc(sizeof(linked_list_t) * size);

	// if initialization failed, return error code 1
	if (hash_table->buckets == NULL) return 1;

	// initialize the starting bucket count
	hash_table->bucket_count = size;

	// initialize each linked list
	for (int i = 0; i < hash_table->bucket_count; i++) {
		linked_list_init(&(hash_table->buckets[i]));
	}

	return 0;
}

/*
Description
	adds the key value pair to the hash table, assumes the key isn't
	already in the table

Params
	hash_table: the hash table to add the key value pair to
	key: a string representing the index of the value
	value: the value (void *) associated with the key

Return
	nothing
*/
static int set_key_value(hash_table_t *hash_table, char *key, void *value) {
	// create the new key value pair
	hash_table_kv_t *key_value =
	    (hash_table_kv_t *) malloc(sizeof(hash_table_kv_t));

	// set the key and value
	key_value->key = key;
	key_value->value = value;

	// get the index to the bucket from the key
	unsigned long long index = crc64(key) % hash_table->bucket_count;

	// add it to the list
	return linked_list_add(&hash_table->buckets[index], (void *)key_value);
}

/*
Description
	Checks for the need to rehash the hash table and rehashes if required.

Params
	hash_table: the hash table to potentially rehash

Return
	nothing
*/
static void rehash_check(hash_table_t *hash_table) {

	// calculate the key-value pair threshold
	int rehash_threshold = hash_table->bucket_count * LOAD_FACTOR;

	// if the amount of key-value pairs is less than the threshold, don't rehash
	if (hash_table_count_kv(hash_table) < rehash_threshold) return;

	// the temporary hash table we will be moving the key value pairs to
	hash_table_t temp_hash_table;

	// initialize the temp hash table, make its length grow by a factor of GROWTH_FACTOR
	init_with_size(&temp_hash_table, hash_table->bucket_count * GROWTH_FACTOR);

	// iterate over every key-value pair of the hash_table
	for (int i = 0; i < hash_table->bucket_count; i++) {
		// linked list node used to iterate through the list
		linked_list_node_t *curr_node = hash_table->buckets[i].sent;

		while (curr_node != NULL) {
			// get the key value pair
			hash_table_kv_t *key_value = (hash_table_kv_t *)curr_node->data;

			// add the key value pair to the temporary hash table
			set_key_value(&temp_hash_table, key_value->key, key_value->value);

			// increment the node
			curr_node = curr_node->next;
		}

		// now that we have copied all the keys from the bucket, free it
		linked_list_free(&(hash_table->buckets[i]));
	}

	// free the buckets array now that we have copied over all key-value pairs
	free(hash_table->buckets);

	// copy the key value pairs and new length back to the original hash table
	hash_table->buckets = temp_hash_table.buckets;
	hash_table->bucket_count = temp_hash_table.bucket_count;
}

int hash_table_init(hash_table_t *hash_table) {
	// initialize the hash table with the default size
	return init_with_size(hash_table, INITIAL_SIZE);
}

int hash_table_count_kv(hash_table_t *hash_table) {
	// total number of key-value pairs in our hash table
	int key_value_count = 0;

	// sum up all the key value pairs
	for (int i = 0; i < hash_table->bucket_count; i++) {
		key_value_count += hash_table->buckets[i].length;
	}

	return key_value_count;
}

void *hash_table_get(hash_table_t *hash_table, char *key) {
	// get the index to the bucket from the key
	unsigned long long index = crc64(key) % hash_table->bucket_count;

	// linked list node used to iterate through the list
	linked_list_node_t *curr_node = hash_table->buckets[index].sent;

	// iterate through the list
	while (curr_node != NULL) {

		// get the key value pair
		hash_table_kv_t *key_value = (hash_table_kv_t *)curr_node->data;

		// if we found a matching key, return the cooresponding value
		if (strcmp(key_value->key, key) == 0) {
			return key_value->value;
		}

		// don't forget to increment to the next node
		curr_node = curr_node->next;
	}

	// nothing was found
	return NULL;
}

void *hash_table_get_or_set(hash_table_t *hash_table, char *key, void *value) {
	// try to get the value associated with the key
	void *curr_value = hash_table_get(hash_table, key);

	// if there is already a key value pair, return the key's value
	if (curr_value) return curr_value;

	// before we add another key value pair, check if we have to rehash the table
	rehash_check(hash_table);

	// add the key value pair to the hash table
	set_key_value(hash_table, key, value);

	return NULL;
}

hash_table_kv_t *hash_table_to_array(hash_table_t *hash_table) {
	int key_value_count = hash_table_count_kv(hash_table);

	hash_table_kv_t *final_array =
	    (hash_table_kv_t *) malloc(sizeof(hash_table_kv_t) * key_value_count);

	// return out of the function if we fail to allocate
	if (final_array == NULL) return NULL;

	// keep track of where we are in the array
	int array_index = 0;

	// iterate over every key-value pair of the hash_table
	for (int i = 0; i < hash_table->bucket_count; i++) {
		// linked list node used to iterate through the list
		linked_list_node_t *curr_node = hash_table->buckets[i].sent;

		while (curr_node != NULL) {

			// set the key at the current index
			final_array[array_index].key = 
				((hash_table_kv_t *)curr_node->data)->key;

			// set the value at the current index
			final_array[array_index].value = 
				((hash_table_kv_t *)curr_node->data)->value;

			// increment the node
			curr_node = curr_node->next;

			// don't forget to increment the array index
			array_index++;
		}
	}

	return final_array;
}

void hash_table_free(hash_table_t *hash_table) {
	// free each linked list
	for (int i = 0; i < hash_table->bucket_count; ++i) {
		// get a refference to the list
		linked_list_t *list = &hash_table->buckets[i];

		// used for iterating over each node
		linked_list_node_t *curr_node = list->sent;

		// loop over the linked list
		while (curr_node != NULL) {

			// get the key value pair
			hash_table_kv_t *key_value = (hash_table_kv_t *)curr_node->data;

			// free the key and value (assumes they were allocated on heap)
			free(key_value->key);
			free(key_value->value);

			// increment to the next node
			curr_node = curr_node->next;
		}

		// free the rest of the list
		linked_list_free(list);
	}

	// finally, free the buckets array
	free(hash_table->buckets);
}