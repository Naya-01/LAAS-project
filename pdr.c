#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "utils_v1.h"

#define DATA_KEY 369
#define SEM_KEY 248
#define NBR_CLIENTS 1000
#define PERM 0666

int main (int argc, char *argv[]) {
  // GET SHARED MEMORY 
  if(argc != 3){
    perror("Il faut exactements 2 arguments (NumCompte | Montant)");
  }
  int numeroDeCompte = argv[1];
  int montant = argv[2];

  if(numeroDeCompte < 0 || numeroDeCompte > 1000){
    perror("Numéro de compte invalide");
  }

  int shm_id = sshmget(DATA_KEY, NBR_CLIENTS * sizeof(int), 0);
  int* ptrLDC = sshmat(shm_id); // pointeur vers livre de compte
  int sem_id = sem_get(KEY_WRITE, 1); // semaphore
  
  printf("Ancien solde : %d \n",ptrLDC[numeroDeCompte]);

  printf("Opération en cours...\n");
  
  sem_down0(sem_id);

  // ecrire mémoire partagée
  if(montant < 0){
    printf("Retrait de %d \n",montant);
    ptrLDC[numeroDeCompte] = ptrLDC[numeroDeCompte] - montant;
  }else{
    printf("Dépôt de %d \n",montant);
    ptrLDC[numeroDeCompte] = ptrLDC[numeroDeCompte] + montant;
  }

  sem_up0(sem_id);
  
  printf("Nouveau solde : %d \n",ptrLDC[numeroDeCompte]);
  
  sshmdt(ptrLDC);
}