// Operating Systems: sample code  (c) Tomáš Hudec
// Processes
// exit(3), atexit(3), _exit(2)
#include <stdlib.h>	// exit(3), atexit(3), EXIT_SUCCESS, EXIT_FAILURE
#include <stdio.h>	// fprintf(3)
#include <unistd.h>	// _exit(2)
#include <stdbool.h>	// false, true

// Assignment:
// 1. Try to run with a negative number as the first argument: ./atexit -1
// 2. Enable different leaving methods.

// Compare results by setting the value to 0, 1, or 2
#define EXIT_BY_RETURN		0
#define EXIT_BY_EXIT		1
#define EXIT_BY__EXIT		2
#define EXIT_TYPE		EXIT_BY_EXIT
char	*exit_types[] =		{ "return", "exit()", "_exit()" };

#define DEFAULT_BUF_SIZE	(1<<8)
int	*buf1 = NULL;
int	*buf2 = NULL;

// allocation of a resource, in this case a memory
int allocate_resource_1(size_t size)
{
    printf("Trying to allocate resource 1, size = %zd.\n", size);
    if (NULL == (buf1 = malloc(size))) {
        perror("Failure: malloc");
        return false;
    }
    printf("Success.\n");
    return true;
}

// deallocation of the resource 1
void deallocate_resource_1(void)
{
    free(buf1);
    buf1 = NULL;
    printf("Resource 1 deallocated.\n");
}

// allocation of a resource, in this case a memory
int allocate_resource_2(size_t size)
{
    printf("Trying to allocate resource 2, size = %zd.\n", size);
    if (NULL == (buf2 = malloc(size))) {
        perror("Failure: malloc");
        return false;
    }
    printf("Success.\n");
    return true;
}

// deallocation of the resource 2
void deallocate_resource_2(void)
{
    free(buf2);
    buf2 = NULL;
    printf("Resource 2 deallocated.\n");
}

// this will be executed upon process exit, the function shall be registered by atexit(3)
void finish()
{
    // dealocation of resource 1, only if it is not NULL
    if(buf1 != NULL){
        deallocate_resource_1();
    }

    // dealocation of resource 2, only if it is not NULL
    if(buf2 != NULL){
        deallocate_resource_2();
    }

    printf("Finished.\n");
    // it is not portable to call the exit(3) in a function registered by atexit(3)
    // exit(0);
}

int main(int argc, char *argv[])
{
    size_t	size = DEFAULT_BUF_SIZE;

    // add the exit function -- to be executed upon exit(3) call
    atexit(finish);

    // allocate the first resource
    if (!allocate_resource_1(size)) {	// in case of failure:
        printf("Calling exit().\n");
        exit(EXIT_FAILURE);
    }

    if (argc > 1)
        // set the size from the first argument if it was given
        size = strtol(argv[1], NULL, 0);

    // allocate the second resource
    if (!allocate_resource_2(size)) {	// in case of failure:
        printf("Calling exit().\n");
        // no need to deallocate resource 1 before calling exit(3)
        exit(EXIT_FAILURE);
    }

    printf("Working with resources.\n");	// do some work with resources
    sleep(2);

    // There is no need to explicitly deallocate resources before leaving by exit(3) or return.
    // We registered cleanup functions by atexit(3).

    printf("Leaving main() using: %s.\n", exit_types[EXIT_TYPE]);
#if EXIT_TYPE == EXIT_BY_EXIT
    exit(EXIT_SUCCESS);	// a library function
#elif EXIT_TYPE == EXIT_BY__EXIT
    _exit(EXIT_SUCCESS);	// a system call (not for general use!)
#else
	return EXIT_SUCCESS;	// return from main() = call exit(3)
#endif
}
