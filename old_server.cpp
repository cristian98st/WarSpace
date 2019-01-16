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

    int gameSocket[2];
    // gameSocket[0]-client process | gameSocket[1]-game process
    pid_t client;
    pid_t game;

    int generalSocket[2];
    //generalSocket[0]-client process with the general title | generalSocket[1]-game process

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, gameSocket) < 0)
    {
        perror("[] Error at creating game-client socket. \n");
        return errno;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, generalSocket) < 0)
    {
        perror("[] Error at creating game-general socket. \n");
        return errno;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[-] Error in connection. \n");
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

    while (1)
    {
        newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
        if (newSocket < 0)
        {
            perror("[-] Error at accepting. \n");
            return errno;
        }

        printf("[server] Ther client with the %d descriptor, from the address %s has connected. \n", newSocket, conv_addr(newAddr));

        if ((game = fork()) == 0)
        {
            close(sockfd);

            if ((client = fork()) < 0)
            {
                perror("Error at client fork. \n");
                return errno;
            }
            else if (client == 0)
            {
                //client process

                int chosen = 0;
                while (chosen == 0)
                {
                    recv(newSocket, buffer, 1024, 0);
                    close(gameSocket[1]);
                    if (strcmp(buffer, "disconect") == 0)
                    {
                        printf("[server] Ther client with the %d descriptor, from the address %s has disconnected. \n", newSocket, conv_addr(newAddr));
                        break;
                    }

                    else if (strcmp(buffer, "general") == 0)
                    {
                        write(gameSocket[0], buffer, 1024);
                        bzero(buffer, 1024);
                        read(gameSocket[1], buffer, 1024);
                        if (strcmp(buffer, "1") == 0)
                        {
                            chosen = 1;
                            
                        }
                        else
                        {
                            bzero(buffer, 1024);
                            strcpy(buffer, "");
                        }
                    }

                    else if (strcmp(buffer, "commander") == 0)
                    {
                        close(generalSocket[1]);
                    }

                    else if (strcmp(buffer, "wrrz") == 0)
                    {
                        close(generalSocket[1]);
                    }

                    else if (atoi(buffer) != 0)
                    {
                        close(generalSocket[1]);
                    }
                }

                if (strcmp(buffer, "disconect") == 0)
                    break;
                else
                    bzero(buffer, 1024);
            }

            else
            {
                //game process
                int isGeneral = -1, noCommander = 0, generalAlive = 0;
                close(gameSocket[0]);
                close(generalSocket[0]);

                while (isGeneral != 0)
                {
                    bzero(buffer, 1024);
                    read(gameSocket[1], buffer, 1024);
                    if (strcmp(buffer, "general") == 0)
                    {
                        if (isGeneral == -1)
                        {
                            bzero(buffer, 1024);
                            strcpy(buffer, "1");
                            write(gameSocket[1], buffer, 1024);
                            isGeneral = 3;
                        }
                        else
                        {
                            bzero(buffer, 1024);
                            strcpy(buffer, "A general is already in position in the fleet. Please choose another role."); // general existent
                        }
                        write(gameSocket[1], buffer, 1024); // send the answer
                    }

                    else if (strcmp(buffer, "commander") == 0)
                    {
                        if (isGeneral == 0 || noCommander == 10)
                        {
                            bzero(buffer, 1024);
                            strcpy(buffer, "The fleet needs a general or the maximum number of commanders has been reached. Please choose another role."); // no general or max no. of comanders
                        }
                        else
                        {
                            bzero(buffer, 1024);
                            strcpy(buffer, "1");
                            noCommander++;
                        }
                        write(gameSocket[1], buffer, 1024); // send the answer
                    }

                    else if (strcmp(buffer, "wrrz") == 0)
                    {
                        if (isGeneral == 0)
                        {
                            bzero(buffer, 1024);
                            strcpy(buffer, "There is no human being in this starbase. Please choose another role."); // no enemy
                        }
                        else if (noCommander > 0)
                        {
                            bzero(buffer, 1024);
                            strcpy(buffer, "1"); // attacking commander
                            noCommander--;
                        }
                        else if (isGeneral == 1)
                        {
                            bzero(buffer, 1024);
                            strcpy(buffer, "3"); // destroyed the general
                            isGeneral--;
                        }
                        else if (isGeneral > 1)
                        {
                            bzero(buffer, 1024);
                            strcpy(buffer, "2"); // took one life of the general
                            isGeneral--;
                        }
                        write(gameSocket[1], buffer, 1024); // send the answer
                    }

                    else if (atoi(buffer) != 0)
                    {
                        char aux[1024];
                        strcpy(aux, "A commander has won his ");
                        strcat(aux, buffer);
                        strcat(aux, " battle!");
                        bzero(buffer, 1024);
                        strcpy(buffer, aux); // a commander has returned succesful
                        write(generalSocket[1], buffer, 1024);
                        noCommander++;
                    }
                }
            }
        }
    }

    return 0;
}
