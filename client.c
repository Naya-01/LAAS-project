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

int delay = 0;
int size_vR = 0;
int port;
int numeroCompte;
char *ip;
int pipefd[2];
bool end = 0;

int initSocketClient(char *ServerIP, int Serverport)
{
    int sockfd = ssocket();
    sconnect(ServerIP, Serverport, sockfd);
    return sockfd;
}

structVirement getVirement(char *ligne)
{
    char delim[] = " ";
    char *ptr = strtok(ligne, delim); // option (qu'on va pas enregistrer)
    ptr = strtok(NULL, delim);        // numCompteBeneficiaire
    int numBeneficiaire = atoi(ptr); // on enregistre le num beneficiaire
    ptr = strtok(NULL, delim); // montant
    int montant = atoi(ptr); // on enregistre le montant

    structVirement virement;
    virement.montant = montant;
    virement.numBeneficiaire = numBeneficiaire;
    virement.numEmetteur = numeroCompte;
    return virement;
}

void endChild()
{
    end = 1;
}

void minuterieHandler()
{
    sigset_t set;
    ssigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    ssigaction(SIGUSR1, endChild);
    ssigprocmask(SIG_UNBLOCK, &set, NULL);
    sclose(pipefd[0]);
    while (!end)
    {
        sleep(delay);
        structVirement virement;
        virement.montant = 0;
        // -1 pour dire aux virements récurrent qu'on est un heartbeat pour le lancement des virements récurrents
        virement.numBeneficiaire = -1;
        virement.numEmetteur = -1;
        swrite(pipefd[1], &virement, sizeof(virement));
    }
    char *text = "Fin de la minuterie..\n";
    write(1, text, strlen(text));
    sclose(pipefd[1]);
    exit(EXIT_SUCCESS);
}

void virementRecurrentHandler()
{
    sclose(pipefd[1]);
    structVirement virementsRecurrent[100];
    while (!end)
    {
        structVirement virement;
        sread(pipefd[0], &virement, sizeof(virement));

        if (virement.numBeneficiaire == -1)
        {
            int sockfd = initSocketClient(ip, port);
            // Effectuer les virements récurrent
            swrite(sockfd, &size_vR, sizeof(int)); // nbr virement envoyé au serveur
            for (int i = 0; i < size_vR;i++){ // Envoi des virements au serveur
                swrite(sockfd, &virementsRecurrent[i], sizeof(structVirement));
            }
            
            sclose(sockfd);
        }
        else if (virement.numBeneficiaire == -2) // Fin des virements récurrents
        {
            end = 1;
        }
        else{
            // Ajouter le virement aux récurrents
            virementsRecurrent[size_vR] = virement;
            size_vR++;
        }
    }
    printf("Fin des virements récurrents..\n");
    sclose(pipefd[0]);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{

    if (argc != 5)
    {
        perror("Il manque des arguments !");
        exit(EXIT_FAILURE);
    }

    // Récupération des données
    ip = argv[1];
    port = atoi(argv[2]);
    numeroCompte = atoi(argv[3]);
    delay = atoi(argv[4]);

    spipe(pipefd); // Création du pipe

    sigset_t set;
    ssigemptyset(&set);
    sigaddset(&set, SIGUSR1); // Bloquer le SIGUSR1 pour le gérer plus tard dans l'enfant minuterie
    ssigprocmask(SIG_BLOCK, &set, NULL);

    int pidMinuterie = fork_and_run0(&minuterieHandler); // Lancement de la minuterie
    fork_and_run0(&virementRecurrentHandler); // Lancement des virements récurrents

    sclose(pipefd[0]);

    printf("Bienvenue sur le terminal de votre compte bancaire !\n");
    printf("Voici vos informations : \n");
    printf("Votre numéro de compte : %d\nTout les virements récurrent seront éxécutés toutes les %d secondes.\n", numeroCompte, delay);
    printf("Veuillez entrez votre commande : ");
    char ligne[256];
    while (!end)
    {
        readLimitedLine(ligne, 256);
        if (ligne[0] == '+')
        {
            structVirement virement = getVirement(ligne);

            printf("Virement en cours..\n");
            int sockfd = initSocketClient(ip, port);

            // Nombre de virements envoyé au serveur
            int size = 1;
            swrite(sockfd,&size,sizeof(int));
            // Le virement envoyé au serveur
            swrite(sockfd, &virement, sizeof(structVirement));
            // Récupération du solde envoyé par le serveur
            int solde;
            sread(sockfd, &solde, sizeof(int));
            printf("Solde restant : %d\n", solde);

            sclose(sockfd);
        }
        else if (ligne[0] == '*')
        {
            structVirement virement = getVirement(ligne);
            // Envoie du virement à l'enfant chargé des virement récurrents
            swrite(pipefd[1], &virement, sizeof(virement));
            printf("Le virement récurrent a bien été ajouté !\n");
        }
        else if (ligne[0] == 'q')
        {
            printf("Fin du programme\n");
            skill(pidMinuterie, SIGUSR1); // sigusr1 pour fin de la minuterie
            structVirement virement;
            virement.montant = 0;
            virement.numBeneficiaire = -2; // - 2 pour fin des virements récurrents
            virement.numEmetteur = numeroCompte;
            swrite(pipefd[1], &virement, sizeof(virement));
            end = 1;
        }
        else
        {
            printf("Commande non reconnue !\n");
        }
        if (end != 1)
        {
            printf("Veuillez entrez votre commande : ");
        }
    }

    sclose(pipefd[1]);
    exit(EXIT_SUCCESS);
}