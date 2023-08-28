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
#define INITIAL_SIZE 24

// rehashes the whole hash_table to be GROWTH_FACTOR times the size
static int rehash_table(hash_table_t *hash_table) {

}

int hash_table_init(hash_table_t *hash_table) {
	// initialize the array of linked lists
	hash_table->buckets = (linked_list_t *) malloc(sizeof(linked_list_t) * INITIAL_SIZE);

	// if initialization failed, return error code 1
	if(hash_table->buckets == NULL) return 1;

	// initialize the starting bucket count
	hash_table->bucket_count = INITIAL_SIZE;

	// initialize each linked list
	for (int i = 0; i < hash_table->bucket_count; i++) {
		linked_list_init(&(hash_table->buckets[i]));
	}

	return 0;
}

void *hash_table_get(hash_table_t *hash_table, char *key) {
	// get the index to the bucket from the key
	unsigned long long index = crc64(key) % hash_table->bucket_count;

	// linked list node used to iterate through the list
	linked_list_node_t *curr_node = hash_table->buckets[index].sent;

	// iterate through the list
	while(curr_node != NULL) {

		// get the key value pair
		hash_table_kv_t *key_value = (hash_table_kv_t *)curr_node->data;

		// if we found a matching key, return the cooresponding value
		if(strcmp(key_value->key, key) == 0) {
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
	if(curr_value) return curr_value;

	// the key value pair isn't already in the hash table, so we need to create one
	hash_table_kv_t *key_value = (hash_table_kv_t *) malloc(sizeof(hash_table_kv_t));

	// set the key and value
	key_value->key = key;
	key_value->value = value;

	// get the index to the bucket from the key
	unsigned long long index = crc64(key) % hash_table->bucket_count;

	// add it to the list
	int add_result = linked_list_add(&hash_table->buckets[index], (void *)key_value);

	// handle error here (ask prof)

	return NULL;
}

void hash_table_free(hash_table_t *hash_table) {
	// free each linked list
	for (int i = 0; i < hash_table->bucket_count; ++i) {
		// get a refference to the list
		linked_list_t *list = &hash_table->buckets[i];

		// used for iterating over each node
		linked_list_node_t *curr_node = list->sent;

		// loop over the linked list
		while(curr_node != NULL) {

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