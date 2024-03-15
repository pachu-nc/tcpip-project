#include "comm.h"
#include "graph.h"
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h> /*for struct hostent*/
#include "net.h"
#include <unistd.h> // for close

int set_fd_parameter;
int check_set_fdi_parameter;
int set_fd;
extern node_t *return_glnode_pointer(glthread_t *node);

unsigned int udp_port_number = 40000 ;
static char recv_buffer[MAX_PACKET_BUFFER_SIZE];
static char send_buffer[MAX_PACKET_BUFFER_SIZE];

static unsigned int get_next_udp_port_number() {
	return udp_port_number++;
}

void init_udp_socket(node_t *node){
	node->udp_port_number = get_next_udp_port_number();

	/*Create a UDP socket*/

	int udp_sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	/*Bind the socket*/

	struct sockaddr_in node_addr;
	node_addr.sin_family 		= AF_INET;
	node_addr.sin_port		= node->udp_port_number;
	node_addr.sin_addr.s_addr	= INADDR_ANY;

	if(bind(udp_sock_fd, (struct sockaddr *)&node_addr, sizeof(struct sockaddr)) == -1 ) {
		printf("Error: Socket bind failed for node %s\n", node->node_name );
		return;
	}
	node->udp_sock_fd = udp_sock_fd;
}


static int _send_pkt_out(int sock_fd, char* pkt_data, unsigned int pkt_size,
				unsigned int dest_udp_port_num){
	int rc;
	struct sockaddr_in dest_addr;

	struct hostent *host = (struct hostent *) gethostbyname("127.0.0.1");
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = dest_udp_port_num;
	dest_addr.sin_addr = *((struct in_addr *)host->h_addr);

	rc = sendto(sock_fd, pkt_data, pkt_size, 0,
			(struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
	return rc;

}
extern void 
layer2_frame_recv(node_t *node, interface_t *interface, 
		char *pkt, unsigned int pkt_size);

int pkt_receive(node_t *node, interface_t *interface, 
		char *pkt, unsigned int pkt_size){
	/*This function is entry point of data into the data link layer 
	 * from the physical layer. Ingress journey of the packet 
	 * starts from here in the TCP/IP stack.*/
	
	/*Make room in the packet buffer by shifting the data towards 
	 * right, so that the tcp/ip stack can append more headers to 
	 * the packet as required*/
	
	pkt = pkt_buffer_shift_right(pkt, pkt_size, MAX_PACKET_BUFFER_SIZE - MAX_NAME_SIZE);
	
	/*Do further processing of the packet*/
	layer2_frame_recv(node, interface, pkt, pkt_size);


	return 0;
}
static void _pkt_receive(node_t *receiving_node, 
				char *pkt_with_aux_data,
				unsigned int pkt_size){
	char *recv_intf_name = pkt_with_aux_data;
	interface_t *recv_intf = get_node_if_by_name(receiving_node, recv_intf_name);
	
	if(!recv_intf){
		printf("\nError: Packet received on unknown interface %s on node %s\n",
				recv_intf->if_name, receiving_node->node_name);
		return;
	}
	
	pkt_receive(receiving_node, recv_intf, pkt_with_aux_data+ MAX_NAME_SIZE,
			pkt_size - MAX_NAME_SIZE);
}

/*Public API- used by othre modules*/

int
send_pkt_out(char *pkt, unsigned int pkt_size,
		interface_t *interface){
	int rc; 
	node_t *sending_node = interface->att_node;
	node_t *nbr_node = get_nbr_node(interface);
	
	if(!nbr_node)
		return -1;
	
	if(pkt_size + MAX_NAME_SIZE > MAX_PACKET_BUFFER_SIZE){
        printf("Error : Node :%s, Pkt Size exceeded\n", sending_node->node_name);
        return -1;
    	}

	unsigned int dst_udp_port_num = nbr_node->udp_port_number;

	/*Sending node from one node to the other using UDP socket*/	
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0){
		printf("\nError: Sending socket creation failed. Errno = %d \n",errno);
		return -1;
	}
	
	/*get the name of the other interface to which to send the data.*/	

	interface_t *other_interface = &interface->link->intf1 == interface ? \
					&interface->link->intf2 : &interface->link->intf1;

	memset(send_buffer, 0 , MAX_PACKET_BUFFER_SIZE);

	char *pkt_with_aux_data = send_buffer;
	
	/*Copy the name of the interface in the send buffer*/
	strncpy(pkt_with_aux_data, other_interface->if_name, MAX_NAME_SIZE);
	pkt_with_aux_data[MAX_NAME_SIZE -1 ] = '\0';

	/*Copy the actual data in the send buffer*/
	memcpy(pkt_with_aux_data + MAX_NAME_SIZE , pkt, pkt_size);
	
	/*Send the data out*/
	
	rc = _send_pkt_out(sock, pkt_with_aux_data, pkt_size + MAX_NAME_SIZE, 
				dst_udp_port_num);
	close(sock);
	return rc;
}
static void *
_network_start_receiver_thread( void * arg){
	node_t *node;
	glthread_t *curr;
	
	/*fd_set is a collection of all the socket descriptors. All the socket descriptors are to be added to it in order to monitor them.*/

	fd_set active_sock_fd_set, backup_sock_fd_set;

	int sock_max_fd = 0;
	int bytes_recvd = 0;
	
	graph_t *topo = (void *) arg;

	int addr_len = sizeof(struct sockaddr);
	/*Initialize the socket descriptors to 0*/
	FD_ZERO(&active_sock_fd_set);
	FD_ZERO(&backup_sock_fd_set);

	/*Iterate through all the socket descriptors of the node, and add them to fd_set data structure*/

	struct sockaddr_in sender_addr;
	
	ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr){
		node = return_glnode_pointer(curr);
		if(!node->udp_sock_fd){
			continue;
		}
		if(node->udp_sock_fd > sock_max_fd)
			sock_max_fd = node->udp_sock_fd;

		FD_SET(node->udp_sock_fd, &backup_sock_fd_set);
	} ITERATE_GLTHREAD_END(&topo->node_list, curr)

	while(1) {

		/*Copy all the udp_sock_fd from backup to active*/
		memcpy(&active_sock_fd_set, &backup_sock_fd_set, sizeof(fd_set));
		
		/*select has max socketfd +1, select system call is a blocking system call. 
		 * i.e. as long active_sock_fd_set is activated the select systems call 
		 * remains blocked. */
		select(sock_max_fd +1, &active_sock_fd_set, NULL, NULL, NULL);
		
		/*These lines below select is called or invoked only if there is change in
		 * the fd that the select call is monitoring.*/	

		/*Check if the data has arrived on any node by traversing the node.*/
		ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr){
			node = return_glnode_pointer(curr);
			
			if(FD_ISSET(node->udp_sock_fd, &active_sock_fd_set)){

				memset(recv_buffer, 0, MAX_PACKET_BUFFER_SIZE);
				bytes_recvd = recvfrom(node->udp_sock_fd, (char *)recv_buffer, 
							MAX_PACKET_BUFFER_SIZE, 0,
							(struct sockaddr *)&sender_addr,&addr_len);
				_pkt_receive(node, recv_buffer, bytes_recvd);
			}
		}ITERATE_GLTHREAD_END(&topo->node_list, curr)
	} //while(1)
}
void network_start_pkt_receiver_thread(graph_t *topo){
	pthread_attr_t attr;
	pthread_t recv_pkt_thread;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	/*Forking a thread*/

	pthread_create(&recv_pkt_thread, &attr,
			_network_start_receiver_thread,
			(void *)topo);

}
