// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections, Synchronization: mutex, condition variable
//
// Assignment:
// Design and implement a semaphore with counter
// using POSIX mutex and condition variable.
//
// Zadání:
// Navrhněte a implementujte semafor s čítačem
// pomocí posixového mutexu a podmínkové proměnné.
//
// Modified: 2016-11-21, 2021-11-09
#include <stdio.h>
#include <errno.h>
#include <limits.h>			// UINT_MAX
#include <pthread.h>
#include <stdbool.h>

// semaphore type
typedef struct {
	int counter;                // semaphore counter
	pthread_mutex_t	mutex;		// for mutual exclusion inside semaphore functions
	pthread_cond_t	cond;		// for signaling
	int signal_counter;
} pt_sem_t;
#define PT_SEM_COUNTER_MAX	INT_MAX
#define PT_SEM_COUNTER_MIN	INT_MIN

// initialize a semaphore
int pt_sem_init(pt_sem_t *sem, const unsigned int value);
// destroy a semaphore
int pt_sem_destroy(pt_sem_t *sem);
// the wait operation
int pt_sem_wait(pt_sem_t *sem);
// the signal operation
int pt_sem_post(pt_sem_t *sem);
// return current value of the counter
int pt_sem_get_value(pt_sem_t *sem);
// return values:
#define PT_SEM_OK		0	// 0: no error
#define PT_SEM_ERROR_MUTEX	1	// 1: error while working with mutex
#define PT_SEM_ERROR_COND	2	// 2: error while working with condition variable
#define PT_SEM_OVERFLOW		3	// 3: overflow of the counter

// implementation

// initialize a semaphore structure members
int pt_sem_init(pt_sem_t *sem, const unsigned int value)
{
    // check that value is in int range
    if (value > ((unsigned int)PT_SEM_COUNTER_MAX)){
        errno = EOVERFLOW;
        return PT_SEM_OVERFLOW;
    }

    // initialization of the mutex, with default attributes (NULL)
    if ((errno = pthread_mutex_init(&sem->mutex, NULL)))
        return PT_SEM_ERROR_MUTEX;

    // initialization of the condition variable, with default attributes (NULL)
    if ((errno = pthread_cond_init(&sem->cond, NULL)))
        return PT_SEM_ERROR_COND;

    // setting init values to counters
    sem->counter = value;
    sem->signal_counter = 0;

    return PT_SEM_OK;
}

// destroy a semaphore structure members
int pt_sem_destroy(pt_sem_t *sem)
{
    // release system resources allocated by the mutex
    if ((errno = pthread_mutex_destroy(&sem->mutex)))
        return PT_SEM_ERROR_MUTEX;

    // release system resources allocated by the condition variable
    if ((errno = pthread_cond_destroy(&sem->cond)))
        return PT_SEM_ERROR_COND;

    return PT_SEM_OK;
}

// the wait operation
int pt_sem_wait(pt_sem_t *sem)
{
    // lock mutex to ensure exclusive access to shared resources
    if((errno = pthread_mutex_lock(&sem->mutex))){
        return PT_SEM_ERROR_MUTEX;
    }

    // check for possible counter overflow, then decrement counter
    if(sem->counter == PT_SEM_COUNTER_MIN){
        errno = EOVERFLOW;
        return PT_SEM_OVERFLOW;
    }
    sem->counter--;

    // if counter is negative, thread will be blocked
    if (sem->counter < 0)
    {
        do {
            // unlocks the mutex and block calling thread until the condition variable is signaled
            if((errno = pthread_cond_wait(&sem->cond, &sem->mutex)))
                return PT_SEM_ERROR_COND;
            // ensures that by one signal call, only one thread is allowed to run
            // it is done by checking signal counter and than consuming from (decrement) the counter
            // when all signals already consumed, thread is blocked again on cond
        } while(sem->signal_counter <= 0);

        // check for possible counter overflow, then decrement counter
        if(sem->signal_counter == PT_SEM_COUNTER_MIN){
            errno = EOVERFLOW;
            return PT_SEM_OVERFLOW;
        }
        sem->signal_counter--;
    }

    // unclocks mutex to allow other threads access to shared resources
    if((errno = pthread_mutex_unlock(&sem->mutex)))
        return PT_SEM_ERROR_MUTEX;

    return PT_SEM_OK;
}

// the signal operation
int pt_sem_post(pt_sem_t *sem)
{
    // lock mutex to ensure exclusice access to shared resources
    if((errno = pthread_mutex_lock(&sem->mutex)))
        return PT_SEM_ERROR_MUTEX;

    // check for possible counter overflow, then increment counter
    if(sem->counter == PT_SEM_COUNTER_MAX){
        errno = EOVERFLOW;
        return PT_SEM_OVERFLOW;
    }
    sem->counter++;

    // send signal only if any thread is blocked
    if (sem->counter <= 0) {
        // send signal to cond, at least one thread is woken up
        if((errno = pthread_cond_signal(&sem->cond)))
            return PT_SEM_ERROR_COND;

        // check for possible counter overflow, then increment counter
        if(sem->signal_counter == PT_SEM_COUNTER_MAX){
            errno = EOVERFLOW;
            return PT_SEM_OVERFLOW;
        }
        sem->signal_counter++;
    }

    // unlocks mutex to allow other threads access to shared resources
    if((errno = pthread_mutex_unlock(&sem->mutex)))
        return PT_SEM_ERROR_MUTEX;

    return PT_SEM_OK;
}

// return current value of the counter
int pt_sem_get_value(pt_sem_t *sem)
{
    return sem->counter;
}

// EOF
