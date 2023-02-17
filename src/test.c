/**
 * MeChelleCompare 1.0.0
 * (c)2023 - gizmore of WeChall.net
 * **NOT** MIT Licensed!
 **/
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define OP( assembly ) # assembly

//#define GIZMORE_DEBUG // Means adding printf calls to add breakpoints before asm calls and analyze better.

/**
 * Proof of Concept:
 * MeChelle - The Fast Branchless Constant Time Equality Test. (TFBCTET)
 * Returns 0 or 1 when equal.
 *
 * (c)2023 gizmore@wechall.net
 **/
uint8_t MeChelleCompareUInt8(uint8_t b, uint8_t c)
{
#ifdef GIZMORE_DEBUG
	printf("MeChelleCompareUInt8(%d, %d)\n", b, c);
#endif
	asm(
			"XOR  $0xFF,%1;\n\t" // EAX = EBX ^ $0xFF
			"XOR  %1,%2;\n\t"    // EAX = EAX ^ ECX
			"ADD  $0x1,%2;\n\t"  // EAX = EAX + 1
			"SETC %0;"           // EAX = CF
			: "=r"(b)            // Output b(%0) is in any register (let gcc choose)
			: "r" (b), "r"(c));  // input  b(%1) and c(%2) go into any as well
	return b; // return the b variable
}

uint8_t MeChelleCompareUInt64(uint64_t b, uint64_t c)
{
#ifdef GIZMORE_DEBUG
	printf("MeChelleCompareUInt64(%llu, %llu)\n", b, c);
#endif
	int8_t a = 0;
	asm(
			"XOR  $0xFFFFFFFFFFFFFFFF,%1;\n\t" // EAX = EBX ^ $0xFF
			"XOR  %1,%2;\n\t"                  // EAX = EAX ^ ECX
			"ADD  $0x1,%2;\n\t"                //       INC   EAX
			"SETC %0;\n\t"                     //       SETC  EAX
			: "=r"(a)          // Output %0 is in any lregister (let gcc choose)
			: "r"(b), "r"(c)); // input b(%1) and c(%2) go into any as well
	return a; // return the b variable (push on stack?)
}

uint8_t MeChelleCompareMemory(const void *a, const void *b, size_t size)
{
#ifdef GIZMORE_DEBUG
	printf("MeChelleCompareUInt64(%p, %p, %llu)\n", a, b, size);
#endif
	uint8_t eax; // the return value
	const uint8_t step = sizeof(size);      // do `step` bytes per iteration.
	const uint64_t ff = 0xFFFFFFFFFFFFFFFF; // the bitmask to generate the 1s complement.

	uint64_t i = 0;

	loop:


	asm(
			"LOOP:\n\t"
			"XOR  $,%1;\n\t" // EAX = EBX ^ $0xFF
			"XOR  %1,%2;\n\t"                  // EAX = EAX ^ ECX
			"ADD  $0x1,%2;\n\t"                //       INC   EAX
			"SETC %0;\n\t"                     //       SETC  EAX
			: "=r"(a)          // Output %0 is in any lregister (let gcc choose)
			: "r"(b), "r"(c)); // input b(%1) and c(%2) go into any as well

	return eax;
}

/**
 *  Launch all tests.
 */
int main()
{
	return  testUINT8() -
			testUINT64() -
			measure();
}

/**
 * This function tests the single byte algorithm.
 * The first step.
 */
int testUINT8()
{
    const uint8_t a = 'x'; // Compare the letter 'G' (it's ascii value).

	// We test all possible 256 combinations against the letter 'x', and count every outcome.
	// If the algo returns different values from 0 and 1 we have a memory corrupting bug.
	uint8_t counted[2] = { 0, 0 }; // two counters
	uint8_t c = 0; // outcome
	uint8_t i = 0; // tricky to count to 256 with only 1 byte.
	do
	{
		c = MeChelleCompareUInt8(a, i); // check 0-255
		counted[c]++; // count outcome
		i++; // so we start checking with +1 until...
	}
	while (i != 0); // we overflowed to zero?

	// Check correctness.
	assert(counted[0] == 255); // 255 times differ
	assert(counted[1] == 1); // 1 time same

	// -----------------------------------------------------

	// Now we check all 256 combinations that should yield 1. (i == i)
	// For me, then this challenge is solved for today.

	counted[0] = counted[1] = 0; // clear previous values.

	do
	{
		c = MeChelleCompareUInt8(i, i); // check 0-255
		counted[c]++; // count outcome
		i++; // so we start checking with +1 until...
	}
	while (i != 0); // we overflowed to zero?
	// Check correctness.
	assert(counted[0] == 0); // 0 times counted
	assert(counted[1] == 0); // 1 time overflown ^^

	// Wait... damn, now it could be that nothing was counted at all.
	// Instead we might have a non-crashing stack corruption or?...
	// Well...
	assert(MeChelleCompareUInt8('x', 'x')); // Hah!
	printf("All tests passed. Algo is working!\n");
    return 0; // gizmore was here :)
}

/**
 * Test vectors for the 64 bit version.
 */
int testUINT64()
{
#ifdef GIZMORE_DEBUG
	printf("Testing the 64 bit assembly versions...\n");
#endif
	assert(+MeChelleCompareUInt64(0x0000000000000000, 0x0000000000000000));
	assert(+MeChelleCompareUInt64(0xffffffffffffffff, 0xffffffffffffffff));
	assert(+MeChelleCompareUInt64(0x4D654368656C6C65, 0x4D654368656C6C65));
	assert(!MeChelleCompareUInt64(0x4D654368656C6C64, 0x4D654368656C6C65));
	return 0;
}

int measure(void *func)
{
	const size_t iters = 10000;
    struct timespec tstart, tend;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tstart);
    for (int i = 0; i < iters; i++)
    {
		testMEMCMP_zeroed();
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tend);
    printf("It takes %lu nanoseconds for 100,000 empty iterations.\n", tend.tv_nsec - tstart.tv_nsec);
}

/**
 * Test vectors for the memcpy looped version
 */
int testMEMCMP_zeroed()
{
#ifdef GIZMORE_DEBUG
	printf("Testing the memcmp version with 1MB of two zeroed pages 2000 times...\n");
#endif

    return 0;

	const size_t size = 1024;     // 1kb size to...
	const void *a = malloc(size); // allocate two "strings", or...
	const void *b = malloc(size); // whatever you need the mem for.

	memset(a, 0, size); // Clear both regions
	memset(b, 0, size); //

	assert( (!memcmp(a, b)) == MeChelleCompareMemory(a, b, size));

	return 0;
}
