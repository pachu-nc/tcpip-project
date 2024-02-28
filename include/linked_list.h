#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H_



#define TRAVERSE_LINKED_LIST(head)							\
		node_t **curr; 								\
		curr = head;								\
		while(*curr) { printf("<%s %d %d %s> -> ", 				\
				(*curr)->data.emp_name,(*curr)->data.emp_num,		\
				(*curr)->data.emp_salary,(*curr)->data.emp_project); 	\
				curr = &(*curr)->next;					\
		}									\
		printf("NULL ")								\

			
typedef struct employee_ {
	//char *emp_name;
	char emp_name[20];
	char emp_project[20];
	int emp_num;
	int emp_salary;
}emp_t;

typedef struct node_ {
	emp_t data;
	struct node_ *next;
}node_t;


int insert(node_t **, emp_t *);
void traverse_ll(node_t **);
void search_ll(node_t **, int);

#define offset_field(name_struct,field_name) \
	(unsigned int) &((name_struct *)0)->field_name 

#endif /*__LINKED_LIST_H*/
