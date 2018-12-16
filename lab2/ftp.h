#ifndef __FTP_H
#define __FTP_H

#include <stdint.h>

#define TRUE 1
#define FALSE 0
#define FTP_RESPONSE_SIZE 20

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
Ftp ftp_init(uint8_t *host, uint8_t* username, uint8_t* password);

/**
 * @brief Establishes connection with a FTP server. Once the function returns, the server is ready for new commands
 * 
 * @return Returns the socket file descriptor. Upon errors it returns -1 (failed to connect with server)
 */
int ftp_connectToServer(const Ftp *ftp);

int16_t ftp_getResponse(int sockfd);

int ftp_sendCommand(int sockfd, const char *command, const char *argument);

int ftp_authenticateUser(const Ftp *ftp, int sockfd);
#endif