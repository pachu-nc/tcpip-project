#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_codes.h"
#include "graph.h"
#include "glthread.h"

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

void initialize_glthread(glthread_t *glnode) {
	glnode->next = NULL;
	glnode->prev = NULL;
}

/**
 * @brief
 *
 * Description: Insert a new node at the beginning of DLL
 *
 * @param base_node - head of the Linked list
 * @param glnode - pointer to the glnode
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */

void insert_glnode(glthread_t *base_node, glthread_t *glnode){
        glthread_t *curr, *temp;
        curr = base_node;
        
	initialize_glthread(glnode);
        if(base_node->next == NULL) {
        //if(curr->next == NULL) {
                curr->next = glnode;
                glnode->prev = curr;
                return;
        }

	temp = curr->next;
        curr->next = glnode;
        glnode->prev = curr;
        glnode->next = temp;
        temp->prev = glnode;
}

node_t * return_glnode_pointer (glthread_t *curr ){
	/*returns the pointer to the starting address of the node*/
        return  (node_t *)( (char *)curr - (char *)offset(node_t,glnode));
}
void traverse_glnode(glthread_t **head){
    printf("\n%s %d\n",__func__,__LINE__);
    glthread_t **curr;
	interface_t *intf;
	unsigned int i;
        curr = head;
        node_t *node;
        while(*curr) {
                node = return_glnode_pointer(*curr);
		dump_node_nw_props(node);
		for (i =0 ;i < MAX_INTF_PER_NODE; i++){
			intf = node->intf[i];
			if(!intf) break;
			//dump_interface(intf);
			dump_intf_props(intf);
		}
                curr = &(*curr)->next;
		printf("\n");
        }
}
