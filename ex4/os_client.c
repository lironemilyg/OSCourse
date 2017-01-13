/*
 * os_client.c
 *
 *  Created on: Jan 13, 2017
 *      Author: lirongazit
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdbool.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
	char* ip;
	short port;
	int fdsrc, fddst;
	if (argc != 5) {
		printf("invalid number of arguments\n");
		exit(-1);
	}
	port = (short) strtol(argv[2], NULL, 10);
	if (port < 1) {
		printf("port - invalid arguments\n");
		exit(-1);
	}
	fdsrc = open(argv[3], O_RDONLY);
	if (fdsrc < 0) {
		printf("error open() input file %s: %s\n", argv[3], strerror(errno));
		return -1;
	}
	fddst = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fddst < 0) {
		printf("error open() output file %s: %s\n", argv[4], strerror(errno));
		return -1;
	}

	int sockfd = 0, nread = 0;
	//char recvBuff[BUF_SIZE];
	char sendBuff[BUF_SIZE];
	struct sockaddr_in serv_addr;
	struct sockaddr_in my_addr, peer_addr;
	socklen_t addrsize = sizeof(struct sockaddr_in);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Error : Could not create socket \n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port); // Note: htons for endiannes
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

	printf("connecting...\n");
	/* Note: what about the client port number? */
	/* connect socket to the above address */
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
			< 0) {
		printf("\n Error : Connect Failed. %s \n", strerror(errno));
		return 1;
	}

	bool flag = true;
	int numsrc, nsent, nwrite, numdst;

	while (flag) {
		numsrc = read(fdsrc, sendBuff, BUF_SIZE);
		if (numsrc < 0) {
			printf("error read() from input file: %s\n", strerror(errno));
			exit(errno);
		} else if (numsrc == 0) {
			break;
		}

		//sending src buffer to server
		int totalsent = 0;
		int notwritten = strlen(sendBuff);
		/* keep looping until nothing left to write*/
		while (notwritten > 0) {
			/* notwritten = how much we have left to write
			 totalsent  = how much we've written so far
			 nsent = how much we've written in last write() call */
			nsent = write(sockfd, sendBuff + totalsent, notwritten);
			if(nsent<0){
				printf("error occured - write to server \n");
				return -1;
			}
			totalsent += nsent;
			notwritten -= nsent;
		}
		if (totalsent != numsrc) {
			printf("error occured - write to server \n");
			return -1;
		}

		//read dst buffer from server
		int totalRcv = 0;
		memset(sendBuff, '0', sizeof(sendBuff));
		while ((nread = read(sockfd, sendBuff+totalRcv, sizeof(sendBuff) - totalRcv)) > 0) {
			sendBuff[nread] = 0;
			if (fputs(sendBuff, stdout) == EOF) {
				printf("\n Error : Fputs error\n");
			}
			totalRcv += nread;
		}
		if (nread < 0) {
			perror("\n Read error \n");
		}
		if (totalsent != totalRcv) {
			printf("error occured - read from server \n");
			return -1;
		}

		//write to dst file
		numdst = 0;
		while (numdst < numsrc) {
			// ALWAYS look at write()'s return value, don't assume everything is written
			nwrite = write(fddst, sendBuff + numdst, numsrc - numdst);
			if (nwrite < 0) {
				printf("error write() to output file: %s\n", strerror(errno));
				return -1;
			}

			// increment our counter
			numdst += nwrite;
		}
	}
	close(fdsrc);
	close(fddst);
	close(sockfd);
	return 0;
}
