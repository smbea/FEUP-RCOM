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

int parseFTPBegin(char *string)
{
    char subbuff[7];
    memcpy(subbuff, &string[0], 6);
    subbuff[7] = '\0';
    return memcmp(subbuff, "ftp://", 6);
}

/*Parses user and password assumes they have a username:password@ format where password can be empty*/
int parseUserPassword(char *userPass, char *username, char *password)
{
    size_t length = strlen(userPass);
    unsigned int index = 0;
    char *user[length];
    char *pass[length];

    while ((*userPass) != ':' && (*userPass) != '@')
    {
        user[index] = (*userPass);
        userPass++;
        index++;
        if (index > length)
            return -1;
    }

    index = 0;

    while ((*userPass) != '@')
    {
        password[index] = (*userPass);
        userPass++;
        index++;
        if (index > length)
            return -1;
    }

    username = user;
    password = pass;

    return 0;
}

/* Parses host, assumes host will have a <host_name>.[com/pt/edu/...] format ends with '/'*/
int parseHost(char *string, char* hostname)
{
    size_t length = strlen(userPass);
    char* host_name[length];
    unsigned int index = 0;
    while ((*string) != '.')
    {
        host_name[index] = (*string);
        string++;
        index++;
        if (index > length)
            return -1;
    }

    while ((*string) != '/')
    {
        host_name[index] = (*string);
        string++;
        index++;
        if (index > length)
            return -1;
    }
    hostname = host_name;
    return 0;
}

int parsePath(char* string, char** path){
    
}

