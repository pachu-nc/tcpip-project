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

#define IS_GLTHREAD_LIST_EMPTY(glthreadptr)         \
    ((glthreadptr)->next == 0 && (glthreadptr)->prev == 0)


#define GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthreadptr, offset)  \
    (void *)((char *)(glthreadptr) - offset)


typedef struct glthread_ {
        struct glthread_ *next;
        struct glthread_ *prev;
}glthread_t;

void insert_glnode(glthread_t *, glthread_t *);
void traverse_glnode(glthread_t **);
void initialize_glthread(glthread_t *);
void remove_glnode(glthread_t *);
node_t *return_glnode_pointer(glthread_t *);
#define offset(name_struct,field_name) \
	(unsigned int )&((name_struct *)0)->field_name

void
glthread_priority_insert(glthread_t *base_glthread,
                         glthread_t *glthread,
                         int (*comp_fn)(void *, void *),
                         int offset);

glthread_t *
dequeue_glthread_first(glthread_t *base_glthread);

#endif /*GLTHREAD_H*/
