//#include "glthread.h"
#include <stdio.h>
#include "graph.h"
#include <sys/socket.h>
#include <memory.h>
#include <stdlib.h>
#include "tcpconst.h"
#include "comm.h"
#include "net.h"
#include <arpa/inet.h> /*for inet_ntop & inet_pton*/
#include "layer3.h"


bool_t
l3_is_direct_route(l3_route_t *l3_route){
	return (l3_route->is_direct);
}


bool_t
is_layer3_local_delivery(node_t *node, unsigned int dst_ip){

    /* Check if dst_ip exact matches with any locally configured
     * ip address of the router*/

    /*checking with node's loopback address*/
    char dest_ip_str[16];
    dest_ip_str[15] = '\0';
    char *intf_addr = NULL;

    dst_ip = htonl(dst_ip);
    inet_ntop(AF_INET, &dst_ip, dest_ip_str, 16);

    if(strncmp(NODE_LO_ADDR(node), dest_ip_str, 16) == 0)
        return TRUE;

    /*checking with interface IP Addresses*/

    unsigned int i = 0;
    interface_t *intf;

    for( ; i < MAX_INTF_PER_NODE; i++){

        intf = node->intf[i];
        if(!intf) return FALSE;

        if(intf->intf_nw_prop.is_ipadd_config == FALSE)
            continue;

        intf_addr = IF_IP(intf);

        if(strncmp(intf_addr, dest_ip_str, 16) == 0)
            return TRUE;
    }
    return FALSE;
}


l3_route_t*
rt_glue_to_l3_route(glthread_t *curr){
	return (node_t *) ( (char *)curr -  (char *)(offset(l3_route_t, rt_glue)));
}
void
init_rt_table(rt_table_t **rt_table){
	*rt_table = calloc(1, sizeof(rt_table_t));
	initialize_glthread(&((*rt_table)->route_list));
}

l3_route_t *
rt_table_lookup(rt_table_t *rt_table, char *ip_addr, char mask){

    glthread_t *curr;
    l3_route_t *l3_route;

    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){

        l3_route = rt_glue_to_l3_route(curr);
        if(strncmp(l3_route->dest, ip_addr, 16) == 0 &&
                l3_route->mask == mask){
            return l3_route;
        }
    } ITERATE_GLTHREAD_END(&rt_table->route_list, curr);
}

void
delete_rt_table_entry(rt_table_t *rt_table,
        char *ip_addr, char mask){

    char dst_str_with_mask[16];

    apply_mask(ip_addr, mask, dst_str_with_mask);
    l3_route_t *l3_route = rt_table_lookup(rt_table, dst_str_with_mask, mask);

    if(!l3_route)
        return;

    remove_glnode(&l3_route->rt_glue);
    free(l3_route);
}

void
dump_rt_table(rt_table_t *rt_table){

    glthread_t *curr = NULL;
    l3_route_t *l3_route = NULL;

    printf("L3 Routing Table:\n");
    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){

        l3_route = rt_glue_to_l3_route(curr);
        printf("\t%-18s %-4d %-18s %s\n",
                l3_route->dest, l3_route->mask,
                l3_route->is_direct ? "NA" : l3_route->gw_ip,
                l3_route->is_direct ? "NA" : l3_route->oif);

    } ITERATE_GLTHREAD_END(&rt_table->route_list, curr);
}


static bool_t
_rt_table_entry_add(rt_table_t *rt_table, l3_route_t *l3_route){

    l3_route_t *l3_route_old = rt_table_lookup(rt_table,
            l3_route->dest, l3_route->mask);
	
    /*Don't do anything if the new route and the old route are the same.*/
    if(l3_route_old &&
            IS_L3_ROUTES_EQUAL(l3_route_old, l3_route)){

        return FALSE;
    }
    
    /*If the new route is different from the old route the delete the old route */

    if(l3_route_old){
        delete_rt_table_entry(rt_table, l3_route_old->dest, l3_route_old->mask);
    }

    initialize_glthread(&l3_route->rt_glue);
    insert_glnode(&rt_table->route_list, &l3_route->rt_glue);
    return TRUE;
}


/*Look up L3 routing table using longest prefix match*/

l3_route_t*
l3rib_lookup_lpm(rt_table_t *rt_table, unsigned int dest_ip){
   
     l3_route_t *l3_route = NULL,
     *lpm_l3_route = NULL,
     *default_l3_rt = NULL;

    glthread_t *curr = NULL;
    char subnet[16];
    char dest_ip_str[16];
    char longest_mask = 0;

    dest_ip = htonl(dest_ip);
    inet_ntop(AF_INET, &dest_ip, dest_ip_str, 16);
    dest_ip_str[15] = '\0';

    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){

        l3_route = rt_glue_to_l3_route(curr);
        memset(subnet, 0, 16);
        apply_mask(dest_ip_str, l3_route->mask, subnet);

        if(strncmp("0.0.0.0", l3_route->dest, 16) == 0 &&
                l3_route->mask == 0){
            default_l3_rt = l3_route;
        }
        else if(strncmp(subnet, l3_route->dest, strlen(subnet)) == 0){
            if( l3_route->mask > longest_mask){
                longest_mask = l3_route->mask;
                lpm_l3_route = l3_route;
            }
        }
    }ITERATE_GLTHREAD_END(&rt_table->route_list, curr);
    return lpm_l3_route ? lpm_l3_route : default_l3_rt;

}
void 
rt_table_add_route(rt_table_t *rt_table, char *dst,
			char mask, char *gw, char *oif){
	/*Check if the entry already exists*/

   unsigned int dst_int;
   char dst_str_with_mask[16];

   apply_mask(dst, mask, dst_str_with_mask);

   inet_pton(AF_INET, dst_str_with_mask, &dst_int);

   l3_route_t *l3_route = l3rib_lookup_lpm(rt_table, dst_int);

   /*Trying to add duplicate route!!*/
   assert(!l3_route);
   l3_route = calloc(1, sizeof(l3_route_t));
   strncpy(l3_route->dest, dst_str_with_mask, 16);
   l3_route->dest[15] = '\0';
   l3_route->mask = mask;

   if(!gw && !oif)
       l3_route->is_direct = TRUE;
   else
       l3_route->is_direct = FALSE;

   if(gw && oif){
        strncpy(l3_route->gw_ip, gw, 16);
        l3_route->gw_ip[15] = '\0';
        strncpy(l3_route->oif, oif, MAX_NAME_SIZE);
        l3_route->oif[MAX_NAME_SIZE - 1] = '\0';
   }

   if(!_rt_table_entry_add(rt_table, l3_route)){
        printf("Error : Route %s/%d Installation Failed\n",
            dst_str_with_mask, mask);
        free(l3_route);
   }


}
void
rt_table_add_direct_route(rt_table_t *rt_table,
                          char *dst, char mask){
	rt_table_add_route(rt_table, dst, mask,	0, 0);
}

static void
layer3_ip_pkt_recv_from_bottom(node_t *node, interface_t *interface,
        ip_hdr_t *pkt, unsigned int pkt_size){
	
	ip_hdr_t *ip_hdr = pkt;
        char dest_ip_addr[16];
	//printf("\nDest_ip = %x\n",ip_hdr->dst_ip);
	unsigned int dst_ip = htonl(ip_hdr->dst_ip);
	inet_ntop(AF_INET, &dst_ip, dest_ip_addr, 16);

	l3_route_t *l3_route = l3rib_lookup_lpm(NODE_RT_TABLE(node), ip_hdr->dst_ip);

	if(!l3_route){
		/*Router doesnt know what to do with this packet => Drop it*/
		printf("Router %s: Cannot route this packet with IP %s",
				node->node_name, dest_ip_addr);
		return;
	}
	
	/*If the packet is accepted then check if the packet is
	 * destined for local address*/

	if(l3_is_direct_route(l3_route)){
		/*Local Delivery case 1: pkt is destined to self(This is the router only)
		 * Case 2: pkt is destined to a host directly connected to the subnet*/
		
		/*if the IP address present in the IP header exactly matches with 
		 * the IP of any interface attached to the node*/

		if(is_layer3_local_delivery(node, ip_hdr->dst_ip)){
			/*Local Delivery Case 1:*/
			switch(ip_hdr->protocol){
				case ICMP_PRO:
					printf("\nIP Address: %s, Ping Success\n", dest_ip_addr);
					break;
				default:
					;
			}
		}
		else {
			/*Direct Delivery Case 2:*/
		        demote_pkt_to_layer2(
                	node,           /*Current processing node*/
                	0,              /*Dont know next hop IP as dest is present in local subnet*/
                	NULL,           /*No oif as dest is present in local subnet*/
                	(char *)ip_hdr, pkt_size,  /*Network Layer payload and size*/
                	ETH_IP);        /*Network Layer need to tell Data link layer, what type of payload it is passing down*/

		      return;
		}
	}
	else {
		/*L3 Forwarding Case*/
		/*decrement the TTL since the packet is fowarded to the next hop*/
		ip_hdr->ttl--;

		if(ip_hdr->ttl == 0){
			/*Drop the packet if the TTL value is 0*/
			return;
		}

		unsigned int next_hop_ip;
		inet_pton(AF_INET, l3_route->gw_ip, &next_hop_ip);
		next_hop_ip = htonl(next_hop_ip);
		demote_pkt_to_layer2(
                node,           /*Current processing node*/
                next_hop_ip,              
                l3_route->oif,           
                (char *)ip_hdr, pkt_size,  /*Network Layer payload and size*/
                ETH_IP);        /*Network Layer need to tell Data link layer, what type of payload it is passing down*/
	}
}

static void
layer3_pkt_recv_from_bottom(node_t *node, interface_t *interface,
                            char *pkt, unsigned int pkt_size, 
                            int L3_protocol_type){

	ip_hdr_t *pkt_3 = (ip_hdr_t *)pkt;
	switch(L3_protocol_type){
		case ETH_IP:
        	case IP_IN_IP:
            		layer3_ip_pkt_recv_from_bottom(node, interface, (ip_hdr_t *)pkt, pkt_size);
            		break;
        	default:
            		;

	}
}

/* A public API to be used by L2 or other lower Layers to promote
 * pkts to Layer 3 in TCP IP Stack*/
void
promote_pkt_to_layer3(node_t *node,            /*Current node on which the pkt is received*/
                      interface_t *interface,  /*ingress interface*/
                      char *pkt, unsigned int pkt_size, /*L3 payload*/
                      int L3_protocol_number){  /*obtained from eth_hdr->type field*/
	layer3_pkt_recv_from_bottom(node, interface, pkt, pkt_size, L3_protocol_number);
}


static void
layer3_pkt_receieve_from_top(node_t *node, char *pkt,
        unsigned int size, int protocol_number,
        unsigned int dest_ip_address){

    ip_hdr_t iphdr;
    initialize_ip_hdr(&iphdr);

    /*Now fill the non-default fields*/
    iphdr.protocol = protocol_number;

    unsigned int addr_int = 0;
    inet_pton(AF_INET, NODE_LO_ADDR(node), &addr_int);
    addr_int = htonl(addr_int);

    iphdr.src_ip = addr_int;
    iphdr.dst_ip = dest_ip_address;
    iphdr.total_length = (short)iphdr.ihl +
                         (short)(size/4) +
                         (short)((size % 4) ? 1 : 0);


    l3_route_t *l3_route = l3rib_lookup_lpm(NODE_RT_TABLE(node),
                                iphdr.dst_ip);

    if(!l3_route){
        printf("Node : %s : No L3 route\n",  node->node_name);
        return;
    }

    char *new_pkt = NULL;
    unsigned int new_pkt_size = 0 ;

    new_pkt_size = iphdr.total_length * 4;
    new_pkt = calloc(1, MAX_PACKET_BUFFER_SIZE);

    memcpy(new_pkt, (char *)&iphdr, iphdr.ihl * 4);

    if(pkt && size)
        memcpy(new_pkt + (iphdr.ihl * 4), pkt, size);

    /*Now Resolve Next hop*/
    bool_t is_direct_route = l3_is_direct_route(l3_route);

    unsigned int next_hop_ip;

    if(!is_direct_route){
        /*Case 1 : Forwarding Case*/
        inet_pton(AF_INET, l3_route->gw_ip, &next_hop_ip);
        next_hop_ip = htonl(next_hop_ip);
    }
    else{
        /*Case 2 : Direct Host Delivery Case*/
        /*Case 4 : Self-Ping Case*/
        /* The Data link layer will differentiate between case 2
         * and case 4 and take appropriate action*/
        next_hop_ip = dest_ip_address;
    }

    char *shifted_pkt_buffer = pkt_buffer_shift_right(new_pkt,
            new_pkt_size, MAX_PACKET_BUFFER_SIZE);

    demote_pkt_to_layer2(node,
            next_hop_ip,
            is_direct_route ? 0 : l3_route->oif,
            shifted_pkt_buffer, new_pkt_size,
            ETH_IP);

    free(new_pkt);
}

/*An API to be used by L4 or L5 to push the pkt down the TCP/IP
 * stack to layer 3*/
void
demote_packet_to_layer3(node_t *node,
                        char *pkt, unsigned int size,
                        int protocol_number, /*L4 or L5 protocol type*/
                        unsigned int dest_ip_address){

	layer3_pkt_receieve_from_top(node, pkt, size, 
			protocol_number, dest_ip_address);
}
