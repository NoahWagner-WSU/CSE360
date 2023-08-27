/*
NAME:
DATE:
ClASS:
ASSIGNMENT:
DESC:
*/

#include "hash_table.h"
#include "linked_list.h"

#define GROWTH_FACTOR 3
#define INITIAL_SIZE 24

// not sure what "Growth of the table will be transparent to the code using your hash table module." means, ask prof
typedef struct kv_pair kv_pair_t;

struct kv_pair {
	char *key;
	void *value;
};

// rehashes the whole hash_table to be GROWTH_FACTOR times the size
static int rehash_table(hash_table_t *hash_table) {

}