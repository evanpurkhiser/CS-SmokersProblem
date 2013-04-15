#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

// An agent semaphore represents items on the table
sem_t agent_ready;

// Each smoker semaphore represents when a smoker has the items they need
sem_t smoker_semaphors[3];

// This is an array of strings describing what each smoker type needs
char* smoker_types[3] = { "matches & tobacco", "matches & paper", "tobacco & paper" };

// This list represents item types that are on the table. This should corrispond
// with the smoker_types, such that each item is the smoker has. So the first
// item would be paper, then tobacco, then matches.
bool items_on_table[3] = { false, false, false };

// Each pusher pushes a certian type item, manage these with this semaphore
sem_t pusher_semaphores[3];

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
			smoker_id, smoker_types[type_id]);

		// Wait for the proper combination of items to be on the table
		sem_wait(&smoker_semaphors[type_id]);
		sem_post(&agent_ready);

		// We're smoking now
		printf("\033[0;37mSmoker %d \033[0;32m<<\033[0m Now smoking\n", smoker_id);
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
	int pusher_id = *(int*) arg;

	for (int i = 0; i < 12; ++i)
	{
		// Wait for this pusher to be needed
		sem_wait(&pusher_semaphores[pusher_id]);
		sem_wait(&mutex);

		// Check if the other item we need is on the table
		if (items_on_table[(pusher_id + 1) % 3])
		{
			items_on_table[(pusher_id + 1) % 3] = false;
			sem_post(&smoker_semaphors[(pusher_id + 2) % 3]);
		}
		else if (items_on_table[(pusher_id + 2) % 3])
		{
			items_on_table[(pusher_id + 2) % 3] = false;
			sem_post(&smoker_semaphors[(pusher_id + 1) % 3]);
		}
		else
		{
			// The other item's aren't on the table yet
			items_on_table[pusher_id] = true;
		}

		sem_post(&mutex);
	}

	return NULL;
}

/**
 * The agent puts items on the table
 */
void* agent(void* arg)
{
	int agent_id = *(int*) arg;

	for (int i = 0; i < 6; ++i)
	{
		nanosleep((struct timespec[]){{0, rand() % 200000000}}, NULL);

		// Wait for a lock on the agent
		sem_wait(&agent_ready);

		// Release the items this agent gives out
		sem_post(&pusher_semaphores[agent_id]);
		sem_post(&pusher_semaphores[(agent_id + 1) % 3]);

		// Say what type of items we just put on the table
		printf("\033[0;35mAgent giving out %s\033[0;0m\n", smoker_types[(agent_id + 2) % 3]);
	}

	return NULL;
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
	sem_init(&agent_ready, 0, 1);

	// Initalize te mutext semaphore
	sem_init(&mutex, 0, 1);

	// Initialize the semaphores for each of the 2 different types of smokers
	for (int i = 0; i < 3; ++i)
	{
		sem_init(&smoker_semaphors[i], 0, 0);
	}

	// Smoker ID's will be passed to the threads. Allocate the ID's on the stack
	int smoker_ids[6];

	pthread_t smoker_threads[6];

	// Create the 6 smoker threads with IDs
	for (int i = 0; i < 6; ++i)
	{
		smoker_ids[i] = i;

		if (pthread_create(&smoker_threads[i], NULL, smoker, &smoker_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}

	// Pusher ID's will be passed to the threads. Allocate the ID's on the stack
	int pusher_ids[6];

	pthread_t pusher_threads[6];

	for (int i = 0; i < 3; ++i)
	{
		pusher_ids[i] = i;

		if (pthread_create(&pusher_threads[i], NULL, pusher, &pusher_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}

	// Agent ID's will be passed to the threads. Allocate the ID's on the stack
	int agent_ids[6];

	pthread_t agent_threads[6];

	for (int i = 0; i < 3; ++i)
	{
		agent_ids[i] =i;

		if (pthread_create(&agent_threads[i], NULL, agent, &agent_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}

	// Make sure all the smokers are done smoking
	for (int i = 0; i < 6; ++i)
	{
		pthread_join(smoker_threads[i], NULL);
	}

	return 0;
}
