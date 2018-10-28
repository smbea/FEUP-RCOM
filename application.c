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



int generateControlPacket(int start_end_flag, char* file_size, char* packet)
{

		packet = malloc(500);
	int i = 0, j = 0;
	char * temp;

//start or end
	if(start_end_flag == start)
		sprintf(temp, "%d",start);
	else if(start_end_flag == end)
		sprintf(temp, "%d",end);

		strcat(packet,temp);
	 printf("1. %s -%s\n",temp,packet);
	i++;

//filesizeindicator
	 sprintf(temp, "%d",fileSizeIndicator);
	 strcat(packet,temp);
	printf("1. %s -%s\n",temp,packet);

//file size
		strcat(packet,file_size);
		printf("1. %s -%s\n", file_size,packet);

		packet[i++] = fileNameIndicator;

//file name
		sprintf(temp, "%d",fileNameIndicator);
 	 	strcat(packet,temp);

//fileName
	strcat(packet, sendFile.fileName);
		printf("3. %s\n", packet);
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

	char fs[4];
	sprintf(fs, "%d", sendFile.fileSize);
	//printf("packet size: %d\n",fs[0]);


	char packet[255];
	int packet_size = generateControlPacket(0x02, fs, packet);

	int k;
	//printf("packet:\n");
	for(k= 0; k < packet_size; k++)
		//printf("%x\n", packet[k]);

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
