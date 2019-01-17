//server code

#include <iostream>
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
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctime>

#define PORT 7777

extern int errno;

using namespace std;

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
    char generalmsg[1024];

    pid_t roleP;   // process treats the client
    pid_t acceptP; // accepts clients

    int gameSocket[2]; // communication between the main proces (the game) and the client's process
    // [0]-the client | [1]-the game

    int generalSocket[2]; // communication between the main process and the general
    // [0]-the general | [1]-the game

    int wrrzSocket[2]; // communication between the main process and the wrrz
    // [0]-the wrrz | [1]-the game

    int commanderSocket[2]; // communication between the main process and the commander
    // [0]-the commander | [1]-the game

    // game parameters:
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

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, wrrzSocket) < 0) // wrrz socketpair
    {
        perror("[] Error at creating game-wrrz socket. \n");
        return errno;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, commanderSocket) < 0) // commander socketpair
    {
        perror("[] Error at creating game-commander socket. \n");
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

    if ((acceptP = fork()) < 0)
    {
        perror("[-] Error at first fork(). \n");
        return errno;
    }
    else if (acceptP != 0) // main process
    {
                                                                                                                                                            cout<<acceptP<<endl;
        int isGeneral = -1, noCommander = 0, generalAlive = 0;
        int wrrz = 0;
        fflush(stdout);
        while (1)
        {
                close(gameSocket[0]);
                close(generalSocket[0]);
                close(wrrzSocket[0]);
                close(commanderSocket[0]);

                if ((read(gameSocket[1], buffer, 1024)) < 0)
                {
                    perror("[client] Error at read 1.\n");
                    return errno;
                }

                // treating the requested role and sending the answer:

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
                        write(gameSocket[1], buffer, 1024); // send the answer
                    }
                                                                                                                                                                cout<<"IsGeneral: "<<isGeneral<<buffer<<endl;

                    fflush(stdout);
                                                                                                                                                                cout<<"Written: "<<buffer<<endl;
                }

                else if (strcmp(buffer, "commander") == 0)
                {
                    if (isGeneral == -1 || isGeneral == 0 || noCommander == 10)
                    {
                        bzero(buffer, 1024);
                        strcpy(buffer, "The fleet needs a general or the maximum number of commanders has been reached. Please choose another role."); // no general or max no. of comanders
                        write(gameSocket[1], buffer, 1024); // send the answer
                    }
                    else
                    {
                        bzero(buffer, 1024);
                        strcpy(buffer, "1");
                        noCommander++;
                        write(gameSocket[1], buffer, 1024); // send the answer

                        bzero(generalmsg, 1024);
                        strcpy(generalmsg, "A commander has joined our fleet, general!\n");
                        write(generalSocket[1], generalmsg, 1024);
                    }
                }

                else if (strcmp(buffer, "wrrz") == 0)
                {
                    if (isGeneral == 0 || isGeneral == -1)
                    {
                        bzero(buffer, 1024);
                        strcpy(buffer, "There is no human being in this starbase. Please choose another role."); // no enemy
                    }
                    else if (noCommander > 0)
                    {
                        bzero(buffer, 1024);
                        strcpy(buffer, "0"); // attacking commander
                        noCommander--;

                        bzero(generalmsg, 1024);
                        strcpy(generalmsg, "fight");
                        write(commanderSocket[1], generalmsg, 1024);

                        bzero(generalmsg, 1024);
                        strcpy(generalmsg, "A commander is counter-attacking an wrrz ship, general!\n");
                        write(generalSocket[1], generalmsg, 1024);
                    }
                    else if (isGeneral == 1)
                    {
                        bzero(buffer, 1024);
                        strcpy(buffer, "3"); // destroyed the general
                        isGeneral--;

                        bzero(generalmsg, 1024);
                        strcpy(generalmsg, "fall");
                        write(generalSocket[1], generalmsg, 1024);
                    }
                    else if (isGeneral > 1)
                    {
                        bzero(buffer, 1024);
                        strcpy(buffer, "2"); // took one life of the general
                        isGeneral--;

                        if (isGeneral == 2)
                        {
                            bzero(generalmsg, 1024);
                            strcpy(generalmsg, "Sir! All commanders have fallen in battle and an Wrrz space fighter approches us...\n We have lost our shields sir! Activating auxiliary shields!\n Action failed. We are in great danger, sir!\n");
                            write(generalSocket[1], generalmsg, 1024);
                        }
                        else if (isGeneral == 1)
                        {
                            bzero(generalmsg, 1024);
                            strcpy(generalmsg, "Sir! All commanders have fallen in battle and an Wrrz space fighter approches us...\n They have destroyed our generator. We are imobilized! \n");
                            write(generalSocket[1], generalmsg, 1024);
                        }
                    }
                    write(wrrzSocket[1], buffer, 1024); // send the answer
                }
                else if (atoi(buffer) != 0)
                {
                    bzero(generalmsg, 1024);
                    strcpy(generalmsg, "General! A commander has returned victorious from the battlefield. This is his ");
                    strcat(generalmsg, buffer);
                    strcat(generalmsg, " win!\n");
                    write(generalSocket[1], generalmsg, 1024);
                    noCommander++;
                }
        }
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

            if ((roleP = fork()) < 0)
            {
                perror("[-] Error at second fork(). \n");
                return errno;
            }
            else if (roleP == 0) // client's process
            {
                do
                {
                    close(sockfd);
                    // preprocessing
                    int chosen = 0;
                    char client_role[1024];
                    close(gameSocket[1]);
                    close(generalSocket[1]);
                    close(wrrzSocket[1]);
                    close(commanderSocket[1]);

                    if ((read(newSocket, buffer, 1024)) < 0) // read the chosen role
                    {
                        perror("[client] Error at read 1.\n");
                        return errno;
                    }

                    strcpy(client_role, buffer);

                    if ((write(gameSocket[0], buffer, 1024)) < 0) // sent the received role
                    {
                        perror("[client] Error at write 1.\n");
                        return errno;
                    }

                    if (strcmp(client_role, "wrrz") != 0)
                    {
                        bzero(buffer, 1024);
                                                                                                                                                                    cout<<"buffer: "<<buffer<<endl;
                        if ((read(gameSocket[0], buffer, 1024)) < 0) // read the answer for the chosen role for commander and general
                        {
                            perror("[client] Error at read 1.\n");
                            return errno;
                        }
                                                                                                                                                                    cout<<"received: "<<buffer<<endl;
                    }
                    else if ((read(wrrzSocket[0], buffer, 1024)) < 0) // read the answer for the chosen role for wrrz
                    {
                        perror("[client] Error at read 1.\n");
                        return errno;
                    }

                    if ((strcmp(client_role, "general")) == 0) // is general
                    {
                        if (strcmp(buffer, "1") == 0) // and accepted
                        {
                            write(newSocket, buffer, 1024);
                            int alive = 1;
                            while (alive)
                            {
                                read(generalSocket[0], generalmsg, 1024);
                                if (strcmp(generalmsg, "fall") == 0)
                                {
                                    write(newSocket, generalmsg, 1024);
                                    alive = 0;
                                }
                                else
                                {
                                    write(newSocket, generalmsg, 1024);
                                }
                            }
                            break;
                        }
                        else // and not accepted
                            write(newSocket, buffer, 1024);
                    }
                    else if ((strcmp(client_role, "commander")) == 0) // is commander
                    {
                        if (strcmp(buffer, "1") == 0) // and accepted
                        {
                                                                                                                                                                cout<<"commander, accepted. buffer: "<<buffer<<endl;
                            write(newSocket, buffer, 1024);
                            while (strcmp(buffer, "fight") != 0)
                            {
                                read(commanderSocket[0], buffer, 1024);
                                if (strcmp(buffer, "fight") == 0)
                                {
                                    write(newSocket, buffer, 1024);
                                    break;
                                }
                            }
                        }
                        else // and not accepted
                        {
                            write(newSocket, buffer, 1024);
                        }
                    }
                    else if ((strcmp(client_role, "wrrz")) == 0) // is wrrz
                    {
                        if(strcmp(buffer, "0") !=0 && strcmp(buffer, "2") !=0 && strcmp(buffer, "3") !=0)
                            {
                                write(newSocket, buffer, 1024);
                            }
                        else 
                        {
                            write(newSocket, buffer, 1024);
                            break;          
                        }
                    }
                }while(strcmp(buffer, "1") !=0 && strcmp(buffer, "0") != 0 && strcmp(buffer, "2") !=0 && strcmp(buffer, "3") != 0);
                
                if (strcmp(buffer, "0") != 0 || strcmp(buffer, "2") !=0 || strcmp(buffer, "3") != 0)
                    break;
            }
        }
    }

    return 0;
}
