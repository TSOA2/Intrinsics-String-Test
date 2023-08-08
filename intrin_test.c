/*
 * Measure the performance improvements with intel intrinsics
 * TSOA
 */


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <time.h>

#ifdef __linux__
#define CLOCK CLOCK_MONOTONIC_RAW
#else
#define CLOCK CLOCK_MONOTONIC
#endif

#define ITERATIONS (10000000)

/* 1 for same, 0 if not */
int normal_compare(char a[16], char b[16])
{
	for (int i = 0; i < 16; i++) {
		if (a[i] != b[i]) {
			return 0;
		}
	}

	return 1;
}

/* 1 for same, 0 if not */
int intrinsic_compare(char a[16], char b[16])
{
	uint64_t h1;
	uint64_t h2;
	__m128i a_128;
	__m128i b_128;
	__m128i dest;

	a_128 = _mm_load_si128((const __m128i *) a);
	b_128 = _mm_load_si128((const __m128i *) b);
	dest = _mm_cmpeq_epi16(a_128, b_128);

	h1 = (uint64_t) _mm_extract_epi64(dest, 1);
	h2 = (uint64_t) _mm_extract_epi64(dest, 0);

	if ((h1 & h2) == (uint64_t) -1) { /* underflow back to top */
		return 1;
	}

	return 0;
}

/* Generate random 16-byte array 
 * Return 1 if equal, 0 if not
 */

int generate_random(char a_string[16], char b_string[16])
{
	srand(time(NULL));

	for (int i = 0; i < 16; i++) {
		a_string[i] = rand() % 255;
	}

	/* Pick a random choice: strings are equal or not */
	if (rand() & 1) { /* strings are equal */
		memcpy(b_string, a_string, 16);
		return 1;
	}

	for (int i = 0; i < 16; i++) {
		b_string[i] = rand() % 255;
	}
	return 0;
}

int main(int argc, char **argv)
{
	/*
	 * Must be aligned to 16 bytes (128 bits).
	 * Otherwise exception is generated, which ruins the experiment
	 */

	static char a_string[16]  __attribute__((aligned(16)));
	static char b_string[16]  __attribute__((aligned(16)));

	int equal;

	int intrin_return;
	int normal_return;

	double intrin_time = 0.0;
	double normal_time = 0.0;

	struct timespec start, end;

	for (size_t i = 0; i < ITERATIONS; i++) {
		equal = generate_random(a_string, b_string);

		clock_gettime(CLOCK, &start);
		normal_return = normal_compare(a_string, b_string);
		clock_gettime(CLOCK, &end);
		normal_time += (double) (end.tv_nsec-start.tv_nsec)/1000000000 + (end.tv_sec-start.tv_sec);

		if (normal_return != equal) {
			(void) printf("normal fail.\n");
		}

		clock_gettime(CLOCK, &start);
		intrin_return = intrinsic_compare(a_string, b_string);
		clock_gettime(CLOCK, &end);
		intrin_time += (double) (end.tv_nsec-start.tv_nsec)/1000000000 + (end.tv_sec-start.tv_sec);

		if (intrin_return != equal) {
			(void) printf("intrin fail.\n");
		}
	}


	normal_time /= ITERATIONS;
	intrin_time /= ITERATIONS;

	(void) printf("INTRIN TIME: %.15lf, NORMAL TIME: %.15lf\n", intrin_time, normal_time);
	return 0;
}
