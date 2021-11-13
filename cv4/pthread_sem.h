// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections, Synchronization: mutex, condition variable
//
// Assignment:
// Design and implement a semaphore with counter
// using POSIX mutex and condition variable.
//
// Zadání:
// Navrhněte a implementujte semafor s čítačem
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
	int counter;			// semaphore counter
	pthread_mutex_t	mutex;		// for mutual exclusion inside semaphore functions
	pthread_cond_t	cond;		// for signaling
	bool mutex_initialized;
	bool cond_initialized;
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
// return values:
#define PT_SEM_OK		0	// 0: no error
#define PT_SEM_ERROR_MUTEX	1	// 1: error while working with mutex
#define PT_SEM_ERROR_COND	2	// 2: error while working with condition variable
#define PT_SEM_OVERFLOW		3	// 3: overflow of the counter

// implementation

// initialize a semaphore structure members
int pt_sem_init(pt_sem_t *sem, const unsigned int value)
{
	// an alternative initialization of the mutex
	// mutex = PTHREAD_MUTEX_INITIALIZER;
	// mutex_initialized = true;
	// initialization of the mutex
	if (pthread_mutex_init(&sem->mutex, NULL)) 
	{
		perror("pthread_mutex_init");
		return PT_SEM_ERROR_MUTEX;
	}
	sem->mutex_initialized = true;

	if (pthread_cond_init(&sem->cond, NULL))
	{                                    
		perror("pthread_cond_init");                                        
		return PT_SEM_ERROR_COND;
	}
	sem->cond_initialized = true;
	sem->counter = value - 1;
	return PT_SEM_OK;
}

// destroy a semaphore structure members
int pt_sem_destroy(pt_sem_t *sem)
{
	if (sem->mutex_initialized)
	{
		// release system resources allocated by the mutex
		if (pthread_mutex_destroy(&sem->mutex))
		{
			perror("pthread_mutex_destroy");
			return PT_SEM_ERROR_MUTEX;
		}
		sem->mutex_initialized = false;
	}
	
	if (sem->cond_initialized)
	{
		if (pthread_cond_destroy(&sem->cond))
		{
			perror("pthread_cond_destroy");
			return PT_SEM_ERROR_COND;
		}
		sem->cond_initialized = false;
	}
	return PT_SEM_OK;
}

// error codes for mutex unlock that are not checked:
//	ec	reason why it cannot occur
//	EINVAL	would fail already while locking
//	EPERM	thread owns the mutex because lock did not fail
int pt_sem_wait(pt_sem_t *sem)
{
	pthread_mutex_lock(&sem->mutex);
	while (sem->counter < 0)
		pthread_cond_wait(&sem->cond, &sem->mutex);
	sem->counter = sem->counter - 1;
	pthread_mutex_unlock(&sem->mutex);
	
	return PT_SEM_OK;
}

int pt_sem_post(pt_sem_t *sem)
{
	pthread_mutex_lock(&sem->mutex);
        sem->counter = sem->counter + 1;
	if (sem->counter >= 0)
		pthread_cond_signal(&sem->cond);
	pthread_mutex_unlock(&sem->mutex);
        
	return PT_SEM_OK;
}

// EOF
