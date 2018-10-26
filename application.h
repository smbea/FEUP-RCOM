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
