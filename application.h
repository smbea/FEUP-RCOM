#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dataLink.h"

struct applicationLayer {
  int port_fd; /*Descritor correspondente à porta série*/
  int status; /*TRANSMITTER | RECEIVER*/
  int send_fd;
};

struct applicationLayer application;

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