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

// the thread function argument, contents are described below
struct thread_arg {
	int id;
	int *chops;
	pthread_cond_t *cond;
	pthread_mutex_t *mutex;
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
 *	simulates the behavior of every philosopher at a dinner table. Each
 *	eats for a random amount of time, then thinks for a random amount
 *	of time. The philosopher is done eating when they eat for a total of
 *	100 seconds. In order to eat, each philosopher must obtain both
 *	chopsticks to the let and right of them. To avoid deadlocks, picking
 *	up the two adjacent chopsticks is made atomic through mutexes
 *	and conditional variables.
 *
 * Params:
 *	Recieves a thread arg struct that contains the following members:
 *	id: the id of the philosopher, corresponds to a position on the table
 *	chops: an array of chopsticks on the table, each index representing the
 *	       number of available chopsticks (0 or 1)
 *	cond: the conditional variable used to notify philosophers to check 
 *	      for chopsticks
 *	mutex: the mutex used to syncronize threads
 * Return:
 *	NULL always
 */
void *philosopher(void *args);

/* Description:
 *	Simulates grabing both adjacent chopsticks if they are available.
 *	If both aren't available, this function will block execution of calling
 *	thread. Only returns if both chopsticks are picked up.
 *
 * Params:
 *	Takes the same thread_arg as the calling thread. Only uses id, 
 *	cond, and mutex members.
 *	
 * Return:
 *	nothing
 */
void grab_chops(struct thread_arg *);

/* Description:
 *	Simulates setting both adjacent chopsticks down. Notifies all other
 *	threads currently waiting on the conditional variable "cond" after
 *	setting both chopsticks down. Philosophers currently waiting in
 *	grab_chops will be signaled to check if their adjacent chopsticks
 *	are available.
 *
 * Params:
 *	Takes the same thread_arg as the calling thread. Only uses id, 
 *	cond, and mutex members.
 *
 * Return:
 *	nothing
 */
void release_chops(struct thread_arg *);

int main(int argc, char *argv[])
{
	pthread_t threads[PHILOSOPHERS];
	// array of all chopsticks
	int *chopsticks;

	// initialize the chopsticks on the heap
	chopsticks = malloc(sizeof(*chopsticks) * PHILOSOPHERS);

	pthread_cond_t *cond = malloc(sizeof(*cond));
	pthread_mutex_t *mutex = malloc(sizeof(*mutex));

	// initialize mutex and conditional variable
	pthread_cond_init(cond, NULL);
	pthread_mutex_init(mutex, NULL);

	// initialize the chopsticks (1 for available)
	for (int i = 0; i < PHILOSOPHERS; ++i) {
		chopsticks[i] = 1;
	}

	// initialize all philosophers
	for (int i = 0; i < PHILOSOPHERS; i++) {
		struct thread_arg *args = malloc(sizeof(*args));
		args->id = i;
		args->chops = chopsticks;
		args->cond = cond;
		args->mutex = mutex;
		pthread_create(&threads[i], NULL, philosopher, (void *)args);
	}

	// wait for all philosophers
	for (int i = 0; i < PHILOSOPHERS; ++i) {
		pthread_join(threads[i], NULL);
	}

	// cleanup the chopsticks, mutex and conditional variable
	pthread_mutex_destroy(mutex);
	pthread_cond_destroy(cond);

	free(chopsticks);
	free(cond);
	free(mutex);

	return 0;
}

void *philosopher(void *args)
{
	struct thread_arg *func_args = (struct thread_arg *)args;

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
		printf("Philosopher %d is thinking for %d seconds... (current total: %d)\n", 
			func_args->id, think_time, total_think_time);
		sleep(think_time);

		total_think_time += think_time;

		// try to grab chopsticks, wait here until we can
		grab_chops(func_args);

		// eat
		int eat_time = randomGaussian(9, 3);
		if (eat_time < 0)
			eat_time = 0;
		printf("Philosopher %d is eating for %d seconds... (current total: %d)\n", 
			func_args->id, eat_time, total_eat_time);
		sleep(eat_time);

		total_eat_time += eat_time;

		// make the chopsticks available to other philosophers
		release_chops(func_args);
	}
	printf("Philosopher %d finished eating (think total: %d, eat total: %d)\n", 
		func_args->id, total_think_time, total_eat_time);
	free(args);
}

void grab_chops(struct thread_arg *args) 
{
	pthread_mutex_lock(args->mutex);

	// find the adjacent chopsticks 
	int chop1 = args->id;
	int chop2 = (args->id + 1) % PHILOSOPHERS;

	// keep waiting as long as both chopsticks aren't available
	while(args->chops[chop1] != 1 || args->chops[chop2] != 1) {
		pthread_cond_wait(args->cond, args->mutex);
	}

	// take chopsticks
	args->chops[chop1] = 0;
	args->chops[chop2] = 0;

	pthread_mutex_unlock(args->mutex);
}

void release_chops(struct thread_arg *args) 
{
	pthread_mutex_lock(args->mutex);

	// find the adjacent chopsticks
	int chop1 = args->id;
	int chop2 = (args->id + 1) % PHILOSOPHERS;

	// put down the chopsticks
	args->chops[chop1] = 1;
	args->chops[chop2] = 1;

	// tell everyone that these chopsticks are available
	pthread_cond_broadcast(args->cond);

	pthread_mutex_unlock(args->mutex);
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