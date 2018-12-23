#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <math.h>


struct Address{
    char user[256];
    char password[256];
    char host[256];
    char port[5];
    char path[1024];
    char filename[256];
} Address;


int parseFTPBegin(char *string);
int parseUserPassword(char* userPass, char* username, char* password);
int parseHost(char *string, char* hostname);
int parsePort(char* string, char* port);
int parsePath(char* string, char* path, char* filename);
int parseFTPaddress(char* addressString, struct Address* address);
void printAddress(struct Address address);


int string2int(char* string);
