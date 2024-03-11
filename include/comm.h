#ifndef __COMM_H__
#define __COMM_H__
#include "graph.h"
#define MAX_PACKET_BUFFER_SIZE   2048

int send_pkt_out(char *, unsigned int, interface_t *);
int pkt_receive(node_t *, interface_t *, char *, unsigned int);
#endif /*_CCOMM_H_*/
