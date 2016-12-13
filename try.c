/*
 * fifo_writer.c
 *
 *  Created on: Dec 2, 2016
 *      Author: lirongazit
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#define FILEPATH "./try.txt"
#define WRITEBYTE 4096

FILE *fd;
// Time measurement structures
struct timeval t1, t2;
bool timeFlag = false;
double elapsed_microsec;

int main(int argc, char* argv[]) {
	int NUM, i, writed;
	NUM=4096;
	fd = fopen(FILEPATH,"w+");

	char buff[WRITEBYTE];
	for (i = 0; i < WRITEBYTE; ++i) {
		buff[i] = 'a';
	}

	for (i=0;i<NUM;i++) {
		printf("%d\n",i);
		fwrite(buff,1, WRITEBYTE,fd);
	}
	fclose(fd);

	exit(0);
}
