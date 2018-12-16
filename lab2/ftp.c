#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

#include "ftp.h"

// SOME FTP SERVERS
// ftp.secyt.gov.ar (welcome message response is multiline)
// speedtest.tele2.net (anonymous ftp, several files, however parent directory...)
// ftp://test.rebex.net/ (has directories and is not anonymous. user: demo. password: password)
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
	//Ftp ftp = ftp_init("test.rebex.net", "demo", "password");
	Ftp ftp = ftp_init("speedtest.tele2.net", NULL, NULL, "512KB.zip");
	
	int sockfd = ftp_connectToServer(&ftp), sockfd_data;
	
	ftp_authenticateUser(&ftp, sockfd) ;
	
	ftp_sendPassiveCommand(&ftp, sockfd, &sockfd_data);
	ftp_sendRetrieveCommand(&ftp, sockfd, sockfd_data);
	
	//ftp_getResponse(sockfd);
	close(sockfd);
	return 0;
}

Ftp ftp_init(uint8_t *host, uint8_t* username, uint8_t* password, uint8_t* filename) {
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
		memcpy(ftp.password, password, strlen(password));

	// add file name
	memset(ftp.path, 0, 1024);
	memcpy(ftp.path, filename, strlen(filename));

	return ftp;
}

/**
 * @brief Create a Socket object
 * 
 * @param ipv4_address 
 * @param port 
 * @return int The file descriptor for the socket
 */
static int createSocket(const char *ipv4_address, uint16_t port) {
	int	sockfd;
	
	/* Open socket in bidirectional connection mode (SOCK_STREAM) using IPv4 protocol (AF_INET) */
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    	perror("socket()");
        return -1;
    }
	
	/* Setup and connect to the server */
	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET; /* IPv4 */
	server_addr.sin_addr.s_addr = inet_addr(ipv4_address); /* 32 bit Internet address network byte ordered */
	server_addr.sin_port = htons(port); /* server TCP port must be network byte ordered */
   	
	if(connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
        perror("connect()");
		return -2;
	}

	return sockfd;
}

int ftp_connectToServer(const Ftp *ftp) {

	int	sockfd = createSocket(ftp->host_ipv4_address, ftp->port);
	
	/* Wait for server response */
	uint16_t response = ftp_getResponse(sockfd, NULL);
	if(response == 220) {
		printf("Success: Established connection with %s\n", ftp->hostname);
		return sockfd;
	}
	else if (response == 120) {
		printf("Wait: Server sent 120 code. Waiting for a new response\n");
		response = ftp_getResponse(sockfd, NULL);
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

int16_t ftp_getResponse(int sockfd, char *response) {
	bool reachedTelnetEOF = FALSE; // flag that tells if we reached the telnet EOF, setting the end of the response
	bool isMultiLineResponse = FALSE; // flag that tells if response is multiline
	char buf[FTP_RESPONSE_SIZE]; // buffer to hold response text
	char responseCode[3]; // buffer to hold response code
	size_t responseSize = 0;
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

		// append new data retrieved from server to response buffer
		if(response != NULL) {
			memcpy(response + responseSize, buf, read_bytes);
			responseSize += read_bytes;
		}
	}

	return atoi(responseCode);
}

int ftp_sendCommand(int sockfd, const char *command, const char *argument, char *responseBuffer) {
	char buffer[1024];
	sprintf(buffer, "%s %s\r\n", command, argument);
	write(sockfd, buffer, strlen(buffer));
	return ftp_getResponse(sockfd, responseBuffer);
}

int ftp_sendUserCommand(const Ftp *ftp, int sockfd) {
	int responseCode = ftp_sendCommand(sockfd, "USER", ftp->user, NULL);
	
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
	int responseCode = ftp_sendCommand(sockfd, "PASS", ftp->password, NULL);

	switch(responseCode) {
		case 230:
			printf("User logged in\n");
			return 0;
		case 202:
			printf("This server doesn't support password authentication\n");
			return 1;
		case 332:
			printf("Account login is required\n");
			return 2;
		case 530:
			printf("Invalid password\n");
			return -1;		
		case 500: case 501: case 503: case 421:
			printf("Panic: Password command responded with %d\n", responseCode);
			return -2;
		default:
			printf("Unexpected response %d\n", responseCode);
			return -3;
	}
}

int ftp_authenticateUser(const Ftp *ftp, int sockfd) {
	ftp_sendUserCommand(ftp, sockfd);
	ftp_sendPasswordCommand(ftp, sockfd);
}


/**
 * @brief Parses the server response to PASV command
 * 
 * @param response The server's response
 * @param ipv4_address Buffer to store the address in IPv4 dotted format (at least 16 bytes long)
 * @param port Buffer to store the port
 * @return int 
 */
static int parsePassiveCommandResponse(char *response, char ipv4_address[16], uint16_t *port) {
	// Entering Passive Mode (90,130,70,73,106,78).
	char *index = strchr(response, '(') + 1; // already pointing for IP MSB
	uint8_t ip1, ip2, ip3, ip4, port1, port2;
	sscanf(index, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
	sprintf(ipv4_address, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
	*port = port1*256 + port2;

	return 0;
}

int ftp_sendPassiveCommand(const Ftp *ftp, int sockfd, int *sockfd_data) {
	char responseText[512] = {0};
	char ipv4[16];
	uint16_t port;
	int responseCode = ftp_sendCommand(sockfd, "PASV", "", responseText);
	// TODO check for successful response code
	printf("%s\n", responseText);
	parsePassiveCommandResponse(responseText, ipv4, &port);

	// create socket and connect to server data channel
	int data = createSocket(ipv4, port);
	*sockfd_data = data;

	return responseCode;
}

int ftp_sendRetrieveCommand(const Ftp *ftp, int sockfd, int sockfd_data) {
	// create file locally
	FILE* file;
	file = fopen(ftp->path, "w");

	if(file == NULL)
		exit(-1);
	
	// issue ftp command
	ftp_sendCommand(sockfd, "RETR", ftp->path, NULL);

	// listen to server response and write to file
	char buf[FTP_FILE_RESPONSE_SIZE];
	ssize_t read_bytes;
	while((read_bytes = read(sockfd_data, buf, FTP_FILE_RESPONSE_SIZE)) != 0) {
		printf("print packet\n");
		fwrite(buf, read_bytes, 1, file);
	}

	printf("download ended\n");

	fclose(file);

	return 0;
}