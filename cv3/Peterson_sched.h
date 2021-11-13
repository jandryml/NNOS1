// Operating Systems: sample code  (c) Tomáš Hudec
// Critical Sections
// Peterson's algorithm / Petersonův algoritmus
// busy waiting uses sched_yield(2) / aktivní čekání používá sched_yield(2)
//
// Last modification: 2015-11-25, 2020-12-02
//
// The race on SMP systems is NOT solved, i.e. it fails on SMP systems.
// Selhání na systémech SMP NENÍ zabráněno, tj. program selhává na systémech SMP.

#include <stdbool.h>
#include <sched.h>		// sched_yield(2)

// Peterson's data: flag for each process/thread and turn
// static variables are always initialized to 0 (C standard)
static volatile	
bool flag[2];
static volatile
int turn;

// Peterson's init
static inline	// note: inline is not used unless asked for optimization
void Peterson_init(void) __attribute__((always_inline));

// Peterson's wait
static inline	// note: inline is not used unless asked for optimization
void Peterson_wait(int self) __attribute__((always_inline));

// Peterson's signal
static inline
void Peterson_post(int self) __attribute__((always_inline));


// Peterson's init
static inline	// note: inline is not used unless asked for optimization
void Peterson_init(void)
{
	flag[0] = false;	// thread 0: access to the critical section NOT needed
	flag[1] = false;	// thread 1: access to the critical section NOT needed
	// turn = 0;		// not needed, it's always set before reading
}

// Peterson's wait
static inline	// note: inline is not used unless asked for optimization
void Peterson_wait(int self)
{
	flag[self] = true;			// require access to the critical section
	turn = !self;				// the other thread takes precedence
	while (flag[!self] && turn != self)
		sched_yield();	// spin lock -- busy waiting with giving up the CPU
}

// Peterson's signal
static inline	// note: inline is not used unless asked for optimization
void Peterson_post(int self)
{
	flag[self] = false;			// leave the critical section
}

