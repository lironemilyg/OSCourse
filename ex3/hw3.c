/*
 * ass3.c
 *
 *  Created on: Dec 29, 2016
 *      Author: lirongazit
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>


typedef struct node {
	int data;
	struct node* next;
	struct node* prev;
} Node;

typedef struct int_l{
	Node* head;
	Node* tail;
	int size;
	pthread_mutex_t lock;
}intlist;

void intlist_init(intlist* list){
	list->size = 0;
	list->head = NULL;
	list->tail = NULL;
}

void nodeList_destroy(Node* head){
	if(!head){
		return;
	}
	nodeList_destroy(head->next);
	free(head);
}

void intlist_destroy(intlist* list){
	if(!list){
		return;
	}
	nodeList_destroy(list->head);
	free(list);
}

void intlist_push_head(intlist* list, int value){
	Node* newNode = (Node*) malloc(sizeof(*newNode));
	newNode->prev = NULL;
	if(list->head == NULL && list->tail == NULL){
		list->tail = newNode;
		newNode->next = NULL;
	}
	else{
		newNode->next = list->head;
		list->head->prev = newNode;
	}
	list->head = newNode;
	++list->size;
}

int intlist_size(intlist* list){
	return list->size;
}

void intlist_remove_last_k(intlist* list, int k){
	Node* temp;
	if(k > list->size){
		k = list->size;
		temp = list->head;
	}
//	if(k <= list->size - k){
	else {
		temp = list->tail;
		while (k > 1) {
			temp = temp->prev;
			--k;
		}
	}
//	}
//	else{
//		temp = list->head;
//		int size = list->size - k;
//		while(size > 0){
//			temp = temp->next;
//			--size;
//		}
//	}
	temp->prev->next = NULL;
	temp->prev = NULL;
	list->size = list->size - k;
	nodeList_destroy(temp);
}

int intlist_pop_tail(intlist* list){
	Node* temp;
	int val;
	temp = list->tail;
	temp->prev->next = NULL;
	temp->prev = NULL;
	--list->size;
	val = temp->data;
	nodeList_destroy(temp);
	return val;
}

pthread_mutex_t* intlist_get_mutex(intlist* list){
	return &list->lock;
}


void printList(Node* head) {
	Node* current = head;
	while (current != NULL) {
		printf("%d", current->data);
		if (!current->next) {
			putchar('\n');
		} else {
			putchar(' ');
		}
		current = current->next;
	}
}

int main() {
	intlist* list = (intlist*) malloc(sizeof(*list));;
	intlist_init(list);
	intlist_push_head(list,8);
	intlist_push_head(list,7);
	intlist_push_head(list,6);
	intlist_push_head(list,5);
	intlist_push_head(list,4);
	intlist_push_head(list,3);
	intlist_push_head(list,2);
	intlist_push_head(list,1);
	printf("size is = %d \n",intlist_size(list));
	printList(list->head);
//	intlist_remove_last_k(list,3);
//	int res = intlist_pop_tail(list);
//	printf("res pop need to be 4 - result = %d \n",res);
//	intlist_destroy(list);
	printf("Finishing...........\n");
}
