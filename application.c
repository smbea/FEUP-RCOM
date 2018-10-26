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
#include <math.h>

int log2(int x)
{
	return (int)(log10(x)/log10(2.0) + 0.5);
}


char* generateControlPacket(int start_end_flag, char* file_name, char* file_size)
{
	int i = 0, j = 0;

	char* packet;

	if(start_end_flag == START)
		packet[0] = 0x02;
	else if(start_end_flag == END)
		packet[0] = 0x03;

	i++;

	packet[i++] = FILE_SIZE;
	packet[i++] = (char)strlen(file_size);

	for(j = 0; j < strlen(file_size); j++)
		packet[i++] = file_size[j];

	packet[i++] = FILE_NAME;
	packet[i++] = (char)strlen(file_name);

	for(j = 0; j < strlen(file_name); j++)
		packet[i++] = file_name[j];

	return packet;
}

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
	if(status == TRANSMITTER) application.send_fd = open(send_file,O_RDWR);
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
