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
	newNode->data = value;
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
		list->head = NULL;
		list->tail = NULL;
	}
	else {
		temp = list->tail;
		while (k > 1) {
			temp = temp->prev;
			--k;
		}
		list->tail = temp->prev;
		list->tail->next = NULL;
	}
	printf("temp data is = %d \n",temp->data);
	temp->prev = NULL;
	list->size = list->size - k;
	nodeList_destroy(temp);
}

int intlist_pop_tail(intlist* list){
	Node* temp;
	int val;
	temp = list->tail;
	list->tail = list->tail->prev;
	temp->prev->next = NULL;
	temp->prev = NULL;
	--list->size;
	val = temp->data;
	printf("data is %d",val);
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

void printListRev(Node* tail) {
	Node* current = tail;
	while (current != NULL) {
		printf("%d", current->data);
		if (!current->prev) {
			putchar('\n');
		} else {
			putchar(' ');
		}
		current = current->prev;
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
//	printf("size is = %d \n",intlist_size(list));
	printList(list->head);
	intlist_remove_last_k(list,3);
	printList(list->head);
//	printListRev(list->tail);
	int res = intlist_pop_tail(list);
	printf("res pop need to be 5 - result = %d \n",res);
	printList(list->head);

	intlist_destroy(list);
	printf("\nFinishing...........\n");
}
