/*
 * fifo_reader.c
 *
 *  Created on: Dec 2, 2016
 *      Author: lirongazit
 */


#include <sys/time.h>
#include <stdio.h>
#include <stdbool.h>
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
#define READBYTE 1024

int main(){
	char in[READBYTE];
	int readlen,fd, i,j;
	bool flag = true;
	sleep(15);
	// Time measurement structures
	struct timeval t1, t2;
	double elapsed_microsec;
    fd = open(FILEPATH, O_RDONLY | O_CREAT | O_TRUNC | O_NONBLOCK ,0644);
    if (fd < 0){
    	printf("Error opening file for reading: %s\n", strerror(errno));
    	return -1;
    }

	if( gettimeofday(&t1, NULL) < 0){
		printf("Error getting time: %s\n", strerror(errno));
		return -1;
	}


	i = 0;
//    while (1) {
//    	readlen = read(fd, in, READBYTE);
//    	printf("%d\n",readlen);
//    	if( readlen < 0){
//    		printf("Error reading fifo: %s\n", strerror(errno));
//    		return -1;
//    	}
//    	for(j=0; j<readlen; j++){
//    		if('a' == in[j]){
//    			i++;
//    		}
//    	}
//    	if (readlen < READBYTE) {
//    		break;
//    	}
//    }

    while ( (readlen = read(fd, in, READBYTE)) > 0 ) {
    	printf("%d\n",readlen);
    	for(j=0; j<readlen; j++){
    		if('a' == in[j]){
    			i++;
    		}
    	}
    }

    if( gettimeofday(&t2, NULL) < 0){
		printf("Error getting time: %s\n", strerror(errno));
		return -1;
	}

	// Counting time elapsed
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were read in %f microseconds through FIFO\n", i ,elapsed_microsec);

	return 0;
}

