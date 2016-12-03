/*
 * fifo_reader.c
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

int main(){
	printf("begin read\n");
	sleep(1);
	// Time measurement structures
	struct timeval t1, t2;
	double elapsed_microsec;
    int fd, i;
    fd = open(FILEPATH, O_WRONLY);
    if (fd < 0){
    	printf("Error opening file for reading: %s\n", strerror(errno));
    	return -1;
    }

	if( gettimeofday(&t1, NULL) < 0){
		printf("Error getting time: %s\n", strerror(errno));
		return -1;
	}

	if( gettimeofday(&t2, NULL) < 0){
		printf("Error getting time: %s\n", strerror(errno));
		return -1;
	}
	char in;
	i = 0;

    while (read(fd, &in, sizeof(char))>0) {
    	printf("%c\n",in);
        if('a' == in)
        	i++;
    }

	// Counting time elapsed
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were read in %f microseconds through FIFO\n", i ,elapsed_microsec);
	return 0;
}

