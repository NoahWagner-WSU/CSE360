#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define PHILOSOPHERS 5
#define TOTAL_EAT_SEC 100

struct thread_arg {
	int id;
	pthread_mutex_t *chopsticks;
};

/* Description:
 *	successive calls to randomGaussian produce integer return values
 *	having a gaussian distribution with the given mean and standard
 *	deviation.  Return values may be negative.
 *
 * Params:
 *	mean: the average of the integer to be generated
 *	stddev: the standard devation of the integer to be generated
 *
 * Return:
 *	a random integer
 */
int randomGaussian(int mean, int stddev);

/* Description:
 *	the behavior of every philosopher at a dinner table. Each eats for
 *	a random amount of time, then thinks for a random amount of time.
 *	The philosopher is done eating when they eat for a total of 100
 *	seconds. In order to eat, each philosopher must obtain both chopsticks
 *	to the let and right of them. NOTE: add statement specifying method
 *	used to avoid race conditions
 *
 * Params:
 *
 *
 * Return:
 *
 */
void *philosopher(void *args);

int main(int argc, char *argv[])
{
	pthread_t threads[PHILOSOPHERS];
	// NOTE: I might not need all these chopsticks if I use cond_wait
	// NOTE: replace with an array of chopsticks instead
	pthread_mutex_t *chopsticks;

	// initialize the chopsticks on the heap
	chopsticks = malloc(sizeof(*chopsticks) * PHILOSOPHERS);

	// initialize the mutexes
	for (int i = 0; i < PHILOSOPHERS; ++i) {
		pthread_mutex_init(&chopsticks[i], NULL);
	}

	// initialize all philosophers
	for (int i = 0; i < PHILOSOPHERS; i++) {
		struct thread_arg *args = malloc(sizeof(*args));
		args->id = i;
		args->chopsticks = chopsticks;
		pthread_create(&threads[i], NULL, philosopher, (void *)args);
	}

	// wait for all philosophers
	for (int i = 0; i < PHILOSOPHERS; ++i) {
		pthread_join(threads[i], NULL);
	}

	// NOTE: call pthread_mutex_destroy here?

	// cleanup the mutexes
	free(chopsticks);

	// initialize each philosopher on a
	return 0;
}

void *philosopher(void *args)
{
	int id = ((struct thread_arg *)args)->id;
	pthread_mutex_t *chopsticks = ((struct thread_arg *)args)->chopsticks;

	struct timespec curr_time;

	// seed the random function based off of current time
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &curr_time) == -1) {
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		return NULL;
	}
	srand(curr_time.tv_nsec);

	int total_eat_time = 0;
	int total_think_time = 0;
	while (total_eat_time < TOTAL_EAT_SEC) {
		// think
		int think_time = randomGaussian(11, 7);
		if (think_time < 0)
			think_time = 0;
		printf("Philosopher %d is thinking for %d seconds... (current total: %d)\n", id, think_time, total_think_time);
		sleep(think_time);

		total_think_time += think_time;

		// NOTE: write a function that grabs both or one chopsticks, make it act like semop() using cond_wait

		// eat
		int eat_time = randomGaussian(9, 3);
		if (eat_time < 0)
			eat_time = 0;
		printf("Philosopher %d is eating for %d seconds... (current total: %d)\n", id, eat_time, total_eat_time);
		sleep(eat_time);

		total_eat_time += eat_time;

		// NOTE: release both chopsticks (lock and unlock mutex) make a function to do this
	}
	free(args);
	printf("Philosopher %d finished eating (think total: %d, eat total: %d)\n", id, total_think_time, total_eat_time);
}

int randomGaussian(int mean, int stddev)
{
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5))
		return (int) floor(mu + sigma * cos(f2) * f1);
	else
		return (int) floor(mu + sigma * sin(f2) * f1);
}