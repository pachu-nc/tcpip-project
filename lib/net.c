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
	/*Add it as direct route in routing table*/
    	rt_table_add_direct_route(NODE_RT_TABLE(node), ip_addr, 32);     

	return TRUE;
}
bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask) {
	
	interface_t *interface = get_node_if_by_name(node, local_if);
	
	if(!interface) assert(0);

//	strcpy(IF_IP(interface),ip_addr);
	strncpy(IF_IP(interface),ip_addr,16);
	IF_IP(interface)[15] = '\0';
	interface->intf_nw_prop.mask = mask;
	interface->intf_nw_prop.is_ipadd_config = TRUE;
    	rt_table_add_direct_route(NODE_RT_TABLE(node), ip_addr, mask);
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
    
    else{
         printf("\t l2 mode = %s", intf_l2_mode_str(IF_L2_MODE(interface)));
         printf("\t vlan membership : ");
         int i = 0;
         for(; i < MAX_VLAN_MEMBERSHIP; i++){
            if(interface->intf_nw_prop.vlans[i]){
                printf("%u  ", interface->intf_nw_prop.vlans[i]);
            }
         }
         printf("\n");
    }
   
}


/*Returns the local interface of the node which is configured
 * with subnet in which 'ip_addr' lies
 * */
interface_t *
node_get_matching_subnet_interface(node_t *node, char *ip_addr){

    unsigned int i = 0;
    interface_t *intf;

    char *intf_addr = NULL;
    char mask;
    char intf_subnet[16];
    char subnet2[16];

    for(i = 0 ; i < MAX_INTF_PER_NODE; i++){

        intf = node->intf[i];

        if(!intf) break;
    
        if(intf->intf_nw_prop.is_ipadd_config == FALSE)
            continue;

        intf_addr = IF_IP(intf);
        mask = intf->intf_nw_prop.mask;

        memset(intf_subnet, 0 , 16);
        memset(subnet2, 0 , 16);
        apply_mask(intf_addr, mask, intf_subnet);
        apply_mask(ip_addr, mask, subnet2);
        if(strncmp(intf_subnet, subnet2, 16) == 0){
            return intf;
        }
    }
}

void dump_nw_graph(graph_t *graph){
    node_t *node;
    interface_t *interface;
    glthread_t *curr;
    unsigned int i;

    printf("Topology Name = %s\n", graph->topology_name);
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



/*When pkt moves from top to down in TCP/IP stack, we would need
  room in the pkt buffer to attach more new headers. Below function
  simply shifts the pkt content present in the start of the pkt buffer
  towards right so that new room is created*/
char *
pkt_buffer_shift_right(char *pkt, unsigned int pkt_size,
                       unsigned int total_buffer_size){
	
    char *temp = NULL;
    bool_t need_temp_memory = FALSE;

    if(pkt_size * 2 > total_buffer_size){
        need_temp_memory = TRUE;
    }
    
    if(need_temp_memory){
        temp = calloc(1, pkt_size);
        memcpy(temp, pkt, pkt_size);
        memset(pkt, 0, total_buffer_size);
        memcpy(pkt + (total_buffer_size - pkt_size), temp, pkt_size);
        free(temp);
        return pkt + (total_buffer_size - pkt_size);
    }
    
    memcpy(pkt + (total_buffer_size - pkt_size), pkt, pkt_size);
    memset(pkt, 0, pkt_size);
    return pkt + (total_buffer_size - pkt_size);

}

void
dump_interface_stats(interface_t *interface){

    printf("%s   ::  PktTx : %u, PktRx : %u",
        interface->if_name, interface->intf_nw_prop.pkt_sent,
        interface->intf_nw_prop.pkt_recv);
}

void
dump_node_interface_stats(node_t *node){

    interface_t *interface;

    uint32_t i = 0;

    for(; i < MAX_INTF_PER_NODE; i++){
        interface = node->intf[i];
        if(!interface)
            return;
        dump_interface_stats(interface);
        printf("\n");
    }
}

/*Helper functions*/

bool_t
is_same_subnet(char *ip_addr, char mask,
               char *other_ip_addr){

    char intf_subnet[16];
    char subnet2[16];

    memset(intf_subnet, 0 , 16);
    memset(subnet2, 0 , 16);

    apply_mask(ip_addr, mask, intf_subnet);
    apply_mask(other_ip_addr, mask, subnet2);

    if(strncmp(intf_subnet, subnet2, 16) == 0){
        return TRUE;
    }
    return FALSE;

}


/*R1-eth0 is said to be bidirectional only if
*   1. R1-eth0 and neighbour R2-eth1 are up &&
*   2. R1-eth0 and R2-eth1 and both configured with IP address &&
*   3. IP addersses are in the same subnet
*   4. None of the interfaces are operating in the L2 mode(ACCESS/TRUNK)
*/
bool_t
is_interface_l3_bidirectional(interface_t *interface){
 
    /*if interface is in L2 mode*/
    if(IF_L2_MODE(interface) == ACCESS ||
        IF_L2_MODE(interface) == TRUNK)
        return FALSE;

    /* If interface is not configured
     * with IP address*/
    if(!IS_INTF_L3_MODE(interface))
        return FALSE;

    interface_t *other_interface = &interface->link->intf1 == interface ?    \
            &interface->link->intf2 : &interface->link->intf1;

    if(!other_interface)
        return FALSE;

    if(!IF_IS_UP(interface) ||
            !IF_IS_UP(other_interface)){
        return FALSE;
    }

    if(IF_L2_MODE(other_interface) == ACCESS ||
        IF_L2_MODE(interface) == TRUNK)
        return FALSE;

    if(!IS_INTF_L3_MODE(other_interface))
        return FALSE;

    if(!(is_same_subnet(IF_IP(interface), IF_MASK(interface),
        IF_IP(other_interface)) &&
        is_same_subnet(IF_IP(other_interface), IF_MASK(other_interface),
        IF_IP(interface)))){
        return FALSE;
    }

    return TRUE;

}