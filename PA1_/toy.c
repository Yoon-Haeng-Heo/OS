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
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

int main(int argc, const char *argv[])
{
	fprintf(stderr, "pid  = %d\n", getpid());
	fprintf(stderr, "argc = %d\n", argc);

	for (int i = 0; i < argc; i++) {
		fprintf(stderr, "argv[%d] = %s\n", i, argv[i]);
	}

	if (argc >= 3 && strncmp(argv[1], "sleep", strlen("sleep")) == 0) {
		int sleep_sec = sleep_sec = atoi(argv[2]);
		sleep(sleep_sec);
	}

	fprintf(stderr, "done!\n");

	return 0;
}
