#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"
#include "error_codes.h"
//node_t *root = NULL;
//node_t *root = NULL;

int main() {
	int ret;	
	node_t *root = NULL;
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

//	printf("\n %s %d %d %s \n",emp1->emp_name,emp1->emp_num,emp1->emp_salary,emp1->emp_project);
	//printf("Data2 = %p\n",emp1);
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
	return SUCCESS;
}
