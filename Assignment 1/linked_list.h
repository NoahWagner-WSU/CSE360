#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/*
represents each node of data in the linked list, nodes are independent of data
type (user must provide void pointer to data)
*/
typedef struct linked_list_node linked_list_node_t;

struct linked_list_node {
	void *data;
	linked_list_node_t *next;
};

/*
list type the user will be dealing with, holds the sentinal node and other 
relavent information about the list. The user should only change the list
struct through this libraries functions. The user may read from the list
to get its length.
*/
typedef struct linked_list linked_list_t;

struct linked_list {
	linked_list_node_t *sent;
	int length;
};

/*
Description
	initializes a list struct to have a length of 0 and sent = NULL

Params
	list: the list to initialize

Return
	nothing
*/
void linked_list_init(linked_list_t *list);

/*
Description
	adds a node with the specified data to the beginning of the list

Params
	list: the list to add the data to
	data: a void pointer to the data the user wants to add

Return
	returns 0 on success and 1 on failure to allocate space for the data
*/
int linked_list_add(linked_list_t *list, void *data);

/*
Description
	frees all memory associated with the linked list (including node data)

Params
	list: the list to free from memory

Return
	nothing
*/
void linked_list_free(linked_list_t *list);

#endif