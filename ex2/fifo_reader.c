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

int main() {
	int fd;
	char in[READBYTE];
	int readlen, i, j;

	bool flag = true;
	// Time measurement structures
	struct timeval t1, t2;
	double elapsed_microsec;
	struct stat s;

	//SIGINT Handler
	struct sigaction oldact;
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	if (sigaction(SIGINT, &act, &oldact) < 0) {
		printf("Error sigaction SIGINT: %s\n", strerror(errno));
		exit(-1);
	}

	if (stat(FILEPATH, &s) < 0) {
		sleep(3);
	}

	fd = open(FILEPATH , O_RDONLY);
	if (fd < 0) {
		printf("Error opening file for reading: %s\n", strerror(errno));
		exit(-1);
	}

	if (gettimeofday(&t1, NULL) < 0) {
		printf("Error getting time: %s\n", strerror(errno));
		exit(-1);
	}

	i = 0;
	while (flag == true) {
		readlen = read(fd, in, READBYTE);
		if (readlen < 0) {
			printf("Error: read failed - errno %s\n", strerror(errno));
			exit(-1);
		}
		if (readlen < READBYTE) {
			flag = false;
		}
		for (j = 0; j < readlen; ++j) {
			if ('a' == in[j])
				i++;
		}
		close(fd);
	}

	if (gettimeofday(&t2, NULL) < 0) {
		printf("Error getting time: %s\n", strerror(errno));
		exit(-1);
	}

	// Counting time elapsed
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were read in %f microseconds through FIFO\n", i,
			elapsed_microsec);

	if (close(fd) < 0) {
		printf("Error close file: %s\n", strerror(errno));
		exit(-1);
	}

	if (sigaction(SIGINT, &oldact, NULL) < 0) {
		printf("Error restore sigaction SIGINT: %s\n", strerror(errno));
		exit(-1);
	}
	exit(0);
}

