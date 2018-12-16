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

static void setIPFromHostName(Ftp *ftp){
	/**
	 * Get the address by hostname
	 * The address is in network byte order, which is the same as big endian
	 *  thus, the most significant byte is the first byte
	 * For google.com, the bytes are 0xd8 0x3a 0xd3 0x2e
	 * [SOURCE](https://www.ibm.com/support/knowledgecenter/en/SSB27U_6.4.0/com.ibm.zvm.v640.kiml0/asonetw.htm)
	 */
	struct hostent *h = gethostbyname(ftp->hostname);
	
	/*
	 * Convert the address from network byte order to IPv4 address in standard dot notation
	 * The google example, now will be translated to 216.58.201.174
	 */
	struct in_addr in;
	memcpy(&in.s_addr, h->h_addr, 4); // h->h_addr is the first address, extracted from the array (char **) h->h_addr_list
	char* address = inet_ntoa(in);
	
	// fill ftp structure
	memcpy(ftp->host_ipv4_address, address, strlen(address));
	memcpy(ftp->host_network_byte_order, h->h_addr, 4);
}

int main() {
	Ftp ftp = ftp_init("ftp.secyt.gov.ar", NULL, NULL);
	int sockfd = ftp_connectToServer(&ftp);
	
	ftp_authenticateUser(&ftp, sockfd) ;
	
	
	//ftp_getResponse(sockfd);
	close(sockfd);
	return 0;
}

Ftp ftp_init(uint8_t *host, uint8_t* username, uint8_t* password) {
	Ftp ftp;
	// add the host name
	memset(ftp.hostname, 0, 256);
	memcpy(ftp.hostname, host, strlen(host));

	// fill address in network byte order and ipv4 dotted format
	setIPFromHostName(&ftp);

	// add port (default)
	ftp.port = 21;

	// add username
	memset(ftp.user, 0, 256);
	if(username == NULL)
		memcpy(ftp.user, "anonymous", 10);
	else 
		memcpy(ftp.user, username, strlen(username));
	
	// add password 
	memset(ftp.password, 0, 256);
	if(username == NULL) 
		memcpy(ftp.password, "ident", 10);
	else
		memcpy(ftp.password, username, strlen(username));

	return ftp;
}

int ftp_connectToServer(const Ftp *ftp) {
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
	server_addr.sin_addr.s_addr = inet_addr(ftp->host_ipv4_address); /* 32 bit Internet address network byte ordered */
	server_addr.sin_port = htons(ftp->port); /* server TCP port must be network byte ordered */
   	
	if(connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
        perror("connect()");
		exit(0);
	}

	/* Wait for server response */
	uint16_t response = ftp_getResponse(sockfd);
	if(response == 220) {
		printf("Success: Established connection with %s\n", ftp->hostname);
		return sockfd;
	}
	else if (response == 120) {
		printf("Wait: Server sent 120 code. Waiting for a new response\n");
		response = ftp_getResponse(sockfd);
		if(response == 220) {
			printf("Success: Established connection with %s\n", ftp->hostname);
			return sockfd;
		} else {
			return -1; // TODO
		}
	}
	else if(response == 421) {
		printf("Failed: Service is not available\n");
	}
}

int16_t ftp_getResponse(int sockfd) {
	bool reachedTelnetEOF = FALSE; // flag that tells if we reached the telnet EOF, setting the end of the response
	bool isMultiLineResponse = FALSE; // flag that tells if response is multiline
	char buf[FTP_RESPONSE_SIZE]; // buffer to hold response text
	char responseCode[3]; // buffer to hold response code

	// read the first 3 bytes, which are the response code
	read(sockfd, &responseCode, 3);

	// read the next byte which is either a space (single line response) or '-' (multiline response)
	read(sockfd, buf, 1);
	if(buf[0] == '-')
		isMultiLineResponse = TRUE;
	else if(buf[0] != ' ')
		return -1; // unexpected
	
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

	return atoi(responseCode);
}

int ftp_sendCommand(int sockfd, const char *command, const char *argument) {
	char buffer[1024];
	sprintf(buffer, "%s %s\r\n", command, argument);
	write(sockfd, buffer, strlen(buffer));
	return ftp_getResponse(sockfd);
}

int ftp_sendUserCommand(const Ftp *ftp, int sockfd) {
	int responseCode = ftp_sendCommand(sockfd, "USER", ftp->user);
	
	switch(responseCode) {
		case 230:
			printf("User logged in\n");
			return 0;
		case 331: case 332:
			printf("Password expected\n");
			return 1;
		case 530:
			printf("Invalid username %s\n", ftp->user);
			return -1;
		case 500: case 501: case 421:
			printf("Panic: User command responded with %d\n", responseCode);
			return -2;
		default:
			printf("Unexpected response %d\n", responseCode);
			return -3;
	}
}

int ftp_sendPasswordCommand(const Ftp *ftp, int sockfd) {
	int responseCode = ftp_sendCommand(sockfd, "PASS", ftp->password);
}

int ftp_authenticateUser(const Ftp *ftp, int sockfd) {
	ftp_sendUserCommand(ftp, sockfd);
	ftp_sendPasswordCommand(ftp, sockfd);
}