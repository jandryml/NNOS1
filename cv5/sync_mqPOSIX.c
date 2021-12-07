// Operating Systems: sample code  (c) Tomáš Hudec
// Synchronization: Semaphores / Condition Variables / Message Queues

// Modified: 2016-11-21, 2021-11-23

// Assignment:
//
// The program has to print the numbers in the increasing order (1, 2)
// independently on the given sleep value. The order has to be preserved
// even if another sleep is inserted at arbitrary position in the program.
// Synchronize using
//	a) POSIX condition variable,
//	b) POSIX semaphore,
//	c) System V semaphore,
//	d) POSIX message queue,
//	e) System V message queue.

// Zadání:
//
// Program musí vypsat čísla v pořadí 1, 2 nezávisle na zadané hodnotě
// čekáni sleep. Pořadí musí zůstat zachováno i v případě vložení dalšího
// sleep na jakékoliv místo v programu.
// Synchronizujte použitím
//	a) posixové podmínkové proměnné,
//	b) posixového semaforu,
//	c) semaforu System V,
//	d) posixové fronty zpráv,
//	e) fronty zpráv System V.

#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>		// included for permission macros
#include <fcntl.h>		// for O_* constants

#define QUEUE_NAME "/mq_OS_t5_sync-jadr80322"
#define MQ_LEN           1 	// max message count in mq
#define MSG_LEN          1 	// max message length (in bytes)

int sleep_time = 0;

mqd_t work_queue;		// mq descriptor
int mq_allocated = 0;		// value to signalize allocation of mq by process

void release_resources()
{
	// release only if mq was allocated by process
	if(mq_allocated){
		// closes the mq descriptor (work_queue)
		if (mq_close(work_queue) == (mqd_t) -1) {
			perror("mq_close");
		}

		// removes mq (specified by QUEUE_NAME)
		if (mq_unlink(QUEUE_NAME) == -1) {
			perror("mq_unlink");
		}
	}
}

// print by the second thread, called from thread_f1()
void f1(void)
{
	sleep(sleep_time);	// simulation of some work

	printf("1\n");		// MUST be printed FIRST

	char message = 'y';
	
	// send a message to a mq
		// work_queue - mq descriptor
		// message - message to send
		// MSG_LEN - max message length
		// 0 - message priority
	if(mq_send(work_queue, &message, MSG_LEN, 0) == -1){
		perror("mq_send");
		exit(EXIT_FAILURE);
	}

	return;
}

// print by the first thread, called from main()
void f2(void)
{
	char message;
	
	// receive message, when no message in queue -> thread is blocked until message arrives
		// work_queue - mq descriptor
		// message - variable where to save the message
		// MSG_LEN - expected message length
		// NULL - if variable is provided, it is used to save message priority (we dont save the priority, thus NULL provided)
	if((mq_receive(work_queue, &message, MSG_LEN, NULL)) == -1) {
		perror("mq_receive");
		exit(EXIT_FAILURE);
	}
	
	printf("2\n");		// MUST be printed AFTER printing 1

	return;
}

// 2nd thread
void *thread_f1(void *arg)
{
	f1();
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread;
	int rc;

	// remove mq if already exists
	// errors are not checked here, if unlink fails, then process ends on mq_open error
	mq_unlink(QUEUE_NAME); 
	
	atexit(release_resources); // register mq cleanup function to execute on exit

	// mq initialization 
 	
	// mq attributes configuration
	struct mq_attr mqattr;
	mqattr.mq_flags = 0; // blocking mq - is set by default
	mqattr.mq_maxmsg = MQ_LEN; // sets max message count in mq
	mqattr.mq_msgsize = MSG_LEN; // sets max message length in mq
	mqattr.mq_curmsgs = 0; // current present message count in mq - is set by default

	// creation of new mq
		// QUEUE_NAME - created mq name
		// oflag - defines behavior of mq creation:
			// O_CREAT - create mq if it doesn't exists
			// O_EXCL - fails with error if mq with given name already exists
			// O_RDWR - opens mq for both send and receive
		// mode - defines permissions on created mq:
			// S_IRUSR - read permission, owner
			// S_IWUSR - write permission, owner
		// mqattr - mq attributes - only max message count and max message length is set
		// return value - newly created mq descriptor or -1 on error
	work_queue = mq_open(QUEUE_NAME, O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR , &mqattr);
	if (work_queue == (mqd_t) -1) {
		perror("mq_open");
		exit(EXIT_FAILURE);
	}
	mq_allocated = 1;
		

	if (argc > 1) {
		sleep_time = atoi(argv[1]);
	}
	// printf("Waiting for sleep_time = %d seconds.\n", sleep_time);

	rc = pthread_create(&thread, NULL, thread_f1, NULL);
	if (rc) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	f2();

	(void) pthread_join(thread, NULL);

	return 0;
}

// EOF
