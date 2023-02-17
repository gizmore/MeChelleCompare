/**
 * MeChelleCompare 1.0.0
 * (c)2023 - gizmore of WeChall.net
 * **NOT** MIT Licensed!
 **/
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define GIZMORE_DEBUG // Means adding printf calls to add breakpoints before asm calls and analyze better.

/**
 * Proof of Concept:
 * MeChelle - The branchless equality test.
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
			"ADD  $0x1,%2;\n\t" //        INC   EAX
			"SETC %0;"           //       SETC  EAX
			: "=r"(b) // Output %0 is in any register (let gcc choose)
			: "r"(b), "r"(c)); // input b(%1) and c(%2) go into any as well
	return b; // return the b variable (push on stack?)
}

uint8_t MeChelleCompareUInt64(uint64_t a, uint64_t b)
{
    return 0;
}

/**
 * This main function tests the correctness of the algorithm.
 */
int main()
{
    const uint8_t a = 'x'; // Compare the letter 'G' (it's ascii value).

	// We test all possible 256 combinations against the letter 'x', and count every outcome.
	// If the algo returns different values from 0 and 1 we have a memory corrupting bug.
	uint8_t counted[2] = {0, 0}; // two counters
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
	printf("All tests passed. Algo is working!");
    return 0; // gizmore was here :)
}
