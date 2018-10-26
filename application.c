#include "application.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

int main(int argc, char** argv){

	int port;

	if (argc < 4)
	{
		printf("Incorrect number of arguments\n");
		exit(1);
	}

	//port
	if (atoi(argv[1]) == 0)
		port = COM1;
	else if (atoi(argv[1]) == 1)
		port = COM2;
	else
	{
		printf("First argument invalid\n");
		exit(1);
	}

	//status
	if (atoi(argv[2]) == 0)
		status = TRANSMITTER;
	else if (atoi(argv[2]) == 1)
		status = RECEIVER;
	else
	{
		printf("Second argument invalid\n");
		exit(1);
	}

	//file
	char * send_file;
	send_file = argv[3];
	if(status = TRANSMITTER) application.send_fd = open(send_file,O_RDWR);
	else application.send_fd = open(send_file,O_WRONLY);

	if (application.send_fd < 0)
	{
		perror(send_file);
		exit(-1);
	}


	application.send_fd = llopen(port, status);

	return 0;

}


int getFileSize(int fd) {
	struct stat statbuf;
	
	if(fstat(fd, &statbuf) != 0) {
		perror(NULL);
		return -1;
	}

	return statbuf.st_size;
}