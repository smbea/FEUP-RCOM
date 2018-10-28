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



int generateControlPacket(int start_end_flag, char* packet)
{
	int i = 0, j = 0;


	if(start_end_flag == start)
		packet[0] = APP_START;
	else if(start_end_flag == end)
		packet[0] = APP_END;


	packet[++i] = fileSizeIndicator;
	packet[++i] = sizeof(sendFile.fileSize);

	int s = sizeof(sendFile.fileSize);

	for(i = 0; s-- > 0 ; i++){
		packet[i] = (sendFile.fileSize>>(s*8))&0xff;
	}

	packet[++i] = fileNameIndicator;
	packet[++i] = (char)strlen(sendFile.fileName);

	for(j = 0; j < strlen(sendFile.fileName); j++)
		packet[++i] = sendFile.fileName[j];

		for(j = 0; j < i; j++)
			printf("%x\n", packet[j]);

	return i;
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
	sendFile.fileName = argv[3];
	if(status == TRANSMITTER) sendFile.fd = open(sendFile.fileName,O_RDWR);
	else sendFile.fd = open(sendFile.fileName,O_WRONLY);

	if (sendFile.fd < 0)
	{
		perror(sendFile.fileName);
		exit(-1);
	}

	sendFile.fileSize = getFileSize(sendFile.fd);

	//llopen
	application.fd = llopen(port, status);

	//testing
	char packet[255];
	int packet_size = generateControlPacket(start, packet);

	llwrite(application.fd, packet, packet_size);


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
