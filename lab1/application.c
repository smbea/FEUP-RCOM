#include "application.h"
#include "math.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <math.h>
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


	// File size information
	packet[++i] = fileSizeIndicator;
	int s = (log2(sendFile.fileSize))/ 8 + 1;
	packet[++i] = s;

	for(i = 3; s > 0; i++, s--){
		packet[i] = (sendFile.fileSize>>((s-1)*8))&0xff;
	}

	// File name field
	packet[i] = fileNameIndicator;
	packet[++i] = (unsigned char)strlen(sendFile.fileName);

	for(j = 0, i++; j < strlen(sendFile.fileName); j++, i++)
		packet[i] = sendFile.fileName[j];

	return i;
}


int main(int argc, char** argv){

	int port;
	application.dataSize = 512 ;
	application.dataPacketSize = application.dataSize + 4;
	application.sequenceNumber = 1;

	if(argc < 3 || (argc < 4 && (atoi(argv[2]) == 0))){
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
	if(status == TRANSMITTER)
		sendFile.fd = open(sendFile.fileName,O_RDWR);

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

	if(status == TRANSMITTER) sendData();
	else readData();

	llclose(application.fd,status);

	return 0;

}

//untested
int sendData(){
	printf("\n----------Control packet----------\n");
	sendControlPacket(start);

	printf("\n-----------Data packets------------\n");
	sendDataPackets();

	printf("\n----------Control packet----------\n");
	sendControlPacket(end);
	return 0;
}


int readData(){
	printf("\n----------Control packet----------\n");
	readControlPacket(start);
	printf("\n-----------Data packets------------\n");
	readDataPackets();
	printf("\n----------Control packet----------\n");
	readControlPacket(end);
	return 0;
}



void sendControlPacket(int start_end_flag){
	unsigned char packet[application.dataPacketSize];
	int packet_size = generateControlPacket(start, packet);

	llwrite(application.fd, packet, packet_size);
}

int getFileInfo(unsigned char* buf, unsigned char* file, int info)
{
	int index = 1, i = 0, fileInfoSize= 0;
	while(1){
		if(buf[index++] == info)
		{

			fileInfoSize  = (int)buf[index++];
			for(i = 0; i < fileInfoSize; i++){
				file[i] = buf[index++];
			}
			file[fileInfoSize] = '\0';
			break;
		}
		else{
			index += ((int)buf[index]) +1; //skips
		}
	}

	return fileInfoSize+1;
}

int convertS2int(unsigned char* s, int t)
{
	int i, final = 0, pot = 0;
	for(i = t - 2; i >= 0; i--)
	{
		final += s[i] * pow(256,pot);
		pot++;
	}
	return final;
}

void readControlPacket(int start_end_flag){
	unsigned char packet[application.dataPacketSize];
	unsigned char fileName[255], fileSize[255];

	llread(application.fd, packet);

	getFileInfo(packet, fileName, fileNameIndicator);
	int sizeofSize = getFileInfo(packet, fileSize, fileSizeIndicator);

	if(start_end_flag == start){
		sendFile.fd = open((char *) fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
		if (sendFile.fd < 0)
			{
				perror(sendFile.fileName);
				exit(-1);
			}

		sendFile.fileSize = convertS2int(fileSize, sizeofSize);
	}
}


void sendDataPackets(){
	int res = 0;
	unsigned char data[application.dataSize+1];
	unsigned char packet[application.dataPacketSize];
	int packetSize;

	while((res = read(sendFile.fd,&data,application.dataSize))>0){
		packetSize = generateDataPacket(data,res,packet);

		if(llwrite(application.fd,packet,packetSize)>0){
			printf("\n %d. sent %d bytes",application.sequenceNumber,res);
			application.sequenceNumber++;
		}
		else{
			printf("Alarm count reached maximum value: Exit(1)\n");
			exit(1);
		}
	}
}

int roundExcess(int a, int b)
{
		int quocInt = (int) a/b;
		double quocDouble = (double)((double)a/(double)b);
		if((quocDouble - (double)quocInt) != 0)
			return quocInt + 1;
		else
			return quocInt;
}

void readDataPackets(){
	int packetsSending = roundExcess(sendFile.fileSize, application.dataSize);
	int count = 1;
	unsigned char buffer[application.dataPacketSize+6+1];
	int res = 0;
	printf("Reading data packets\n");

	while(count <= packetsSending){
		printf("\n %d.  ",count);
		res = llread(application.fd,buffer)-dataPHSize;
		if(res > 0){
			write(sendFile.fd, buffer + dataPHSize, res);
			printf(", received %d bytes\n", res);
			count++;
		}
		else if(res ==-2-dataPHSize){
			printf("Duplicate\n");

		}
		else if(res ==-3-dataPHSize){
			printf("Timeout\n");
			exit(1);
		}
		else{
			printf("Packet was rejected, resend!\n");
		}
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
