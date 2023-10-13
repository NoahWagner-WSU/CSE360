#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/wait.h>

/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */
int randomGaussian(int mean, int stddev);

void philosopher(int id, int sem_id, struct sembuf **actions);

int main(int argc, char *argv[]){
	struct sembuf get01[2] = {{0, -1, 0}, {1, -1, 0}};
	struct sembuf get12[2] = {{1, -1, 0}, {2, -1, 0}};
	struct sembuf get23[2] = {{2, -1, 0}, {3, -1, 0}};
	struct sembuf get34[2] = {{3, -1, 0}, {4, -1, 0}};
	struct sembuf get40[2] = {{4, -1, 0}, {0, -1, 0}};
	struct sembuf set01[2] = {{0, 1, 0}, {1, 1, 0}};
	struct sembuf set12[2] = {{1, 1, 0}, {2, 1, 0}};
	struct sembuf set23[2] = {{2, 1, 0}, {3, 1, 0}};
	struct sembuf set34[2] = {{3, 1, 0}, {4, 1, 0}};
	struct sembuf set40[2] = {{4, 1, 0}, {0, 1, 0}};

	struct sembuf set4[2] = {{4, 1, 0}};

	struct sembuf *actions[10] = {
		get01, 
		get12, 
		get23, 
		get34, 
		get40, 
		set01, 
		set12, 
		set23, 
		set34, 
		set40
	};

	int sem_id = semget(IPC_PRIVATE, 5, IPC_CREAT | IPC_EXCL | 0600);

	struct timespec curr_time;

	semop(sem_id, actions[5], 2);
	semop(sem_id, actions[7], 2);
	semop(sem_id, set4, 1);

	for(int i = 0; i < 5; i++) {
		switch(fork()) {
			case -1:
				perror("Error: ");
				return errno;
			case 0:
				clock_gettime(CLOCK_MONOTONIC_RAW, &curr_time);
				srand(curr_time.tv_nsec);
				philosopher(i, sem_id, actions);
				return 0;
			default:
				break;
		}
	}

	// wait for all children to finish before exiting
	int status = 0;
	while(wait(&status) > 0);
	return 0;
}

void philosopher(int id, int sem_id, struct sembuf **actions) {
	int total_eat_time = 0;
	while(total_eat_time < 100) {
		// think
		int think_time = randomGaussian(11, 7);
		if(think_time < 0) 
			think_time = 0;
		printf("Philosopher %d is thinking for %d seconds...\n", id, think_time);
		sleep(think_time);

		// try to grab both chop-sticks
		semop(sem_id, actions[id], 2);

		int eat_time = randomGaussian(9, 3);
		if(eat_time < 0) 
			eat_time = 0;
		printf("Philosopher %d is eating for %d seconds...\n", id, eat_time);
		sleep(eat_time);

		total_eat_time += eat_time;

		// release chopsticks
		semop(sem_id, actions[id + 5], 2);
	}

	printf("Philosopher %d finished eating\n", id);
}

int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5)) 
		return (int) floor(mu + sigma * cos(f2) * f1);
	else            
		return (int) floor(mu + sigma * sin(f2) * f1);
}