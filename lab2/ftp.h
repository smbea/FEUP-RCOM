#ifndef __FTP_H
#define __FTP_H

#include <stdint.h>

#define TRUE 1
#define FALSE 0
#define FTP_RESPONSE_SIZE 51

typedef uint8_t bool;


typedef struct {
	// server address information
	char hostname[256]; // the ftp server hostname
	char host_network_byte_order[5]; // holds the server address in network byte order (4 bytes)
	char host_ipv4_address[16]; // the server address in ipv4 dotted format (maximum 15 chars)
	uint16_t port; // todo

	// user
	char user[256];
	char password[256];
	
	// path
	char path[1024];
} Ftp;

/**
 * @brief 
 * 
 * @param host 
 * @return Ftp 
 */
Ftp initFtp(uint8_t *host);

/**
 * Fills ftp structure with the server address in network byte order and ipv4 dotted format
 */
void setIPFromHostName(Ftp *ftp);

/**
 * @brief Establishes connection with a FTP server. Once the function returns, the server is ready for new commands
 * 
 * @return Returns the socket file descriptor. Upon errors it returns -1 (failed to connect with server)
 */
int connectToFtpServer(const Ftp *ftp);

int16_t getFtpResponse(int sockfd);

int sendFtpCommand(int sockfd, char* command, char* argument);
#endif