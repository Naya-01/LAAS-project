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

void endServerHandler(int sig)
{
    end = 1;
}

int initSocketServer(int port)
{
    // TODO à compléter
    int sockfd = ssocket();
    sbind(port, sockfd);
    slisten(sockfd, BACKLOG);
    return sockfd;
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

    int sockfd = initSocketServer(serverPort);
    printf("Le serveur tourne sur le port : %i \n", serverPort);

    ssigprocmask(SIG_UNBLOCK, &set, NULL);

    while (!end)
    {
        /* client trt */
        int newsockfd = accept(sockfd, NULL, NULL);


        int shm_id = sshmget(DATA_KEY, NBR_CLIENTS * sizeof(int), 0);
        int sem_id = sem_get(SEM_KEY, 1);

        structVirement virement;
        sread(newsockfd, &virement, sizeof(virement));

        // pointeur vers livre de compte
        int *ptrLDC = sshmat(shm_id);

        //Sémaphore, on bloque 
        sem_down0(sem_id);

        ptrLDC[virement.numBeneficiaire]+=virement.montant;
        ptrLDC[virement.numEmetteur]-=virement.montant;

        sem_up0(sem_id);

        swrite(newsockfd, &ptrLDC[virement.numEmetteur], sizeof(int));
    }

    exit(EXIT_SUCCESS);
}