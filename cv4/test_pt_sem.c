// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections, Synchronization: mutex, condition variable
//
// Last modification: 2016-11-21

#include <errno.h>
#include <pthread.h>
#include "pthread_sem.h"	// pthread semaphores

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>		// getopt

#define THREADS		8
#define THREADS_IN_CS	3

// print binary
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)		\
	(byte & 0x80 ? '1' : '0'),	\
	(byte & 0x40 ? '1' : '0'),	\
	(byte & 0x20 ? '1' : '0'),	\
	(byte & 0x10 ? '1' : '0'),	\
	(byte & 0x08 ? '1' : '0'),	\
	(byte & 0x04 ? '1' : '0'),	\
	(byte & 0x02 ? '1' : '0'),	\
	(byte & 0x01 ? '1' : '0') 

unsigned int bit_field;

pt_sem_t pt_sem;		// semaphore

void pt_sem_cleanup() {
	if (pt_sem_destroy(&pt_sem))
		perror("pt_sem_destroy");
}

void *thread_CS(void *arg)
{
	int id = *(int *)arg;
	int i;

	for (i = 0; i < 3; ++i) {
		pt_sem_wait(&pt_sem);
		bit_field |= 1 << id;		// set my bit
		printf("%2d: Inside CS: 0x%02X " BYTE_TO_BINARY_PATTERN "\n",
			id, bit_field, BYTE_TO_BINARY(bit_field));
		usleep(100000);
		bit_field &= ~(1 << id);	// clear my bit
		pt_sem_post(&pt_sem);
	}
	return NULL;
}


//
// main / hlavní program
//
int main(int argc, char *argv[])
{
	int i;
	// structure array for threads / pole záznamů pro vlákna
	struct thread_info_t {
		pthread_t id;
		int arg;
	} thread_info[THREADS];

	// init for the critical section access control
	if (pt_sem_init(&pt_sem, THREADS_IN_CS)) {
		perror("pt_sem_init");
		exit(EXIT_FAILURE);
	}
	atexit(pt_sem_cleanup);

	// start several threads performing transactions
	for (i = 0; i < THREADS; ++i) {
		thread_info[i].arg = i;
		if (pthread_create(&thread_info[i].id, NULL, thread_CS,
				   &thread_info[i].arg)) {
			// failed to create a thread
			perror("pthread_create");
			return 1;
		}
	}

	// wait for threads to finish / čekej na dokončení vláken
	for (i = 0; i < THREADS; ++i) {
		pthread_join(thread_info[i].id, NULL);
	}

	return 0;
}


// EOF
