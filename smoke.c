#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>

bool paper_on_table  = false;
bool match_on_table  = false;
bool tabaco_on_table = false;

typedef void *(*smokers_t)(void *);

/**
 * Smoker 0 has an infinite supply of paper
 */
void* smoker_0(void* arg)
{
	printf("Smoker zero waiting to smoke (has paper)\n");
}

/**
 * Smoker 1 has an infinite supply of matches
 */
void* smoker_1(void* arg)
{
	printf("Smoker one waiting to smoke (has matches)\n");

}

/**
 * Smoker 2 has an infinite supply of tabaco
 */
void* smoker_2(void* arg)
{
	printf("Smoker two waiting to smoke (has tabaco)\n");


}

/**
 * The main thread handles the agent's abritatin
 */
int main(int argc, char* arvg[])
{
	pthread_t smoker_threads[3];
	smokers_t smoker_functions[3] = { smoker_0, smoker_1, smoker_2 };

	// Create the smoker threads
	for (int i = 0; i < 3; ++i)
	{
		if(pthread_create(&smoker_threads[i], NULL, smoker_functions[i], NULL) == EAGAIN)
		{
			perror("Insufficent resources to create thread\n");
			return 0;
		}
	}

	// Join all of the smoker threads on exit
	for (int i = 0; i < 3; ++i)
	{
		pthread_join(smoker_threads[i], NULL);
	}
}
