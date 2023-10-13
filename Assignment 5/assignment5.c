#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/wait.h>

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
 *	to the let and right of them. To avoid a race condition, the action
 *	of taking both chopsticks is made atommic through semaphores.
 *
 * Params:
 *	id: the id of the philosopher (used to tell which chopsticks to use)
 *	sem_id: the id of the 5 semaphores (each representing a chopstick)
 *	get_both: a sembuf that will decrement adjacent semaphores
 *	set_both: a sembuf that will increment adjacent semaphores
 *	
 * Return:
 *	nothing
 */
void philosopher(int id, 
		 int sem_id, 
		 struct sembuf *get_both, 
		 struct sembuf *set_both);

int main(int argc, char *argv[])
{
	// actions that each philospher could take (id is temporary for now)
	struct sembuf get_both[2] = {{0, -1, 0}, {0, -1, 0}};
	struct sembuf set_both[2] = {{0, 1, 0}, {0, 1, 0}};

	// used to initialize all the semaphores to 1
	struct sembuf set_all[5] = {
		{0, 1, 0}, 
		{1, 1, 0}, 
		{2, 1, 0}, 
		{3, 1, 0}, 
		{4, 1, 0}
	};

	// initialize all the semaphores, each representing a chopstick
	int sem_id = semget(IPC_PRIVATE, 5, IPC_CREAT | IPC_EXCL | 0600);

	struct timespec curr_time;

	// set all semaphores to 1
	semop(sem_id, set_all, 5);

	// fork each child process, each running philosopher()
	for(int i = 0; i < 5; i++) {
		switch(fork()) {
			case -1:
				perror("Error: ");
				return errno;
			case 0:
				clock_gettime(CLOCK_MONOTONIC_RAW, &curr_time);
				srand(curr_time.tv_nsec);
				philosopher(i, sem_id, get_both, set_both);
				return 0;
			default:
				break;
		}
	}

	// wait for all children to finish before exiting
	int status = 0;
	while(wait(&status) > 0);

	// NOTE: FREE THE SEMAPHORES!!!!
	return 0;
}

void philosopher(int id, 
		 int sem_id, 
		 struct sembuf *get_both, 
		 struct sembuf *set_both)
{

	// change the get id's to be the semaphores adjacent to my id
	get_both[0].sem_num = id;
	get_both[1].sem_num = (id + 1) % 5;

	// change the set id's to be the semaphores adjacent to my id
	set_both[0].sem_num = id;
	set_both[1].sem_num = (id + 1) % 5;

	int total_eat_time = 0;
	while(total_eat_time < 100) {
		// think
		int think_time = randomGaussian(11, 7);
		if(think_time < 0) 
			think_time = 0;
		printf("Philosopher %d is thinking for %d seconds...\n", id, think_time);
		sleep(think_time);

		// try to grab both chop-sticks
		semop(sem_id, get_both, 2);

		// eat
		int eat_time = randomGaussian(9, 3);
		if(eat_time < 0) 
			eat_time = 0;
		printf("Philosopher %d is eating for %d seconds...\n", id, eat_time);
		sleep(eat_time);

		total_eat_time += eat_time;

		// release chopsticks
		semop(sem_id, set_both, 2);
	}

	printf("Philosopher %d finished eating\n", id);
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