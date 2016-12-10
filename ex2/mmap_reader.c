/*
 * mmap_reader.c
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


#define FILEPATH "./mmapped.bin"

struct sigaction oldact;

void my_signal_handler(int signum) {
	if (signum == SIGUSR1) {
		int fd, i, result, fileSize;
		int flag = 1;
		struct stat s;
		char* arr;
		bool a_flag = true;
		struct timeval t1, t2;
		double elapsed_microsec;

		fd = open(FILEPATH, O_RDWR | O_CREAT);
		if (-1 == fd) {
			printf("Error opening file for writing: %s\n", strerror(errno));
			exit(-1);
		}

		if (stat(FILEPATH, &s) < 0) {
			printf("Error getting stat of file: %s\n", strerror(errno));
			return; // ERROR!
		}
		fileSize = s.st_size;

		// Force the file to be of the NUM size as the (mmapped) array
		result = lseek(fd, fileSize, SEEK_SET);
		if (-1 == result) {
			printf("Error calling lseek() to 'stretch' the file: %s\n",
					strerror(errno));
			exit(-1);
		}
		//Now the file is ready to be mmapped.
		arr = (char*) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED,
				fd, 0);

		if (MAP_FAILED == arr) {
			printf("Error mmapping the file: %s\n", strerror(errno));
			exit(-1);
		}

		if (gettimeofday(&t1, NULL) < 0) {
			printf("Error getting time: %s\n", strerror(errno));
			exit(-1);
		}

		i = 0;
		while (a_flag) {
			if (arr[i] == '\0')
				a_flag = false;
			if((arr[i] == '\0') || (arr[i] == 'a'))
				i++;
		}

		if (gettimeofday(&t2, NULL) < 0) {
			printf("Error getting time: %s\n", strerror(errno));
			exit(-1);
		}

		if (-1 == munmap(arr, fileSize)) {
			printf("Error un-mmapping the file: %s\n", strerror(errno));
			exit(-1);
		}

		// Counting time elapsed
		elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
		elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

		// Final report
		printf("%d were read in %f microseconds through MMAP\n", i,
				elapsed_microsec);
		// un-mmaping doesn't close the file, so we still need to do that.
		if (close(fd)) {
			printf("Error close file: %s\n", strerror(errno));
			exit(-1);
		}
		if (unlink(FILEPATH) < 0) {
			printf("Error remove the file from the disk: %s\n",
					strerror(errno));
			exit(-1);
		}
		if (sigaction(SIGTERM, &oldact, NULL) < 0) {
			printf("Error restore sigaction SIGTERM: %s\n", strerror(errno));
			exit(-1);
		}
		// Exit gracefully
		exit(0);
	}
}

int main() {
	// Structure to pass to the registration syscall
	struct sigaction new_action;
	// Assign pointer to our handler function
	new_action.sa_handler = my_signal_handler;
	// Remove any special flag
	new_action.sa_flags = 0;

	//SIGTERM handler
	//taking from - https://www.linuxprogrammingblog.com/code-examples/sigaction
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	if (sigaction(SIGTERM, &act, &oldact) < 0) {
		printf("Error sigaction SIGTERM: %s\n", strerror(errno));
		exit(-1);
	}

	// Register the handler
	if (0 != sigaction(SIGUSR1, &new_action, NULL)) {
		printf("Signal handle registration failed. %s\n", strerror(errno));
		exit(-1);
	}
	while (1) {
		sleep(2);
	}


}
