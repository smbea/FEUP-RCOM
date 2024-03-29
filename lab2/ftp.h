#ifndef __FTP_H
#define __FTP_H

#include <stdint.h>

#define TRUE 1
#define FALSE 0
#define FTP_RESPONSE_SIZE 2048
#define FTP_FILE_RESPONSE_SIZE 1024

/* Aplication specifications */
#define USERNAME_MAX_LEN 256
#define PASSWORD_MAX_LEN 256
#define HOST_MAX_LEN 256
#define PATH_MAX_LEN 256
#define FILENAME_MAX_LEN 256


typedef uint8_t bool;


typedef struct {
	// server address information
	char hostname[HOST_MAX_LEN]; // the ftp server hostname
	char host_network_byte_order[5]; // holds the server address in network byte order (4 bytes)
	char host_ipv4_address[16]; // the server address in ipv4 dotted format (maximum 15 chars)
	uint16_t port;

	// user
	char user[USERNAME_MAX_LEN];
	char password[PASSWORD_MAX_LEN];
	
	// path
	char path[PATH_MAX_LEN];
	char fileName[FILENAME_MAX_LEN];
} Ftp;

/**
 * @brief Initializes a Ftp structure
 * 
 * @param host The host name (e.g. speedtest.tele2.net)
 * @param username The username. For anonymous connections use NULL
 * @param password The password. For anonymous connections use NULL
 * @param path The path where the file is located. For root directory use NULL
 * @param filename The filename to be downloaded.
 * @return A pointer to the dynamically allocated Ftp structure (already filled). Upon memory allocation errors or invalid host or filename parameters, NULL is returned and a message is sent to output buffer stream
 */
Ftp* ftp_init(uint8_t *host, uint8_t* username, uint8_t* password, uint8_t *path, uint8_t* filename);

/**
 * @brief Establishes connection with a FTP server. Once the function returns, the server is ready for new commands
 * 
 * @return Returns the socket file descriptor. Upon errors it returns -1 (failed to connect with server)
 */

/**
 * @brief Establishes connection with the FTP server
 * 
 * @param ftp The ftp server
 * @return The file descriptor for the open socket. This socket must be used to communicate with server's control/command channel
 * @retval -1 Failed to create the socket
 * @retval -2 Unexpedted server response
 * @retval -3 Failed to establish connection, service is not available
 */
int ftp_connectToServer(const Ftp *ftp);

/**
 * @brief 
 * 
 * @param sockfd The file descriptor for the open socket with server for the command channels
 * @param response A buffer to store the response. If the response text is not relevante, use NULL. NOTICE: The function assumes the buffer is big enough, is not allocating any memory
 * @retval -1 Unexpected server response. Invalid character at the fourth byte, expected space or '-'
 * @retval >0 The response code (one of the supported FTP status code)
 */
int16_t ftp_getResponse(int sockfd, char *response);

/**
 * @brief Generic function to issue FTP commands and argument, if any
 * 
 * @param sockfd The file descriptor for the open socket with server for the command channel
 * @param command The FTP command (e.g 'CWD', 'RETR', ...)
 * @param argument The command argument. For commands without arguments, use an empty string
 * @param responseBuffer A buffer to store the server response text. If this information is not relevante, use a NULL pointer. NOTICE: The function doesn't handle memory allocation, thus make sure you use a big enough buffer. @see ftp_getResponse()
 * @return @see ftp_getResponse
 */
int ftp_sendCommand(int sockfd, const char *command, const char *argument, char *responseBuffer);

/**
 * @brief Issues a 'USER' command to the FTP server
 * 
 * @param ftp The ftp connection
 * @param sockfd The file descriptor for the open socket with server for the command channel
 * @retval 0 User logged in.
 * @retval 1 Username accepted, server is waiting for password
 * @retval -1 Username is invalid
 * @retval -2 Internal error, invalid command or server is not available
 * @retval -3 Unexpected server response
 */
int ftp_sendUserCommand(const Ftp *ftp, int sockfd);

/**
 * @brief Issues a 'PASS' command to the FTP server
 * 
 * @param ftp The ftp connection
 * @param sockfd The file descriptor for the open socket with server for the command channel 
 * @retval 0 User logged in
 * @retval 1 This server doesn't support password authentication
 * @retval -1 Account login is required. Maybe username was not accepted or sent to the server
 * @retval -2 Invalid password
 * @retval -3 Interal error, invalid command syntax or server is not available
 * @retval -4 Unexpected server response
 */
int ftp_sendPasswordCommand(const Ftp *ftp, int sockfd);

/**
 * @brief Handles user authentication, sending the username and password to the server
 * 
 * @param ftp The ftp connection
 * @param sockfd The file descriptor for the open socket with server for the command channel
 * @return Returns 0 if User logged in successfully, otherwise something went wrong
 */
int ftp_authenticateUser(const Ftp *ftp, int sockfd);

/**
 * @brief Asks server to switch to passive mode and creates a new socket to communicate with the new address:port provided by the server
 * 
 * @param ftp The ftp connection
 * @param sockfd The file descriptor for the open socket with server for the command channel
 * @param sockfd_data Memory address for storing the new socket file descriptor 
 * @return If <0 an error ocurred, otherwise the server successfully switched to passive mode and the new socket established connection
 */
int ftp_sendPassiveCommand(const Ftp *ftp, int sockfd, int *sockfd_data);

/**
 * @brief Retrieves file from FTP server
 * 
 * @param ftp The ftp connection
 * @param sockfd The file descriptor for the open socket with server for the command channel
 * @param sockfd_data The file descriptor for the open socket with server for the data channel
 * @retval -1 Failed to create local file
 * @retval -2 Server rejected retrieve command
 * @retval -3 File download failed
 * @retval 0 File downloaded successfully
 */
int ftp_sendRetrieveCommand(const Ftp *ftp, int sockfd, int sockfd_data);

/**
 * @brief Changes the FTP current directory
 * 
 * @param ftp The ftp connection
 * @param sockfd The file descriptor for the open socket with server for the command channel
 * @retval 0 Successfully changed current directory
 * @retval -1 Failed to change current directory
 */
int ftp_changeDirectoryCommand(const Ftp *ftp, int sockfd);

#endif
