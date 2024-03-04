#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"
#include "error_codes.h"
#include "dll.h"
#include "cmdtlv.h"
#include "libcli.h"
#include "graph.h"
//node_t *root = NULL;
//node_t *root = NULL;
//
#define CMCODE_SHOW_NODE	1

node_t *root = NULL;
//dll_t *head = NULL;

#if 1
int
node_callback_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable){
    printf("%s() is called ...\n", __FUNCTION__);
   // traverse_dll(&head);
    return 0;
}

int
validate_node_name(char *value){

    printf("%s() is called with value = %s\n", __FUNCTION__, value);
    return VALIDATION_SUCCESS; /*else return VALIDATION_FAILED*/
}

#endif
int main() {
#if 1
	init_libcli();

    	param_t *show   = libcli_get_show_hook();
    	param_t *debug  = libcli_get_debug_hook();
    	param_t *config = libcli_get_config_hook();
   	param_t *clear  = libcli_get_clear_hook();
    	param_t *run 	= libcli_get_run_hook();
	int ret;	
	
	/*Implementing CMD1: show node dll*/
	{	
		/*show node*/
		static param_t node;
		init_param(&node,
			   CMD,
			   "node",
			   0,
			   0,
			   INVALID,
			   0,
			   "Help: Traverse DLL");
		libcli_register_param(show,&node);
		
		{
			static param_t node_name;
			init_param(&node_name,
			   	  LEAF,
			   	  0,
			   	  node_callback_handler,
			   	  validate_node_name,
			   	  STRING,
			   	  "node-action",
			   	  "Help: node action");
		libcli_register_param(&node,&node_name);
		set_param_cmd_code(&node_name, CMCODE_SHOW_NODE);
		}
	}
#endif
#if 0	
	ret = insert(&root, 2); 
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	
	ret = insert(&root, 3);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	
	ret = insert(&root, 5);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	
	ret = insert(&root, 1);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	
	traverse_ll(root);
	
	ret = insert(&root, 1);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
#endif
	//printf("\nAddress of root = %p\n",&root);
	//printf("\nAddress of root1 = %p\n",root);
	//printf("%s\n",__func__);
//DLL
#if 0
	dll_emp_t *emp1 = (dll_emp_t *) malloc(sizeof(dll_emp_t));
	dll_emp_t *emp2 = (dll_emp_t *) malloc(sizeof(dll_emp_t));
	dll_emp_t *emp3 = (dll_emp_t *) malloc(sizeof(dll_emp_t));
	dll_emp_t *emp4 = (dll_emp_t *) malloc(sizeof(dll_emp_t));
//	emp1->emp_name = (emp_t *) malloc(sizeof("Arun")+1);
	strcpy(emp1->emp_name,"Arun");
	strcpy(emp1->emp_project,"SAT");
	emp1->emp_salary = 100000;
	emp1->emp_num = 1111;
	strcpy(emp2->emp_name,"Kiran");
	strcpy(emp2->emp_project,"PAT");
	emp2->emp_salary = 100001;
	emp2->emp_num = 1112;
	strcpy(emp3->emp_name,"Parun");
	strcpy(emp3->emp_project,"TAT");
	emp3->emp_salary = 100002;
	emp3->emp_num = 1113;
	strcpy(emp4->emp_name,"Karan");
	strcpy(emp4->emp_project,"MAT");
	emp4->emp_salary = 100003;
	emp4->emp_num = 1114;


#endif	
//Linked List
#if 0
	emp_t *emp1 = (emp_t *) malloc(sizeof(emp_t));
	emp_t *emp2 = (emp_t *) malloc(sizeof(emp_t));
	emp_t *emp3 = (emp_t *) malloc(sizeof(emp_t));
	emp_t *emp4 = (emp_t *) malloc(sizeof(emp_t));
//	emp1->emp_name = (emp_t *) malloc(sizeof("Arun")+1);
	strcpy(emp1->emp_name,"Arun");
	strcpy(emp1->emp_project,"SAT");
	emp1->emp_salary = 100000;
	emp1->emp_num = 1111;
	strcpy(emp2->emp_name,"Kiran");
	strcpy(emp2->emp_project,"PAT");
	emp2->emp_salary = 100001;
	emp2->emp_num = 1112;
	strcpy(emp3->emp_name,"Parun");
	strcpy(emp3->emp_project,"TAT");
	emp3->emp_salary = 100002;
	emp3->emp_num = 1113;
	strcpy(emp4->emp_name,"Karan");
	strcpy(emp4->emp_project,"MAT");
	emp4->emp_salary = 100003;
	emp4->emp_num = 1114;
#endif
//	printf("\n %s %d %d %s \n",emp1->emp_name,emp1->emp_num,emp1->emp_salary,emp1->emp_project);
	//printf("Data2 = %p\n",emp1);
#if 0
	ret = insert(&root, emp1);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	ret = insert(&root, emp2);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	
	ret = insert(&root, emp3);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	
	ret = insert(&root, emp4);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}

	//traverse_ll(&root);
	TRAVERSE_LINKED_LIST(&root);
	search_ll(&root,1116);
#endif

#if 0
	ret = dll_insert(&head, emp1);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	ret = dll_insert(&head, emp2);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	
	ret = dll_insert(&head, emp3);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}
	
	ret = dll_insert(&head, emp4);
	if (ret < 1) {
		printf("\nFAILURE!!\n");
	}

	//traverse_dll(&head);

	//TRAVERSE_DLL(&head);

//	printf("Offset = %d\n",offset(dll_emp_t, emp_project));
#endif	
/*Graph Implementation*/
#if 1
	build_graph_topo();
#endif
	//support_cmd_negation(config);

    	//start_shell();

	return SUCCESS;
}
