#ifndef __GLTHREAD_H__
#define __GLTHREAD_H__

#include "graph.h"

typedef struct node_ node_t;
//typedef struct dll_node_ dll_t;

#define BASE(glthreadptr)   ((glthreadptr)->next)
#define ITERATE_GLTHREAD_BEGIN(glthreadptrstart, glthreadptr)                                      \
{                                                                                                  \
    glthread_t *_glthread_ptr = NULL;                                                              \
    glthreadptr = BASE(glthreadptrstart);                                                          \
    for(; glthreadptr!= NULL; glthreadptr = _glthread_ptr){                                        \
        _glthread_ptr = (glthreadptr)->next;

#define ITERATE_GLTHREAD_END(glthreadptrstart, glthreadptr)                                        \
        }}

typedef struct glthread_ {
        struct glthread_ *next;
        struct glthread_ *prev;
}glthread_t;

void insert_glnode(glthread_t *, glthread_t *);
void traverse_glnode(glthread_t **);
void init_glthread(glthread_t *);
void remove_glthread(glthread_t *);
node_t *return_glnode_pointer(glthread_t *);
#define offset(name_struct,field_name) \
	(unsigned int )&((name_struct *)0)->field_name

#endif /*GLTHREAD_H*/
