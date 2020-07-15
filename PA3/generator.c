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
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include "types.h"
#include "generator.h"

/* Barrier to synchronize generators */
static pthread_barrier_t barrier;

int generator_delay_usec = 0;

/* Assorted generator functions */
int generator_fn_constant(int id)
{
	return 43;
}

int generator_fn_random(int id)
{
	if (generator_delay_usec) usleep(generator_delay_usec);
	return MIN_VALUE + (random() % (MAX_VALUE - MIN_VALUE));
}

int (*assign_generator_fn(int id, enum generator_types type))(int)
{
	switch(type) {
	case generator_constant:
		return &generator_fn_constant;
	case generator_random:
		return &generator_fn_random;
	case generator_mixed:
		return id % 2 == 0 ? &generator_fn_constant : &generator_fn_random;
	default:
		assert(0);
		break;
	}

	/* Unreachable */
	assert(0);
}

struct generator {
	pthread_t thread;
	int id;
	int (*generator_fn)(int id);
	unsigned long generated[MAX_VALUE];
};
static struct generator *generators = NULL;

void __enqueue_rb(int value);

void *generator_main(void *_args_)
{
	struct generator *my = (struct generator *)_args_;

	if (verbose) printf("Generator %d started...\n", my->id);

	pthread_barrier_wait(&barrier); /* 1st barrier */

	for (unsigned long i = 0; i < nr_generate; i++) {
		/* Generate a number */
		int value = my->generator_fn(my->id);

		/* The generator inserts the generated number into the ring buffer */
		__enqueue_rb((int)value);
		
		/* Account for the generated value */
		my->generated[value]++;

		if (verbose && i && i % (nr_generate >> 4) == 0) {
			printf("Generator %d generated %lu / %lu (%lu%%)\n",
					my->id, i, nr_generate, i * 100 / nr_generate);
		}
	}
	if (verbose) printf("Generator %d finished...\n", my->id);

	pthread_barrier_wait(&barrier); /* 2nd barrier */

	/* Wait for main thread to collect generated value counts */
	pthread_barrier_wait(&barrier);	/* 3rd barrier */

	return 0;
}

int spawn_generators(const enum generator_types type)
{
	assert(nr_generators > 0);
	assert(nr_generate > 0);

	generators = calloc(nr_generators, sizeof(*generators));
	assert(generators);
	bzero(generators, sizeof(*generators) * nr_generators);

	pthread_barrier_init(&barrier, NULL, nr_generators + 1);

	for (int i = 0; i < nr_generators; i++) {
		struct generator *g = generators + i;
		g->id = i;
		g->generator_fn = assign_generator_fn(i, type);
		pthread_create(&g->thread, NULL, generator_main, g);
	}

	pthread_barrier_wait(&barrier);	/* 1st barrier */

	return 0;
}

void do_generate(void)
{
	pthread_barrier_wait(&barrier);	/* 2nd barrier */
}

void fini_generators(unsigned long values[])
{
	/* Collect generation result */
	for (int i = 0; i < nr_generators; i++) {
		struct generator *g = generators + i;
		for (int n = MIN_VALUE; n < MAX_VALUE; n++) {
			values[n] += g->generated[n];
		}
	}

	for (int n = MIN_VALUE; n < MAX_VALUE; n++) {
		if (!values[n]) continue;
	}

	pthread_barrier_wait(&barrier);	/* 3rd barrier */

	for (int i = 0; i < nr_generators; i++) {
		struct generator *g = generators + i;
		if ((unsigned long)(g->thread) != 0) {
			pthread_join(g->thread, NULL);
		}
	}
	free(generators);
}
