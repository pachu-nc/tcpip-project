#ifndef __LAYER2_H__
#define __LAYER2_H__

#include <net.h>
#include <glthread.h>
#include "tcpconst.h"
#include <stdlib.h>  /*for calloc*/
#include "graph.h"
#include "utils.h"

/*ARP table is maintained as a linked list of ARP entries*/
typedef struct arp_table_ {
	glthread_t arp_entries;
}arp_table_t;

/*ARP Table Format:
 * ----------------------------
 *| KEY(IP ADDR)| MAC	| OIF | 
 * ---------------------------*/

/*Max number of entries in the ARP Table = Max number of neighbours of host/router*/
typedef struct arp_entry {
	ip_add_t ip_addr; /*Key*/
	mac_add_t mac_addr;
	char oif_name[MAX_NAME_SIZE];
	glthread_t arp_glue;
}arp_entry_t;



#pragma pack (push,1)
typedef struct arp_hdr_{

    short hw_type;          /*1 for ethernet cable*/
    short proto_type;       /*0x0800 for IPV4*/
    char hw_addr_len;       /*6 for MAC*/
    char proto_addr_len;    /*4 for IPV4*/
    short op_code;          /*1 - req or 2- reply*/
    mac_add_t src_mac;      /*MAC of OIF interface*/
    unsigned int src_ip;    /*IP of OIF*/
    mac_add_t dst_mac;      /*?*/
    unsigned int dst_ip;        /*IP for which ARP is being resolved*/
} arp_hdr_t;


/*#pragma is included so that compiler doesn't include any padding in the structure. Since it is a standard header format*/
#pragma pack (push,1)
typedef struct ethernet_hdr_{
	mac_add_t dst_mac;
	mac_add_t src_mac;
	unsigned short type; /*Also called Length*/
	char payload[248]; /*Max allowed is 1500*/
	unsigned int FCS;
}ethernet_hdr_t;
#pragma(pop)



#define ETH_HDR_SIZE_EXCL_PAYLOAD   \
    (sizeof(ethernet_hdr_t) - sizeof(((ethernet_hdr_t *)0)->payload))

#define ETH_FCS(eth_hdr_ptr, payload_size)  \
    (*(unsigned int *)(((char *)(((ethernet_hdr_t *)eth_hdr_ptr)->payload) + payload_size)))


static inline bool_t
l2_framce_recv_qualify_on_interface(interface_t *interface,
		ethernet_hdr_t *ethernet_hdr) {
/*
	if(!IS_INTF_L3_MODE(interface))
		return FALSE;
*/
	/*Return TRUE receiving machine should accept the frame*/
	/*If the mac address of the interface is the same as the 
	 * destination MAC then return true*/
	if(memcmp(IF_MAC(interface), ethernet_hdr->dst_mac.mac,
				sizeof(mac_add_t)) == 0){
		return TRUE;
	}
	
	/*If the destination mac is broadcast address then return True*/
/*	if(IS_BROADCAST_ADDRESS(ethernet_hdr->dst_mac.mac))
		return TRUE;
*/
	return FALSE;
}

static inline void
SET_COMMON_ETH_FCS(ethernet_hdr_t *ethernet_hdr,
                   unsigned int payload_size,
                   unsigned int new_fcs){

//    if(is_pkt_vlan_tagged(ethernet_hdr)){
//        VLAN_ETH_FCS(ethernet_hdr, payload_size) = new_fcs;
//    }
//    else{
        ETH_FCS(ethernet_hdr, payload_size) = new_fcs;
//    }
}

#if 1
/*fn to get access to ethernet payload address*/
static inline char *
GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr_t *ethernet_hdr){

   //if(is_pkt_vlan_tagged(ethernet_hdr)){
   //     return ((vlan_ethernet_hdr_t *)(ethernet_hdr))->payload;
   //}
   //else
       return ethernet_hdr->payload;
}
#endif


static inline bool_t 
l2_frame_recv_qualify_on_interface(interface_t *interface, 
					ethernet_hdr_t *ethernet_hdr){
	
	/* Presence of IP address on interface makes it work in L3 mode,
     	* while absence of IP-address automatically make it work in
     	* L2 mode provided that it is operational either in ACCESS mode or TRUNK mode.*/

	if(!interface->intf_nw_prop.is_ipadd_config)
		return FALSE;

	/*Return TRUE if the receiving machine should accept the message*/
#if 1
	if(memcmp(IF_MAC(interface),
        		ethernet_hdr->dst_mac.mac,
        		sizeof(mac_add_t)) == 0){
        	/*case 1*/
        	return TRUE;
	}
#endif
/*
	if(IS_BROADCAST_ADDRESS(ethernet_hdr->dst_mac.mac)){
		return TRUE;
	}
	*/
	return TRUE;
}

void
init_arp_table(arp_table_t **arp_table);

arp_entry_t *
arp_table_lookup(arp_table_t *arp_table, char *ip_addr);

void
clear_arp_table(arp_table_t *arp_table);

void
delete_arp_entry(arp_entry_t *arp_entry);

void
delete_arp_table_entry(arp_table_t *arp_table, char *ip_addr);

bool_t
arp_table_entry_add(arp_table_t *arp_table, arp_entry_t *arp_entry);

void
dump_arp_table(arp_table_t *arp_table);

void
arp_table_update_from_arp_reply(arp_table_t *arp_table,
                                arp_hdr_t *arp_hdr, interface_t *iif);

void
send_arp_broadcast_request(node_t *node,
                           interface_t *oif,
			   char *ip_addr);

#if 0

/*APIs to be used to create topologies*/
void
node_set_intf_l2_mode(node_t *node, char *intf_name, intf_l2_mode_t intf_l2_mode);

void
node_set_intf_vlan_membsership(node_t *node, char *intf_name, unsigned int vlan_id);
void
add_arp_pending_entry(arp_entry_t *arp_entry,
                        arp_processing_fn,
                        char *pkt,
                        unsigned int pkt_size);

#endif

arp_entry_t *
create_arp_sane_entry(arp_table_t *arp_table, char *ip_addr);

#if 0
static bool_t
arp_entry_sane(arp_entry_t *arp_entry){

    return arp_entry->is_sane;
}

#endif
#endif /*_LAYER2_H__*/
