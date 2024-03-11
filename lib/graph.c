#include <assert.h>
#include "graph.h"
//#include "dll.h"
#include "glthread.h"
#include <stdlib.h>




extern void network_start_pkt_receiver_thread(graph_t *topo);
/**
 * @brief
 *
 * Description: Creates a new Graph 
 *
 * @param topology_name - Name of the Graph
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */

graph_t *
create_new_graph(char *topology_name) {
	graph_t *graph = (graph_t *) malloc(sizeof(graph_t));
	strcpy(graph->topology_name,topology_name);
	initialize_glthread(&graph->node_list);
	return graph;
}


/**
 * @brief
 *
 * Description: Creates a new Graph node
 *
 * @param graph - Pointer to the graph
 * @param node_name - Name of the new node to be created.
 * 
 *
 * @return
 *
 * @bug :(optional)
 * @warning :(optional)
 */

node_t *
create_graph_node(graph_t *graph, char *node_name){
	node_t *node = calloc(1, sizeof(node_t));
	strcpy(node->node_name, node_name);
	init_udp_socket(node);
	insert_glnode(&graph->node_list, &node->glnode);
	return node;
} 

void
insert_link_between_two_nodes(node_t *node1,
        node_t *node2,
        char *from_if_name,
        char *to_if_name,
        unsigned int cost){

	int empty_intf_slot;

	link_t *link = (link_t*) malloc(sizeof(link_t));

	strcpy(link->intf1.if_name, from_if_name);
	strcpy(link->intf2.if_name, to_if_name);
	
	/*Set Back pointer to the node. This will help in accessing the neighbour nodes on a given inetrface*/
	link->intf1.link = link;
	link->intf2.link = link;
	link->intf1.att_node = node1;
	link->intf2.att_node = node2;
	link->cost = cost;
	
	/*Plugin internface ends into Nodes*/
	
	empty_intf_slot = get_node_intf_available_slot(node1);
	node1->intf[empty_intf_slot] = &link->intf1;
	empty_intf_slot = get_node_intf_available_slot(node2);
	node2->intf[empty_intf_slot] = &link->intf2;

	init_intf_nw_prop(&link->intf1.intf_nw_prop);
    	init_intf_nw_prop(&link->intf2.intf_nw_prop);
    	
	/*Now Assign Random generated Mac address to the Interfaces*/
    	interface_assign_mac_address(&link->intf1);
    	interface_assign_mac_address(&link->intf2);
}

void dump_interface(interface_t *interface){
	link_t *link = interface->link;
	
	node_t *nbr_node = get_nbr_node(interface);
	printf("Interface Name = %s\n\tNbr Node %s, Local Node : %s, cost = %u\n",
            interface->if_name,
            nbr_node->node_name,
            interface->att_node->node_name,
            link->cost);
}
void dump_graph(graph_t *graph) {
	printf("\n======== Topology Name = %s =========\n\n",graph->topology_name);
	traverse_glnode(&graph->node_list);
}


graph_t *build_graph_topo() {
	graph_t *topo = create_new_graph("Generic Graph");
	
	node_t *R0_re = create_graph_node(topo, "R0_re");
	node_t *R1_re = create_graph_node(topo, "R1_re");
	node_t *R2_re = create_graph_node(topo, "R2_re");

	insert_link_between_two_nodes(R0_re, R1_re, "eth0/0", "eth0/1", 1);
	insert_link_between_two_nodes(R1_re, R2_re, "eth0/2", "eth0/3", 1);
	insert_link_between_two_nodes(R0_re, R2_re, "eth0/4", "eth0/5", 1);

	node_set_loopback_address(R0_re, "122.1.1.0");
	node_set_intf_ip_address(R0_re, "eth0/4", "40.1.1.1", 24);
	node_set_intf_ip_address(R0_re, "eth0/0", "20.1.1.1", 24);
	
	node_set_loopback_address(R1_re, "122.1.1.1");
	node_set_intf_ip_address(R1_re, "eth0/1", "20.1.1.2", 24);
	node_set_intf_ip_address(R1_re, "eth0/2", "30.1.1.1", 24);
	
	node_set_loopback_address(R2_re, "122.1.1.2");
	node_set_intf_ip_address(R2_re, "eth0/3", "30.1.1.2", 24);
	node_set_intf_ip_address(R2_re, "eth0/5", "40.1.1.2", 24);
	
	network_start_pkt_receiver_thread(topo);
	return topo;
}
