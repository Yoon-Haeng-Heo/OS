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

#ifndef __GENERATOR_H__
#define __GENERATOR_H__

enum generator_types {
	generator_unknown = 0,
	generator_random,
	generator_constant,
	generator_mixed,
};

int spawn_generators(const enum generator_types);
void do_generate(void);
void fini_generators(unsigned long []);

#endif
