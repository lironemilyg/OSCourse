/*
 * mmap_writer.c
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

#define FILEPATH "/tmp/mmapped.bin"

int main(int argc, char* argv[]) {
	int NUM, RPID;
	int i;
	int fd;
	int result;
	// Time measurement structures
	struct timeval t1, t2;
	double elapsed_microsec;
	char *arr;
	if (argc == 3) {
		NUM = atoi(argv[1]);
		RPID = atoi(argv[2]);
	} else {
		puts("invalid number of arguments");
		return 1; //exit(1)
	}
	// open a file for writing.
	// Note: read/write mode needs to match
	// the required access in mmap (not intuitive)
	fd = open(FILEPATH, O_RDWR | O_CREAT);
	if (-1 == fd) {
		printf("Error opening file for writing: %s\n", strerror(errno));
		return -1;
	}
	//taking from - http://stackoverflow.com/questions/4568681/using-chmod-in-a-c-program
	char mode[] = "0600";
	 i = strtol(mode, 0, 8);
	if (chmod(FILEPATH, i) < 0) {
		printf("error in chmod(%s, %s) - %d (%s)\n", FILEPATH, mode, errno, strerror(errno));
		return 1;
	}
	// Force the file to be of the NUM size as the (mmapped) array
	result = lseek(fd, NUM-1, SEEK_SET);
	if (-1 == result) {
		printf("Error calling lseek() to 'stretch' the file: %s\n", strerror(errno));
		return -1;
	}

	// Something has to be written at the end of the file,
	// so the file actually has the new size.
	result = write(fd, "", 1);
	if (1 != result) {
		printf("Error writing last byte of the file: %s\n", strerror(errno));
		return -1;
	}

	//Now the file is ready to be mmapped.
	arr = (char*) mmap(NULL,NUM, PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);

	if (MAP_FAILED == arr) {
		printf("Error mmapping the file: %s\n", strerror(errno));
		return -1;
	}

	if( gettimeofday(&t1, NULL) < 0){
		printf("Error getting time: %s\n", strerror(errno));
		return -1;
	}

//	// now write to the file as if it were memory
//	for (i = 0; i < NUM -1; ++i) {
//		arr[i] = 'a';
//	}
//	arr[NUM-1]='\0';

	if( gettimeofday(&t2, NULL) < 0){
		printf("Error getting time: %s\n", strerror(errno));
		return -1;
	}

	free(arr);  // this also ensures the changes commit to the file
	if (-1 == munmap(arr, NUM)) {
		printf("Error un-mmapping the file: %s\n", strerror(errno));
		return -1;
	}

	// Counting time elapsed
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	//taking from - http://stackoverflow.com/questions/6168636/how-to-trigger-sigusr1-and-sigusr2
	kill(RPID, SIGUSR1);
	// Final report
	printf("%d were written in %f microseconds through MMAP\n", NUM,elapsed_microsec);
	// un-mmaping doesn't close the file, so we still need to do that.
	if (close(fd)){
		printf("Error close file: %s\n", strerror(errno));
		return -1;
	}

	// Exit gracefully
	return 0;
}
