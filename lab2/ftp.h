#ifndef __FTP_H
#define __FTP_H

#include <stdint.h>


typedef struct {
	uint8_t protocol[6];
	uint8_t user[256];
	uint8_t password[256];
	uint8_t host[4];
	uint8_t port[4];
	uint8_t path[4];
} Ftp;


/**
 * Given an hostname (such as google.com) it returns the address in IPv4 dotted notation (216.58.201.174)
 * @param hostname
 * @return 
 */
char* getIPv4_FromHostName(const char* hostname);

#endif