#include "graph.h"
#include <memory.h>
#include <stdio.h>
#include "utils.h"
#include <assert.h>
#include <glthread.h>

/*Just some Random number generator*/
static unsigned int
hash_code(void *ptr, unsigned int size){
    unsigned int value=0, i =0;
    char *str = (char*)ptr;
    while(i < size)
    {
        value += *str;
        value*=97;
        str++;
        i++;
    }
    return value;
}


/*Heuristics, Assign a unique mac address to interface*/
void
interface_assign_mac_address(interface_t *interface){

    node_t *node = interface->att_node;

    if(!node)
        return;

    unsigned int hash_code_val = 0;
    hash_code_val = hash_code(node->node_name, MAX_NAME_SIZE);
    hash_code_val *= hash_code(interface->if_name, MAX_NAME_SIZE);
    memset(IF_MAC(interface), 0, sizeof(IF_MAC(interface)));
    memcpy(IF_MAC(interface), (char *)&hash_code_val, sizeof(unsigned int));
}


bool_t node_set_loopback_address(node_t *node, char *ip_addr) {
	
	node->node_nw_prop.is_lb_addr_config = TRUE;
	strcpy(NODE_LO_ADDR(node),ip_addr);
	strncpy(NODE_LO_ADDR(node), ip_addr, 16);
	NODE_LO_ADDR(node)[15] = '\0';
	return TRUE;
}
bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask) {
	
	interface_t *interface = get_node_if_by_name(node, local_if);
	
	if(!interface) assert(0);

//	strcpy(IF_IP(interface),ip_addr);
	strncpy(IF_IP(interface),ip_addr,sizeof(ip_add_t)+1);
	IF_IP(interface)[sizeof(ip_add_t)] = '\0';
	interface->intf_nw_prop.mask = TRUE;
	interface->intf_nw_prop.is_ipadd_config = TRUE;
	return TRUE;
}

bool_t node_unset_intf_ip_address(node_t *node, char *local_if){

    return TRUE;
}

void dump_node_nw_props(node_t *node){

    printf("\nNode Name = %s\n", node->node_name);
    //printf("\nNode Name = %s, udp_port_no = %u\n", node->node_name, node->udp_port_number);
    printf("\t node flags : %u ", node->node_nw_prop.flags);
    if(node->node_nw_prop.is_lb_addr_config){
        printf("\t lo addr : %s/32", NODE_LO_ADDR(node));
    }
    printf("\n");
}

void dump_intf_props(interface_t *interface){

    dump_interface(interface);

    if(interface->intf_nw_prop.is_ipadd_config){
        printf("\tIP Addr = %s/%u", IF_IP(interface), interface->intf_nw_prop.mask);
        printf("\tMAC : %x:%x:%x:%x:%x:%x\n",
                IF_MAC(interface)[0], IF_MAC(interface)[1],
                IF_MAC(interface)[2], IF_MAC(interface)[3],
                IF_MAC(interface)[4], IF_MAC(interface)[5]);
    }
    /*
    else{
         printf("\t l2 mode = %s", intf_l2_mode_str(IF_L2_MODE(interface)));
         printf("\t vlan membership : ");
         int i = 0;
         for(; i < MAX_VLAN_MEMBERSHIP; i++){
            if(interface->intf_nw_props.vlans[i]){
                printf("%u  ", interface->intf_nw_props.vlans[i]);
            }
         }
         printf("\n");
    }
    */
}

void dump_nw_graph(graph_t *graph){
    printf("\n%s %d\n",__func__,__LINE__);
    node_t *node;
    interface_t *interface;
    glthread_t *curr;
    unsigned int i;
    printf("\n%s %d graph = %x\n",__func__,__LINE__,graph);

    printf("Topology Name = %s\n", graph->topology_name);
    printf("\n%s %d\n",__func__,__LINE__);
    ITERATE_GLTHREAD_BEGIN(&graph->node_list, curr){
	node = return_glnode_pointer(curr);
	dump_node_nw_props(node);
	for( i = 0; i < MAX_INTF_PER_NODE; i++){
           	interface = node->intf[i];
            	if(!interface) break;
            	dump_intf_props(interface);
       	}
    } ITERATE_GLTHREAD_END(&graph->node_list, curr)

    //traverse_glnode(&graph->node_list);
}
