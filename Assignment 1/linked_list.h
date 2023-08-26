#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/*
represents each node of data in the linked list, nodes are independent of data
type (user must provide void pointer to data)
*/
typedef struct {
	void *data;
	linked_list_node_t *next;
} linked_list_node_t;

/*
list type the user will be dealing with, holds the sentinal node and other 
relavent information about the list. The user should only change the list
struct through this libraries functions. The user may read from the list
to get the length or access to the sentinal node.
*/
typedef struct {
	linked_list_node_t *sent;
	int length;
} linked_list_t;

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
	gets the data pointer at a given index

Params
	list: the list to get the data from
	index: what node to get the data from (starting from 0)

Return
	the void data pointer
*/
void *linked_list_get_data(linked_list_t *list, int index);

/*
Description
	adds a node with the specified data to the end of the list

Params
	list: the list to add the data to
	data: a void pointer to the data the user wants to add

Return
	On success, returns 0 on success and 1 on failure to allocate space for 
	the data
*/
int linked_list_append(linked_list_t *list, void *data);

/*
Description
	runs the specified function for every node in the list

Params
	list: the list to run the provided function on
	function: the function to run for every node in the list, recieves the void 
		data pointer from each node as an argument

Return
	nothing
*/
void linked_list_map(linked_list_t *list, (void)(*function)(void *));

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