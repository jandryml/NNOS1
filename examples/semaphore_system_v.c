// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections
// POSIX semaphores:
// sem_init(3), sem_wait(3), sem_post(3), sem_destroy(3)
//
// Modified: 2015-11-11, 2016-11-14, 2017-04-18
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>		// POSIX semaphores

#define ITERATIONS 1000000	// the number of operations

volatile int count = 0;		// shared variable

time_t start_time;

// declare a semaphore
sem_t semaphore;
bool sem_initialized = false;

// release all allocated resources, used in atexit(3)
void release_resources(void)
{       
	if (sem_initialized) {
		sem_destroy(&semaphore);
		// return value is not checked, possible errors that shall not happen:
		//	EINVAL (not a valid semaphore)	-- semaphore is valid and initialized
		//	EBUSY (semaphore is locked)	-- we never exit while the semaphore blocks
		sem_initialized = false;
	}
}

// simple (not guaranteed) synchronization:
// wait for time change (a new second) -- at most one second
static void sync_threads(void)
{
	while (time(NULL) == start_time) {
		// do nothing except chew CPU slices for up to one second
		// nedělá nic než spotřebovává procesorový čas zjišťováním času
	}
}

void *ThreadAdd(void *arg)
{
	int i;
	// register int reg;

	sync_threads();		// synchronize threads start / synchronizace startu vláken

	for (i = 0; i < ITERATIONS; ++i) {
		// ENTRY SECTION
		sem_wait(&semaphore);
		// CRITICAL SECTION
		// Compilation of "count++" is platform (CPU) dependent because
		// the CPU can store the value into register, increase that register
		// and store the result back into the memory variable.
		// We simulate this behavior by three commands and a local variable.
		// The compiler is instructed to not optimize this code.
		count++; // can be compiled using a CPU register:
		// reg = count;		// save the global count locally
		// reg = reg + 1;	// increment the local copy
		// count = reg;		// store the local value into the global count
		// EXIT SECTION
		sem_post(&semaphore);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tid1, tid2;

	atexit(release_resources);	// release resources at program exit

	start_time = time(NULL);

	// initialize the semaphore counter to one
	// parameters:
	//   pointer to the semaphore variable,
	//   sharing type (0 = between threads, 1 = between processes)
	//   semaphore counter value
	if (sem_init(&semaphore, 0, 1)) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	sem_initialized = true;

	// create two threads
	if (pthread_create(&tid1, NULL, ThreadAdd, NULL)) {
		fprintf(stderr, "ERROR creating thread 1\n");
		return EXIT_FAILURE;
	}
	if (pthread_create(&tid2, NULL, ThreadAdd, NULL)) {
		fprintf(stderr, "ERROR creating thread 2\n");
		return EXIT_FAILURE;
	}

	// wait for the threads termination
	if (pthread_join(tid1, NULL)) {
		fprintf(stderr, "ERROR joining thread 1\n");
		return EXIT_FAILURE;
	}
	if (pthread_join(tid2, NULL)) {
		fprintf(stderr, "ERROR joining thread 2\n");
		return EXIT_FAILURE;
	}

	// resources are released using atexit(3)

	// check the result
	if (count < 2 * ITERATIONS) {
		fprintf(stderr, "BOOM! count is %d, should be %d\n", count,
			2 * ITERATIONS);
		return EXIT_FAILURE;
	}
	else {
		printf("OK! count is %d\n", count);
		return EXIT_SUCCESS;
	}
}
