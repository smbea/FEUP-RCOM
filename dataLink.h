#ifndef DATALINK_H
#define DATALINK_H

#include "stateMachine.h"

stateMachine st;


struct linkLayer {
int baudRate; /*Velocidade de transmissão*/
unsigned char sequenceNumber; /*Número de sequência da trama: 0, 1*/
unsigned int timeout; /*Valor do temporizador: 1 s*/
unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
unsigned char frame[512]; /*Trama*/
};

struct linkLayer dataLink;

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
int llwrite(int fd, unsigned char * buffer, int length);
int llread(int fd, unsigned char * buffer);
int llclose(int fd, int r_e_flag);
void open_receiver(int fd);
void open_emissor(int fd);
void send_SET(int fd);
void send_UA(int fd);
int send_I(int fd, unsigned char *data, int length, byte bcc2);
void byteStuffing(unsigned char *buffer, int length, unsigned char *stuffedBuffer, int* newLength);
int byteDestuffing(unsigned char* stuffedBuffer, int length, unsigned char* destuffedBuffer);
void atende(int signo);
unsigned char getBCC(unsigned char* buffer, int length, int r_e_flag);
void genNextNs();
void genNextNr(unsigned char received_ns);

#endif
