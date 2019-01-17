//client code

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
#include <time.h>
#include <ctime>

using namespace std;

extern int errno;

int port = 7777;

void validateRole(int validRole, char (&buffer)[1024], int &role)
{
    while (validRole == 0)
    {
        if (strcmp(buffer, "general") == 0)
        {
            role = 0;
            validRole = 1;
        }
        else if (strcmp(buffer, "commander") == 0)
        {
            role = 1;
            validRole = 1;
        }
        else if (strcmp(buffer, "wrrz") == 0)
        {
            role = 2;
            validRole = 1;
        }
        else
        {
            printf("Unvalid role was chosen. Please enter again your role:\n");
            bzero(buffer, 1024);
            read(0, buffer, 1024);
        }
    }
}

void availRole(char (&buffer)[1024], int &role, int sd)
{
    int valid = 0;
    while (valid == 0)
    {
        if (strcmp(buffer, "1") != 0 && strcmp(buffer, "2") != 0 && strcmp(buffer, "3") != 0 && strcmp(buffer, "0") != 0)
        {
            printf("%s\n", buffer);
            bzero(buffer, 1024);
            read(0, buffer, 0);
            validateRole(0, buffer, role); // is the role existent?
            write(sd, buffer, 1024);       // send it to the server
            bzero(buffer, 1024);
            read(sd, buffer, 1024); // get the answer
        }
        else
            valid = 1;
    }
}

int main(int argc, char *argv[])
{
    int sd;
    struct sockaddr_in server;
    char buffer[1024];

    int wins = 0, valid = 0;
    int fightResult;
    int role; // 0-general, 1-commander, 2-wrrz

    srand(time(NULL));

    if (argc != 2)
    {
        printf("Too few argumets.\n");
        return -1;
    }

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[-]Error at socket()\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[-]Error at connection.\n");
        return errno;
    }

    bzero(buffer, 1024);
    printf("Who are you, stranger?\n");
    fflush(stdout);
    read(0, buffer, 1024); // cin role
    validateRole(0, buffer, role);

    if ((write(sd, buffer, 1024)) == -1) //send role
    {
        perror("[-] Error at sending to the server.\n");
        return errno;
    }

    bzero(buffer, 1024);
    read(sd, buffer, 1024); // get response

    availRole(buffer, role, sd); // check if the response was good
                                 // at this point a role has been assigned to the client

    if (role == 1) // commander
    {
        do
        {
            bzero(buffer, 1024);
            read(sd, buffer, 1024);
            if (strcmp(buffer, "fight") == 0)
            {
                printf("A wrrz spaceship has arrived. We have been ordered to counter-attack it. To battle!\n");
                fightResult = rand() % 101;
                if (fightResult % 2 == 0)
                {
                    printf("Shields are down! Auxiliar shields are inoperative. Critical damage! Abandon the ship! \n You have died on duty as a hero!\n");
                    return 0;
                }
                else
                {
                    wins++;
                    if (wins > 2)
                        printf("Congratsulations! You have won your %d fight against the hideous Wrrz. You are a champion!", wins);
                    else if(wins > 4)
                        printf("Wow, commander! You are an ace in spacefights and a champion of the fleet! This is your %d win!!\n", wins);
                    else
                        printf("Congratsulations! You have won your %d fight against the hideous Wrrz.", wins);
                    connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr));
                    bzero(buffer, 1024);
                    fflush(stdout);
                    buffer[0] = wins + '0';
                    write(sd, buffer, 1024); // sned the role as the number of wins
                }
            }
        } while (strcmp(buffer, "fight") != 0);
    }

    else if (role == 2) // wrrz
    {
        if(strcmp(buffer, "0") == 0)
            printf("You have done your undespicable duty. You have attacked a human Commander but you have run away before you could see the damage you have done... Despiteful...\n");
        else if(strcmp(buffer, "2") == 0)
            printf("You have done your undespicable duty. You have successfully attacked the human's General from the back... Shame on you...\n");
        else if(strcmp(buffer, "3") == 0)
            printf("Ah, you have destroyed the most honorous of the human race, the General himself. They will make you the leader of the pack... this if none of your crew memebers will stab you in the back...\n Hey! Look behind!!... Oh, that's too bad... you have died...\n");
    }

    else //general
    {
        int alive = 3;

        while (alive)
        {
            bzero(buffer, 1024);
            read(sd, buffer, 1024);

            if (strcmp(buffer, "fall") == 0)
            {
                printf("The Wrrz have won, general! We are overnumbered! Perhaps a new hope will emerge from the human race once again in the future to save us...\n");
                break;
            }
            else 
                printf("%s", buffer);
        }
    }

    return 0;
}