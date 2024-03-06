#include "graph.h"
#include <stdio.h>
#include "libcli.h"
#include "cmdtlv.h"
#include "cmdcodes.h"



extern graph_t *topo;

//node_t *root = NULL;

/*Generic Topology Commands*/
static int
show_nw_topology_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable){
    printf("%s() is called ...\n", __FUNCTION__);
    int CMDCODE = 1;
    CMDCODE = EXTRACT_CMD_CODE(tlv_buf);

    switch(CMDCODE){
	case CMDCODE_SHOW_NW_TOPOLOGY :
		printf("\nComing in the case\n");
		dump_nw_graph(topo);
		break;
	default:
		;}
    return 0;
}

void nw_init_cli(){
	
	init_libcli();

    	param_t *show   = libcli_get_show_hook();
    	param_t *debug  = libcli_get_debug_hook();
    	param_t *config = libcli_get_config_hook();
   	param_t *clear  = libcli_get_clear_hook();
    	param_t *run 	= libcli_get_run_hook();
	int ret;	
	
	/*Implementing CMD1: show node dll*/
	{	
		/*show topology*/
		static param_t topology;
		init_param(&topology, CMD, "topology",show_nw_topology_handler, 0, INVALID, 0, "Dump Complete Network topology");
		libcli_register_param(show,&topology);
		set_param_cmd_code(&topology, CMDCODE_SHOW_NW_TOPOLOGY);
	}
	support_cmd_negation(config);
}
