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

#ifndef __TYPES_H__
#define __TYPES_H__

typedef int bool;
#define true	1
#define false	0

extern bool verbose;



/* Belows are for the framework, so don't use them */

enum lock_types {
	lock_spinlock = 0,
	lock_mutex = 1,
	lock_semaphore = 2,
};

#define MIN_VALUE 0
#define MAX_VALUE 128

extern int nr_generators;
extern unsigned long nr_generate;

extern int counter_delay_usec;
extern int generator_delay_usec;

#define __print_message(string, args...) \
	if (verbose) { \
		printf(string, ##args); \
		fflush(stdout); \
	}

#endif
