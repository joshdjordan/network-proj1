/*
Name: Josh Jordan
Date: 9-16-2023
Description: project 1 server side of TCP connection program
Notes:
*/

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(int argc,char *argv[])
{
    /****************
     * SERVER - TCP *
     ****************/

    // declarations
    int socketDescriptor,connected_socketDescriptor;
    int returnCode,portNumber,fileNameSize,file_size;
    int totalBytes = 0;
    struct sockaddr_in server_address;
    struct sockaddr_in from_address;
    socklen_t fromLength = sizeof(struct sockaddr_in);
    char buffer[100];

    if(argc < 2)
    {
        printf("Useage is: server <portNumber>");
        exit(1);
    }
    portNumber = atoi(argv[1]);
    socketDescriptor = socket(AF_INET,SOCK_STREAM,0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = INADDR_ANY;

    returnCode = bind(socketDescriptor,(struct sockaddr *) &server_address,sizeof(server_address));
    if (returnCode < 0)
    {
        printf("error on bind, maybe someone is using this port?\n");
        exit(1);
    }
    listen(socketDescriptor,5);
    connected_socketDescriptor = accept(socketDescriptor,(struct sockaddr *) &from_address, &fromLength);
    memset(buffer,0,100);

    returnCode = read(connected_socketDescriptor,&fileNameSize,sizeof(int));
    printf("received file name size: %d bytes\n",returnCode);

    returnCode = read(connected_socketDescriptor,&buffer,100);
    if(returnCode <= 0)
    {
        perror("read");
        exit(1);
    }
    printf("Received file name: '%s'\n",buffer);

    printf("Server is waiting for the file size...\n");

    int file_size_n = 0;
    returnCode = read(connected_socketDescriptor, &file_size_n, sizeof(int));
    if(returnCode < 0)
    {
        perror("read");
        exit(1);
    }
    file_size = ntohl(file_size_n);  // Convert back to host byte order
    printf("Received size of file: %d bytes\n", file_size);

    return 0;
}