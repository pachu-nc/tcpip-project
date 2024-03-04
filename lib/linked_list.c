#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"
#include "error_codes.h"


ll_node_t * get_new_node(emp_t *data){
	ll_node_t *new_node = (ll_node_t *) malloc (sizeof(ll_node_t));
	//printf("\n %s %d %d %s \n",data->emp_name,data->emp_num,data->emp_salary,data->emp_project);
	//printf("\nData = %p\n",data);
	if(!new_node)
		return (ll_node_t *)FAILURE;
	//new_node->data.emp_name = (emp_t *) malloc(sizeof(data->emp_name)+1);
	strcpy(new_node->data.emp_name, data->emp_name);
	new_node->data.emp_num = data->emp_num;
	new_node->data.emp_salary = data->emp_salary;
	strcpy(new_node->data.emp_project, data->emp_project);
	new_node->next = NULL;
	return new_node;
}
//Using Single Pointer
#if 0
int insert(node_t *root, int data){
	node_t *new_node, *temp;
	new_node = get_new_node(data);
	if( root == NULL){
		printf("\nroot is null\n");
		root = new_node;
		return SUCCESS;
	}
	
	else {
		//printf("\nin else condition\n");
		temp = root;
		while( temp->next != NULL) {
		//	printf("\nTraverse till the last node temp->data \n");
			temp = temp->next;
		}
		temp->next = new_node;
	}
	return SUCCESS;
}
#endif
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


//Using Double pointers
#if 1
int insert(ll_node_t **root, emp_t *data) {
//	printf("%s\n",__func__);	
	ll_node_t *new_node, **curr;
	curr = root;

//	printf("\n %s %d %d %s \n",data->emp_name,data->emp_num,data->emp_salary,data->emp_project);
//	printf("\nData1 = %p\n",data);
	new_node = get_new_node(data);

	//printf("\nAddress of new_node = %p\n",new_node);	
// Note: Insert at the end
#if 10
	if( *curr == NULL){
		*curr= new_node;
		return SUCCESS;
	}
	while((*curr)){
		//printf("\ncurr = %p\n",curr);
		//printf("\n*(curr)->next = %p\n",(*curr)->next);
		//printf("\n&(*curr)->next = %p\n",&(*curr)->next);
		curr = &(*curr)->next;
	}
	new_node->next = *curr;
	*curr = new_node;
	return SUCCESS;
#endif

// Insert at the beginning
#if 0	
	if( *curr == NULL){
		*curr= new_node;
		return SUCCESS;
	}
	else{
		
		new_node->next = (*curr);	
	}	*curr = new_node;

/*Adjust Pointers*/
	return SUCCESS;
#endif
// Note: Insert at the end
#if 0
	if( *root == NULL){
		*root= new_node;
		return SUCCESS;
	}
	
	curr = root;
	
	while((*curr)->next != NULL){
		*curr = (*curr)->next;
	}
	(*curr)->next = new_node;

	return SUCCESS;
#endif
}

#endif

/**
 * @brief
 *
 * Description: Traversing the Linked list
 *
 * @param root - head of the Linked list
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */

void traverse_ll (ll_node_t **root){
	ll_node_t **curr;
	curr = root;
///	printf("\nTraversing\n");
	while(*curr){
		printf("<%s %d %d %s> -> ",(*curr)->data.emp_name,(*curr)->data.emp_num,(*curr)->data.emp_salary,(*curr)->data.emp_project);
		curr = &(*curr)->next;
	}
	printf("NULL\n");
}

/**
 * @brief
 *
 * Description: Searching the Linked list
 *
 * @param root - head of the Linked list
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */

void search_ll(ll_node_t **root, int data){
	ll_node_t **curr;
	curr = root;
	printf("\nSearching...\n");
	while(*curr){
		if((*curr)->data.emp_num == data){
			printf("\nEmployee with empid %d Found\n",data);
			return SUCCESS;
		}
		curr = &(*curr)->next;
	}

	printf("\nEmployee with empid %d not Found\n",data);
}
