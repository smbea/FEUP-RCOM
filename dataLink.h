#ifndef DATALINK_H
#define DATALINK_H

#include "stateMachine.h"

#define BAUDRATE B38400
stateMachine st;

enum R_E_FLAG{
  EMISSOR_FLAG = 0,
  RECEIVER_FLAG = 1
};

enum port{
  COM1 = 0,
  COM2 = 1
};

//functions
int llopen(int port, int f);
void open_receiver(int fd);
void atende(int signo);
void open_emissor(int fd);
void send_SET(int fd);
void send_UA(int fd);

#endif
