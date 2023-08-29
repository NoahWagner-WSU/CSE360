/*
NAME: Noah Wagner
DATE: 8/29/23
ClASS: CSE360
ASSIGNMENT: Assignment 1
DESC: A basic linked list implementation used by hash table functions.
*/

#include <stdlib.h>
#include "linked_list.h"

void linked_list_init(linked_list_t *list) {
	list->length = 0;
	list->sent = NULL;
}

int linked_list_add(linked_list_t *list, void *data) {
	// allocate space for new node
	linked_list_node_t *new_node =
	    (linked_list_node_t *) malloc(sizeof(linked_list_node_t));

	// if allocation failed, return error code 1
	if (new_node == NULL) return 1;

	// insert the new node to the beginning of the list
	new_node->next = list->sent;
	list->sent = new_node;

	// set the node data
	new_node->data = data;

	// add 1 to the lists length
	list->length++;

	return 0;
}

void linked_list_free(linked_list_t *list) {
	linked_list_node_t *prev_node = NULL;
	linked_list_node_t *curr_node = list->sent;

	// iterate over every node
	while (curr_node != NULL) {
		// increment both the previous and current node
		prev_node = curr_node;
		curr_node = curr_node->next;

		// free the previous node
		free(prev_node->data);
		free(prev_node);
	}
}