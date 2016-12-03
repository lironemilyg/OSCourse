/*
 * copy_tool.c
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

int main(int argc, char* argv[]) {
	char *srcFile;
	char* destFile;
	char *srcMap, *dstMap;
	int srcfd,destfd,fileSize;
	int result;
	struct stat s;
	// Time measurement structures
	struct timeval t1, t2;
	double elapsed_microsec;
	char *arr;
	if (argc == 3) {
		srcFile = argv[1];
		destFile = argv[2];
	} else {
		puts("invalid number of arguments");
		return 1; //exit(1)
	}

	//try to open src file
	if ((srcfd = open(srcFile, O_RDWR | O_CREAT)) < 0) {
		printf("Error opening src file: %s\n", strerror(errno));
		return -1;
	}

	if ((destfd = open(destFile,  O_RDWR | O_CREAT,0777)) < 0) {
		printf("Error opening dst file: %s\n", strerror(errno));
		return -1;
	}

	if (stat(srcFile, &s) < 0) {
		printf("Error getting stat of src file: %s\n", strerror(errno));
		return -1; // ERROR!
	}

	fileSize = s.st_size;

	if (fileSize == 0)
		return 0;

	if (truncate(destFile, fileSize) < 0){
		printf("Error truncate the dst file: %s\n", strerror(errno));
		return -1; // ERROR
	}

	srcMap = (char*) mmap(NULL,fileSize, PROT_READ | PROT_WRITE,MAP_SHARED, srcfd, 0);
	dstMap = (char*) mmap(NULL,fileSize, PROT_READ | PROT_WRITE,MAP_SHARED, destfd, 0);

	if (MAP_FAILED == srcMap) {
		printf("Error mmapping the src file: %s\n", strerror(errno));
		return -1;
	}

	if (MAP_FAILED == dstMap) {
		printf("Error mmapping the dst file: %s\n", strerror(errno));
		return -1;
	}
	//assist in - https://www.tutorialspoint.com/c_standard_library/c_function_memcpy.htm
	if (memcpy(dstMap, srcMap, fileSize) == NULL){
		printf("Error memcpy to the dst file: %s\n", strerror(errno));
		if (-1 == munmap(srcMap, fileSize)) {
				printf("Error un-mmapping the src file: %s\n", strerror(errno));
				if (close(srcfd)){
					printf("Error close src file: %s\n", strerror(errno));
					return -1;
				}

				if (close(destfd)){
					printf("Error close dst file: %s\n", strerror(errno));
					return -1;
				}
				return -1;
			}

			if (-1 == munmap(dstMap, fileSize)) {
				printf("Error un-mmapping the dst file: %s\n", strerror(errno));
				if (close(srcfd)){
					printf("Error close src file: %s\n", strerror(errno));
					return -1;
				}

				if (close(destfd)){
					printf("Error close dst file: %s\n", strerror(errno));
					return -1;
				}
				return -1;
			}
		if (close(srcfd)){
			printf("Error close src file: %s\n", strerror(errno));
			return -1;
		}

		if (close(destfd)){
			printf("Error close dst file: %s\n", strerror(errno));
			return -1;
		}
		return -1;
	}

	if (-1 == munmap(srcMap, fileSize)) {
		printf("Error un-mmapping the src file: %s\n", strerror(errno));
		if (close(srcfd)){
			printf("Error close src file: %s\n", strerror(errno));
			return -1;
		}

		if (close(destfd)){
			printf("Error close dst file: %s\n", strerror(errno));
			return -1;
		}
		return -1;
	}

	if (-1 == munmap(dstMap, fileSize)) {
		printf("Error un-mmapping the dst file: %s\n", strerror(errno));
		if (close(srcfd)){
			printf("Error close src file: %s\n", strerror(errno));
			return -1;
		}

		if (close(destfd)){
			printf("Error close dst file: %s\n", strerror(errno));
			return -1;
		}
		return -1;
	}

	char mode[] = "0777";
	 int i = strtol(mode, 0, 8);
	if (chmod(destFile, i) < 0) {
		printf("error in chmod(%s, %s) - %d (%s)\n", destFile, mode, errno, strerror(errno));
		return 1;
	}

	if (close(srcfd)){
		printf("Error close src file: %s\n", strerror(errno));
		return -1;
	}

	if (close(destfd)){
		printf("Error close dst file: %s\n", strerror(errno));
		return -1;
	}

}

