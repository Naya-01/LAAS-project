#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "utils_v1.h"

#define DATA_KEY 369
#define SEM_KEY 248
#define NBR_CLIENTS 1000

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    perror("Il faut exactements 2 arguments (NumCompte | Montant)");
    exit(EXIT_FAILURE);
  }
  int numeroDeCompte = atoi(argv[1]);
  int montant = atoi(argv[2]);

  if (numeroDeCompte < 0 || numeroDeCompte >= 1000)
  {
    perror("Numéro de compte invalide");
    exit(EXIT_FAILURE);
  }

  // GET SHARED MEMORY
  int shm_id = sshmget(DATA_KEY, NBR_CLIENTS * sizeof(int), 0);
  // semaphore
  int sem_id = sem_get(SEM_KEY, 1);

  printf("Opération en cours...\n");

  sem_down0(sem_id);

  // pointeur vers livre de compte
  int *ptrLDC = sshmat(shm_id);

  printf("Ancien solde : %d €\n", ptrLDC[numeroDeCompte]);

  // ecrire mémoire partagée
  if (montant < 0)
  {
    printf("Retrait de %d €\n", montant * -1);
  }
  else
  {
    printf("Dépôt de %d €\n", montant);
  }

  ptrLDC[numeroDeCompte] = ptrLDC[numeroDeCompte] + montant;

  printf("Nouveau solde : %d €\n", ptrLDC[numeroDeCompte]);

  sshmdt(ptrLDC);

  sem_up0(sem_id);

  exit(EXIT_SUCCESS);
}