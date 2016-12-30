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
	newNode->next = list->head;
	list->head = newNode->next;
	++list->size;
}

int intlist_size(intlist* list){
	return list->size;
}

void intlist_remove_last_k(intlist* list, int k){
	Node* temp;
	if(k <= list->size - k){
		temp = list->tail;
		while(k > 1){
			temp = temp->prev;
			--k;
		}
	}
	else{
		temp = list->head;
		int size = list->size - k;
		while(size > 0){
			temp = temp->next;
			--size;
		}
	}
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

int main() {
	int arr[] = {1,2,3,4,5,6};
	Node* head = createFromArray(arr,6);
	printList(head);
	head = reverseList(head);
	printList(head);
	destroyList(head);
	return 0;
}



