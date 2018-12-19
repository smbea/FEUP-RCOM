#include <math.h>

#include "parser.h"

/*! - Tested function -> Good*/

/*!
Attention this function returns 0 on success*/
int parseFTPBegin(char* string)
{
    char subbuff[7];
    memcpy(subbuff, &string[0], 6);
    subbuff[7] = '\0';
    string += 6;
    return memcmp(subbuff, "ftp://", 6);
}

/*!Parses user and password assumes they have a username:password@ format where password can be empty*/
int parseUserPassword(char* userPass, char* username, char* password)
{
    size_t length = strlen(userPass);
    int index = 0, userLength, passLength;
    char user[length];
    char pass[length];

    while (((*userPass) != ':') && ((*userPass) != '@') && ((*userPass) != '/'))
    {
        user[index] = *((char*)(userPass));
        userPass++;
        index++;
        if (index > length)
            return -1;
    }
    user[index] = '\0';
    userLength = index + 1;
    index = 0;

    if((*userPass) == '@'){
        memcpy(username, user, userLength);
        memcpy(password, "0\0", 3);    
        printf("No password provided!\n");
        return 1;
    }

    if((*userPass) == '/'){
        memcpy(username, user, userLength);
        memcpy(password, "21\0", 3);    
        printf("No port provided!\n");
        return 3;
    }
 
    userPass++;

    while ((*userPass) != '@' && (*userPass) != '/')
    {
        pass[index] = *((char*)(userPass));
        userPass++;
        index++;
        if (index > length)
            return -1;
    }
    pass[index] = '\0';
    passLength = index +1;

    memcpy(username, user, userLength); 
    memcpy(password, pass, passLength);

    if((*userPass) == '@')
        return 1;
    else if((*userPass) == '/')
        return 2;
    else
        return -1;
}

/* !Parses host, assumes host will have a <host_name>.[com/pt/edu/...] format ends with '/'*/
int parseHost(char* string, char* hostname)
{
    size_t length = strlen(string);
    char host_name[length];
    unsigned int index = 0;
    while ((*string) != '.')
    {
        host_name[index] = *((char*)(string));
        string++;
        index++;
        if (index > length)
            return -1;
    }

    while ((*string) != '/' && (*string) != ':')
    {
        host_name[index] = *((char*)(string));
        string++;
        index++;
        if (index > length)
            return -1;
    }
    host_name[index] = '\0';
    memcpy(hostname,host_name, index +1);
    return 0;
}


int string2int(char* string){
    size_t len = strlen(string);
    int num = 0;
    int i, index  = 0;

    for(i = len - 1; i >= 0; i--)
    {
        num += (string[index] - '0') * pow(10,i);
        index++;
    }

    return num;
}
/*!*/
int parsePort(char* string, char* port){
    
    size_t length = strlen(string);
    unsigned int index = 0;
    char portString[length];

    while((*string) != '/')
    {
        portString[index] = *((char*)(string));
        printf("%c\n", *((char*)(string)));
        string++;
        index++;
        if (index > length)
            return -1;
    }
    portString[index] = '\0';
    memcpy(port, portString, (index+1));

    return 0;
}
/*!*/
int parsePath(char* string, char* path, char* filename){

    size_t length = strlen(string);
    unsigned int index = 0;
	int lastSlash = 0;
    char pathString[length];

    while((*string) != '\0')
    {
        pathString[index] = *((char*)(string));
		if((*string) == '/') lastSlash = index;
        string++;
        index++;
        if (index > length)
            return -1;
    }
    
    pathString[index] = '\0';
	memcpy(filename, pathString + lastSlash + 1, length - lastSlash + 1);
    memcpy(path, pathString, index - strlen(filename));
	path[index - strlen(filename)] = '\0';
    return 0;
}



int parseFTPaddress(char* addressString, struct Address* address){
    char alpha[25] = {0};
    char beta[25] = {0};
    char alpha2[25];
    char beta2[25];
    int flag = 1;
    
    if(parseFTPBegin(addressString))
        return -1;
    addressString += 6;
    printf("A: %s\n", addressString);
    int ret = parseUserPassword(addressString, alpha, beta);
    printf("S: %s\n", alpha);
        printf("S: %s\n", beta);

    switch(ret)
    {
        case 1:
            memcpy(address->user, alpha, strlen(alpha)+1);
            memcpy(address->password, beta, strlen(beta)+1);
            if((*beta) != '0')
                addressString += strlen(alpha) + strlen(beta) + 2;
            else
                addressString += strlen(alpha) + 1;
            break;
        case 2:
        case 3:
            address->user[0] = '\0';
            address->password[0] = '\0';
            memcpy(address->host, alpha, strlen(alpha)+1);
            memcpy(address->port, beta, strlen(beta)+1);
            flag = 0;
           if(ret == 2)
                addressString += strlen(alpha) + strlen(beta) + 2;
            else
                addressString += strlen(alpha) + 1;
            break;
        default:
            return -1;
    }



    if(flag != 0)
    {    
        ret = parseUserPassword(addressString, alpha2, beta2);
        if(ret != 2 && ret != 3)
            return -1;
        else{
            memcpy(address->host, alpha2, strlen(alpha2)+1);
            memcpy(address->port, beta2, strlen(beta2)+1);
            if(ret == 2)
                addressString += strlen(alpha2) + strlen(beta2) + 2;
            else
                addressString += strlen(alpha2) + 1;

        }
    }
    
    parsePath(addressString, address->path, address->filename);

    return 0;

}



void printAddress(struct Address address){
    printf("______Address received______\n");
    printf("username: %s\n", address.user);
    printf("password: %s\n", address.password);
    printf("host: %s\n", address.host);
    printf("port: %s\n", address.port);
    printf("path: %s\n", address.path);
	printf("filename: %s\n", address.filename);

}
