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

int xor_buffers(char* srcbuf, int numsrc, int fdkey) {
	printf("brakepoint 1 - in xor first char %c bytes\n", srcbuf[0]);
	int numkey, num, i;
	char keybuf[numsrc];
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
	for (i = 0; i < numsrc; ++i)
		srcbuf[i] = srcbuf[i] ^ keybuf[i];
	return 0;
}

int main(int argc, char *argv[]) {
	short port;
	int fdkey, keylen, nread;
	bool flag = true;
	//initialize args
	if (argc != 4 && argc != 3) {
		printf("invalid number of arguments\n");
		exit(-1);
	}
	port = (short) strtol(argv[1], NULL, 10);
	if (port < 1) {
		printf("port - invalid arguments \n");
		exit(-1);
	}
	char* keyfilename = argv[2];

	if (argc == 4) {
		keylen = strtol(argv[3], NULL, 10);
		if (keylen < 1) {
			printf("keylen - invalid arguments\n");
			exit(-1);
		}
		fdkey = open(keyfilename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
		if (fdkey < 0) {
			printf("error open() output file %s: %s\n", argv[2],
					strerror(errno));
			return errno;
		}
		if (create_key_file(fdkey, keylen) < 0) {
			printf("error in creating key file\n");
			exit(-1);
		}
		close(fdkey);
	}

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

	while (1) {
		/* new connection */
		socklen_t addrsize = sizeof(struct sockaddr_in);
		connfd = accept(listenfd, (struct sockaddr*) &peer_addr, &addrsize);
		if (connfd < 0) {
			printf("\n Error : Accept Failed. %s \n", strerror(errno));
			exit(errno);
		}
		int forked = fork();
		if (forked == 0) {
			int fdkey = open(keyfilename, O_RDONLY);
			if (fdkey < 0) {
				printf("error open() key file %s: %s\n", keyfilename,
						strerror(errno));
				return errno;
			}
			while (flag) {
				int size = BUF_SIZE + 5;
				char srcbuf[size];
				char numsrcstr[5];
				int totalRcv = 0;
				printf("breakpoint 2 - buffer size : %d   \n", size);
				memset(srcbuf, '0', sizeof(srcbuf));
				//read buffer from client
				while ((nread = read(connfd, srcbuf + totalRcv, size - totalRcv))
						> 0) {
					srcbuf[nread] = 0;
					if (fputs(srcbuf, stdout) == EOF) {
						printf("\n Error : Fputs error\n");
					}
					totalRcv += nread;
				}
				if (nread < 0) {
					perror("\n Read error \n");
				}
				strncpy(numsrcstr, srcbuf, 4);
				numsrcstr[4] ='\0';
				printf("breakpoint 2 - need to read str : %s   \n", numsrcstr);
				int needToRead = (int) strtol(numsrcstr, NULL, 10);
				printf("breakpoint 3 - need to read int : %d   \n", needToRead);

				if (needToRead == 0) {
					break;
				} else if (needToRead < BUF_SIZE) {
					flag = false;
				} else if (totalRcv == 0) {
					break;
				}

				//xor buffers
				printf("brakepoint 4 - before xor first char %c bytes\n",
						srcbuf[4]);
				printf("brakepoint 4 - before xor first char %c bytes\n",
						srcbuf[5]);
				printf("brakepoint 4 - before xor first char %c bytes\n",
						srcbuf[6]);
				printf("brakepoint 4 - before xor first char %c bytes\n",
						srcbuf[7]);
				printf("brakepoint 4 - before xor first char %c bytes\n",
						srcbuf[8]);
				printf("brakepoint 4 - before xor first char %c bytes\n",
						srcbuf[9]);
				printf("brakepoint 4 - before xor first char %c bytes\n",
						srcbuf[10]);
				printf("brakepoint 4 - before xor first char %c bytes\n",
										srcbuf[11]);
				printf("\nbrakepoint 4.5 -after rcv in file from client : %s\n",
						srcbuf);

				if (xor_buffers(&srcbuf[5], needToRead, fdkey) < 0) {
					printf("error occured - xor buffers failed \n");
					return -1;
				}

				int totalsent = 0;
				int notwritten = needToRead;
				while (notwritten > 0) {
					/* notwritten = how much we have left to write
					 totalsent  = how much we've written so far
					 nsent = how much we've written in last write() call */
					nsent = write(connfd, &srcbuf[5] + totalsent, notwritten);
					if (nsent < 0) {
						printf("error occured - write to server \n");
						return -1;
					}
					totalsent += nsent;
					notwritten -= nsent;
				}
				printf("brakepoint 5 - after send to client : %d bytes\n",
						totalsent);
				printf("\nbrakepoint 5.5 -after send enc file to client : %s\n",
						&srcbuf[5]);
				if (needToRead != totalsent) {
					printf(
							"error occured - sending enc file to server failed \n");
					return -1;
				}
			}
			printf("brakepoint 6 - disconnect from socket\n\n");
			close(connfd);
			close(fdkey);
		} else if (forked < 0) {
			printf("error occured - forked failed \n");
			return -1;
		}
	}
	close(listenfd);
	return 0;
}

