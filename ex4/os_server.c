/*
 * os_server.c
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
#define RAND_FILE "/dev/urandom"

//taking from - http://stackoverflow.com/questions/2572366/how-to-use-dev-random-or-urandom-in-c
int create_key_file(int fdkey, int keylen) {
	int randomData = open(RAND_FILE, O_RDONLY);
	char myRandomData[BUF_SIZE];
	size_t randomDataLen = 0;
	while (randomDataLen < keylen) {
		ssize_t result = read(randomData, myRandomData + randomDataLen,
				(sizeof myRandomData) - randomDataLen);
		if (result < 0) {
			printf("error reading from %s file\n", RAND_FILE);
			return -1;
		}
		result = write(fdkey, myRandomData, result);
		if (result < 0) {
			printf("error writing to key file file\n");
			return -1;
		}
		randomDataLen += result;
	}
	close(randomData);
	return 0;
}

int xor_buffers(char* srcbuf, int numsrc, char* keyfilename) {
	int numkey, num, i;
	int fdkey = open(keyfilename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fdkey < 0) {
		printf("error open() output file %s: %s\n", keyfilename,
				strerror(errno));
		return errno;
	}
	char keybuf[BUF_SIZE];
	// set number of bytes read from the key file to 0
	numkey = 0;
	// iterate reading from key until reaching numsrc bytes
	while (numkey < numsrc) {
		num = read(fdkey, keybuf + numkey, numsrc - numkey);
		if (num < 0) {
			printf("error read() key: %s\n", strerror(errno));
			exit(errno);
		} else if (num == 0) {
			if (lseek(fdkey, SEEK_SET, 0) < 0) {
				printf("error lseek() key: %s\n", strerror(errno));
				exit(errno);
			}
		} else {
			numkey += num;
		}
	}
	// now we have 'numsrc' bytes - from both input and key file
	// perform encryption operation
	// (using srcbuf, no need for another buffer)
	for (i = 0; i < numsrc; ++i)
		srcbuf[i] = srcbuf[i] ^ keybuf[i];
	close(fdkey);
	return 0;
}

int main(int argc, char *argv[]) {
	short port;
	int fdkey, keylen, nread;
	bool flag = true;
	//initialize arsg
	if (argc != 4 || argc != 3) {
		printf("invalid number of arguments\n");
		exit(-1);
	}
	port = (short) strtol(argv[2], NULL, 10);
	if (port < 1) {
		printf("port - invalid arguments\n");
		exit(-1);
	}
	char* keyfilename = argv[2];
	fdkey = open(keyfilename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fdkey < 0) {
		printf("error open() output file %s: %s\n", argv[2], strerror(errno));
		return errno;
	}
	if (argc == 4) {
		keylen = strtol(argv[3], NULL, 10);
		if (keylen < 1) {
			printf("keylen - invalid arguments\n");
			exit(-1);
		}
		if (create_key_file(fdkey, keylen) < 0) {
			printf("error in creating key file\n");
			exit(-1);
		}
	}
	close(fdkey);

	int totalsent, nsent, len, n = 0, listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr, my_addr, peer_addr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY = any local machine address
	serv_addr.sin_port = htons(port);

	if (bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) {
		printf("\n Error : Bind Failed. %s \n", strerror(errno));
		exit(errno);
	}

	if (listen(listenfd, 10)) {
		printf("\n Error : Listen Failed. %s \n", strerror(errno));
		exit(errno);
	}

	while (flag) {
		/* new connection */
		socklen_t addrsize = sizeof(struct sockaddr_in);
		connfd = accept(listenfd, (struct sockaddr*) &peer_addr, &addrsize);
		if (connfd < 0) {
			printf("\n Error : Accept Failed. %s \n", strerror(errno));
			exit(errno);
		}
		int forked = fork();
		if (forked == 0) {
			char srcbuf[BUF_SIZE];
			int totalRcv = 0;
			memset(srcbuf, '0', sizeof(srcbuf));
			//read buffer from client
			while ((nread = read(connfd, srcbuf + totalRcv,
					sizeof(srcbuf) - totalRcv)) > 0) {
				srcbuf[nread] = 0;
				if (fputs(srcbuf, stdout) == EOF) {
					printf("\n Error : Fputs error\n");
				}
				totalRcv += nread;
			}
			if (nread < 0) {
				perror("\n Read error \n");
			}
			//xor buffers
			if (xor_buffers(srcbuf, totalRcv, keyfilename) < 0) {
				printf("error occured - xor buffers failed \n");
				return -1;
			}

			totalsent = 0;
			int notwritten = strlen(srcbuf);
			/* keep looping until nothing left to write*/
			while (notwritten > 0) {
				/* notwritten = how much we have left to write
				 totalsent  = how much we've written so far
				 nsent = how much we've written in last write() call */
				nsent = write(connfd, srcbuf + totalsent, notwritten);
				if (nread != nsent) {
					printf("error occured - write to client \n");
					return -1;
				}
				totalsent += nsent;
				notwritten -= nsent;
			}
			if (totalRcv != totalsent) {
				printf("error occured - sending enc file to server failed \n");
				return -1;
			}
		} else if (forked < 0) {
			printf("error occured - forked failed \n");
			return -1;
		}

		/* close socket  */
		close(connfd);
	}

	return 0;
}

