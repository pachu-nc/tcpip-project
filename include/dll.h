#ifndef __DLL_H__
#define __DLL_H__

#include "graph.h"
//#include <stdio.h>
typedef struct node_ node_t;
typedef struct dll_node_ dll_t;

#if 1
#define TRAVERSE_DLL(head)							\
		dll_t **curr; 								\
		curr = head;								\
		while(*curr) { printf("<%s %d %d %s> -> ", 				\
				(*curr)->data.emp_name,(*curr)->data.emp_num,		\
				(*curr)->data.emp_salary,(*curr)->data.emp_project); 	\
				curr = &(*curr)->next;					\
		}									\
		printf("NULL \n")								\

#endif			
#if 0
typedef struct dll_employee_ {
	//char *emp_name;
	char emp_name[20];
	char emp_project[20];
	int emp_num;
	int emp_salary;
	}dll_emp_t;
#endif
typedef struct dll_node_ {
	node_t *data;
	struct dll_node_ *next;
	struct dll_node_ *prev;
}dll_t;


int dll_insert(dll_t **, node_t *);
void traverse_dll(dll_t **);
void search_dll(dll_t **, int);
dll_t *initialize_dll(dll_t *);
#define offset(name_struct,field_name) \
	(unsigned int )&((name_struct *)0)->field_name

#endif /*DLL_H*/
