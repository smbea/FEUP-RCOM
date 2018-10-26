#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dataLink.h"

struct applicationLayer {
  int fd; /*Descritor correspondente à porta série*/
  int status; /*TRANSMITTER | RECEIVER*/
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


/**
 * @brief Gets the file size in bytes
 * 
 * @param fd The file descriptor of the file
 * @return int Returns -1 upon errors and prints an appropriate message, otherwise returns the file size in bytes
 */
int getFileSize(int fd);