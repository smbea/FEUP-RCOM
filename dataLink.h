#ifndef DATALINK_H
#define DATALINK_H

#include "stateMachine.h"

//variables
#define BAUDRATE B38400

enum port{
  COM1 = 1,
  COM2 = 2
};

stateMachine st;

//functions
int llopen(int port, int f);
int llclose(int fd);


#endif