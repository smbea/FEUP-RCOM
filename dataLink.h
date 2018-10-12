#ifndef DATALINK_H
#define DATALINK_H


//variables
#define BAUDRATE B38400

enum port{
  COM1 = 1,
  COM2 = 2
};

//functions
int llopen(int port, int f);


#endif