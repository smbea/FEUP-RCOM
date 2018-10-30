#ifndef DATALINK_H
#define DATALINK_H

#include "stateMachine.h"

enum R_E_FLAG{
  EMISSOR_FLAG = 0,
  RECEIVER_FLAG = 1
};

enum port{
  COM1 = 0,
  COM2 = 1
};

/**
 * @brief 
 * 
 * @param port 
 * @param status 
 * @retval -1 Unknown port
 * @retval -2 Couldn't open serial port interface
 * @retval -3 Coudln't get current terminal interface configuration
 * @retval -4 Failed to apply terminal interface configuration
 */
int llopen(int port, int status);

/**
 * @brief 
 * 
 * @param fd 
 * @param buffer 
 * @param length 
 * @return int 
 */
int llwrite(int fd, unsigned char * buffer, int length);

/**
 * @brief 
 * 
 * @param fd 
 * @param buffer 
 * @return int 
 */
int llread(int fd, unsigned char * buffer);

/**
 * @brief 
 * 
 * @param fd 
 * @param r_e_flag 
 * @return int 
 */
int llclose(int fd, int r_e_flag);

void send_SET(int fd);
void send_UA(int fd);
int send_I(int fd, unsigned char *data, int length, byte bcc2);
void byteStuffing(unsigned char *buffer, int length, unsigned char *stuffedBuffer, int* newLength);
int byteDestuffing(unsigned char* stuffedBuffer, int length, unsigned char* destuffedBuffer);
void atende(int signo);
unsigned char getBCC(unsigned char* buffer, int length);
void genNextNs();
void genNextNr(unsigned char received_ns);

#endif
