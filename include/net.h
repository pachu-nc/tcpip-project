#ifndef __NET_H__
#define __NET_H__

#include <utils.h>
#include <graph.h>

typedef struct node_ node_t;
typedef struct graph_ graph_t;
typedef struct interface_ interface_t;
typedef struct arp_table_ arp_table_t;


typedef struct ip_add_ {
	unsigned char ip_add[16];
}ip_add_t;



typedef struct mac_add_ {
	unsigned char mac[6];
}mac_add_t;


typedef struct node_nw_prop_{
	/* Used to find various device types capabilities of
     	* the node and other features*/
    	unsigned int flags;
	/*L2 Properties*/

	arp_table_t *arp_table;

	/*L3 Properties*/
	bool_t is_lb_addr_config;
	ip_add_t lb_addr; /*Loopback address of the node*/
}node_nw_prop_t;	

extern void init_arp_table(arp_table_t **arp_table);
/*@brief
 *
 * @param: node_nw_prop_t -	pointer to the node network properties
 *
 * */

static inline void 
init_node_nw_prop(node_nw_prop_t *node_nw_prop ){
	node_nw_prop->flags = 0;
	node_nw_prop->is_lb_addr_config = FALSE;
	memset(node_nw_prop->lb_addr.ip_add, 0, 16);
	
	/*Init ARP table*/
	init_arp_table(&(node_nw_prop->arp_table));
       	
}

typedef struct intf_nw_props_ {
	/*L2 Properies*/
	mac_add_t mac_add;
	bool_t is_ipadd_config; /*set to TRUE of ip add is configured. intf operates in L3 mode if set.*/
	ip_add_t ip_addr;
	char mask;
}intf_nw_props_t;


/*@brief
 *
 * @param: node_nw_prop_t -	pointer to the node network properties
 *
 * */

static inline void 
init_intf_nw_prop(intf_nw_props_t *intf_nw_prop ){
	
	memset(intf_nw_prop->mac_add.mac , 0 ,
        sizeof(intf_nw_prop->mac_add.mac));
	intf_nw_prop->is_ipadd_config = FALSE;
	memset(intf_nw_prop->ip_addr.ip_add, 0, 16);
	intf_nw_prop->mask = 0;
       	
}

/*GET shorthand Macros*/

#define IF_MAC(intf_ptr)		((intf_ptr)->intf_nw_prop.mac_add.mac)
#define IF_IP(intf_ptr)			((intf_ptr)->intf_nw_prop.ip_addr.ip_add)

#define NODE_LO_ADDR(node_ptr)		(node_ptr->node_nw_prop.lb_addr.ip_add)
#define NODE_ARP_TABLE(node_ptr)    	(node_ptr->node_nw_prop.arp_table)
#define NODE_MAC_TABLE(node_ptr)    	(node_ptr->node_nw_prop.mac_table)


/*APIs to set Network Node properties*/

bool_t node_set_loopback_address(node_t *node, char *ip_addr);
bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask);
bool_t node_unset_intf_ip_address(node_t *node, char *local_if);

char *pkt_buffer_shift_right(char *pkt, unsigned int pkt_size,
		 unsigned int total_buffer_size);


/*Dumping Functions to dump network information
 * on nodes and interfaces*/
void dump_nw_graph(graph_t *graph);
void dump_node_nw_props(node_t *node);
void dump_intf_props(interface_t *interface);



/*Helper Routines*/
interface_t *
node_get_matching_subnet_interface(node_t *node, char *ip_addr);


#endif /*_NET_H_*/
