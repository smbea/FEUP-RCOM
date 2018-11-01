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

#define dataPHSize 4

//untested
int generateDataPacket(unsigned char* data, int size, unsigned char* packet){

	int h;
	int index = 3;
	int l2 = size / 256;
	int l1 = size % 256;

	packet[0] = APP_DATA;
	packet[1] = application.sequenceNumber % 255;
	
	packet[2] = (unsigned char)l2;
	packet[3] = (unsigned char)l1;

	for(h = 0; h <= size; h++)
	{
		packet[++index] = data[h];
	}
	return index+1;

	
}



int generateControlPacket(int start_end_flag, unsigned char* packet)
{
	int i = 0, j = 0;


	if(start_end_flag == start)
		packet[0] = APP_START;
	else if(start_end_flag == end)
		packet[0] = APP_END;


	packet[++i] = fileSizeIndicator;
	packet[++i] = sizeof(sendFile.fileSize);

	int s = sizeof(sendFile.fileSize);

	for(i = 3; s-- > 0 ; i++){
		packet[i] = (sendFile.fileSize>>(s*8))&0xff;
	}

	packet[++i] = fileNameIndicator;
	packet[++i] = (unsigned char)strlen(sendFile.fileName);

	for(j = 0; j < strlen(sendFile.fileName); j++)
		packet[++i] = sendFile.fileName[j];

	return i;
}


int main(int argc, char** argv){

	int port;
	application.dataSize = 512 ;
	application.dataPacketSize = application.dataSize + 4;
	application.sequenceNumber = 1;

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
	if(application.fd < 0) {
		printf("ERROR: It wasn't possible to establish connection. Cannot proceed\n");
		exit(-1);
	}
	application.sequenceNumber = 0;

	if(status == TRANSMITTER) sendData();
	else readData();

	return 0;

}

//untested
int sendData(){
	printf("Sent control packet\n");
	sendControlPacket(start);

	printf("Begining to send data packets\n");
	sendDataPackets();
	return 0;
}


int readData(){
	readControlPacket(start);
	readDataPackets();
	return 0;
}



void sendControlPacket(int start_end_flag){
	unsigned char packet[application.dataPacketSize];
	int packet_size = generateControlPacket(start, packet);

	llwrite(application.fd, packet, packet_size);
}


void readControlPacket(int start_end_flag){
	unsigned char packet[application.dataPacketSize];
	llread(application.fd, packet);

}


void sendDataPackets(){
	int res = 0;
	unsigned char data[application.dataSize];
	unsigned char packet[application.dataPacketSize];
	int packetSize;

	while((res = read(sendFile.fd,&data,application.dataSize))>0){
		packetSize = generateDataPacket(data,res,packet);
		
		if(llwrite(application.fd,packet,packetSize)>0){
			printf("\n %d. sent %d bytes\n",application.sequenceNumber,res);
			application.sequenceNumber++;
		}
	}
}

void readDataPackets(){
	int packetsSending = (int) sendFile.fileSize/application.dataSize;
	int count = 1;
	unsigned char buffer[application.dataPacketSize+6];

	while(count <= packetsSending){
		printf("\n %d. received %d bytes\n",count, llread(application.fd,buffer)-dataPHSize);
		count++;
	}
}


int getFileSize(int fd) {
	struct stat statbuf;

	if(fstat(fd, &statbuf) != 0) {
		perror(NULL);
		return -1;
	}

	return statbuf.st_size;
}
