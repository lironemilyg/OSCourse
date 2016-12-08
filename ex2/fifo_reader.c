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


#define FILEPATH "./osfifo"
#define READBYTE 1024

int fd;

void my_signal_handler(int signum) {
	if (signum == SIGUSR1) {
		if (close(fd) < 0) {
			printf("Error close file: %s\n", strerror(errno));
			exit(-1);
		}

		if (unlink(FILEPATH) < 0) {
			printf("Error remove the file from the disk: %s\n",
					strerror(errno));
			exit(-1);
		}
	}

}

int main() {
//	fd = 0;
	char in[READBYTE];
	int readlen, i, j;

	bool flag = true;
	// Time measurement structures
	struct timeval t1, t2;
	double elapsed_microsec;
	struct stat s;
	//taking from - https://www.linuxprogrammingblog.com/code-examples/sigaction
	struct sigaction oldact;
	struct sigaction act;

	memset(&act, '\0', sizeof(act));

	/* Use the sa_sigaction field because the handles has two additional parameters */
	act.sa_handler = SIG_IGN;

	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGINT, &act, &oldact) < 0) {
		printf("Error sigaction SIGTERM: %s\n", strerror(errno));
		exit(-1);
	}
	if (stat(FILEPATH, &s) < 0) {
		if (mkfifo(FILEPATH, 0600) < 0) {
			if ( errno != 17) { //file exist
				printf("Error mkfifo file: %s\n", strerror(errno));
				exit(-1);
			}
		}
	}

	fd = open(FILEPATH, O_RDONLY | O_CREAT | O_TRUNC, 0644);
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
		printf("Error restore sigaction SIGTERM: %s\n", strerror(errno));
		exit(-1);
	}
	exit(0);
}

