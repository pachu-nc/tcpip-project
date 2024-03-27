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
	return (l3_route_t *) ( (char *)curr -  (char *)(offset(l3_route_t, rt_glue)));
}
void
init_rt_table(rt_table_t **rt_table){
	*rt_table = calloc(1, sizeof(rt_table_t));
	initialize_glthread(&((*rt_table)->route_list));
}

static void
l3_route_free(l3_route_t *l3_route){

    assert(IS_GLTHREAD_LIST_EMPTY(&l3_route->rt_glue));
    spf_flush_nexthops(l3_route->nexthops);
    free(l3_route);
}

void
clear_rt_table(rt_table_t *rt_table){

    glthread_t *curr;
    l3_route_t *l3_route;

    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){

        l3_route = rt_glue_to_l3_route(curr);
        if(l3_is_direct_route(l3_route))
            continue;
        remove_glnode(curr);
        l3_route_free(l3_route);
    } ITERATE_GLTHREAD_END(&rt_table->route_list, curr);
}

nexthop_t *
l3_route_get_active_nexthop(l3_route_t *l3_route){

    if(l3_is_direct_route(l3_route))
        return NULL;

    nexthop_t *nexthop = l3_route->nexthops[l3_route->nxthop_idx];
    assert(nexthop);

    l3_route->nxthop_idx++;

    if(l3_route->nxthop_idx == MAX_NXT_HOPS ||
        !l3_route->nexthops[l3_route->nxthop_idx]){
        l3_route->nxthop_idx = 0;
    }
    return nexthop;
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

    int i = 0;
    glthread_t *curr = NULL;
    l3_route_t *l3_route = NULL;
    int count = 0;
    printf("L3 Routing Table:\n");
    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){

        l3_route = rt_glue_to_l3_route(curr);
        count++;
        if(l3_route->is_direct){
            if(count != 1){
                printf("\t|===================|=======|====================|==============|==========|\n");
            }
            else{
                printf("\t|======= IP ========|== M ==|======== Gw ========|===== Oif ====|== Cost ==|\n");
            }
            printf("\t|%-18s |  %-4d | %-18s | %-12s |          |\n",
                    l3_route->dest, l3_route->mask, "NA", "NA");
            continue;
        }

        for( i = 0; i < MAX_NXT_HOPS; i++){
            if(l3_route->nexthops[i]){
                if(i == 0){
                    if(count != 1){
                        printf("\t|===================|=======|====================|==============|==========|\n");
                    }
                    else{
                        printf("\t|======= IP ========|== M ==|======== Gw ========|===== Oif ====|== Cost ==|\n");
                    }
                    printf("\t|%-18s |  %-4d | %-18s | %-12s |  %-4u    |\n",
                            l3_route->dest, l3_route->mask,
                            l3_route->nexthops[i]->gw_ip,
                            l3_route->nexthops[i]->oif->if_name, l3_route->spf_metric);
                }
                else{
                    printf("\t|                   |       | %-18s | %-12s |          |\n",
                            l3_route->nexthops[i]->gw_ip,
                            l3_route->nexthops[i]->oif->if_name);
                }
            }
        }
    } ITERATE_GLTHREAD_END(&rt_table->route_list, curr);
    printf("\t|===================|=======|====================|==============|==========|\n");
}


static bool_t
_rt_table_entry_add(rt_table_t *rt_table, l3_route_t *l3_route){
    
    initialize_glthread(&l3_route->rt_glue);
    insert_glnode(&rt_table->route_list, &l3_route->rt_glue);
    return TRUE;
}


/*Look up L3 routing table using longest prefix match*/

l3_route_t *
l3rib_lookup(rt_table_t *rt_table,
             uint32_t dest_ip,
             char mask){

    char dest_ip_str[16];
    glthread_t *curr = NULL;
    char dst_str_with_mask[16];
    l3_route_t *l3_route = NULL;

    tcp_ip_covert_ip_n_to_p(dest_ip, dest_ip_str);

    apply_mask(dest_ip_str, mask, dst_str_with_mask);

    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){

        l3_route = rt_glue_to_l3_route(curr);

        if(strncmp(dst_str_with_mask, l3_route->dest, 16) == 0 &&
            l3_route->mask == mask){
            return l3_route;
        }
    } ITERATE_GLTHREAD_END(&rt_table->route_list, curr);

    return NULL;
}


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
			char mask, char *gw, char *oif, uint32_t spf_metric){
    
   uint32_t dst_int;
   char dst_str_with_mask[16];
   bool_t new_route = FALSE;

   apply_mask(dst, mask, dst_str_with_mask);
   inet_pton(AF_INET, dst_str_with_mask, &dst_int);
   dst_int = htonl(dst_int);

   l3_route_t *l3_route = l3rib_lookup(rt_table, dst_int, mask);

   if(!l3_route){
       l3_route = calloc(1, sizeof(l3_route_t));
       strncpy(l3_route->dest, dst_str_with_mask, 16);
       l3_route->dest[15] = '\0';
       l3_route->mask = mask;
       new_route = TRUE;
       l3_route->is_direct = TRUE;
   }

   int i = 0;

   /*Get the index into nexthop array to fill the new nexthop*/
   if(!new_route){
       for( ; i < MAX_NXT_HOPS; i++){

           if(l3_route->nexthops[i]){
                if(strncmp(l3_route->nexthops[i]->gw_ip, gw, 16) == 0 &&
                    l3_route->nexthops[i]->oif == oif){
                    printf("Error : Attempt to Add Duplicate Route\n");
                    return;
                }
           }
           else break;
       }
   }

   if( i == MAX_NXT_HOPS){
        printf("Error : No Nexthop space left for route %s/%u\n",
            dst_str_with_mask, mask);
        return;
   }

   if(gw && oif){
        nexthop_t *nexthop = calloc(1, sizeof(nexthop_t));
        l3_route->nexthops[i] = nexthop;
        l3_route->is_direct = FALSE;
        l3_route->spf_metric = spf_metric;
        nexthop->ref_count++;
        strncpy(nexthop->gw_ip, gw, 16);
        nexthop->gw_ip[15] = '\0';
        nexthop->oif = oif;
   }
 if(new_route){
       if(!_rt_table_entry_add(rt_table, l3_route)){
           printf("Error : Route %s/%d Installation Failed\n",
                   dst_str_with_mask, mask);
           l3_route_free(l3_route);
       }
   }
}
void
rt_table_add_direct_route(rt_table_t *rt_table,
                          char *dst, char mask){
	rt_table_add_route(rt_table, dst, mask,	0, 0, 0);
}

static void
layer3_ip_pkt_recv_from_bottom(node_t *node, interface_t *interface,
        ip_hdr_t *pkt, unsigned int pkt_size){
	
	ip_hdr_t *ip_hdr = pkt;
        char dest_ip_addr[16];
        char *l4_hdr, *l5_hdr;

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
				case IP_IN_IP:
                    			/*Packet has reached ERO, now set the packet onto its new
                    			  Journey from ERO to final destination*/
                    			layer3_ip_pkt_recv_from_bottom(node, interface,
                    			        (ip_hdr_t *)INCREMENT_IPHDR(ip_hdr),
                    			        IP_HDR_PAYLOAD_SIZE(ip_hdr));
                    			return;

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
#if 0 
		unsigned int next_hop_ip;
		inet_pton(AF_INET, l3_route->gw_ip, &next_hop_ip);
		next_hop_ip = htonl(next_hop_ip);
		demote_pkt_to_layer2(
                node,           /*Current processing node*/
                next_hop_ip,              
                l3_route->oif,           
                (char *)ip_hdr, pkt_size,  /*Network Layer payload and size*/
                ETH_IP);        /*Network Layer need to tell Data link layer, what type of payload it is passing down*/
                #endif

 /* If route is non direct, then ask LAyer 2 to send the pkt
     * out of all ecmp nexthops of the route*/
    uint32_t next_hop_ip;
    nexthop_t *nexthop = NULL;

    nexthop = l3_route_get_active_nexthop(l3_route);
    assert(nexthop);

    inet_pton(AF_INET, nexthop->gw_ip, &next_hop_ip);
    next_hop_ip = htonl(next_hop_ip);

    /*tcp_dump_l3_fwding_logger(node,
        nexthop->oif->if_name, nexthop->gw_ip);
    */
    demote_pkt_to_layer2(node,
            next_hop_ip,
            nexthop->oif->if_name,
            (char *)ip_hdr, pkt_size,
            ETH_IP); /*Network Layer need to tell Data link layer,
                       what type of payload it is passing down*/

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
    nexthop_t *nexthop = NULL;

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
        /* If route is non direct, then ask Layer 2 to send the pkt
        * out of all ecmp nexthops of the route*/
        uint32_t next_hop_ip;
        nexthop = l3_route_get_active_nexthop(l3_route);

        if(!nexthop){
            free(new_pkt);
            return;
        }
        inet_pton(AF_INET, nexthop->gw_ip, &next_hop_ip);
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
            is_direct_route ? 0 : nexthop->oif->if_name,
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
