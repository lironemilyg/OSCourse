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
	//initialize
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
	char sendBuff[BUF_SIZE+1];
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

	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
			< 0) {
		printf("\n Error : Connect Failed. %s \n", strerror(errno));
		return 1;
	}

	bool flag = true;
	int numsrc, nsent, nwrite, numdst;
	char numsrcstr[5];
	while (flag) {
		memset(sendBuff, '0', sizeof(sendBuff));
		numsrc = read(fdsrc, sendBuff, BUF_SIZE);
		if (numsrc < 0) {
			printf("error read() from input file: %s\n", strerror(errno));
			exit(errno);
		} else if (numsrc < BUF_SIZE) {
			flag = false;
		}
		sprintf(numsrcstr, "%04d", numsrc);
		printf("braekepoint 1 -after read input - sending to server: %d bytes\n", numsrc);
		numsrcstr[4]='\0';
		nsent = write(sockfd, numsrcstr, 5);
		if (nsent < 0) {
			printf("error occured - write size to server \n");
			return -1;
		}

		printf("need to write : %s   \n", numsrcstr);
		//sending src buffer to server
		int totalsent = 0;
		int notwritten = BUF_SIZE;
		/* keep looping until nothing left to write*/
		while (notwritten > 0) {
			/* notwritten = how much we have left to write
			 totalsent  = how much we've written so far
			 nsent = how much we've written in last write() call */
			nsent = write(sockfd, sendBuff + totalsent, notwritten);
			if (nsent < 0) {
				printf("error occured - write to server \n");
				return -1;
			}
			totalsent += nsent;
			notwritten -= nsent;
		}
		if ((totalsent != numsrc) && (flag ==true)) {
			printf("error occured - total write to server failed \n");
			return -1;
		}
		printf("braekepoint 2 -after send src to server - sending to server: %d bytes\n", totalsent);
		printf("\nbraekepoint 2.5 -after send src to server - sending to server: : %s\n", sendBuff);

		//read dst buffer from server
		int totalRcv = 0;
		memset(sendBuff, '0', sizeof(sendBuff));
		while ((nread = read(sockfd, sendBuff + totalRcv,
				numsrc - totalRcv)) > 0) {
			sendBuff[nread] = 0;
			if (fputs(sendBuff, stdout) == EOF) {
				printf("\n Error : Fputs error\n");
			}
			totalRcv += nread;
		}
		if (nread < 0) {
			printf("\n Read error :%s\n", strerror(errno));
		}
		if (numsrc != totalRcv) {
			printf("error occured - read from server \n");
			return -1;
		}

		printf("braekepoint 3 -after rcv enc file from server : %d bytes\n", totalRcv);
		printf("\nbraekepoint 3.5 -after rcv enc file from server : %s\n", sendBuff);
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
		printf("braekepoint 4 -after write to dst file: %d bytes\n\n", numdst);
	}
	close(fdsrc);
	close(fddst);
	close(sockfd);
	printf("braekepoint 5 -disconnect drom server\n");
	return 0;
}

