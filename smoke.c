#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

bool papers_on_table  = false
bool tobacco_on_table = false
bool matches_on_table = false;

// Setup our smoker threads
pthread_t smoker_threads[6];

// Each smoker semaphore represents when a smoker has the items they need
sem_t smoker_semaphors[3];

// This is an array of strings describing what each smoker type needs
char* smoker_needs[3] = { "matches & tobacco", "matches & paper", "tobacco & paper" };

// An agent semaphore represents items on the table
sem_t agent;

/**
 * Smoker function, handles waiting for the item's that they need, and then
 * smoking. Repeat this three times
 */
void* smoker(void* arg)
{
	int smoker_id = *(int*) arg;
	int type_id   = smoker_id % 3;

	// Smoke 3 times
	for (int i = 0; i < 3; ++i)
	{
		printf("\033[0;37mSmoker %d \033[0;31m>>\033[0m Waiting for %s\n",
			smoker_id, smoker_needs[type_id]);

		// Wait for the proper combination of items to be on the table
		sem_wait(&smoker_semaphors[type_id]);
		sem_post(&agent);

		// We're smoking now
		printf("\033[0;37mSmoker %d \0332[0;32m<<\033[0m Now smoking", smoker_id);
	}

	return NULL;
}

// The mutex protectes the pusher from losing
sem_t mutex;

/**
 * The pusher is responsible for releasing the proper smoker semaphore when the
 * right item's are on the table.
 */
void* pusher(void* arg)
{

}


/**
 * The main thread handles the agent's arbitration of items.
 */
int main(int argc, char* arvg[])
{
	// Seed our random number since we will be using random numbers
	srand(time(NULL));

	// There is only one agent semaphore since only one set of items may be on
	// the table at any given time. A values of 1 = nothing on the table
	sem_init(&agent, 0, 1);

	// Initalize te mutext semaphore
	sem_init(&mutex, 0, 1);

	// Initialize the semaphores for each of the 2 different types of smokers
	for (int i = 0; i < 3; ++i)
	{
		sem_init(&smoker_semaphors[i], 0, 0);
	}

	// Smoker ID's will be passed to the threads. Allocate the ID's on the stack
	int smoker_ids[6];

	// Create the 6 smoker threads with IDs
	for (unsigned int i = 0; i < 6; ++i)
	{
		smoker_ids[i] = i;

		if(pthread_create(&smoker_threads[i], NULL, smoker, &smoker_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}







	// Join all threads at the end
	for (int i = 0; i < 6; ++i)
	{
		pthread_join(smoker_threads[i], NULL);
	}

	return 0;
}
