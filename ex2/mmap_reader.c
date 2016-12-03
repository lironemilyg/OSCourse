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

#define FILEPATH "/tmp/mmapped.bin"

void my_signal_handler(int signum) {
	if (signum == SIGUSR1) {
		int fd, i, result, fileSize;
		int flag = 1;
		struct stat s;
		char* arr;
		struct timeval t1, t2;
		double elapsed_microsec;
		fd = open(FILEPATH, O_RDWR | O_CREAT);
		if (-1 == fd) {
			printf("Error opening file for writing: %s\n", strerror(errno));
			return;
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
			return;
		}
		//Now the file is ready to be mmapped.
		arr = (char*) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED,
				fd, 0);

		if (MAP_FAILED == arr) {
			printf("Error mmapping the file: %s\n", strerror(errno));
			return;
		}
		int a_flag = 1;
		i = 0;

		if (gettimeofday(&t1, NULL) < 0) {
			printf("Error getting time: %s\n", strerror(errno));
			return;
		}

		while (a_flag) {
			if (arr[i] != 'a')
				a_flag = 0;
			else
				i++;
		}

		if (gettimeofday(&t2, NULL) < 0) {
			printf("Error getting time: %s\n", strerror(errno));
			return;
		}

		free(arr);  // this also ensures the changes commit to the file
		if (-1 == munmap(arr, fileSize)) {
			printf("Error un-mmapping the file: %s\n", strerror(errno));
			return;
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
			return;
		}
		if (unlink(FILEPATH) < 0) {
			printf("Error remove the file from the disk: %s\n",
					strerror(errno));
			return;
		}
	}
}

int main() {
	// Structure to pass to the registration syscall
	struct sigaction new_action;
	// Assign pointer to our handler function
	new_action.sa_handler = my_signal_handler;
	// Remove any special flag
	new_action.sa_flags = 0;
	// Register the handler
	if (0 != sigaction(SIGUSR1, &new_action, NULL)) {
		printf("Signal handle registration failed. %s\n", strerror(errno));
		return -1;
	}
	while (1) {
		sleep(2);
	}
	// Exit gracefully
	return 0;
}
