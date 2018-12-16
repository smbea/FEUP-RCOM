#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ftp.h"


#include <stdio.h>


int main() {
	Ftp ftp = initFtp("speedtest.tele2.net");

	char * ipv4 = getIPv4_FromHostName(ftp.host);
	connectToFtpServer(ipv4, NULL);
}

char* getIPv4_FromHostName(const char* hostname) {
	/**
	 * Get the address by hostname
	 * The address is in network byte order, which is the same as big endian
	 *  thus, the most significant byte is the first byte
	 * For google.com, the bytes are 0xd8 0x3a 0xd3 0x2e
	 * [SOURCE](https://www.ibm.com/support/knowledgecenter/en/SSB27U_6.4.0/com.ibm.zvm.v640.kiml0/asonetw.htm)
	 */
	struct hostent *h = gethostbyname(hostname);
	
	/*
	 * Convert the address from network byte order to IPv4 address in standard dot notation
	 * The google example, now will be translated to 216.58.201.174
	 */
	struct in_addr in;
	memcpy(&in.s_addr, h->h_addr, 4); // h->h_addr is the first address, extracted from the array (char **) h->h_addr_list
	char* address = inet_ntoa(in); // the returned string is statically allocated (careful with sub sequent calls)
	
	return address;
}

int connectToFtpServer(const char* server_address, unsigned char* port) {
	uint16_t tcp_port = 21; // default port for FTP
	int	sockfd;
	
	/* Open socket in bidirectional connection mode (SOCK_STREAM) using IPv4 protocol (AF_INET) */
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    	perror("socket()");
        exit(0);
    }
	
	/* Setup and connect to the server */
	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET; /* IPv4 */
	server_addr.sin_addr.s_addr = inet_addr(server_address); /* 32 bit Internet address network byte ordered */
	if(port != NULL) tcp_port = (uint16_t) *port;
	server_addr.sin_port = htons(tcp_port); /* server TCP port must be network byte ordered */
   	
	if(connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
        perror("connect()");
		exit(0);
	}
	
	return sockfd;
}