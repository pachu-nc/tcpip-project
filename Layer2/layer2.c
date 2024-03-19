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


static void
promote_pkt_to_layer2(node_t *node, interface_t *iif,
        ethernet_hdr_t *ethernet_hdr,
        uint32_t pkt_size){

    switch(ethernet_hdr->type){
        case ARP_MSG:
            {
                /*Can be ARP Broadcast or ARP reply*/
                arp_hdr_t *arp_hdr = (arp_hdr_t *)(GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr));
                switch(arp_hdr->op_code){
                    case ARP_BROAD_REQ:
                        process_arp_broadcast_request(node, iif, ethernet_hdr);
                        break;
                    case ARP_REPLY:
                        process_arp_reply_msg(node, iif, ethernet_hdr);
                        break;
                    default:
                        break;
                }
            }
            break;
        case ETH_IP:
            //promote_pkt_to_layer3(node, iif,
              //      GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr),
                //    pkt_size - GET_ETH_HDR_SIZE_EXCL_PAYLOAD(ethernet_hdr),
                  //  ethernet_hdr->type);
            break;
        default:
            ;
    }
}

void
layer2_frame_recv(node_t *node, interface_t *interface, 
		char *pkt, unsigned int pkt_size){
	/*Entry point into the TCP/IP stack from the bottom*/

	unsigned int vlan_id_to_tag = 0;

    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *)pkt;

    if(l2_frame_recv_qualify_on_interface(interface,
                                          ethernet_hdr, &vlan_id_to_tag) == FALSE){

        printf("L2 Frame Rejected on node %s\n", node->node_name);
        return;
    }

    printf("L2 Frame Accepted on node %s \n", node->node_name);
	
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

/*Interface config APIs for L2 mode configuration*/

void
interface_set_l2_mode(node_t *node,
                      interface_t *interface,
                      char *l2_mode_option){

    intf_l2_mode_t intf_l2_mode;

    if(strncmp(l2_mode_option, "access", strlen("access")) == 0){
        intf_l2_mode = ACCESS;
    }
    else if(strncmp(l2_mode_option, "trunk", strlen("trunk")) ==0){
        intf_l2_mode = TRUNK;
    }
    else{
        assert(0);
    }

#if 1
    /*Case 1 : if interface is working in L3 mode, i.e. IP address is configured.
     * then disable ip address, and set interface in L2 mode*/
    if(IS_INTF_L3_MODE(interface)){
        interface->intf_nw_prop.is_ipadd_config_backup = TRUE;
        interface->intf_nw_prop.is_ipadd_config = FALSE;

        IF_L2_MODE(interface) = intf_l2_mode;
        return;
    }
#endif
    
    /*Case 2 : if interface is working neither in L2 mode or L3 mode, then
     * apply L2 config*/
    if(IF_L2_MODE(interface) == L2_MODE_UNKNOWN){
        IF_L2_MODE(interface) = intf_l2_mode;
        return;
    }

    /*case 3 : if interface is operating in same mode, and user config same mode
     * again, then do nothing*/
    if(IF_L2_MODE(interface) == intf_l2_mode){
        return;
    }

    /*case 4 : if interface is operating in access mode, and user config trunk mode,
     * then overwrite*/
    if(IF_L2_MODE(interface) == ACCESS &&
            intf_l2_mode == TRUNK){
        IF_L2_MODE(interface) = intf_l2_mode;
        return;
    }

    /* case 5 : if interface is operating in trunk mode, and user config access mode,
     * then overwrite, remove all vlans from interface, user must enable vlan again
     * on interface*/
    if(IF_L2_MODE(interface) == TRUNK &&
           intf_l2_mode == ACCESS){

        IF_L2_MODE(interface) = intf_l2_mode;

        unsigned int i = 0;
	
	for ( ; i < MAX_VLAN_MEMBERSHIP; i++){
            interface->intf_nw_prop.vlans[i] = 0;
        }
    }
}


/*APIs to be used to create topologies*/
void
node_set_intf_l2_mode(node_t *node, char *intf_name,
                        intf_l2_mode_t intf_l2_mode){

    interface_t *interface = get_node_if_by_name(node, intf_name);
    assert(interface);

    interface_set_l2_mode(node, interface, intf_l2_mode_str(intf_l2_mode));
}


void
interface_set_vlan(node_t *node,
                   interface_t *interface,
                   unsigned int vlan_id){

    /* Case 1 : Cant set vlans on interface configured with ip
     * address*/
    if(IS_INTF_L3_MODE(interface)){
        printf("Error : Interface %s : L3 mode enabled\n", interface->if_name);
        return;
    }

    /*Case 2 : Cant set vlan on interface not operating in L2 mode*/
    if(IF_L2_MODE(interface) != ACCESS &&
        IF_L2_MODE(interface) != TRUNK){
        printf("Error : Interface %s : L2 mode not Enabled\n", interface->if_name);
        return;
    }

    /*case 3 : Can set only one vlan on interface operating in ACCESS mode*/
    if(interface->intf_nw_prop.intf_l2_mode == ACCESS){

        unsigned int i = 0, *vlan = NULL;
        for( ; i < MAX_VLAN_MEMBERSHIP; i++){
            if(interface->intf_nw_prop.vlans[i]){
                vlan = &interface->intf_nw_prop.vlans[i];
            }
        }
        if(vlan){
            *vlan = vlan_id;
            return;
        }
        interface->intf_nw_prop.vlans[0] = vlan_id;
    }
    /*case 4 : Add vlan membership on interface operating in TRUNK mode*/
    if(interface->intf_nw_prop.intf_l2_mode == TRUNK){

        unsigned int i = 0, *vlan = NULL;

        for( ; i < MAX_VLAN_MEMBERSHIP; i++){

            if(!vlan && interface->intf_nw_prop.vlans[i] == 0){
                vlan = &interface->intf_nw_prop.vlans[i];
            }
            else if(interface->intf_nw_prop.vlans[i] == vlan_id){
                return;
            }
        }
        if(vlan){
            *vlan = vlan_id;
            return;
        }
        printf("Error : Interface %s : Max Vlan membership limit reached", interface->if_name);
    }
}

void
node_set_intf_vlan_membsership(node_t *node, char *intf_name,
                                unsigned int vlan_id){

    interface_t *interface = get_node_if_by_name(node, intf_name);
    assert(interface);

    interface_set_vlan(node, interface, vlan_id);
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
/*Vlan Management Routines*/

/* Return new packet size if pkt is tagged with new vlan id*/

ethernet_hdr_t *
tag_pkt_with_vlan_id(ethernet_hdr_t *ethernet_hdr,
                     unsigned int total_pkt_size,
                     int vlan_id,
                     unsigned int *new_pkt_size){

    unsigned int payload_size  = 0 ;
    *new_pkt_size = 0;

    /*If the pkt is already tagged, replace it*/
    vlan_8021q_hdr_t *vlan_8021q_hdr =
        is_pkt_vlan_tagged(ethernet_hdr);


    if(vlan_8021q_hdr){
        payload_size = total_pkt_size - VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;
        vlan_8021q_hdr->tci_vid = (short)vlan_id;

        /*Update checksum, however not used*/
        SET_COMMON_ETH_FCS(ethernet_hdr, payload_size, 0);

        *new_pkt_size = total_pkt_size;
        return ethernet_hdr;
    }

    /*If the pkt is not already tagged, tag it*/
    /*Fix me : Avoid declaring local variables of type
     ethernet_hdr_t or vlan_ethernet_hdr_t as the size of these
     variables are too large and is not healthy for program stack
     memory*/
    ethernet_hdr_t ethernet_hdr_old;
    memcpy((char *)&ethernet_hdr_old, (char *)ethernet_hdr,
                ETH_HDR_SIZE_EXCL_PAYLOAD - sizeof(ethernet_hdr_old.FCS));

    payload_size = total_pkt_size - ETH_HDR_SIZE_EXCL_PAYLOAD;
    vlan_ethernet_hdr_t *vlan_ethernet_hdr =
            (vlan_ethernet_hdr_t *)((char *)ethernet_hdr - sizeof(vlan_8021q_hdr_t));

    memset((char *)vlan_ethernet_hdr, 0,
                VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD - sizeof(vlan_ethernet_hdr->FCS));
    memcpy(vlan_ethernet_hdr->dst_mac.mac, ethernet_hdr_old.dst_mac.mac, sizeof(mac_add_t));
    memcpy(vlan_ethernet_hdr->src_mac.mac, ethernet_hdr_old.src_mac.mac, sizeof(mac_add_t));

    /*Come to 802.1Q vlan hdr*/
    vlan_ethernet_hdr->vlan_8021q_hdr.tpid = VLAN_8021Q_PROTO;
    vlan_ethernet_hdr->vlan_8021q_hdr.tci_pcp = 0;
    vlan_ethernet_hdr->vlan_8021q_hdr.tci_dei = 0;
    vlan_ethernet_hdr->vlan_8021q_hdr.tci_vid = (short)vlan_id;

    /*Type field*/
    vlan_ethernet_hdr->type = ethernet_hdr_old.type;
    /*No need to copy data*/

    /*Update checksum, however not used*/
    SET_COMMON_ETH_FCS((ethernet_hdr_t *)vlan_ethernet_hdr, payload_size, 0 );
    *new_pkt_size = total_pkt_size  + sizeof(vlan_8021q_hdr_t);
    return (ethernet_hdr_t *)vlan_ethernet_hdr;
}

/* Return new packet size if pkt is untagged with the existing
 * vlan 801.1q hdr*/
ethernet_hdr_t *
untag_pkt_with_vlan_id(ethernet_hdr_t *ethernet_hdr,
                     unsigned int total_pkt_size,
                     unsigned int *new_pkt_size){

    *new_pkt_size = 0;

    vlan_8021q_hdr_t *vlan_8021q_hdr =
        is_pkt_vlan_tagged(ethernet_hdr);

    /*NOt tagged already, do nothing*/
    if(!vlan_8021q_hdr){
        *new_pkt_size = total_pkt_size;
        return ethernet_hdr;
    }

    /*Fix me : Avoid declaring local variables of type
     ethernet_hdr_t or vlan_ethernet_hdr_t as the size of these
     variables are too large and is not healthy for program stack
     memory*/
    vlan_ethernet_hdr_t vlan_ethernet_hdr_old;
    memcpy((char *)&vlan_ethernet_hdr_old, (char *)ethernet_hdr,
                VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD - sizeof(vlan_ethernet_hdr_old.FCS));

    ethernet_hdr = (ethernet_hdr_t *)((char *)ethernet_hdr + sizeof(vlan_8021q_hdr_t));

    memcpy(ethernet_hdr->dst_mac.mac, vlan_ethernet_hdr_old.dst_mac.mac, sizeof(mac_add_t));
    memcpy(ethernet_hdr->src_mac.mac, vlan_ethernet_hdr_old.src_mac.mac, sizeof(mac_add_t));

    ethernet_hdr->type = vlan_ethernet_hdr_old.type;

    /*No need to copy data*/
    unsigned int payload_size = total_pkt_size - VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;

    /*Update checksum, however not used*/
    SET_COMMON_ETH_FCS(ethernet_hdr, payload_size, 0);

    *new_pkt_size = total_pkt_size - sizeof(vlan_8021q_hdr_t);
    return ethernet_hdr;
}


