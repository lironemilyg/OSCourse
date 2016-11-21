#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h> // for open flags
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define NAMELEN 100
#define READBYTE 2048

//declaration of helper function
int xorfiles(char *inputName, char *encName, char *keyFile, int kf);

int main(int argc, char *argv[]) { //from tutorial point: https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
	if (argc > 4) {
		printf("Too many arguments supplied.\n");
		return -1;
	} else if (argc < 4) {
		printf("Three arguments expected.\n");
		return -1;
	}

	//Exactly 3 arguments supplied
	DIR *resD, *sourceD;
	char inputName[NAMELEN], encName[NAMELEN];
	char *resultDir, *sourceDir, *keyFile;
	int kf, xorFlag;
	struct stat s;
	struct dirent *sdf;
	sourceDir = argv[1];
	keyFile = argv[2];
	resultDir = argv[3];

	//openning src directory
	if ((sourceD = opendir(sourceDir)) == NULL) {
		printf("Error opening src directory\n");
		return -1; // ERROR!
	}

	//try to open key file
	if ((kf = open(keyFile, O_RDONLY)) < 0) {
		printf("Error opening key file\n");
		return -1;
	}

	if (stat(resultDir, &s) == -1) { //checking if result directory exist
		if ( errno == 2) { //No such file or directory error
			int temp = mkdir(resultDir, 0777);
			if (temp != 0) { //checking creating directory
				printf("Error creating directory\n");
				return -1;
			}
		}
	}

	//result directory exist. try to open result directory
	if ((resD = opendir(resultDir)) == NULL) {
		printf("Error opening result directory\n");
		return -1; // ERROR!
	}

	while ((sdf = readdir(sourceD)) != NULL) {
		//creating filenames
		sprintf(inputName, "%s/%s", sourceDir, sdf->d_name);
		sprintf(encName, "%s/%s", resultDir, sdf->d_name);

		if (stat(inputName, &s) < 0) {
			printf("Error getting stat of file: %s\n", strerror(errno));
			return -1; // ERROR!
		}

		// skip dirs
		if (S_ISDIR(s.st_mode)) {
			continue;
		}

		xorFlag = xorfiles(inputName, encName, keyFile, kf);
		if (xorFlag != 0) {
			break;
		}

	}
	if (close(kf) < 0) {
		printf("Error close key file\n");
		return -1; // ERROR!
	}
	if (closedir(resD) < 0) {
		printf("Error close result directory \n");
		return -1; // ERROR!
	}
	if (closedir(sourceD) < 0) {
		printf("Error close source directory \n");
		return -1; // ERROR!
	}

}

int xorfiles(char *inputName, char *encName, char *keyFile, int kf) {
	int inpF, encF, i, j = 0;
	mode_t mode = O_RDWR | O_CREAT | O_TRUNC;
	char bufI[READBYTE], bufK[READBYTE], bufR[READBYTE];
	ssize_t lenOutput, lenInput, lenKey;
	bool flag = true;

	//try to open input file
	if ((inpF = open(inputName, O_RDONLY)) < 0) {
		printf("Error opening input file\n");
		return -1;
	}

	//try to open output file to write
	if ((encF = open(encName, mode, 644)) < 0) {
		printf("Error opening output file\n");
		return -1;
	}
	lenKey = read(kf, bufK, READBYTE); //read key file - error will check in loop
	//we can use in 1MB of memory so we can save the times our program read/write to file.
	while (flag) {
		lenInput = read(inpF, bufI, READBYTE); //read input file
		if (lenKey < 0 || lenInput < 0) {
			printf("Error reading from file\n");
			if (close(kf) < 0) {
				printf("Error close key file\n");
			}
			if (close(encF) < 0) {
				printf("Error close encripted file\n");
			}
			if (close(inpF) < 0) {
				printf("Error close input file\n");
			}

			return -1;
		}

		for (i = 0; i < lenInput; i++) {
			bufR[i] = bufI[i] ^ bufK[j];
			j++;

			if (lenKey <= j) {  //checking if we end our key buffer.
				if (close(kf) < 0) {
					printf("Error close key file\n");
					return -1; // ERROR!
				}
				if ((kf = open(keyFile, O_RDONLY)) < 0) {
					printf("Error opening key file\n");
					return -1;
				}
				j = 0;
				if ((lenKey = read(kf, bufK, READBYTE)) < 0) {
					printf("Error reading from file\n");
					if (close(kf) < 0) {
						printf("Error close key file\n");
					}
					if (close(encF) < 0) {
						printf("Error close encripted file\n");
					}
					if (close(inpF) < 0) {
						printf("Error close input file\n");
					}
					return -1;
				}
			}
		}
		lenOutput = write(encF, bufR, i); //we write only the bytes we read

		if (lenOutput < 0) {
			printf("Error writing to file: %s\n", strerror(errno));
			if (close(kf) < 0) {
				printf("Error close key file\n");
			}
			if (close(encF) < 0) {
				printf("Error close encripted file\n");
			}
			if (close(inpF) < 0) {
				printf("Error close input file\n");
			}
			return -1;
		}

		if (lenInput < READBYTE) {
			flag = false;
		}
	}

	if (close(encF) < 0) {
		printf("Error close encripted file\n");
		return -1;
	}
	if (close(inpF) < 0) {
		printf("Error close input file\n");
		return -1;
	}
	return 0;
}
