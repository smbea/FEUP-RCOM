#ifndef DATALINK_H
#define DATALINK_H

#include "stateMachine.h"

#define BAUDRATE B38400
stateMachine st;


struct linkLayer {
int baudRate; /*Velocidade de transmissão*/
unsigned char sequenceNumber; /*Número de sequência da trama: 0, 1*/
unsigned int timeout; /*Valor do temporizador: 1 s*/
unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
char frame[255]; /*Trama*/
};

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
void byteStuffing(char * buffer, int length, char * stuffedBuffer);
void genNextNs();
int llwrite(int fd, char * buffer, int length);

#endif
