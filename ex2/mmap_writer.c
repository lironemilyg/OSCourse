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
#define PERMI 0600

int main(int argc, char* argv[]) {
	int NUM, RPID;
	int i;
	int fd;
	int modelo;
	int result;
	// Time measurement structures
	struct timeval t1, t2;
	double elapsed_microsec;
	char *arr;

	//SIGTERM handler
	//taking from - https://www.linuxprogrammingblog.com/code-examples/sigaction
	struct sigaction act;
	struct sigaction oldact;
	memset(&act, '\0', sizeof(act));
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	if (sigaction(SIGTERM, &act, &oldact) < 0) {
		printf("Error sigaction SIGTERM: %s\n", strerror(errno));
		exit(errno);
	}

	if (argc == 3) {
		NUM = strtol(argv[1],NULL,10);
		RPID = strtol(argv[2],NULL,10);
	} else {
		puts("invalid number of arguments");
		exit(-1); //exit(-1)
	}
	// open a file for writing.
	// Note: read/write mode needs to match
	// the required access in mmap (not intuitive)
	fd = open(FILEPATH, O_RDWR | O_CREAT , PERMI);
	if (-1 == fd) {
		printf("Error opening file for writing: %s\n", strerror(errno));
		exit(errno);
	}
	//taking from - http://stackoverflow.com/questions/4568681/using-chmod-in-a-c-program
	char mode[] = "0600";
	modelo = strtol(mode, 0, 8);
	if (chmod(FILEPATH, (mode_t) modelo) < 0) {
		printf("error in chmod(%s, %s) - %d (%s)\n", FILEPATH, mode, errno,
				strerror(errno));
		exit(errno);
	}
	// Force the file to be of the NUM size as the (mmapped) array
	result = lseek(fd, NUM - 1, SEEK_SET);
	if (-1 == result) {
		printf("Error calling lseek() to 'stretch' the file: %s\n",
				strerror(errno));
		exit(errno);
	}

	// Something has to be written at the end of the file,
	// so the file actually has the new size.
	result = write(fd, "", 1);
	if (1 != result) {
		printf("Error writing last byte of the file: %s\n", strerror(errno));
		exit(errno);
	}

	//Now the file is ready to be mmapped.
	arr = (char*) mmap(NULL, (size_t) NUM, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (MAP_FAILED == arr) {
		printf("Error mmapping the file: %s\n", strerror(errno));
		exit(errno);
	}

	if (gettimeofday(&t1, NULL) < 0) {
		printf("Error getting time: %s\n", strerror(errno));
		exit(errno);
	}

	// now write to the file as if it were memory
	for (i = 0; i < NUM - 1; ++i) {
		arr[i] = 'a';
	}
	arr[NUM - 1] = '\0';

	if (gettimeofday(&t2, NULL) < 0) {
		printf("Error getting time: %s\n", strerror(errno));
		exit(errno);
	}

	if (-1 == munmap(arr, (size_t) NUM)) {
		printf("Error un-mmapping the file: %s\n", strerror(errno));
		exit(errno);
	}

	// Counting time elapsed
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;
	//taking from - http://stackoverflow.com/questions/6168636/how-to-trigger-sigusr1-and-sigusr2
	kill(RPID, SIGUSR1);
	// Final report
	printf("%d were written in %f microseconds through MMAP\n", NUM,
			elapsed_microsec);
	// un-mmaping doesn't close the file, so we still need to do that.
	if (close(fd)) {
		printf("Error close file: %s\n", strerror(errno));
		exit(errno);
	}
	if (sigaction(SIGTERM, &oldact, NULL) < 0) {
		printf("Error restore sigaction SIGTERM: %s\n", strerror(errno));
		exit(errno);
	}
	exit(0);
}
