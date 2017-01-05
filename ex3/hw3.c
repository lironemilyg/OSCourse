/*
 * ass3.c
 *
 *  Created on: Dec 29, 2016
 *      Author: lirongazit
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

typedef struct node {
	int data;
	struct node* next;
	struct node* prev;
} Node;

typedef struct int_l {
	Node* head;
	Node* tail;
	int size;
	pthread_cond_t pop_cond;
	pthread_mutex_t lock;
	pthread_mutexattr_t attr;
} intlist;

intlist list;
pthread_cond_t garbage_collector_cond;
bool writer_flag;
bool reader_flag;
bool gc_flag;
int max;

void intlist_init(intlist* list) {
	list->size = 0;
	list->head = NULL;
	list->tail = NULL;
	if (pthread_mutexattr_init(&list->attr) != 0) {
		perror("mutexattr init failed\n");
		exit(-1);
	}
	if (pthread_mutexattr_settype(&list->attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
		perror("mutexattr settype failed\n");
		exit(-1);
	}
	if (pthread_mutex_init(&list->lock, &list->attr) != 0) {
		perror("mutex init failed\n");
		exit(-1);
	}
	if (pthread_cond_init(&list->pop_cond, NULL) != 0) {
		perror("cond init failed\n");
		exit(-1);
	}
}

void nodeList_destroy(Node* head) {
	if (!head) {
		return;
	}
	nodeList_destroy(head->next);
	free(head);
}

void intlist_destroy(intlist* list) {
	if (!list) {
		return;
	}
	nodeList_destroy(list->head);
	if (pthread_mutex_destroy(&list->lock) != 0) {
		perror("mutex destroy failed\n");
		exit(-1);
	}
	if (pthread_cond_destroy(&list->pop_cond) != 0) {
		perror("cond destroy failed\n");
		exit(-1);
	}
	if (pthread_mutexattr_destroy(&list->attr) != 0) {
		perror("mutexattr destroy failed\n");
		exit(-1);
	}
}

void intlist_push_head(intlist* list, int value) {
	Node* newNode = (Node*) malloc(sizeof(*newNode));
	newNode->prev = NULL;
	newNode->data = value;
	if (pthread_mutex_lock(&list->lock) != 0) {
		perror("mutex lock failed\n");
		exit(-1);
	}
	if (list->head == NULL && list->tail == NULL) {
		list->tail = newNode;
		newNode->next = NULL;
	} else {
		newNode->next = list->head;
		list->head->prev = newNode;
	}
	list->head = newNode;
	++list->size;
	if (pthread_cond_signal(&list->pop_cond) != 0) {
		perror("pop_cond signal failed\n");
		exit(-1);
	}
	if (pthread_mutex_unlock(&list->lock) != 0) {
		perror("mutex unlock failed\n");
		exit(-1);
	}
}

int intlist_size(intlist* list) {
	return list->size;
}

void intlist_remove_last_k(intlist* list, int k) {
	Node* temp;
	int kBuckup = k;
	if (pthread_mutex_lock(&list->lock) != 0) {
		perror("mutex lock failed\n");
		exit(-1);
	}
	if (k > list->size) {
		k = list->size;
		kBuckup = k;
		temp = list->head;
		list->head = NULL;
		list->tail = NULL;
	} else {
		temp = list->tail;
		while (k > 1) {
			temp = temp->prev;
			--k;
		}
		if (temp->prev != NULL) {
			list->tail = temp->prev;
			list->tail->next = NULL;
		} else {
			list->tail = NULL;
			list->head = NULL;
		}

	}
	temp->prev = NULL;
	list->size -= kBuckup;
	if (pthread_mutex_unlock(&list->lock) != 0) {
		perror("mutex unlock failed\n");
		exit(-1);
	}
	nodeList_destroy(temp);
}

int intlist_pop_tail(intlist* list) {
	Node* temp;
	int val;
	if (pthread_mutex_lock(&list->lock) != 0) {
		perror("mutex lock failed\n");
		exit(-1);
	}
	while (1 > list->size) {
		pthread_cond_wait(&list->pop_cond, &list->lock);
	}
	temp = list->tail;
	if (temp->prev != NULL) {
		list->tail = list->tail->prev;
		temp->prev->next = NULL;
		temp->prev = NULL;
	} else {
		list->tail = NULL;
		list->head = NULL;
	}
	--list->size;
	if (pthread_mutex_unlock(&list->lock) != 0) {
		perror("mutex unlock failed\n");
		exit(-1);
	}
	val = temp->data;
	nodeList_destroy(temp);
	return val;
}

pthread_mutex_t* intlist_get_mutex(intlist* list) {
	return &list->lock;
}

void *garbage_collector_func(void *t) {
	int size, half;
	while (gc_flag) {
		if (pthread_mutex_lock(intlist_get_mutex(&list)) != 0) {
			perror("mutex lock failed\n");
			exit(-1);
		}
		pthread_cond_wait(&garbage_collector_cond, intlist_get_mutex(&list));
		if (gc_flag == false) {
			if (pthread_mutex_unlock(intlist_get_mutex(&list)) != 0) {
				perror("mutex unlock failed\n");
				exit(-1);
			}
			break;
		}
		size = intlist_size(&list);
		if (size >= max) {
			half = (int) size / 2 + 1;
			intlist_remove_last_k(&list, half);
		}
		if (pthread_mutex_unlock(intlist_get_mutex(&list)) != 0) {
			perror("mutex unlock failed\n");
			exit(-1);
		}
		if (size >= max) {
			printf("GC – %d items removed from the list\n", half);
		}
	}
	pthread_exit((void*) t);
}

void *writer_func(void *t) {
	int r;
	time_t ti;
	srand((unsigned) time(&ti));
	while (writer_flag) {
		r = rand();
		if (intlist_size(&list) >= max) {
			if (pthread_cond_signal(&garbage_collector_cond) != 0) {
				perror("gc_cond signal failed\n");
				exit(-1);
			}
		}
		intlist_push_head(&list, r);
	}
	pthread_exit((void*) t);
}

void *reader_func(void *t) {
	int r;
	while (reader_flag) {
		r = intlist_pop_tail(&list);
	}
	pthread_exit((void*) t);
}

int main(int argc, char* argv[]) {
	int wnum, rnum, time;
	if (argc != 5) {
		printf("invalid number of arguments\n");
		exit(-1);
	}
	wnum = strtol(argv[1], NULL, 10);
	rnum = strtol(argv[2], NULL, 10);
	max = strtol(argv[3], NULL, 10);
	time = strtol(argv[4], NULL, 10);
	if (wnum < 1) {
		printf("wnum - invalid arguments\n");
		exit(-1);
	}
	if (rnum < 1) {
		printf("rnum - invalid arguments\n");
		exit(-1);
	}
	if (max < 1) {
		printf("max - invalid arguments\n");
		exit(-1);
	}
	if (time < 1) {
		printf("time - invalid arguments\n");
		exit(-1);
	}
	int rc, temp;
	long t;
	void *status;
	reader_flag = true;
	writer_flag = true;
	gc_flag = true;

	pthread_t writer_thread[wnum];
	pthread_t reader_thread[rnum];
	pthread_t gc_thread[1];
	//starting flow
	//initialization a global doubly-linked list of integers.
	intlist_init(&list);

	//initialize garbage_collector_cond
	if (pthread_cond_init(&garbage_collector_cond, NULL) != 0) {
		perror("garbage_collector_cond init failed\n");
		exit(-1);
	}

	//Creating a thread for the garbage collector
	t = 0;
	pthread_create(&gc_thread[0], NULL, garbage_collector_func, (void *) t);

	//Creating WNUM threads for the writers.
	for (t = 0; t < wnum; t++) {
		rc = pthread_create(&writer_thread[t], NULL, writer_func, (void *) t);
		if (rc) {
			printf("ERROR in pthread_create(): %s\n", strerror(rc));
			exit(rc);
		}
	}

	//Creating RNUM threads for the readers.
	for (t = 0; t < rnum; t++) {
		rc = pthread_create(&reader_thread[t], NULL, reader_func, (void *) t);
		if (rc) {
			printf("ERROR in pthread_create(): %s\n", strerror(rc));
			exit(rc);
		}
	}

	//Sleep for TIME seconds.
	sleep(time);

	//Stop all running threads
	reader_flag = false;
	for (t = 0; t < rnum; t++) {
		rc = pthread_join(reader_thread[t], &status);
		if (rc) {
			printf("ERROR in pthread_join(): %s\n", strerror(rc));
			exit(rc);
		}
	}
	writer_flag = false;
	for (t = 0; t < wnum; t++) {
		rc = pthread_join(writer_thread[t], &status);
		if (rc) {
			printf("ERROR in pthread_join(): %s\n", strerror(rc));
			exit(rc);
		}
	}
	gc_flag = false;
	if (pthread_cond_signal(&garbage_collector_cond) != 0) {
		perror("gc_cond signal failed\n");
		exit(rc);
	}
	rc = pthread_join(gc_thread[0], &status);
	if (rc) {
		printf("ERROR in pthread_join(): %s\n", strerror(rc));
		exit(rc);
	}

	//print list
	int size = intlist_size(&list);
	fflush(NULL);
	printf("size is: %d\n", size);
	while (size > 0) {
		temp = intlist_pop_tail(&list);
		fflush(NULL);
		printf("%d", temp);
		if (size == 1) {
			putchar('\n');
		} else {
			putchar(' ');
		}
		--size;
	}

	if (pthread_cond_destroy(&garbage_collector_cond) != 0) {
		perror("garbage_collector_cond destroy failed\n");
		exit(-1);
	}

	intlist_destroy(&list);

	pthread_exit(NULL);
}
