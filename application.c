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


int generateDataPacket(char sequenceNumber, char* data, char* packet){

	packet[0] = 0x01;
	packet[1] = sequenceNumber % 255;
	int dataSize = sizeof(data);
	int l2 = dataSize / 256;
	int l1 = dataSize % 256;
	packet[2] = (char)l2;
	packet[3] = (char)l1;
	int h;
	int index = 3;
	for(h = 0; h < dataSize; h++)
	{
		packet[++index] = data[h];
	}
	return index;
}



int generateControlPacket(int start_end_flag, char* file_name, char* file_size, char* packet)
{
	int i = 0, j = 0;


	if(start_end_flag == APP_START)
		packet[0] = 0x02;
	else if(start_end_flag == APP_END)
		packet[0] = 0x03;

	i++;

	packet[i++] = FILE_SIZE;
	packet[i++] = (char)strlen(file_size); //NOPE

	for(j = 0; j < strlen(file_size); j++)
		packet[i++] = file_size[j];

	packet[i++] = FILE_NAME;
	packet[i++] = (char)strlen(file_name); //NOPE

	for(j = 0; j < strlen(file_name); j++)
		packet[i++] = file_name[j];

	return i;
}

int main(int argc, char** argv){

	int port;
	int file_size;
	applicationLayer application;

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

	file_size = getFileSize(application.send_fd);

	char fs[4];
	sprintf(fs, "%d", file_size);

	/*int g;
	for(g= 0; g < 12; g++)
	printf("%x\n", fs[g]);*/
	//debugging

	application.send_fd = llopen(port, status);

	char packet[255];
	int packet_size = generateControlPacket(0x02, "teste1", fs, packet);

	int k;
	printf("packet:");
	for(k= 0; k < packet_size; k++)
		printf("%x\n", packet[k]);

	llwrite(application.send_fd, packet, packet_size);

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
