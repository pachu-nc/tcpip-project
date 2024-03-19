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


/*VLAN support*/

#pragma pack (push,1)
/*Vlan 802.1q 4 byte hdr*/
typedef struct vlan_8021q_hdr_{

    unsigned short tpid; /* = 0x8100*/
    short tci_pcp : 3 ;  /* inital 4 bits not used in this course*/
    short tci_dei : 1;   /*Not used*/
    short tci_vid : 12 ; /*Tagged vlan id*/
} vlan_8021q_hdr_t;

typedef struct vlan_ethernet_hdr_{

    mac_add_t dst_mac;
    mac_add_t src_mac;
    vlan_8021q_hdr_t vlan_8021q_hdr;
    unsigned short type;
    char payload[248];  /*Max allowed 1500*/
    unsigned int FCS;
} vlan_ethernet_hdr_t;
#pragma pack(pop)

static inline unsigned int
GET_802_1Q_VLAN_ID(vlan_8021q_hdr_t *vlan_8021q_hdr){

    return (unsigned int)vlan_8021q_hdr->tci_vid;
}

#define VLAN_ETH_FCS(vlan_eth_hdr_ptr, payload_size)  \
    (*(unsigned int *)(((char *)(((vlan_ethernet_hdr_t *)vlan_eth_hdr_ptr)->payload) + payload_size)))

#define VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD  \
   (sizeof(vlan_ethernet_hdr_t) - sizeof(((vlan_ethernet_hdr_t *)0)->payload))

/* Return 0 if not vlan tagged, else return pointer to 801.1q vlan hdr
 * present in ethernet hdr*/

static inline vlan_8021q_hdr_t *
is_pkt_vlan_tagged(ethernet_hdr_t *ethernet_hdr){

    /*Check the 13th and 14th byte of the ethernet hdr,
     *      * if is value is 0x8100 then it is vlan tagged*/

    vlan_8021q_hdr_t *vlan_8021q_hdr =
        (vlan_8021q_hdr_t *)((char *)ethernet_hdr + (sizeof(mac_add_t) * 2));
    if(vlan_8021q_hdr->tpid == VLAN_8021Q_PROTO)
        return vlan_8021q_hdr;

    return NULL;
}


#define ETH_HDR_SIZE_EXCL_PAYLOAD   \
    (sizeof(ethernet_hdr_t) - sizeof(((ethernet_hdr_t *)0)->payload))

#define ETH_FCS(eth_hdr_ptr, payload_size)  \
    (*(unsigned int *)(((char *)(((ethernet_hdr_t *)eth_hdr_ptr)->payload) + payload_size)))

#if 0
static inline bool_t
l2_framce_recv_qualify_on_interface(interface_t *interface,
		ethernet_hdr_t *ethernet_hdr) {

	if(!IS_INTF_L3_MODE(interface))
		return FALSE;

	/*Return TRUE receiving machine should accept the frame*/
	/*If the mac address of the interface is the same as the 
	 * destination MAC then return true*/
	if(memcmp(IF_MAC(interface), ethernet_hdr->dst_mac.mac,
				sizeof(mac_add_t)) == 0){
		return TRUE;
	}
	
	/*If the destination mac is broadcast address then return True*/
	if(IS_MAC_BROADCAST_ADDR(ethernet_hdr->dst_mac.mac))
		return TRUE;

	return FALSE;
}
#endif
static inline void
SET_COMMON_ETH_FCS(ethernet_hdr_t *ethernet_hdr,
                   unsigned int payload_size,
                   unsigned int new_fcs){

    if(is_pkt_vlan_tagged(ethernet_hdr)){
        VLAN_ETH_FCS(ethernet_hdr, payload_size) = new_fcs;
    }
    else{
      ETH_FCS(ethernet_hdr, payload_size) = new_fcs;
    }
}

#if 1
/*fn to get access to ethernet payload address*/
static inline char *
GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr_t *ethernet_hdr){

   if(is_pkt_vlan_tagged(ethernet_hdr)){
        return ((vlan_ethernet_hdr_t *)(ethernet_hdr))->payload;
   }
   else
       return ethernet_hdr->payload;
}
#endif


static inline bool_t 
l2_frame_recv_qualify_on_interface(interface_t *interface, 
					ethernet_hdr_t *ethernet_hdr,
					unsigned int *output_vlan_id){
	*output_vlan_id = 0;

    vlan_8021q_hdr_t *vlan_8021q_hdr =
                        is_pkt_vlan_tagged(ethernet_hdr);
    /* Presence of IP address on interface makes it work in L3 mode,
     * while absence of IP-address automatically make it work in
     * L2 mode provided that it is operational either in ACCESS mode or TRUNK mode.*/

    /* case 10 : If receiving interface is neither working in L3 mode
     * nor in L2 mode, then reject the packet*/
    if(!IS_INTF_L3_MODE(interface) &&
        IF_L2_MODE(interface) == L2_MODE_UNKNOWN){

        return FALSE;
    }

    /* If interface is working in ACCESS mode but at the
     * same time not operating within a vlan, then it must
     * not accept tagged or untagged frames*/

    if(IF_L2_MODE(interface) == ACCESS &&
        get_access_intf_operating_vlan_id(interface) == 0){

            return FALSE;   /*case 3 and 4*/
    }

    /* if interface is working in ACCESS mode and operating with in
     * vlan, then :
     * 1. it must accept untagged frame and tag it with a vlan-id of an interface
     * 2. Or  it must accept tagged frame but tagged with same vlan-id as interface's vlan operation*/

    unsigned int intf_vlan_id = 0,
                 pkt_vlan_id = 0;

    if(IF_L2_MODE(interface) == ACCESS){

        intf_vlan_id = get_access_intf_operating_vlan_id(interface);

        if(!vlan_8021q_hdr && intf_vlan_id){
            *output_vlan_id = intf_vlan_id;
 	return TRUE; /*case 6*/
        }

        pkt_vlan_id = GET_802_1Q_VLAN_ID(vlan_8021q_hdr);
        if(pkt_vlan_id == intf_vlan_id){
            return TRUE;    /*case 5*/
        }
        else{
            return FALSE;   /*case 5*/
        }
    }

    /* if interface is operating in a TRUNK mode, then it must discard all untagged
     * frames*/

    if(IF_L2_MODE(interface) == TRUNK){

        if(!vlan_8021q_hdr){
            /*case 7 & 8*/
            return FALSE;
        }
    }

    /* if interface is operating in a TRUNK mode, then it must accept the frame
     * which are tagged with any vlan-id in which interface is operating.*/

    if(IF_L2_MODE(interface) == TRUNK &&
            vlan_8021q_hdr){

        pkt_vlan_id = GET_802_1Q_VLAN_ID(vlan_8021q_hdr);
        if(is_trunk_interface_vlan_enabled(interface, pkt_vlan_id)){
            return TRUE;    /*case 9*/
        }
        else{
            return FALSE;   /*case 9*/
        }
    }

    /*If the interface is operating in L3 mode, and recv vlan tagged frame, drop it*/
    if(IS_INTF_L3_MODE(interface) && vlan_8021q_hdr){
        /*case 2*/
        return FALSE;
    }

    /* If interface is working in L3 mode, then accept the frame only when
     * its dst mac matches with receiving interface MAC*/
    if(IS_INTF_L3_MODE(interface) &&
        memcmp(IF_MAC(interface),
        ethernet_hdr->dst_mac.mac,
        sizeof(mac_add_t)) == 0){
        /*case 1*/
        return TRUE;
    }

    /*If interface is working in L3 mode, then accept the frame with
     * broadcast MAC*/
  if(IS_INTF_L3_MODE(interface) &&
        IS_MAC_BROADCAST_ADDR(ethernet_hdr->dst_mac.mac)){
        /*case 1*/
        return TRUE;
    }

    return FALSE;
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


/*APIs to be used to create topologies*/
void
arp_table_update_from_arp_reply(arp_table_t *arp_table,
				arp_hdr_t *arp_hdr, interface_t *iif);


#if 1

/*APIs to be used to create topologies*/
void
node_set_intf_l2_mode(node_t *node, char *intf_name, intf_l2_mode_t intf_l2_mode);

typedef struct arp_pending_entry_ arp_pending_entry_t;


typedef void (*arp_processing_fn)(node_t *,
                                  interface_t *oif,
                                  arp_entry_t *,
                                 arp_pending_entry_t *);


struct arp_pending_entry_{
    glthread_t arp_pending_entry_glue;
    arp_processing_fn cb;
    unsigned int pkt_size;  /*Including ether net hdr*/
    char pkt[0];
};

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

ethernet_hdr_t *
untag_pkt_with_vlan_id(ethernet_hdr_t *ethernet_hdr,
                     unsigned int total_pkt_size,
                     unsigned int *new_pkt_size);

ethernet_hdr_t *
tag_pkt_with_vlan_id(ethernet_hdr_t *ethernet_hdr,
                     unsigned int total_pkt_size,
                     int vlan_id,
                     unsigned int *new_pkt_size);

static inline unsigned int
GET_ETH_HDR_SIZE_EXCL_PAYLOAD(ethernet_hdr_t *ethernet_hdr){

    if(is_pkt_vlan_tagged(ethernet_hdr)){
        return VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;
    }
    else{
        return ETH_HDR_SIZE_EXCL_PAYLOAD;
    }
}

#endif /*_LAYER2_H__*/
