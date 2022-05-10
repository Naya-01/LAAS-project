#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils_v1.h"

#include "virements.h"

#define MESSAGE_SIZE 8192
#define BACKLOG 5
#define NBR_CLIENTS 1000
#define SEM_KEY 248
#define DATA_KEY 369
#define PERM 0666

volatile sig_atomic_t end = 0;
int sockfd;

void endServerHandler(int sig)
{
    char *text = "Fin du serveur\n";
    write(1, text, strlen(text));
    exit(EXIT_SUCCESS);
}

int initSocketServer(int port)
{
    int sockfdTemp = ssocket();
    sbind(port, sockfdTemp);
    slisten(sockfdTemp, BACKLOG);
    return sockfdTemp;
}

int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        perror("Numéro de port manquant ! ");
    }

    int serverPort = atoi(argv[1]);

    sigset_t set;
    ssigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    ssigprocmask(SIG_BLOCK, &set, NULL);

    ssigaction(SIGTERM, endServerHandler);
    ssigaction(SIGINT, endServerHandler);

    sockfd = initSocketServer(serverPort);
    printf("Le serveur tourne sur le port : %i \n", serverPort);

    ssigprocmask(SIG_UNBLOCK, &set, NULL);

    while (!end)
    {
        /* client trt */
        int newsockfd = saccept(sockfd);
        if (newsockfd == -1)
        {
            break;
        }

        int shm_id = sshmget(DATA_KEY, NBR_CLIENTS * sizeof(int), 0);
        int sem_id = sem_get(SEM_KEY, 1);

        structVirement virement;
        sread(newsockfd, &virement, sizeof(virement));

        // Sémaphore, on bloque
        sem_down0(sem_id);

        // pointeur vers livre de compte
        int *ptrLDC = sshmat(shm_id);

        ptrLDC[virement.numBeneficiaire] += virement.montant;
        ptrLDC[virement.numEmetteur] -= virement.montant;

        swrite(newsockfd, &ptrLDC[virement.numEmetteur], sizeof(int));
        sshmdt(ptrLDC);

        sem_up0(sem_id);
        
        sclose(newsockfd);
    }

    char *text = "Fin du serveur\n";
    write(1, text, strlen(text));
    sclose(sockfd);
    exit(EXIT_SUCCESS);
}