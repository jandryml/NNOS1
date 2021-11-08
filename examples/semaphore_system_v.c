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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define ITERATIONS 1000000	// the number of operations

volatile int count = 0;		// shared variable

time_t start_time;
bool sem_initialized = false;

typedef union {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
} semunion_t;

int sID;
semunion_t sdata;

// release all allocated resources, used in atexit(3)
void release_resources(void)
{
	if (sem_initialized) {
        // IPC_RMID - immediately remove the semaphore set
        semctl(sID, 0, IPC_RMID);
        sem_initialized = false;
	}
}

// simple (not guaranteed) synchronization:
// wait for time change (a new second) -- at most one second
static void sync_threads(void)
{
	while (time(NULL) == start_time) {
		// do nothing except chew CPU slices for up to one second
	}
}

void *ThreadAdd(void *arg)
{
	int i;

	sync_threads();		// synchronize threads start / synchronizace startu vláken
    struct sembuf sops;

	for (i = 0; i < ITERATIONS; ++i) {
		// ENTRY SECTION

        // specifies semaphore index
        sops.sem_num = 0;
        // specify operation (wait)
        sops.sem_op = -1;
        // operation flags (if operation fails, undo operation)
        sops.sem_flg = SEM_UNDO;

        // semop - performs semaphore operations
        // parameters:
        // semid - specifies the ID of semaphore set
        // sops - specifies the operation
        // nsops - ??
        // (wait)
        semop(sID, &sops, 1);
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
        // specify operation (post)
        sops.sem_op = +1;
        // (post)
        semop(sID, &sops, 1);
    }
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tid1, tid2;

	atexit(release_resources);	// release resources at program exit

	start_time = time(NULL);

	// allocation of set of semaphores
	// parameters:
	//  key - returns semaphore set identifier according to key
	//  nsems - count of semaphores
	//  semflg - define the permissions
    if ((sID = semget(IPC_PRIVATE, 1, IPC_CREAT|0666)) == -1) {
        perror("semget: semget failed");
        exit(EXIT_FAILURE);
	}

    // set value of semaphore's counter to 1
    sdata.val = 1;

    // semctl - performs the control operation
    // parameters:
    // semid - specifies the ID of semaphore set
    // semnum - specifies index of semaphore
    // cmd - the command executed
    // ... - another parameters used as arguments for the command
    // SETVAL - sets the semaphore value to 1
    if(semctl(sID, 0, SETVAL, sdata) == -1) {
        perror("semctl: semctl failed");
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
