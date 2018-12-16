#ifndef __FTP_H
#define __FTP_H

#include <stdint.h>

#define TRUE 1
#define FALSE 0

typedef uint8_t bool;


typedef struct {
	uint8_t user[256];
	uint8_t password[256];
	uint8_t host[256];
	uint8_t port[4];
	uint8_t path[1024];
} Ftp;

/**
 * @brief 
 * 
 * @param host 
 * @return Ftp 
 */
Ftp initFtp(uint8_t *host) {
	Ftp ftp;
	memset(ftp.host, 0, 256);
	memcpy(ftp.host, host, strlen(host));
	return ftp;
}


/**
 * Given an hostname (such as google.com) it returns the address in IPv4 dotted notation (216.58.201.174)
 * @param hostname
 * @return 
 */
char* getIPv4_FromHostName(const char* hostname);

/**
 * @brief Establishes connection with a FTP server. 
 * 
 * @param server_address The ftp server address in IPv4 numbers-and-dots notation
 * @param port The TCP port. If NULL, the default port 21 is used
 * 
 * @return Returns the file descriptor for the socket
 */
int connectToFtpServer(const char* server_address, unsigned char* port);

int getFtpResponse(int sockfd);

int sendFtpCommand(int sockfd, char* command, char* argument);
#endif