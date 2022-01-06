// Operating Systems: sample code  (c) Tomáš Hudec
// IPC: shared memory
// POSIX:	shm_open(3), umask(2), ftruncate(2), mmap(2), munmap(2), shm_unlink(3), close(2)
// System V:	shmget(2), shmop(2), shmctl(2), ftok(3)

// Modified: 2018-12-13

#include <sys/types.h>
#include <sys/stat.h>			// permissions: S_I…
#include <fcntl.h>			// flags: O_…
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/shm.h>			// library for shared memory facility

#define FOR_ALL		1		// permissions for all

#if FOR_ALL == 1
#	define	MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#else
#	define	MODE	(S_IRUSR | S_IWUSR)
#endif

#define MAP_FAILED ((void *) -1)	// macro for unsuccessful return value of shmat

long int *addr = NULL;			// pointer to shared memory
size_t length = sizeof(*addr);		// size of the shared memory mapping
int shm_key = -1;			// shared memory identifier

// mark shared memory segment for destroy and then detach it from process address space
void release_resources(void)
{
	// shmctl - shared memory control function
		// shm_key - identifier of shared memory
		// IPC_RMID - marks segment to be destroyed (it will happend after last process detaches it)
		// NULL - pointer to structure which hold other informations (it is ignored in our case) 
	if(shm_key != -1 && shmctl(shm_key, IPC_RMID, NULL) == -1) {
		perror("shmctl");
	}

	// detaches only if memory was attached
	if (NULL != addr || MAP_FAILED != addr) {
		// detaches the shared memory segment located at the address specified by addr
		if (-1 == shmdt(addr))
			perror("shdmt");
		addr = NULL;
	}
}

int main(int argc, char *argv[])
{
	mode_t mask;			// save the umask
	long int value = 1234;		// a value to store
	key_t ipc_key;           	// System V IPC key

	// get the value from argument if it was given
	if (argc > 1)
		value = strtol(argv[1], NULL, 0);

	atexit(release_resources);	// remove the shared memory upon exit

	// permissions are evaluated with regards to umask
	mask = umask(0);		// set umask to zero

	// converts a pathname (argv[0]) and a project identifier (127) to a System V IPC key
	if ((ipc_key = ftok(argv[0], 127)) == -1) {
		perror("ftok");
		return EXIT_FAILURE;
	}

	// allocates a System V shared memory segment
		// ipc_key - System V IPC key with which will the segment be associated with
		// length - size of the segment (it will be rounded up to a multiple of page size)
		// shmflg - specifying how access to the segment should be regulated
			// MODE - macro which defines permision on segment
			// IPC_CREATE - create segment if it doesn't exists 
		// return value - identifier of the shared memory segment
	if ((shm_key = shmget(ipc_key, length, MODE | IPC_CREAT)) == -1) {
		perror("shmget");
		return EXIT_FAILURE;
	}

	umask(mask);			// restore umask

	printf("0x%08x\n", ipc_key); fflush(stdout);

	// System V shared memory operations - attaches shared memory segment identified by shm_key to the address space of the calling process
		// shm_key - identifier of the shared memory segment
		// NULL - we don't provide address where to attach the segment -> system chooses a suitable page-aligned address 
		// 0 - we don't specify any additional flags
		// return value - address of the attached shared memory segment
	if((addr = shmat(shm_key, NULL, 0)) == MAP_FAILED) {
		perror("shmat");
		return EXIT_FAILURE;
	}

	// work with the shared memory – writing to it
	fprintf(stderr, "PID %5d: storing the value: %ld\n", getpid(), value);
	*addr = value;

	sleep(3);			// wait for a while

	// work with the shared memory – reading from it
	fprintf(stderr, "PID %5d: expected value:    %ld, value read: %ld\n", getpid(), value, *addr);

	return EXIT_SUCCESS;
}
