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
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "types.h"
#include "counter.h"

static pthread_t counter_thread = 0;

static unsigned long nr_requests = 0;
static unsigned long value_counter[MAX_VALUE] = { 0 };

int counter_delay_usec = 0;

int __dequeue_rb(void);

void *counter_main(void *_args_)
{
	if (verbose) printf("Counting %lu requests...\n", nr_requests);

	for (unsigned long i = 0; i < nr_requests; i++) {
		/* Take out a value from the ring buffer */
		int value = __dequeue_rb();

		/* Count it */
		value_counter[value]++;

		if (counter_delay_usec) usleep(counter_delay_usec);

		if (verbose && i && i % (nr_requests >> 4) == 0) {
			printf("Counter counted %lu / %lu (%lu%%)\n",
					i, nr_requests, i * 100 / nr_requests);
		}
	}

	if (verbose) printf("Counting finished...\n");

	return 0;
}

int spawn_counter(const enum counter_types type, const unsigned long _nr_requests_)
{
	nr_requests = _nr_requests_;

	pthread_create(&counter_thread, NULL, counter_main, NULL);
	return 0;
}

void fini_counter(unsigned long values[])
{
	if (counter_thread) {
		pthread_join(counter_thread, NULL);
		memcpy(values, value_counter, sizeof(unsigned long) * MAX_VALUE);
	}
}
