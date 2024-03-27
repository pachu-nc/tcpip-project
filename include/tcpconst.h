#ifndef __TCPCONST__
#define __TCPCONST__

/*Specified in ethernet_hdr->type*/
#define ARP_BROAD_REQ   1
#define ARP_REPLY       2
#define ARP_MSG         806
#define BROADCAST_MAC   0xFFFFFFFFFFFF
#define ETH_IP          0x0800
#define ICMP_PRO        1
#define ICMP_ECHO_REQ   8
#define ICMP_ECHO_REP   0
#define MTCP            20
#define USERAPP1        21
#define VLAN_8021Q_PROTO    0x8100
#define IP_IN_IP        4

#define NMP_HELLO_MSG_CODE      13 /*Randomly chosen*/
#define INTF_MAX_METRIC     16777215 /*Choosen as per the standard = 2^24 -1*/
#define INTF_METRIC_DEFAULT 1

/*Add DDCP Protocol Numbers*/
#define DDCP_MSG_TYPE_FLOOD_QUERY    1  /*Randomly chosen, should not exceed 2^16 -1*/
#define DDCP_MSG_TYPE_UCAST_REPLY    2  /*Randomly chosen, must not exceed 255*/
#define PKT_BUFFER_RIGHT_ROOM        128
#define MAX_NXT_HOPS        4

#define IP_HDR_INCLUDED (1  << 0)
#define DATA_LINK_HDR_INCLUDED  (1 << 1)


/*Dynamic Registration of Protocol with TCP/IP stack*/
#define MAX_L2_PROTO_INCLUSION_SUPPORTED    16
#define MAX_L3_PROTO_INCLUSION_SUPPORTED    16

#endif /*__TCPCONST__*/
