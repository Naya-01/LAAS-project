#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "utils_v1.h"

#define NBR_CLIENTS 1000
#define SEM_KEY 248
#define KEY 369
#define PERM 0666

int main(int argc, char const *argv[])
{

	if (argc < 2)
	{
		perror("Nombre de parametre invalide !");
		exit(EXIT_FAILURE);
	}

	int type = atoi(argv[1]);

	if (type == 1)
	{
		// CREATE SHARED MEMORY
		sshmget(KEY, NBR_CLIENTS * sizeof(int), IPC_CREAT | PERM);
		// CREATE SEMAPHORE
		sem_create(SEM_KEY, 1, PERM, 1);
	}
	else if (type == 2)
	{

		int sem_id = sem_get(SEM_KEY, 1);
		int shm_id = sshmget(KEY, NBR_CLIENTS * sizeof(int), 0);

		sshmdelete(shm_id);
		sem_delete(sem_id);
	}
	else if (type == 3)
	{

		if (argc != 3)
		{
			perror("Nombre de parametre invalide !");
			exit(EXIT_FAILURE);
		}

		int opt = atoi(argv[2]);

		int sem_id = sem_get(SEM_KEY, 1);

		sem_down0(sem_id);
		sleep(opt);
		sem_up0(sem_id);
	}

	exit(EXIT_SUCCESS);
}