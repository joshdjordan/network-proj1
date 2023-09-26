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
#include <stdbool.h>

#define BUFFSIZE 1000

int readfile(FILE *outputFile,int connected_socketDescriptor,int file_size)
{
    unsigned char buffer[BUFFSIZE];
    int totalBytesRead = 0, totalBytesWritten = 0;

    while(totalBytesRead < file_size)
    {
        int bytesRead = read(connected_socketDescriptor, buffer, sizeof(buffer));
        if(bytesRead < 0)
        {
            perror("Error reading from socket");
            return -1;
        }
        if(bytesRead == 0)
        {
            // Connection closed or no more data to read
            break;
        }
        int bytesToWrite = bytesRead;
        int bytesWritten = fwrite(buffer, 1, bytesToWrite, outputFile);
        if(bytesWritten < 0)
        {
            perror("Error writing to file");
            return -1;
        }
        totalBytesRead += bytesRead;
        totalBytesWritten += bytesWritten;
    }
}

int main(int argc,char *argv[])
{
    /****************
     * SERVER - TCP *
     ****************/

    // declarations
    int socketDescriptor,connected_socketDescriptor;
    int returnCode,portNumber,fileNameSize,file_size;
    int ttlBytes = 0,ttlBytesRecv = 0,ttlBytesWritten = 0;
    struct sockaddr_in server_address;
    struct sockaddr_in from_address;
    socklen_t fromLength = sizeof(struct sockaddr_in);
    char buffer[100];
    FILE *inputFile,*outputFile;

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
    
    while(true)
    {
        memset(buffer,0,100);

        if(strcmp(buffer,"DONE") == 0)
        {
            printf("exiting...");
            break;
        }
        returnCode = read(connected_socketDescriptor,&fileNameSize,sizeof(int));
        printf("received file name size: %d bytes\n",returnCode);

        int convertedFileNameSize = ntohl(fileNameSize); // DMO
        returnCode = read(connected_socketDescriptor,buffer,convertedFileNameSize); 
        if(returnCode <= 0)
        {
            perror("read");
            exit(1);
        }
        printf("Received file name: '%s'\n",buffer);

        int file_size_n = 0;
        returnCode = read(connected_socketDescriptor, &file_size_n, sizeof(int));
        if(returnCode < 0)
        {
            perror("read");
            exit(1);
        }
        file_size = ntohl(file_size_n);  // Convert back to host byte order
        printf("Received size of file: %d byte\n", file_size);

        outputFile = fopen(buffer,"wb");
        if(outputFile == NULL)
        {
            perror("File open failed");
            exit(1);
        }

        if(readfile(outputFile,connected_socketDescriptor,file_size) < 0)
        {
            fclose(outputFile);
            close(connected_socketDescriptor);
            close(socketDescriptor);
            exit(1);
        }
        fclose(outputFile);
    }
    close(connected_socketDescriptor);
    close(socketDescriptor);
    
    return 0;
}