#ifndef __NET_H__
#define __NET_H__

#include <utils.h>
#include <graph.h>

typedef struct node_ node_t;
typedef struct graph_ graph_t;
typedef struct interface_ interface_t;
typedef struct arp_table_ arp_table_t;
typedef struct mac_table_ mac_table_t;
typedef struct rt_table_ rt_table_t;


typedef struct ip_add_ {
	unsigned char ip_add[16];
}ip_add_t;



/*Interface Change Flags, used for Notification to
 * Applications*/
#define IF_UP_DOWN_CHANGE_F         (0)
#define IF_IP_ADDR_CHANGE_F         (1)
#define IF_OPER_MODE_CHANGE_F       (1 << 1)
#define IF_VLAN_MEMBERSHIP_CHANGE_F (1 << 2)
#define IF_METRIC_CHANGE_F          (1 << 3)



typedef enum {
	ACCESS,
	TRUNK,
	L2_MODE_UNKNOWN
}intf_l2_mode_t;

static inline char *
intf_l2_mode_str(intf_l2_mode_t intf_l2_mode){
	switch(intf_l2_mode){
		case ACCESS: 
			return "access";
			break;
		case TRUNK:
			return "trunk";
			break;
		default:
			return "L2_MODE_UNKNOWN";
	}
}


typedef struct mac_add_ {
	unsigned char mac[6];
}mac_add_t;


typedef struct node_nw_prop_{
	/* Used to find various device types capabilities of
     	* the node and other features*/
    	unsigned int flags;
	/*L2 Properties*/
	arp_table_t *arp_table;
	mac_table_t *mac_table;

	/*L3 Properties*/
	rt_table_t *rt_table;
	bool_t is_lb_addr_config;
	ip_add_t lb_addr; /*Loopback address of the node*/
}node_nw_prop_t;	

extern void init_arp_table(arp_table_t **arp_table);
extern void init_mac_table(mac_table_t **mac_table);
extern void init_rt_table(rt_table_t **rt_table);
#define MAX_VLAN_MEMBERSHIP 10

/*Should be Called only for interface operating in Access mode*/
unsigned int
get_access_intf_operating_vlan_id(interface_t *interface);

/*Should be Called only for interface operating in Trunk mode*/
bool_t
get_access_intf_operating_vlan_id(interface_t *interface);
/*Should be Called only for interface operating in Trunk mode*/

bool_t
is_trunk_interface_vlan_enabled(interface_t *interface, unsigned int vlan_id);



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
	/*Init MAC table*/
	init_mac_table(&(node_nw_prop->mac_table));
       	/*Init Routing table*/
	init_rt_table(&(node_nw_prop->rt_table));

}

typedef struct intf_nw_props_ {
	
	/*L1 Properties*/
	bool_t is_up;
	
	/*L2 Properies*/
	mac_add_t mac_add;
	intf_l2_mode_t intf_l2_mode;
	unsigned int vlans[MAX_VLAN_MEMBERSHIP];
	bool_t is_ipadd_config_backup;
	
	/*L3 Properties*/
	bool_t is_ipadd_config; /*set to TRUE of ip add is configured. intf operates in L3 mode if set.*/
	ip_add_t ip_addr;
	char mask;

	/*Interface Statistics*/
	/*Interface Statistics*/
    uint32_t pkt_recv;
    uint32_t pkt_sent;

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
	intf_nw_prop->intf_l2_mode = L2_MODE_UNKNOWN;
    	memset(intf_nw_prop->vlans, 0, sizeof(intf_nw_prop->vlans));

	intf_nw_prop->is_ipadd_config = FALSE;
	intf_nw_prop->is_up = TRUE;
	intf_nw_prop->pkt_recv = 0;
	intf_nw_prop->pkt_sent = 0;
	memset(intf_nw_prop->ip_addr.ip_add, 0, 16);
	intf_nw_prop->mask = 0;
       	
}

/*GET shorthand Macros*/

#define IF_MAC(intf_ptr)				((intf_ptr)->intf_nw_prop.mac_add.mac)
#define IF_IP(intf_ptr)					((intf_ptr)->intf_nw_prop.ip_addr.ip_add)

#define NODE_LO_ADDR(node_ptr)			(node_ptr->node_nw_prop.lb_addr.ip_add)
#define NODE_ARP_TABLE(node_ptr)    	(node_ptr->node_nw_prop.arp_table)
#define NODE_MAC_TABLE(node_ptr)    	(node_ptr->node_nw_prop.mac_table)
#define NODE_RT_TABLE(node_ptr)     	(node_ptr->node_nw_prop.rt_table)


#define IF_L2_MODE(intf_ptr)    		(intf_ptr->intf_nw_prop.intf_l2_mode)
#define IS_INTF_L3_MODE(intf_ptr)   	(intf_ptr->intf_nw_prop.is_ipadd_config == TRUE)
#define IF_IS_UP(intf_ptr)				(intf_ptr->intf_nw_prop.is_up == TRUE)
#define IF_MASK(intf_ptr)  				((intf_ptr)->intf_nw_prop.mask)

/*Macros to Iterate over Nbrs of a node*/

#define ITERATE_NODE_NBRS_BEGIN(node_ptr, nbr_ptr, oif_ptr, ip_addr_ptr) \
    do{                                                                  \
        int i = 0 ;                                                      \
        interface_t *other_intf;                                         \
        for( i = 0 ; i < MAX_INTF_PER_NODE; i++){                        \
            oif_ptr = node_ptr->intf[i];                                 \
            if(!oif_ptr) continue;                                       \
            other_intf = &oif_ptr->link->intf1 == oif_ptr ?              \
            &oif_ptr->link->intf2 : &oif_ptr->link->intf1;               \
            if(!other_intf) continue;                                    \
            nbr_ptr = get_nbr_node(oif_ptr);                             \
            ip_addr_ptr = IF_IP(other_intf);                             \

#define ITERATE_NODE_NBRS_END(node_ptr, nbr_ptr, oif_ptr, ip_addr_ptr)  }}while(0);


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
void dump_node_interface_stats(node_t *node);
void dump_interface_stats(interface_t *interface);



/*Helper Routines*/
interface_t *
node_get_matching_subnet_interface(node_t *node, char *ip_addr);

bool_t
is_interface_l3_bidirectional(interface_t *interface);

bool_t
is_same_subnet(char *ip_addr, char mask,
               char *other_ip_addr);

#endif /*_NET_H_*/
