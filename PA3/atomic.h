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

#ifndef _ATOMIC_H_
#define _ATOMIC_H_

/**
 * Set *@value to @new if *@value was @old.
 * Return the old value of *@value
 *
 * *value  old  new  -->  return *value
 *      0   0    0             0   0
 *      0   0    1             0   1
 *      0   1    0             0   0
 *
 *      1   1    1             1   1
 *      1   1    0             1   0
 *      1   0    0             1   1
 */
static inline int compare_and_swap(int *value, int old, int new)
{
	__asm__ volatile (
		"lock ; cmpxchg %3, %1"
			: "=a"(old), "=m"(*value)
			: "a"(old), "r"(new)
			: "memory" );
	return old;
}
#endif
