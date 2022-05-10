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
structVirement virementsRecurrent[100];
int port;
int numeroCompte;
char* ip;
int pipefd[2]; 
bool end = 0;

int initSocketClient(char *ServerIP, int Serverport)
{
    int sockfd = ssocket();
    sconnect(ServerIP, Serverport, sockfd);
    return sockfd;
}

void endChild(){
    char* text = "Fin de la minuterie..\n";
    write(1,text,strlen(text));
    exit(EXIT_SUCCESS);
}

void minuterieHandler()
{
    sigset_t set;
    ssigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    ssigaction(SIGUSR1,endChild);
    ssigprocmask(SIG_UNBLOCK, &set, NULL);
    sclose(pipefd[0]);
    while (!end)
    {
        sleep(delay);
        structVirement virement;
        virement.montant = 0;
        virement.numBeneficiaire = -1;
        virement.numEmetteur = -1;
        swrite(pipefd[1], &virement, sizeof(virement));
    }
    sclose(pipefd[1]);
}

void virementRecurrentHandler()
{
    sclose(pipefd[1]);
    while (!end)
    {
        structVirement virement;
        sread(pipefd[0], &virement, sizeof(virement));

        if (virement.numBeneficiaire == -1)
        {
            // Envoyer les virements
            int sockfd = initSocketClient(ip, port);
            for (size_t i = 0; i < size_vR; i++)
            {
                swrite(sockfd, &virementsRecurrent[i], sizeof(virement));

                int solde;
                sread(sockfd, &solde, sizeof(int));
            }
            sclose(sockfd);
        }else if(virement.numBeneficiaire == -2){
            end = 1;
        }else
        {
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

    ip = argv[1];
    port = atoi(argv[2]);
    numeroCompte = atoi(argv[3]);
    delay = atoi(argv[4]);

    spipe(pipefd);

    sigset_t set;
    ssigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    ssigprocmask(SIG_BLOCK, &set, NULL);

    int pidMinuterie = fork_and_run0(&minuterieHandler);
    fork_and_run0(&virementRecurrentHandler);

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
            char delim[] = " ";
            char *ptr = strtok(ligne, delim); // option
            ptr = strtok(NULL, delim);        // numCompteBeneficiaire
            int numBeneficiaire = atoi(ptr);
            ptr = strtok(NULL, delim); // montant
            int montant = atoi(ptr);

            structVirement virement;
            virement.montant = montant;
            virement.numBeneficiaire = numBeneficiaire;
            virement.numEmetteur = numeroCompte;

            int sockfd = initSocketClient(ip, port);
            swrite(sockfd, &virement, sizeof(virement));

            int solde;
            sread(sockfd, &solde, sizeof(int));
            printf("Solde restant : %d\n", solde);

            sclose(sockfd);
        }
        else if (ligne[0] == '*')
        {
            char delim[] = " ";
            char *ptr = strtok(ligne, delim); // option
            ptr = strtok(NULL, delim);        // numCompteBeneficiaire
            int numBeneficiaire = atoi(ptr);
            ptr = strtok(NULL, delim); // montant
            int montant = atoi(ptr);

            structVirement virement;
            virement.montant = montant;
            virement.numBeneficiaire = numBeneficiaire;
            virement.numEmetteur = numeroCompte;

            swrite(pipefd[1], &virement, sizeof(virement));
            printf("Le virement récurrent a bien été ajouté !\n");
        }
        else if (ligne[0] == 'q')
        {
            printf("Fin du programme\n");
            skill(pidMinuterie,SIGUSR1);
            structVirement virement;
            virement.montant = 0;
            virement.numBeneficiaire = -2;
            virement.numEmetteur = -2;
            swrite(pipefd[1], &virement, sizeof(virement));
            end = 1;
        }
        else
        {
            printf("Commande non reconnue !\n");
        }
        if(end!=1){
            printf("Veuillez entrez votre commande : ");
        }
    }
    
    sclose(pipefd[1]);
    exit(EXIT_SUCCESS);
}