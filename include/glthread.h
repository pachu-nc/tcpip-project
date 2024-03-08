#ifndef __GLTHREAD_H__
#define __GLTHREAD_H__

#include "graph.h"

typedef struct node_ node_t;
//typedef struct dll_node_ dll_t;



typedef struct glthread_ {
        struct glthread_ *next;
        struct glthread_ *prev;
}glthread_t;

void insert_glnode(glthread_t *, glthread_t *);
void traverse_glnode(glthread_t **);
void init_glthread(glthread_t *);

#define offset(name_struct,field_name) \
	(unsigned int )&((name_struct *)0)->field_name

#endif /*DLL_H*/
