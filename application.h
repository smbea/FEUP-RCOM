#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dataLink.h"

struct applicationLayer {
  int fd; /*Descritor correspondente à porta série*/
  int status; /*TRANSMITTER | RECEIVER*/
  int sequenceNumber;
};

struct applicationLayer application;

enum packetControl{
  fileSizeIndicator = 0,
  fileNameIndicator = 1,
  start = 2,
  end = 3
};

struct sendFile{
  int fd;
  int fileSize;
  char * fileName;
};

struct sendFile sendFile;

enum possibleStatus{
  TRANSMITTER = 0,
  RECEIVER = 1
};

int status;

enum applicationPackets{
  APP_START = 0x02,
  APP_DATA = 0x01,
  APP_END = 0x03
};


/**
 * @brief Gets the file size in bytes
 * 
 * @param fd The file descriptor of the file
 * @return int Returns -1 upon errors and prints an appropriate message, otherwise returns the file size in bytes
 */
int getFileSize(int fd);
void sendControlPacket(int start_end_flag);
void readControlPacket(int start_end_flag);
int generateControlPacket(int start_end_flag, unsigned char* packet);
int sendData();
int readData();
void sendControlPacket(int start_end_flag);
void sendDataPackets();
