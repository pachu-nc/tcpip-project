#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dll.h"
#include "error_codes.h"
#include "linked_list.h"


/**
 * @brief
 *
 * Description: Insert a new node at the end of LL
 *
 * @param root - head of the Linked list
 * @param data - Data of the structure
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */


dll_t * get_new_dll_node(dll_emp_t *data){
	dll_t *new_node = (dll_t *) malloc (sizeof(dll_t));
	//printf("\n %s %d %d %s \n",data->emp_name,data->emp_num,data->emp_salary,data->emp_project);
	//printf("\nData = %p\n",data);
	if(!new_node)
		return (dll_t *)FAILURE;
	//new_node->data.emp_name = (emp_t *) malloc(sizeof(data->emp_name)+1);
	strcpy(new_node->data.emp_name, data->emp_name);
	new_node->data.emp_num = data->emp_num;
	new_node->data.emp_salary = data->emp_salary;
	strcpy(new_node->data.emp_project, data->emp_project);
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
int dll_insert(dll_t **head, dll_emp_t *data){
	dll_t *new_node, **curr;
	curr = head;
	new_node = get_new_dll_node(data);
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
	dll_t **curr, *temp;
	curr = head;
	/*while((*curr)->next){*/  // To Check if the prev member works properly
	while(*curr){
		printf("<%s %d %d %s > -> ",(*curr)->data.emp_name,(*curr)->data.emp_num,(*curr)->data.emp_salary,(*curr)->data.emp_project);
		curr = &(*curr)->next;
	}

	printf("NULL\n");
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








