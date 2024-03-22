#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <stddef.h>
//include "dll.h"
#include "glthread.h"
#include "net.h"
#include <assert.h>
#define MAX_NAME_SIZE		20
#define MAX_TOPOLOGY_NAME	32
#define MAX_INTF_PER_NODE	20

/*Forward Declaration */
typedef struct node_ node_t;
typedef struct link_ link_t;
typedef struct glthread_ glthread_t;

/*@brief
 *
 * @param: if_name -	Name of the interface
 * @param: att_node - 	It is the node to which this interface is attached to.
 * @param: line - 	Link to which the interface is connected to
 *
 * */

typedef struct interface_ {
	char if_name[MAX_NAME_SIZE];
	node_t *att_node; 		/*Attached node*/
	link_t *link; 			/*Link of which this interface is a part of*/
	intf_nw_props_t intf_nw_prop;
}interface_t;


/*@brief
 * Description: Link is nothing but a wire connecting two interfaces.
 *
 * @param: intf1	-	Name of the 1st interface.
 * @param: intf2 	- 	Name of the 2nd interface.
 * @param: cost		- 	Cost associated with this link.
 *
 * */

typedef struct link_ {
	interface_t intf1;
	interface_t intf2;
	unsigned int cost;
}link_t;


/*@brief
 *
 * @param: node_name		-	Name of the Graph Topology
 * @param: intf 		- 	List of Empty interfaces in this node. If a member element of a array is empty then the slot is empty.
 * 
 *
 * */

typedef struct node_ {
	char node_name[MAX_NAME_SIZE];
	interface_t *intf[30];
	glthread_t glnode;
	int udp_port_number; /*Unque udp port num that the node listens on*/
	int udp_sock_fd;	/*udp sock fd that is opened by the node*/
	node_nw_prop_t node_nw_prop;
}node_t;


/*@brief
 *
 * @param: topology_name	-	Name of the Graph Topology
 * @param: node_list 		- 	It is the Linked list of nodes.
 *
 * */

typedef struct graph_ {
	char topology_name[MAX_TOPOLOGY_NAME]; /*Name of the graph topology*/
	glthread_t node_list;	/*It is a linked list of nodes.*/
}graph_t;	



graph_t *build_graph_topo(void);
graph_t *build_linear_topo(void);
graph_t *build_simple_l2_switch_topo(void);
graph_t *build_dualswitch_topo(void);
graph_t *linear_3_node_topo(void);


node_t  *create_graph_node(graph_t *, char*);


node_t *create_graph_node(graph_t *topo, char *node_name);

graph_t *create_new_graph(char *graph_name);
/*Helper functions*/

static inline int
get_node_intf_available_slot(node_t *node){
    
    int i ;

    for( i = 0 ; i < MAX_INTF_PER_NODE; i++){
        if(node->intf[i])
            continue;
        return i;
    }
    return -1;
}


static inline node_t *
get_nbr_node(interface_t *interface){

    assert(interface->att_node);
    assert(interface->link);

    link_t *link = interface->link;
    if(&link->intf1 == interface)
        return link->intf2.att_node;
    else
        return link->intf1.att_node;
}


static inline interface_t *
get_node_if_by_name(node_t *node, char *if_name){
	int i;
	interface_t *intf;
	for(i = 0; i < MAX_INTF_PER_NODE; i++) {
		intf = node->intf[i];
		if(!intf) return NULL;
		if(strncmp(intf->if_name, if_name,MAX_NAME_SIZE) == 0){
			return intf;
		}
	}
	return NULL;
}

static inline node_t *
get_node_by_node_name(graph_t *topo, char *node_name){

    node_t *node;
    glthread_t *curr;

    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr){

        node = return_glnode_pointer(curr);
        if(strncmp(node->node_name, node_name, strlen(node_name)) == 0){
            return node;
	}
    } ITERATE_GLTHREAD_END(&topo->node_list, curr);
    return NULL;
}
#endif /*GRAPH_H__*/
