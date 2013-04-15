#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

sem_t smoker_waits[3];
sem_t agent;

typedef void *(*smokers_t)(void *);

/**
 * Smoker 0 has an infinite supply of paper
 */
void* smoker_0(void* arg)
{
	while (true)
	{
		nanosleep((struct timespec[]){{0, rand() % 200000000}}, NULL);
		printf("\033[0;33mSmoker zero \033[0;31m>>\033[0m Waiting to smoke (has paper)\n");

		sem_wait(&smoker_waits[0]);
		printf("\033[0;33mSmoker zero \033[0;32m<<\033[0m Received matches and tobacco. Now making cigarette.\n");
		sem_post(&agent);
		printf("\033[0;33mSmoker zero \033[0;37m--\033[0m Smoking cigarette.\n");

	}

	return NULL;
}

/**
 * Smoker 1 has an infinite supply of matches
 */
void* smoker_1(void* arg)
{
	while (true)
	{
		nanosleep((struct timespec[]){{0, rand() % 200000000}}, NULL);
		printf("\033[0;34mSmoker one  \033[0;31m>>\033[0m Waiting to smoke (has matches)\n");

		sem_wait(&smoker_waits[1]);
		printf("\033[0;34mSmoker one  \033[0;32m<<\033[0m Received paper and tobacco. Now making cigarette.\n");
		sem_post(&agent);
		printf("\033[0;34mSmoker one  \033[0;37m--\033[0m Smoking cigarette.\n");
	}

	return NULL;
}

/**
 * Smoker 2 has an infinite supply of tobacco
 */
void* smoker_2(void* arg)
{
	while (true)
	{
		nanosleep((struct timespec[]){{0, rand() % 200000000}}, NULL);
		printf("\033[0;35mSmoker two  \033[0;31m>>\033[0m Waiting to smoke (has tobacco)\n");

		sem_wait(&smoker_waits[2]);
		printf("\033[0;35mSmoker two  \033[0;32m<<\033[0m Received matches and paper. Now making cigarette.\n");
		sem_post(&agent);
		printf("\033[0;35mSmoker two  \033[0;37m--\033[0m Smoking cigarette.\n");
	}

	return NULL;
}

/**
 * The main thread handles the agent's arbitration of items.
 */
int main(int argc, char* arvg[])
{
	// Seed our random nmber since we will be using random numbers
	srand(time(NULL));

	// Initalize our agent semaphore
	sem_init(&agent, 0, 1);

	// Setup our smoker threads and function refs
	pthread_t smoker_threads[3];
	smokers_t smoker_functions[3] = { smoker_0, smoker_1, smoker_2 };

	// Create the smoker threads and initalize the semaphores
	for (int i = 0; i < 3; ++i)
	{
		sem_init(&smoker_waits[i], 0, 0);

		if(pthread_create(&smoker_threads[i], NULL, smoker_functions[i], NULL) == EAGAIN)
		{
			perror("Insufficent resources to create thread\n");
			return 0;
		}
	}

	// Let the agent begin distributing his items
	while (true)
	{
		// Lock the agent once he's
		sem_wait(&agent);

		// Distribute two items for one of the three the smokers
		sem_post(&smoker_waits[rand() % 3]);
	}

	// Join all of the smoker threads on exit
	for (int i = 0; i < 3; ++i)
	{
		pthread_join(smoker_threads[i], NULL);
	}
}
