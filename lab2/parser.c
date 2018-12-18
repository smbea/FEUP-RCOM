#include <math.h>

#include "parser.h"

int parseFTPBegin(char *string)
{
    char subbuff[7];
    memcpy(subbuff, &string[0], 6);
    subbuff[7] = '\0';
    string += 6;
    return memcmp(subbuff, "ftp://", 6);
}

/*Parses user and password assumes they have a username:password@ format where password can be empty*/
int parseUserPassword(char* userPass, char* username, char* password)
{
    size_t length = strlen(userPass);
    int index = 0, userLength, passLength;
    char user[length];
    char pass[length];

    while (((*userPass) != ':') && ((*userPass) != '@'))
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
        memcpy(password, "0\0", 2);    
        printf("No password provided!\n");
        return 1;
    }
    else
        userPass++;

    while ((*userPass) != '@')
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

    return 0;
}

/* Parses host, assumes host will have a <host_name>.[com/pt/edu/...] format ends with '/'*/
int parseHost(char *string, char* hostname)
{
    size_t length = strlen(string);
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

    while ((*string) != '/' && (*string) != ':')
    {
        host_name[index] = (*string);
        string++;
        index++;
        if (index > length)
            return -1;
    }
    host_name[index] = '\0';
    hostname = host_name;
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

int parsePath(char* string, char* path){

    size_t length = strlen(string);
    unsigned int index = 0;
    char pathString[length];

    while((*string) != '\0')
    {
        pathString[index] = *((char*)(string));
        printf("%c\n", *((char*)(string)));
        string++;
        index++;
        if (index > length)
            return -1;
    }
    
    pathString[index] = '\0';
    memcpy(path, pathString, (index+1));
    return 0;
}



int parseFTPadress(char* string){
    struct Address adress;
}


/*
int main(void){
    char i[15], b[15];
    parseUserPassword("afo@", i, b);
    printf("user: %s, pass: %s \n", i, b);
    return 0;
}*/