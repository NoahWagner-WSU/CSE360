/*
NAME:
DATE:
ClASS:
ASSIGNMENT:
DESC:
*/

#include <stdio.h>
#include "program.h"

typedef struct { int x, y; } point_t;

int main(int argc, char **argv) {
	// allocate an array of file pointers to be the size of sizeof(FILE *) * (argc - 1) <- if count is specified, else just argc
	// create the main hash_table
	// for every file pointer...
		// run the count word pairs function
	// print output
	// return success exit code

	linked_list_t list;

	linked_list_init(&list);

	for (int i = 0; i < 100; ++i) {
		if(i < 50) {
			int *data = malloc(sizeof(int));
			linked_list_add(&list, data);
		} else if( i < 75) {
			double *data = malloc(sizeof(double));
			linked_list_add(&list, data);
		} else {
			point_t *data = malloc(sizeof(point_t));
			linked_list_add(&list, data);
		}
	}

	linked_list_free(&list);
}