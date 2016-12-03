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

#define FILEPATH "/tmp/osfifo"

int main(int argc, char* argv[]) {
	int NUM, i;
	int temp;
	// Time measurement structures
	struct timeval t1, t2;
	double elapsed_microsec;
	char *arr;
	if (argc == 2) {
		NUM = atoi(argv[1]);
	} else {
		puts("invalid number of arguments");
		return 1; //exit(1)
	}
	printf("setting args\n");
	//taking from - http://stackoverflow.com/questions/2784500/how-to-send-a-simple-string-between-two-programs-using-pipes
    int fd;
    /* create the FIFO (named pipe) */
    if (mkfifo(FILEPATH, 0600) < 0){
    	printf("Error mkfifo file: %s\n", strerror(errno));
    	return -1;
    }
    printf("fifo created\n");
    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC,0644);
    printf("after open\n");
    if (fd < 0){
    	printf("Error opening file for writing: %s\n", strerror(errno));
    	return -1;
    }
    printf("fd %d\n", fd);

	if( gettimeofday(&t1, NULL) < 0){
		printf("Error getting time: %s\n", strerror(errno));
		return -1;
	}

	for (i = 0; i < NUM -1; ++i) {
		temp = write(fd, "a", sizeof("a"));
		fflush(NULL);
		printf("%d\n", 0);
		fflush(NULL);
		if(  temp < 0){
			printf("Error writing to file: %s\n", strerror(errno));
			return -1;
		}
	}

	if( gettimeofday(&t2, NULL) < 0){
		printf("Error getting time: %s\n", strerror(errno));
		return -1;
	}

	// Counting time elapsed
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were written in %f microseconds through FIFO\n", NUM,elapsed_microsec);
	if (close(fd)) {
		printf("Error close file: %s\n", strerror(errno));
		return -1;
	}
	if (unlink(FILEPATH) < 0) {
		printf("Error remove the file from the disk: %s\n",
				strerror(errno));
		return -1;
	}
    return 0;
}
