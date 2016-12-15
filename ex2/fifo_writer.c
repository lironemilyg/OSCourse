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

#define FILEPATH "/tmp/osfifo"
#define WRITEBYTE 1024

int fd, totalWrite;
// Time measurement structures
struct timeval t1, t2;
bool timeFlag = false;
double elapsed_microsec;

void my_signal_handler(int signum);

void my_signal_handler(int signum) {
	if (signum == SIGPIPE) {
		if (!timeFlag) {
			elapsed_microsec = 0.0;
		} else {
			if (gettimeofday(&t2, NULL) < 0) {
				printf("Error getting time: %s\n", strerror(errno));
				exit(errno);
			}
			// Counting time elapsed
			elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
			elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;
		}
		printf("%d were written in %f microseconds through FIFO\n", totalWrite,
				elapsed_microsec);
		printf("SIGPIPE occurred through FIFO\n");

		if (close(fd) < 0) {
			printf("Error close file: %s\n", strerror(errno));
			exit(errno);
		}

		if (unlink(FILEPATH) < 0) {
			printf("Error remove the file from the disk: %s\n",
					strerror(errno));
			exit(errno);
		}
		exit(-1);
	}

}

int main(int argc, char* argv[]) {
	int NUM, i, ret, writed;
	struct stat s;
	bool flag = true;

	//SIGPIPE handler
	struct sigaction new_action;
	new_action.sa_handler = my_signal_handler;
	new_action.sa_flags = 0;
	if (0 != sigaction(SIGPIPE, &new_action, NULL)) {
		printf("Signal handle registration failed. %s\n", strerror(errno));
		exit(errno);

	}

	//SIGINT handler
	//taking from - https://www.linuxprogrammingblog.com/code-examples/sigaction
	struct sigaction oldact;
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	if (sigaction(SIGINT, &act, &oldact) < 0) {
		printf("Error sigaction SIGINT: %s\n", strerror(errno));
		exit(errno);
	}

	if (argc == 2) {
		NUM = strtol(argv[1], NULL, 10);
		if (NUM < 0) {
			printf("negative arg - invalid arguments\n");
			exit(-1); //exit(-1)
		}
	} else {
		printf("invalid number of arguments\n");
		exit(-1); //exit(-1)
	}

	//taking from - http://stackoverflow.com/questions/2784500/how-to-send-a-simple-string-between-two-programs-using-pipes
	//taking fron hw 1
	ret = stat(FILEPATH, &s);
	if (ret < 0) {
		// failed but error isn't ENOENT - something went wrong
		if (errno != ENOENT) {
			printf("error with stat() exist file: %s\n", strerror(errno));
			return -1;
		}
		if (mkfifo(FILEPATH, 0600) < 0) { // error is ENOENT - path doesn't exist
			printf("Error mkfifo file: %s\n", strerror(errno));
			exit(errno);
		}
	} else {
		if (chmod(FILEPATH, 0600) < 0) {
			printf("Error chmod in exist mkfifo file: %s\n", strerror(errno));
			exit(errno);
		}
	}

	fd = open(FILEPATH, O_WRONLY | O_CREAT, 0600);

	if (fd < 0) {
		printf("Error opening file for writing: %s\n", strerror(errno));
		exit(errno);

	}

	char buff[WRITEBYTE];
	for (i = 0; i < WRITEBYTE; ++i) {
		buff[i] = 'a';
	}

	if (gettimeofday(&t1, NULL) < 0) {
		printf("Error getting time: %s\n", strerror(errno));
		exit(errno);

	} else {
		timeFlag = true;
	}

	totalWrite = 0;
	while (flag) {
		if ( WRITEBYTE <= NUM) {
			writed = write(fd, buff, WRITEBYTE);
		} else {
			writed = write(fd, buff, (size_t) NUM);
		}

		if (writed < 0) {
			printf("Error writing to file: %s\n", strerror(errno));
			exit(errno);
		}
		NUM = NUM - writed;
		totalWrite += writed;
		if (writed < WRITEBYTE) {
			flag = false;
		}
	}

	if (gettimeofday(&t2, NULL) < 0) {
		printf("Error getting time: %s\n", strerror(errno));
		exit(errno);
	}

// Counting time elapsed
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were written in %f microseconds through FIFO\n", totalWrite,
			elapsed_microsec);
	sleep(2);
	if (close(fd) < 0) {
		printf("Error close file: %s\n", strerror(errno));
		exit(errno);
	}

	if (unlink(FILEPATH) < 0) {
		printf("Error remove the file from the disk: %s\n", strerror(errno));
		exit(errno);
	}

	if (sigaction(SIGINT, &oldact, NULL) < 0) {
		printf("Error restore sigaction SIGINT: %s\n", strerror(errno));
		exit(errno);
	}
	exit(0);
}
