/*
NAME:
DATE:
ClASS:
ASSIGNMENT:
DESC:
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

	int *value1 = malloc(sizeof(int));
	*value1 = 21;
	point_t *value2 = malloc(sizeof(point_t));
	value2->x = 1;
	value2->y = 2;
	int *value3 = malloc(sizeof(int));
	*value3 = 1;
	int *value4 = malloc(sizeof(int));
	*value4 = 123;
	point_t *value5 = malloc(sizeof(point_t));
	value5->x = 3;
	value5->y = 4;
	char *value6 = malloc(sizeof(char));
	*value6 = 'a';

	char *key1 = malloc(sizeof(char) + 1);
	char *key2 = malloc(sizeof(char) * 2 + 1);
	char *key3 = malloc(sizeof(char) * 3 + 1);
	char *key4 = malloc(sizeof(char) * 4 + 1);
	char *key5 = malloc(sizeof(char) * 5 + 1);
	char *key6 = malloc(sizeof(char) * 6 + 1);

	strcpy(key1, "a");
	strcpy(key2, "ba");
	strcpy(key3, "cat");
	strcpy(key4, "bald");
	strcpy(key5, "crazy");
	strcpy(key6, "no way");

	hash_table_get_or_set(&hash_table, key1, (void *) value1);
	hash_table_get_or_set(&hash_table, key2, (void *) value2);
	hash_table_get_or_set(&hash_table, key3, (void *) value3);
	hash_table_get_or_set(&hash_table, key4, (void *) value4);
	hash_table_get_or_set(&hash_table, key5, (void *) value5);
	hash_table_get_or_set(&hash_table, key6, (void *) value6);

	int *test1 = (int *)hash_table_get_or_set(&hash_table, key1, value6);

	printf("key1 has value: %d\n", *test1);

	printf("key5 has value: x=%d\n", ((point_t *)hash_table_get(&hash_table, "crazy"))->x);

	printf("keyx has value: %p\n", hash_table_get(&hash_table, "asdf lsjd fk!"));

	printf("final hash table length: %d\n", hash_table.bucket_count);

	hash_table_free(&hash_table);

	return 0;
}