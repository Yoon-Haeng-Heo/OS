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
#include <pthread.h>
#include <assert.h>
#include <fcntl.h>

#include "types.h"
#include "locks.h"

#include <sys/time.h>
#include <sys/resource.h>

pthread_barrier_t barrier;

static void *testlock;
static int testlock_held = 0;
static enum lock_types lock_type;

static int nr_tested = 0;
static int testing_duration_sec = 5;

const int nr_testers = 4;

/* Wrapper functions to locks */
static inline const char *__lock_type(void)
{
	if (lock_type == lock_spinlock) {
		return "spinlock";
	} else if (lock_type == lock_mutex) {
		return "mutex";
	}
}

static inline void __lock(void)
{
	if (lock_type == lock_spinlock) {
		acquire_spinlock(testlock);
	} else if (lock_type == lock_mutex) {
		acquire_mutex(testlock);
	}
}

static inline void __unlock(void)
{
	if (lock_type == lock_spinlock) {
		release_spinlock(testlock);
	} else if (lock_type == lock_mutex) {
		release_mutex(testlock);
	}
}

static inline void __init_lock(void)
{
	if (lock_type == lock_spinlock) {
		init_spinlock(testlock);
	} else if (lock_type == lock_mutex) {
		init_mutex(testlock);
	}
}


static int hold_duration_usec = 0;
static int progress = 0;
static bool lock_in_order = true;
static bool keep_testing = true;

static void *test_thread(void *_arg_)
{
	long id = (long)_arg_;
	pthread_barrier_wait(&barrier);
	
	/* Doing test #1 to #4 */
	while (keep_testing) {
		__lock();

		assert(testlock_held == 0);
		testlock_held = 1;
		if (hold_duration_usec) usleep(hold_duration_usec);

		nr_tested++;

		assert(testlock_held == 1);
		testlock_held = 0;
		assert(testlock_held == 0);

		__unlock();
		if (hold_duration_usec) usleep(hold_duration_usec);
	}

	/* Do test #5 */
	pthread_barrier_wait(&barrier);
	
	usleep(id * 10000);
	__lock();
	if (id != progress++) {
		lock_in_order = false;
	}
	__print_message("   %ld acquired the lock\n", id);
	usleep((nr_testers - id) * 100000);
	__unlock();

	pthread_barrier_wait(&barrier);
	return 0;
}

static bool is_busywaiting(void)
{
	struct rusage usage;

	__lock();
	for (int i = 0; i < 2; i++) {
		__print_message(".");
		sleep(1);
	}
	__unlock();
	getrusage(RUSAGE_SELF, &usage);
	// printf("%d.%06d\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
	return usage.ru_utime.tv_sec > testing_duration_sec;
}

void test_lock(enum lock_types _lock_type_)
{
	pthread_t tester[nr_testers];
	pthread_barrier_init(&barrier, NULL, nr_testers + 1);
	int temp = 0;
	lock_type = _lock_type_;
	bool ret = false;

	__print_message("0. Testing '%s'\n", __lock_type());

	/**
	 * We don't know the actual size of the locking primitive object here.
	 * So, just allocate a big memory, and ask to initialize it as a lock ;-)
	 */
	testlock = malloc(4096);
	__init_lock();

	/*********************************************************
	 * Check the mutual exclusive property.
	 * 1. The main thread graps the lock.
	 * 2. Spawn many tester threads.
	 * 3. Tester threads tries to acquire the lock.
	 */
	__print_message("1. Check the basic mutual exclusive property...");
	__lock();
	assert(testlock_held == 0);
	testlock_held = 1;

	for (int i = 0; i < nr_testers; i++) {
		pthread_create(tester + i, NULL, test_thread, (void *)(long)i);
	}
	pthread_barrier_wait(&barrier); /* Wait until test threads are ready */

	assert(testlock_held == 1);
	testlock_held = 0;
	sleep(1);
	assert(testlock_held == 0);
	__unlock();
	__print_message("  [Done]\n");

	/*********************************************************
	 * Testing threads keep torturing the lock for @testing_duration_sec.
	 */
	__print_message("2. Verify the mutual exclusiveness further");
	fflush(stdout);
	for (int i = 0; i < testing_duration_sec; i++) {
		sleep(1);
		__print_message(".");
	}
	__print_message("  [Done]\n");
	fprintf(stderr, "   Performance: %.1f operations/sec\n", (float)nr_tested / testing_duration_sec);

	/*********************************************************
	 * Testing possible-race condition.
	 * Will be blocked indefinitely if a race condition happens.
	 */
	__print_message("3. Check race condition during waking up.......");
	fflush(stdout);
	hold_duration_usec = 1;
	for (int i = 0; i < 100; i++) {
		__lock();
		usleep(hold_duration_usec);
		__unlock();
	}
	__print_message("  [Done]\n");

	__print_message("4. Analyze lock waiting policy....");
	ret = is_busywaiting();
	__print_message("             [Done]\n");
	fprintf(stderr, "   Seem to be a %s lock\n", ret ? "busy-waiting" : "blocking");
	assert((lock_type == lock_spinlock && ret == true) ||
		   (lock_type == lock_mutex && ret == false));

	keep_testing = false;

	__print_message("5. Analyze the lock waiting order...\n");
	pthread_barrier_wait(&barrier);

	pthread_barrier_wait(&barrier);
	fprintf(stderr, "   Waiting %s\n", lock_in_order ? "in order" : "out of order");

	for (int i = 0; i < nr_testers; i++) {
		pthread_join(tester[i], NULL);
	}
	assert(testlock_held == 0);
	if (lock_type == lock_spinlock || (lock_type == lock_mutex && lock_in_order)) {
		fprintf(stderr, "\n >>>> Congraturations! Your %s implementation looks great!! <<<<\n\n", __lock_type());
	} else {
		assert(0 && "wrong lock wait ordering for mutex");
	}

	return;
}
