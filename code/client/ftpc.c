#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#define TOTAL_SIZE 100
#define MAX_FILENAME_SIZE 100

int getFileLength(FILE *fp)
{
    fseek(fp,0,SEEK_END);
    int len = (int)ftell(fp);
    fseek(fp,0,SEEK_SET);
    return len;
}

int main(int argc, char *argv[])
{
    // declarations
    int socketDescriptor,portNumber;
    struct sockaddr_in server_address;
    char fileName[MAX_FILENAME_SIZE],outputFileName[MAX_FILENAME_SIZE];
    char serverIP[29];
    int returnCode = 0,totalBytesRead = 0,totalBytesWritten = 0;
    int file_size = 0;
    FILE *inputFile,*outputFile;

    if (argc < 3)
    {
        printf("Usage: client <ipaddr> <port>\n");
        exit(1);
    }

    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    portNumber = atoi(argv[2]);
    strcpy(serverIP, argv[1]);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = inet_addr(serverIP);

    if (connect(socketDescriptor, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in)) < 0)
    {
        close(socketDescriptor);
        perror("Error connecting stream socket");
        exit(1);
    }

    while (true)
    {
        printf("What is the name of the file you would like to send?\n> ");
        scanf("%s",fileName);
        if (strcmp(fileName,"DONE") == 0)
        {
            break;
        }
        printf("What do you want to name the output file?\n> ");
        scanf("%s",outputFileName);

        // Open the file inside the loop for each file
        inputFile = fopen(fileName,"rb");
        if(inputFile == NULL)
        {
            perror("File open failed");
            exit(1);
        }

        file_size = getFileLength(inputFile);

        int fileName_size = strlen(fileName);
        returnCode = write(socketDescriptor, &fileName_size, sizeof(int));
        if (returnCode < 0)
        {
            perror("write");
            fclose(inputFile); // Close the file if write fails
            exit(1);
        }
        printf("Sent filename size: %d bytes\n", fileName_size);

        returnCode = write(socketDescriptor, fileName, fileName_size);
        if (returnCode < 0)
        {
            perror("write");
            fclose(inputFile); // Close the file if write fails
            exit(1);
        }
        printf("Sent file name: '%s'\n", fileName);

        int file_size_n = htonl(file_size);
        printf("file size: %d bytes\n",file_size);
        returnCode = write(socketDescriptor, &file_size, sizeof(int));
        if (returnCode < 0)
        {
            perror("write");
            fclose(inputFile);
            exit(1);
        }
        printf("Sent file size: %d bytes\n", file_size);

        /****************
         * SENDING FILE *
         ****************/

        char buffer[TOTAL_SIZE];
        int bytes_read = 0;
        while ((bytes_read = fread(buffer, 1, TOTAL_SIZE, inputFile)) > 0)
        {
            int total_sent_bytes = 0;
            while (total_sent_bytes < bytes_read)
            {
                int sent_bytes = write(socketDescriptor, buffer + total_sent_bytes, bytes_read - total_sent_bytes);
                if (sent_bytes < 0)
                {
                    perror("Error sending file data");
                    fclose(inputFile); // Close the file if write fails
                    exit(1);
                }
                total_sent_bytes += sent_bytes;
            }
        }
        printf("\nSent the file.\n");

        // Close the file after sending
        fclose(inputFile);
    }

    close(socketDescriptor);

    return 0;
}
