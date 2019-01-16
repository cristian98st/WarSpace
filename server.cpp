//server code

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 7777

extern int errno;

char *conv_addr(struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    /* adresa IP a clientului */
    strcpy(str, inet_ntoa(address.sin_addr));
    /* portul utilizat de client */
    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}

int main()
{
    int sockfd;
    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;

    socklen_t addr_size;

    char buffer[1024];

    pid_t roleP;   // process treats the client
    pid_t acceptP; // accepts clients

    int gameSocket[2]; // communication between the main proces (the game) and the client's process
    // [0]-the client | [1]-the game

    int generalSocket[2]; // communication between the main process and the general
    // [0]-the general | [1]-the game

    // game parameters:
    int clientfd[10];
    int status; // for wait()

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, gameSocket) < 0) // client socketpair
    {
        perror("[] Error at creating game-client socket. \n");
        return errno;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, generalSocket) < 0) // general socketpair
    {
        perror("[] Error at creating game-general socket. \n");
        return errno;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[-] Error at socket creation. \n");
        return errno;
    }
    printf("[+] Server socket created. \n");

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("[-] Error at binding. \n");
        return errno;
    }
    printf("[+] Bind succesful to port %d. \n", PORT);

    if (listen(sockfd, 10) == 0)
    {
        printf("Listening...\n");
    }
    else
    {
        perror("[-] Error at listening. \n");
        return errno;
    }

    if((acceptP = fork()) < 0)
    {
        perror("[-] Error at first fork(). \n");
        return errno;
    }
    else if(acceptP !=0) // main process
    {
        // commui=nication through socketpairs
    }
    else
    {
        while (1)
        {
            newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
            if (newSocket < 0)
            {
                perror("[-] Error at accepting. \n");
                return errno;
            }
            
            printf("[server] Ther client with the %d descriptor, from the address %s has connected. \n", newSocket, conv_addr(newAddr));

            if((roleP = fork()) < 0)
            {
                perror("[-] Error at second fork(). \n");
                return errno;
            }
            else if(roleP == 0) // client's process
            {
                int chosen = 0;
            }
        }
    }

    return 0;
}
