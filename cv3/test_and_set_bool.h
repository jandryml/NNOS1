// Operating Systems: sample code  (c) Tomáš Hudec
// Critical Sections
// HW method: test-and-set (xchg)
//
// Modified: 2015-11-25, 2017-12-06, 2020-11-23, 2021-11-09 (Intel syntax)
//
// usage:
//
// #include <stdbool.h>
// #include "test_and_set_bool.h"
//
// volatile bool locked = false;
//
// ret = test_and_set(&locked);

// atomic store of true into *locked and return previous value of *locked
__attribute__ ((always_inline)) static inline
int test_and_set(volatile bool *locked);


// atomic store of true into *locked and return previous value of *locked
int test_and_set(volatile bool *locked)
{
	register bool ret = true;		// value to be stored to *locked

	asm volatile (
		"xchg %0, %1"
		: "=r" (ret)			// output is previous value of *locked, stored to ret
		: "m" (*locked), "0" (ret)	// input is *locked and ret
		// "0" refers to the same constraint as %0 ("r", i.e. register, in this case)
		: "memory"			// memory is changed, tells gcc NOT to keep variables in registers
	);

	return ret;
}
