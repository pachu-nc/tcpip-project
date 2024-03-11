#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_codes.h"
#include "graph.h"
#include "glthread.h"



graph_t *build_graph_topo() {
	//printf("%s %d",__func__,__LINE__);
	graph_t *topo = create_new_graph("Generic Graph");
	printf("\nTOPO = %x\n",&topo->node_list);
	node_t *R0_re = create_graph_node(topo, "R0_re");
	node_t *R1_re = create_graph_node(topo, "R1_re");
	node_t *R2_re = create_graph_node(topo, "R2_re");

	insert_link_between_two_nodes(R0_re, R1_re, "eth0/0", "eth0/1", 1);
	usleep(100);
	insert_link_between_two_nodes(R1_re, R2_re, "eth0/2", "eth0/3", 1);
	usleep(100);
	insert_link_between_two_nodes(R0_re, R2_re, "eth0/4", "eth0/5", 1);
	usleep(100);

	node_set_loopback_address(R0_re, "122.1.1.0");
	usleep(100);
	node_set_intf_ip_address(R0_re, "eth0/4", "40.1.1.1", 24);
	usleep(100);
	node_set_intf_ip_address(R0_re, "eth0/0", "20.1.1.1", 24);
	usleep(100);
	
	node_set_loopback_address(R1_re, "122.1.1.1");
	usleep(100);
	node_set_intf_ip_address(R1_re, "eth0/1", "20.1.1.2", 24);
	usleep(100);
	node_set_intf_ip_address(R1_re, "eth0/2", "30.1.1.1", 24);
	usleep(100);
	
	node_set_loopback_address(R2_re, "122.1.1.2");
	usleep(100);
	node_set_intf_ip_address(R2_re, "eth0/3", "30.1.1.2", 24);
	usleep(100);
	node_set_intf_ip_address(R2_re, "eth0/5", "40.1.1.2", 24);
	usleep(100);


	return topo;
	//dump_graph(topo);
	//dump_nw_graph(topo);
}
