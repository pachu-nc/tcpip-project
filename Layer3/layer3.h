#ifndef __LAYER3_H__
#define __LAYER3_H__

#include "glthread.h"
#pragma pack (push,1)

/*The Ip hdr format as per the standard specification*/
typedef struct ip_hdr_{

    unsigned int version : 4 ;  /*version number, always 4 for IPv4 protocol*/    
    unsigned int ihl : 4 ;      /*length of IP hdr, in 32-bit words unit. for Ex, if this value is 5, it means length of this ip hdr is 20Bytes*/
    char tos;
    short total_length;         /*length of hdr + ip_hdr payload*/

    /* Fragmentation Related members, we shall not be using below members
     * as we will not be writing fragmentation code. if you wish, take it
     * as a extension of the project*/
    short identification;       
    unsigned int unused_flag : 1 ;
    unsigned int DF_flag : 1;   
    unsigned int MORE_flag : 1; 
    unsigned int frag_offset : 13;  


    char ttl;
    char protocol;
    short checksum;
    unsigned int src_ip;
    unsigned int dst_ip;
} ip_hdr_t;

#pragma(pop)

static inline void
initialize_ip_hdr(ip_hdr_t *ip_hdr){
    
    ip_hdr->version = 4;
    ip_hdr->ihl = 5; /*We will not be using option field, hence hdr size shall always be 5*4 = 20B*/
    ip_hdr->tos = 0;

    ip_hdr->total_length = 0; /*To be filled by the caller*/

    /*Fragmentation related will not be used
     * int this course, initialize them all to zero*/
    ip_hdr->identification = 0; 
    ip_hdr->unused_flag = 0;
    ip_hdr->DF_flag = 1;
    ip_hdr->MORE_flag = 0;
    ip_hdr->frag_offset = 0;

    ip_hdr->ttl = 64; /*Let us use 64*/
    ip_hdr->protocol = 0; /*To be filled by the caller*/
    ip_hdr->checksum = 0; /*Not used in this course*/
    ip_hdr->src_ip = 0; /*To be filled by the caller*/ 
    ip_hdr->dst_ip = 0; /*To be filled by the caller*/
}



typedef struct rt_table_{
	glthread_t route_list;
}rt_table_t;


typedef struct l3_route_{
	char dest[16]; /*Key*/
	char mask;	/*Key*/
	bool_t is_direct; /*if set to True, then gw_ip and oif has no meaning*/
	char gw_ip[16]; /*Next hop IP*/
	char oif[MAX_NAME_SIZE]; /*OIF*/
	glthread_t rt_glue;
}l3_route_t;

void
init_rt_table(rt_table_t **rt_table);

void
clear_rt_table(rt_table_t *rt_table);

void
delete_rt_table_entry(rt_table_t *rt_table, char *ip_addr, char mask);

void
rt_table_add_route(rt_table_t *rt_table, 
                   char *dst, char mask,
                   char *gw, char *oif);

void
rt_table_add_direct_route(rt_table_t *rt_table,
                          char *dst, char mask);

void
dump_rt_table(rt_table_t *rt_table);


l3_route_t*
l3rib_lookup_lpm(rt_table_t *rt_table,
                 unsigned int dest_ip);

#define IP_HDR_LEN_IN_BYTES(ip_hdr_ptr)  (ip_hdr_ptr->ihl * 4)
#define IP_HDR_TOTAL_LEN_IN_BYTES(ip_hdr_ptr)   (ip_hdr_ptr->total_length * 4)
#define INCREMENT_IPHDR(ip_hdr_ptr) ((char *)ip_hdr_ptr + (ip_hdr_ptr->ihl * 4))
#define IP_HDR_PAYLOAD_SIZE(ip_hdr_ptr) (IP_HDR_TOTAL_LEN_IN_BYTES(ip_hdr_ptr) - \
        IP_HDR_LEN_IN_BYTES(ip_hdr_ptr))


#define IS_L3_ROUTES_EQUAL(rt1, rt2)              \
    ((strncmp(rt1->dest, rt2->dest, 16) == 0) &&  \
    (rt1->mask == rt2->mask) &&                   \
    (rt1->is_direct == rt2->is_direct) &&         \
    (strncmp(rt1->gw_ip, rt2->gw_ip, 16) == 0) && \
    strncmp(rt1->oif, rt2->oif, MAX_NAME_SIZE) == 0)

void
promote_pkt_to_layer3(node_t *node,            /*Current node on which the pkt is received*/
                      interface_t *interface,  /*ingress interface*/
                      char *pkt, unsigned int pkt_size, /*L3 payload*/
                      int L3_protocol_number);  /*obtained from eth_hdr->type field*/



/*An API to be used by L4 or L5 to push the pkt down the TCP/IP
 * stack to layer 3*/
void
demote_packet_to_layer3(node_t *node,
                        char *pkt, unsigned int size,
                        int protocol_number, /*L4 or L5 protocol type*/
                        unsigned int dest_ip_address);















































#endif /*__LAYER3_H*/
