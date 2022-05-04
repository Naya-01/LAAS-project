#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "virements.h"
#include "utils_v1.h"

// PRE: ServerIP : a valid IP address
//      ServerPort: a valid port number
// POST: on success connects a client socket to ServerIP:port
//       return socket file descriptor
//       on failure, displays error cause and quits the program
int initSocketClient(char *ServerIP, int Serverport)
{
    int sockfd = ssocket();
    sconnect(ServerIP, Serverport, sockfd);
    return sockfd;
}

void minuterieHandler()
{
}

void virementRecurrentHandler()
{
}

int main(int argc, char **argv)
{

    if (argc != 5)
    {
        perror("Il manque des arguments !");
        exit(EXIT_FAILURE);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int numeroCompte = atoi(argv[3]);
    int delay = atoi(argv[4]);

    pid_t pidMinuterie = fork_and_run0(&minuterieHandler);
    pid_t pidVR = fork_and_run0(&virementRecurrentHandler);

    /* retrieve player name */
    printf("Bienvenue sur le terminal de votre compte bancaire !\n");
    printf("Voici vos informations : \n");
    printf("Votre numéro de compte : %d\nTout les virements récurrent seront éxécutés toutes les %d secondes.", numeroCompte, delay);
    char ligne[256];

    if (virementManuel)
    {
        structVirement virement;
        virement.montant = 0;         // A completer
        virement.numBeneficiaire = 0; // A completer

        int sockfd = initSocketClient(ip, port);
        swrite(sockfd, &virement, sizeof(virement));

        /* wait server response */
        int solde;
        sread(sockfd, &solde, sizeof(int));
        printf("Solde restant : %d", solde);

        sclose(sockfd);
    }
    else if (virementRecurrent)
    {
    }
    else
    {
        // quit
        exit(EXIT_SUCCESS);
    }
    return 0;
}