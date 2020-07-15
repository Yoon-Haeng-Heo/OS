/**********************************************************************
 * Copyright (c) 2020
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <assert.h>

#include "types.h"

#include "locks.h"
#include "generator.h"
#include "counter.h"

/*************************************************
 * Lock tester.
 * Will be invoked if the program is run with -T
 */
void test_lock(enum lock_types);

/* Common */
int verbose = 1;

/* Generator */
static enum generator_types generator_type = generator_constant;
int nr_generators = 1;
unsigned long nr_generate = 128;

/* Counter */
static enum counter_types counter_type = counter_normal;

/* Ring buffer */
static int nr_slots = 64;

/*********************************************************************
 * Common implementation
 */
void enqueue_into_ringbuffer(int value);
int dequeue_from_ringbuffer(void);
void fini_ringbuffer(void);
int init_ringbuffer(const int nr_slots);

void __enqueue_rb(int value)
{
	assert(value >= MIN_VALUE && value < MAX_VALUE);
	enqueue_into_ringbuffer(value);
}

int __dequeue_rb(void)
{
	int value;

	value = dequeue_from_ringbuffer();
	assert(value >= MIN_VALUE && value < MAX_VALUE);

	return value;
}

static int __init_rb(const int _nr_slots_)
{
	assert(_nr_slots_ > 0);
	return init_ringbuffer(_nr_slots_);
}

static void __fini_rb(void)
{
	fini_ringbuffer();
}

static void __print_usage(const char *argv0)
{
	printf("Usage: %s {options}\n", argv0);
	printf("\n");
	printf(" Run with -l or -m to check the correctness of the lock implementation\n");
	printf("  -l         : Test spinlock implementation\n");
	printf("  -m         : Torture blocking mutex\n");
	printf("\n");
	printf(" Run with -r to check the ring buffer implementation\n");
	printf("  -g [number]: Spawn @number generators for test\n");
	printf("  -n [number]: Generate @number requests per generator\n");
	printf("  -R         : Use random generator rather than constant generator\n");
	printf("  -s [number]: Set the number of slots in the ring buffer\n");
	printf("  -0         : Comprehensive test with realistic values\n");
	printf("  -1         : Test full ring buffer\n");
	printf("  -2         : Test empty ring buffer\n");
	printf("\n");
	printf("  -h | -?    : Print usage\n");
	printf("  -v | -q    : Make verbose or quiet\n");
	printf("\n");
}


int parse_options(int argc, char * const argv[])
{
	char opt;
	bool test_locks = false;
	bool test_ringbuffer = false;
	enum lock_types lock_type = lock_spinlock;

	while ((opt = getopt(argc, argv, "vqg:s:n:RrSml012h?")) != -1) {
		switch(opt) {
		case 'v':
			verbose = 1;
			break;
		case 'q':
			verbose = 0;
			break;
		case 'l':
			test_locks = true;
			break;
		case 'r':
			test_ringbuffer = true;
			break;
		case 'R':
			generator_type = generator_random;
			break;
		case 'g':
			nr_generators = atoi(optarg);
			break;
		case 'n':
			nr_generate = atoll(optarg);
			break;
		case 'm':
			test_locks = true;
			lock_type = lock_mutex;
			break;
		case 's':
			nr_slots = atoi(optarg);
			break;
		case '0':
			test_ringbuffer = true;
			generator_type = generator_random;
			nr_generators = 4;
			nr_generate = (1 << 12);
			verbose = 0;
			break;
		case '1': /* Overflow test */
			test_ringbuffer = true;
			nr_slots = 4;
			generator_type = generator_random;
			nr_generators = 8;
			nr_generate = (1 << 11);
			counter_delay_usec = 20;
			verbose = 0;
			break;
		case '2': /* Underflow test */
			test_ringbuffer = true;
			generator_type = generator_random;
			nr_generators = 4;
			nr_generate = (1 << 12);
			generator_delay_usec = 20;
			verbose = 0;
			break;
		case 'h':
		case '?':
		default:
			__print_usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (!test_locks && !test_ringbuffer) {
		__print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	if (test_locks) {
		test_lock(lock_type);
		exit(0);
	}
	return 0;
}

void compare_results(unsigned long generated_values[], unsigned long counted_values[])
{
	bool mismatch = false;

	for (int i = MIN_VALUE; i < MAX_VALUE; i++) {
		if (generated_values[i] != counted_values[i]) {
			if (!mismatch) {
				mismatch = true;
				printf(">>> Mismatching in generation and counting!!! <<<\n");
				printf("     -----------------------------\n");
				printf("      Value   generated   counted\n");
				printf("     -----------------------------\n");
			}
			fprintf(stderr, "      %4d : %8lu != %-8lu\n",
					i,generated_values[i], counted_values[i]);
		}
	}

	printf("\n");
	fprintf(stderr, ">>> The ring buffer is %sworking properly!! <<<\n", mismatch ? "**NOT** " : "");
	printf("\n");
}

int main(int argc, char * const argv[])
{
	int retval = EXIT_SUCCESS;
	unsigned long generated_values[MAX_VALUE] = {0};
	unsigned long counted_values[MAX_VALUE] = {0};
	unsigned long nr_requests_to_generate;

	struct timeval start, end;
	unsigned long elapsed;

	__print_message("\n");
	__print_message(" _               _      _____         _            \n");
	__print_message("| |    ___   ___| | __ |_   _|__  ___| |_ ___ _ __ \n");
	__print_message("| |   / _ \\ / __| |/ /   | |/ _ \\/ __| __/ _ \\ '__|\n");
	__print_message("| |__| (_) | (__|   <    | |  __/\\__ \\ ||  __/ |   \n");
	__print_message("|_____\\___/ \\___|_|\\_\\   |_|\\___||___/\\__\\___|_|\n");
	__print_message("\n");
	__print_message("                                    2020 Spring\n");
	__print_message("\n");

	if ((retval = parse_options(argc, argv))) {
		goto exit;
	}
	if ((retval = __init_rb(nr_slots))) {
		goto exit;
	}

	nr_requests_to_generate = nr_generate * nr_generators;

	if ((retval = spawn_counter(counter_type, nr_requests_to_generate))) {
		goto exit_ring;
	}

	spawn_generators(generator_type);

	gettimeofday(&start, NULL);
	do_generate();
	gettimeofday(&end, NULL);
	elapsed = (end.tv_sec * 1000000 + end.tv_usec) -
				(start.tv_sec * 1000000 + start.tv_usec);

	fini_generators(generated_values);
	fini_counter(counted_values);

	compare_results(generated_values, counted_values);
	printf(         "     # of requests : %lu\n", nr_requests_to_generate);
	printf(         "  Time to complete : %lu.%06lu\n", elapsed / 1000000, elapsed % 1000000);
	fprintf(stderr, "       Performance : %.1f req/sec\n",
			(float)nr_requests_to_generate * 1000000 / elapsed);
	printf("\n");

exit_ring:
	__fini_rb();
exit:
	return retval;
}
