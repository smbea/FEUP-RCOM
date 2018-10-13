#ifndef DATALINK_H
#define DATALINK_H

#include "stateMachine.h"

//variables
#define BAUDRATE B38400
#define EMISSOR_FLAG 0
#define RECEIVER_FLAG 1

stateMachine st;

enum port{
  COM1 = 0,
  COM2 = 1
};

//functions
int llopen(int port, int f);
void open_receiver(int fd);
void open_emissor(int fd);
int llclose(int fd);
#endif