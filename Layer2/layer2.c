#include "graph.h"
#include "glthread.h"
#include <stdio.h>
#include "layer2.h"
#include <stdlib.h>
#include <sys/socket.h>
#include "comm.h"
#include <arpa/inet.h> /*for inet_ntop & inet_pton*/
#include <assert.h>
#include <string.h>
#include "tcpconst.h"



arp_entry_t *arp_glue_to_arp_entry(glthread_t *curr ){
	/*returns the pointer to the starting address of the node*/
        return  (node_t *)( (char *)curr - (char *)offset(arp_entry_t,arp_glue));
}

void
layer2_frame_recv(node_t *node, interface_t *interface, 
		char *pkt, unsigned int pkt_size){
	/*Entry point into the TCP/IP stack from the bottom*/

	unsigned int vlan_id_to_tag = 0;

    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *)pkt;

    if(l2_frame_recv_qualify_on_interface(interface,
                                          ethernet_hdr) == FALSE){

        printf("L2 Frame Rejected on node %s\n", node->node_name);
        return;
    }

    printf("L2 Frame Accepted on node %s \n", node->node_name);
	
    switch(ethernet_hdr->type){
	    case ARP_MSG: 
		    {
			    arp_hdr_t *arp_hdr = (arp_hdr_t *) (ethernet_hdr->payload);
			    switch(arp_hdr->op_code){
				    case ARP_BROAD_REQ: 
					    process_arp_broadcast_request(node, interface, ethernet_hdr);
				    	    break;
	     			     case ARP_REPLY:
				    	    process_arp_reply_msg(node, interface, ethernet_hdr);
	     				    break;
	    			    default:
	    				break;
			    }
		    }
		    break;
	    default:
		    printf("\nNot an ARP Message\n");
		    break;
    }
#if 0
    /*Handle Reception of a L2 Frame on L3 Interface*/
    if(IS_INTF_L3_MODE(interface)){

       promote_pkt_to_layer2(node, interface, ethernet_hdr, pkt_size);
    }
    else if(IF_L2_MODE(interface) == ACCESS ||
                IF_L2_MODE(interface) == TRUNK){

        unsigned int new_pkt_size = 0;

        if(vlan_id_to_tag){
            pkt = (char *)tag_pkt_with_vlan_id((ethernet_hdr_t *)pkt,
                                                pkt_size, vlan_id_to_tag,
                                                &new_pkt_size);
            assert(new_pkt_size != pkt_size);
        }
        l2_switch_recv_frame(interface, pkt, vlan_id_to_tag ? new_pkt_size : pkt_size);
    }
    else
        return; /*Do nothing, drop the packet*/
#endif
}

void 
init_arp_table(arp_table_t **arp_table) {
	*arp_table = calloc(1, sizeof(arp_table_t));
	initialize_glthread(&((*arp_table)->arp_entries));
}

arp_entry_t *
arp_table_lookup(arp_table_t *arp_table, char *ip_addr){

	glthread_t *curr;
	arp_entry_t *arp_entry;

	ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries, curr){
		arp_entry = arp_glue_to_arp_entry(curr);

		if(strncmp(arp_entry->ip_addr.ip_add, ip_addr, 16) == 0){
			return arp_entry;
		}
	}ITERATE_GLTHREAD_END(arp_table, curr)

	/*Return NULL if no ARP entry is found.*/
	return NULL;
}

void
clear_arp_table(arp_table_t *arp_table){
   glthread_t *curr;
    arp_entry_t *arp_entry;

    ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries, curr){

        arp_entry = arp_glue_to_arp_entry(curr);
        delete_arp_entry(arp_entry);
    } ITERATE_GLTHREAD_END(&arp_table->arp_entries, curr);
}

void
delete_arp_entry(arp_entry_t *arp_entry){
	glthread_t *curr;
    	remove_glnode(&arp_entry->arp_glue);
	free(arp_entry);
}

void
delete_arp_table_entry(arp_table_t *arp_table, char *ip_addr){
	arp_entry_t *arp_entry = arp_table_lookup(arp_table, ip_addr);

    	if(!arp_entry)
        	return;

    	delete_arp_entry(arp_entry);
}

bool_t
arp_table_entry_add(arp_table_t *arp_table, arp_entry_t *arp_entry){
	
	/*Check if the ARP table entry already exists*/

	arp_table_t *arp_entry_old = arp_table_lookup(arp_table,
			arp_entry->ip_addr.ip_add);
	
	/*Check if the ARP table entry already exists and it is exactly same as the current one.
	 * If yes, then don't do anythig. */
	if(arp_entry_old && 
			memcmp(arp_entry_old, arp_entry, sizeof(arp_entry_t)) == 0)
		return FALSE;
	/*If the ARP table entry exists and the entry is not the same as the current one, 
	 * delete the old ARP entry and insert the new ARP entry*/
	if(arp_entry_old){
		delete_arp_table_entry(arp_table, arp_entry->ip_addr.ip_add);
	}
	initialize_glthread(&arp_entry->arp_glue);
	insert_glnode(&arp_table->arp_entries, &arp_entry->arp_glue);
	return TRUE;
}


void
dump_arp_table(arp_table_t *arp_table){

    glthread_t *curr;
    arp_entry_t *arp_entry;
    ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries, curr){
        arp_entry = arp_glue_to_arp_entry(curr);
        printf("IP : %s, MAC : %x:%x:%x:%x:%x:%x, OIF = %s\n",
            arp_entry->ip_addr.ip_add,
            arp_entry->mac_addr.mac[0],
            arp_entry->mac_addr.mac[1],
            arp_entry->mac_addr.mac[2],
            arp_entry->mac_addr.mac[3],
            arp_entry->mac_addr.mac[4],
            arp_entry->mac_addr.mac[5],
            arp_entry->oif_name);
            //arp_entry_sane(arp_entry) ? "TRUE" : "FALSE");
    } ITERATE_GLTHREAD_END(&arp_table->arp_entries, curr);
}

void
arp_table_update_from_arp_reply(arp_table_t *arp_table,
                                arp_hdr_t *arp_hdr, interface_t *iif){
	unsigned int src_ip = 0;
	/*This API must be called only if the second arg for this api is ARP_REPLY*/
	assert(arp_hdr->op_code == ARP_REPLY);
	
	arp_entry_t *arp_entry = calloc(1, sizeof(arp_entry_t));

	/*Get the SRC IP from the ARP header*/
	src_ip = htonl(arp_hdr->src_ip);
	
	/*Convert the src_ip to its string form i.e. "a.b.c.d" and copy it to the ARP entry*/
	inet_ntop(AF_INET, &src_ip, arp_entry->ip_addr.ip_add, 16);

    	arp_entry->ip_addr.ip_add[15] = '\0';

	/*Copy the MAC Address from ARP REPLY message to the arp_entry*/
	memcpy(arp_entry->mac_addr.mac, arp_hdr->src_mac.mac, sizeof(mac_add_t));
 	
	/*Copy the interface name to arp_entry's oif*/
	strncpy(arp_entry->oif_name, iif->if_name, MAX_NAME_SIZE);
	
	/*Add the ARP entry to the ARP table*/
	bool_t rc = arp_table_entry_add(arp_table, arp_entry); 

	/*If adding returns FALSE*/
	if(rc == FALSE){
		free(arp_entry);
	}
}
/*Routine for ARP APIs*/
void
process_arp_reply_msg(node_t *node, interface_t *iif,
			ethernet_hdr_t *ethernet_hdr){
    printf("%s : ARP reply msg recvd on interface %s of node %s\n",
             __FUNCTION__, iif->if_name , iif->att_node->node_name);

    arp_table_update_from_arp_reply( NODE_ARP_TABLE(node),
                    (arp_hdr_t *)GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr), iif);
    
}

void
send_arp_reply_msg(ethernet_hdr_t *ethernet_hdr_in,
                interface_t *oif){

    arp_hdr_t *arp_hdr_in = (arp_hdr_t *)(GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr_in));

    ethernet_hdr_t *ethernet_hdr_reply = (ethernet_hdr_t *)calloc(1, MAX_PACKET_BUFFER_SIZE);

    memcpy(ethernet_hdr_reply->dst_mac.mac, arp_hdr_in->src_mac.mac, sizeof(mac_add_t));
    memcpy(ethernet_hdr_reply->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));

    ethernet_hdr_reply->type = ARP_MSG;

    arp_hdr_t *arp_hdr_reply = (arp_hdr_t *)(GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr_reply));

    arp_hdr_reply->hw_type = 1;
    arp_hdr_reply->proto_type = 0x0800;
    arp_hdr_reply->hw_addr_len = sizeof(mac_add_t);
    arp_hdr_reply->proto_addr_len = 4;

    arp_hdr_reply->op_code = ARP_REPLY;
    memcpy(arp_hdr_reply->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));

    inet_pton(AF_INET, IF_IP(oif), &arp_hdr_reply->src_ip);
    arp_hdr_reply->src_ip =  htonl(arp_hdr_reply->src_ip);

    memcpy(arp_hdr_reply->dst_mac.mac, arp_hdr_in->src_mac.mac, sizeof(mac_add_t));
    arp_hdr_reply->dst_ip = arp_hdr_in->src_ip;

    SET_COMMON_ETH_FCS(ethernet_hdr_reply, sizeof(arp_hdr_t), 0); /*Not used*/

    unsigned int total_pkt_size = ETH_HDR_SIZE_EXCL_PAYLOAD + sizeof(arp_hdr_t);

    char *shifted_pkt_buffer = pkt_buffer_shift_right((char *)ethernet_hdr_reply,
                               total_pkt_size, MAX_PACKET_BUFFER_SIZE);

    send_pkt_out(shifted_pkt_buffer, total_pkt_size, oif);

    free(ethernet_hdr_reply);
}

void
process_arp_broadcast_request(node_t *node,
                           interface_t *iif,
                           ethernet_hdr_t *ethernet_hdr) {

    printf("%s : ARP Broadcast msg recvd on interface %s of node %s\n",
                __FUNCTION__, iif->if_name , iif->att_node->node_name);

   /* ARP broadcast request msg has passed MAC Address check*/
   /* Now, this node need to reply to this ARP Broadcast req
    * msg if Dst ip address in ARP req msg matches iif's ip address*/

    char ip_addr[16];
    arp_hdr_t *arp_hdr = (arp_hdr_t *)GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr);

    unsigned int arp_dst_ip = htonl(arp_hdr->dst_ip);

    inet_ntop(AF_INET, &arp_dst_ip, ip_addr, 16);
    ip_addr[15] = '\0';

    if(strncmp(IF_IP(iif), ip_addr, 16)){

        printf("%s : ARP Broadcast req msg dropped, Dst IP address %s did not match with interface ip : %s\n",
                node->node_name, ip_addr , IF_IP(iif));
        return;
    }

   send_arp_reply_msg(ethernet_hdr, iif);
}

void
send_arp_broadcast_request(node_t *node,
                           interface_t *oif,
                           char *ip_addr) {
	printf("%s : ARP Broadcast msg send request for IP address %s of node %s\n",
                __FUNCTION__, ip_addr,node->node_name );

    /*Take memory which can accomodate Ethernet hdr + ARP hdr*/
    unsigned int payload_size = sizeof(arp_hdr_t);
    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *)calloc(1,
                ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size);

    if(!oif){
        oif = node_get_matching_subnet_interface(node, ip_addr);
        if(!oif){
            printf("Error : %s : No eligible subnet for ARP resolution for Ip-address : %s",
                    node->node_name, ip_addr);
            return;
        }
        if(strncmp(IF_IP(oif), ip_addr, 16) == 0){
            printf("Error : %s : Attemp to resolve ARP for local Ip-address : %s",
                    node->node_name, ip_addr);
            return;
        }
    }

    /*STEP 1 : Prepare ethernet hdr*/
    layer2_fill_with_broadcast_mac(ethernet_hdr->dst_mac.mac);
    memcpy(ethernet_hdr->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));
    ethernet_hdr->type = ARP_MSG;

    /*Step 2 : Prepare ARP Broadcast Request Msg out of oif*/
    arp_hdr_t *arp_hdr = (arp_hdr_t *) ethernet_hdr->payload;
    arp_hdr->hw_type = 1;
    arp_hdr->proto_type = 0x0800;
    arp_hdr->hw_addr_len = sizeof(mac_add_t);
    arp_hdr->proto_addr_len = 4;

    arp_hdr->op_code = ARP_BROAD_REQ;

    memcpy(arp_hdr->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));

    inet_pton(AF_INET, IF_IP(oif), &arp_hdr->src_ip);
    arp_hdr->src_ip = htonl(arp_hdr->src_ip);

    memset(arp_hdr->dst_mac.mac, 0,  sizeof(mac_add_t));

    inet_pton(AF_INET, ip_addr, &arp_hdr->dst_ip);
    arp_hdr->dst_ip = htonl(arp_hdr->dst_ip);

    SET_COMMON_ETH_FCS(ethernet_hdr, sizeof(arp_hdr_t), 0); /*Not used*/
    /*STEP 3 : Now dispatch the ARP Broadcast Request Packet out of interface*/
    send_pkt_out((char *)ethernet_hdr, ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size,oif);
    free(ethernet_hdr);
}

