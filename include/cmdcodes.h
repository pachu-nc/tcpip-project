#ifndef __CMDCODES_H__
#define __CMDCODES_H__

#define CMDCODE_SHOW_NW_TOPOLOGY    1   /*show topology*/
#define CMDCODE_PING                2   /*run <node-name> ping <ip-address>*/
#define CMDCODE_SHOW_NODE_ARP_TABLE 3   /*show node <node-name> arp*/
#define CMDCODE_RUN_ARP             4   /*run node <node-name> resolve-arp <ip-address>*/
#define CMDCODE_INTF_CONFIG_L2_MODE 5   /*config node <node-name> interface <intf-name> l2mode <access|trunk>*/
#define CMDCODE_INTF_CONFIG_IP_ADDR 6   /*config node <node-name> interface <intf-name> ip-address <ip-address> <mask>*/
#define CMDCODE_INTF_CONFIG_VLAN    7   /*config node <node-name> interface <intf-name> vlan <vlan-id>*/
#define CMDCODE_SHOW_NODE_MAC_TABLE 8   /*show node <node-name> mac*/
#define CMDCODE_SHOW_NODE_RT_TABLE  9   /*show node <node-name> rt*/
#define CMDCODE_CONF_NODE_L3ROUTE   10  /*config node <node-name> route <ip-address> <mask> [<gw-ip> <oif>]*/
#define CMDCODE_ERO_PING            11  /*run <node-name> ping <ip-address> ero <ero-ip-address>*/

#define CMDCODE_UNUSED_1            12  /*Not used*/
#define CMDCODE_SHOW_INTF_STATS     13     /*show node <node-name> interface statistics*/
#define CMDCODE_DEBUG_SHOW_NODE_TIMER   14 /*debug show node <node-name> timer*/

#define CMDCODE_RUN_DDCP_QUERY      15  /*run node <node-name> ddcp-query*/
#define CMDCODE_SHOW_DDCP_DB        16  /*show node <node-name> ddcp-db*/
#define CMDCODE_RUN_DDCP_QUERY_PERIODIC 17 /*run node <node-name> ddcp-query periodic <ddcp-q-interval in sec>*/

#define CMDCODE_RUN_SPF             18  /*run node <node-name> spf*/
#define CMDCODE_SHOW_SPF_RESULTS    19  /*show node <node-name> spf-results*/
#define CMDCODE_RUN_SPF_ALL         20  /*run spf all*/

//Logging and Debugging
#define CMDCODE_DEBUG_LOGGING_PER_NODE   21  /*config node <node-name> traceoptions flag <all | no-all | recv | no-recv | send | no-send | stdout | no-stdout>*/
#define CMDCODE_DEBUG_LOGGING_PER_INTF   22  /*config node <node-name> interface <intf-name> traceoptions flag <all | no-all | recv | no-recv | send | no-send | stdout | no-stdout>*/
#define CMDCODE_DEBUG_SHOW_LOG_STATUS    23  /*show node <node-name> log-status*/
#define CMDCODE_DEBUG_GLOBAL_STDOUT      24  /*config global stdout*/
#define CMDCODE_DEBUG_GLOBAL_NO_STDOUT   25  /*config global no-stdout*/
/*Interface Up Down*/
#define CMDCODE_CONF_INTF_UP_DOWN        26 /*config node <node-name> interface <if-name> <up|down>*/

/*Nbrship Mgmt Protocol*/
#define CMDCODE_CONF_NODE_INTF_NBRSHIP_ENABLE       27 /*config node <node-name> [no] nbrship interface <intf-name>*/
#define CMDCODE_CONF_NODE_INTF_ALL_NBRSHIP_ENABLE   28 /*config node <node-name> [no] nbrship interface all*/
#define CMDCODE_CONF_NODE_NBRSHIP_ENABLE            29 /*conf node <node-name> [no] protocol nmp*/
#define CMDCODE_SHOW_NODE_NBRSHIP                   30 /*show node <node-name> nmp nbrships*/
#define CMDCODE_SHOW_NODE_NMP_PROTOCOL_ALL_INTF_STATS  31 /*show node <node-name> interface statistics protocol nmp*/
#define CMDCODE_SHOW_NODE_NMP_STATE                    32 /*show node <node-name> nmp state*/

#define CMDCODE_INTF_CONFIG_METRIC       33 /*config node <node-name> interface <if-name> metric <metric-val>*/


#endif /*__CMDCODES_H */
