#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dll.h"
#include "error_codes.h"
#include "linked_list.h"

/**
 * @brief
 *
 * Description: Initialize a DLL
 *
 * @param node - Pointer to the Linked List 
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */

dll_t *initialize_dll(dll_t *dll) {
	dll_t *head = NULL;
	dll = (dll_t *) malloc(sizeof(dll_t));
	dll->next = NULL;
	dll->prev = NULL;
	head = dll;
	return head;
}


/**
 * @brief
 *
 * Description: Insert a new node at the end of LL
 *
 * @param data - Data of the structure
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */


dll_t * get_new_dll_node(node_t *data){
	
	int i;
	dll_t *new_node = (dll_t *) malloc (sizeof(dll_t));
	if(!new_node)
		return (dll_t *)FAILURE;
	new_node->data = (node_t *) malloc(sizeof(node_t)+1);
	strcpy(new_node->data->node_name, data->node_name);
	for(i = 0; i < MAX_INTF_PER_NODE; i++)
		new_node->data->intf[i] = data->intf[i];
	new_node->next = NULL;
	new_node->prev= NULL;
	return new_node;
}

/**
 * @brief
 *
 * Description: Insert a new node at the beginning of DLL
 *
 * @param head - head of the Linked list
 * @param data - Data of the structure
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */

#if 0
/*Insert a Node at the End*/
int dll_insert(dll_t **head, dll_emp_t *data){
	dll_t *new_node, **curr;
	curr = head;
	new_node = get_new_dll_node(data);
	if( *curr == NULL){
		*curr = new_node;
		return SUCCESS;
	}
	while((*curr)->next){
		curr = &(*curr)->next;
	}
	new_node->prev = (*curr);
	(*curr)->next = new_node;
	
	return SUCCESS;
}

#endif
//Insert a node at the beginning
#if 1
int dll_insert(dll_t **head, node_t *data){
	dll_t *new_node, **curr;
	curr = head;
	//printf("1.%s\n",__func__);
	new_node = get_new_dll_node(data);
	//printf("2.%s\n",__func__);
	if( *curr == NULL){
		*curr = new_node;
		return SUCCESS;
	}
	new_node->next = *curr;
	(*curr)->prev = new_node;
	*curr = new_node;
	return SUCCESS;
}
#endif

/**
 * @brief
 *
 * Description: Insert a new node at the beginning of DLL
 *
 * @param head - head of the Linked list
 * @param data - Data of the structure
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */


void traverse_dll(dll_t **head){
	dll_t **curr;
	curr = head;
	unsigned int i =0;
	interface_t *intf;
	/*while((*curr)->next){*/  // To Check if the prev member works properly
	while((*curr)->next){
		printf(" Node Name = %s\n ",(*curr)->data->node_name);
		for (i =0 ;i < MAX_INTF_PER_NODE; i++){
			intf = (*curr)->data->intf[i];
			if(!intf) break;
			dump_interface(intf);
		}
		curr = &(*curr)->next;
		printf("\n");
	}

//	printf("NULL\n");
//To check if the prev member of the struct works properly
/*
	printf("NULL\n");
	printf("Next = %p Current = %p Prev = %p\n",(*curr)->next, (*curr), (*curr)->prev);
	printf("\nPrev\n");
	temp = (*curr)->prev;
	printf("\n%p Previous\n",temp);
	printf("%s\n",(temp)->data.emp_name);
*/	
}








