#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */
int randomGaussian(int mean, int stddev);

void philosopher(int id);

int main(int argc, char *argv[]){
	for(int i = 0; i < 5; i++) {
		switch(fork()) {
			case -1:
				perror("Error: ");
				return errno;
			case 0:
				philosopher(i);
				return 0;
			default:
				break;
		}
	}
	return 0;
}

void philosopher(int id) {
	printf("I philosopher %d\n", id);
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