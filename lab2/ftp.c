#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "ftp.h"


#include <stdio.h>

// SOME FTP SERVERS
// ftp.secyt.gov.ar (welcome message response is multiline)
// speedtest.tele2.net (anonymous ftp, several files, however parent directory...)

int main() {
	Ftp ftp = initFtp("ftp.secyt.gov.ar");

	char * ipv4 = getIPv4_FromHostName(ftp.host);
	int sockfd = connectToFtpServer(ipv4, NULL);
	getFtpResponse(sockfd);

	// auth
	sendFtpCommand(sockfd, "USER", "anonymous");
	sendFtpCommand(sockfd, "PASS", "ident");
	
	
	//getFtpResponse(sockfd);
	close(sockfd);
	return 0;
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

int getFtpResponse(int sockfd) {
	bool reachedTelnetEOF = FALSE; // flag that tells if we reached the telnet EOF, setting the end of the response
	bool isMultiLineResponse = FALSE; // flag that tells if response is multiline
	bool isLastLine = FALSE; // flag indicating the current line is the last one. To be used when isMultiLineResponse is true. The last line is reached if it starts with the response code
	char buf[FTP_RESPONSE_SIZE]; // buffer to hold response text
	char responseCode[3]; // buffer to hold response code

	// read the first 3 bytes, which are the response code
	read(sockfd, &responseCode, 3);

	// read the next byte which is either a space (single line response) or '-' (multiline response)
	read(sockfd, buf, 1);
	if(buf[0] == '-')
		isMultiLineResponse = TRUE;
	
	// read remaining response
	while(!reachedTelnetEOF) {
		// read up to FTP_RESPONSE_SIZE bytes
		ssize_t read_bytes = read(sockfd, &buf, FTP_RESPONSE_SIZE);

		// disply ftp response
		write(STDOUT_FILENO, &buf, read_bytes);

		// check if we reached end-of-telnet <CRLF>
		if(isMultiLineResponse) {
			/* For multiline responses we reach the end of the response when we find the response code
			 *  i.e. the specification says the last telnet line starts with the response code
			 * Thus, we search for the code on the buffer. if we found it, then we are reading the last telnet line, *  which ends once we reach the end-of-telnet <CRLF>
			 */
			for(int i = 0; i < read_bytes-3; i++)
				if(memcmp(responseCode, buf+i, 3) == 0)
					isMultiLineResponse = FALSE; // small trick to reuse the code below. In fact, the remaning response is no longer multiline
		} else {
			/* 
			 * For single line responses, we reach the end of response when
			 *  the last two bytes are CR and LF respectively
			 */
			if(buf[read_bytes-2] == '\r' && buf[read_bytes-1] == '\n')
				reachedTelnetEOF = TRUE;
		}
	}

	return 0;
}

int sendFtpCommand(int sockfd, char* command, char* argument) {
	char buffer[1024];
	sprintf(buffer, "%s %s\r\n", command, argument);
	write(sockfd, buffer, strlen(buffer));
	getFtpResponse(sockfd);
}