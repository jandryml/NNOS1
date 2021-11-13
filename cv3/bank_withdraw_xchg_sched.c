// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections
//
// Modified: 2015-12-10, 2018-11-05, 2021-11-09

#if !defined _XOPEN_SOURCE || _XOPEN_SOURCE < 600
#       define _XOPEN_SOURCE 600        // enable barriers
#endif

#include <stdio.h>
#include <stdlib.h>			// srand(3), rand(3)
#include <sys/types.h>
#include <unistd.h>			// getpid()
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>			// bool, true, false
#include "test_and_set_bool.h"          // test_and_set() using the xchg instruction

#define INITIAL_AMOUNT	(1<<26)		// initial balance
#define THREADS		(1<<2)		// the number of concurrent threads
#define MAX_WITHDRAW	(1<<6)		// maximum amount per transaction

volatile int balance = INITIAL_AMOUNT;	// shared variable, initial balance

int withdrawn[THREADS];			// the amount withdrawn by each thread

int verbose = 1;			// verbosity

// critical section variables
volatile bool locked = false;

// synchronization variables
// a barrier declaration
pthread_barrier_t a_barrier;
bool barrier_initialized = false;

// release allocated barrier resources, used in atexit(3)
void release_barrier(void)
{
        if (barrier_initialized) {
		// release the system resources allocated by the barrier
                if (pthread_barrier_destroy(&a_barrier))
	        	perror("pthread_barrier_destroy");
                barrier_initialized = false;
	}
}

// synchronize start of all threads
// synchronizace startu vláken
static void sync_threads(void)
{
	// wait at the barrier
	int rc = pthread_barrier_wait(&a_barrier);

	switch (rc) {	// check the return code
		case PTHREAD_BARRIER_SERIAL_THREAD:
			// this will be executed only once (by only one thread)
			// destroy barrier as it won't be needed anymore
			release_barrier();
		case 0:
			break;
		default:
			// error
			perror("pthread_barrier_wait");
			exit(EXIT_FAILURE);
		}
}

void *do_withdrawals(void *arg)
{
	int id = *(int *)arg;
	int i;
	int amount;
	bool finished;

	sync_threads();		// synchronize threads start / synchronizace startu vláken
	
	// each thread makes at most (INITIAL_AMOUNT / THREADS) withdrawals
	for (i = 0, finished = false; i < INITIAL_AMOUNT / THREADS && !finished; ++i) {

		// random amount: 1 to MAX_WITHDRAW
		amount = 1 + (int) (MAX_WITHDRAW * 1.0 * (rand() / (RAND_MAX + 1.0)));
		// try withdrawal
		if (balance < amount) {	// if not enough: reject withdrawal
			if (verbose > 1)
				fprintf(stderr, "Transaction rejected: %d, %d\n", balance, -amount);
		}
		else {
			while(test_and_set(&locked)){
				sched_yield();
			}
			balance -= amount;		// do withdrawal
			locked = false;
			withdrawn[id] += amount;	// sum up total withdrawal by this thread
		}
		// set finished flag if no resources left
		finished = balance <= 0;
	}

	if (verbose > 1)
		printf("Thread %2d: transactions performed: %9d\n", id, i);

	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tids[THREADS];
	int t[THREADS];
	int i;
	int total_withdrawn = 0;

	// id

	// initialization

	atexit(release_barrier);      // release resources at program exit

        // initialize the barrier to the number of threads
        if (pthread_barrier_init(&a_barrier, NULL, THREADS)) {
	        perror("pthread_barrier_init");
		exit(EXIT_FAILURE);
	}
	barrier_initialized = true;

	srand(getpid() * time(NULL));	// RNG init

	// report initial state
	printf("%-20s %9d\n", "The initial balance:", balance);

	// create threads
	for (i = 0; i < THREADS; ++i) {
		t[i] = i;
		if (pthread_create(&tids[i], NULL, do_withdrawals, &t[i])) {
			fprintf(stderr, "ERROR creating thread %d\n", i);
			return EXIT_FAILURE;
		}
	}

	if (verbose)
		printf("Threads started: %d\n", i);

	// wait for the threads termination
	for (i = 0; i < THREADS; ++i) {
		if (pthread_join(tids[i], NULL)) {
			fprintf(stderr, "ERROR joining thread %d\n", i);
			return EXIT_FAILURE;
		}
		// sum up the total withdrawn amount by each thread
		total_withdrawn += withdrawn[i];
		if (verbose)
			printf("%-20s %9d\n", "Thread withdrawal:", withdrawn[i]);
	}

	// report the total amount withdrawn and the new state
	printf("%-20s %9d\n", "The new balance:", balance);
	printf("%-20s %9d\n", "Total withdrawn:", total_withdrawn);

	// check the result and report
	if (balance + total_withdrawn != INITIAL_AMOUNT)
		fprintf(stderr, "LOST TRANSACTIONS DETECTED!\n"
				"initial − new != total withdrawal (%d != %d)\n",
				INITIAL_AMOUNT - balance, total_withdrawn);

	return EXIT_SUCCESS;
}

