/**********************************************************************
 * Copyright (c) 2019
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
#ifndef __VM_H__
#define __VM_H__

#include "types.h"

/* The number of physical page frames of the system */
#define NR_PAGEFRAMES	128

/* The number of PTEs in a page */
#define PTES_PER_PAGE_SHIFT	4
#define NR_PTES_PER_PAGE    (1 << PTES_PER_PAGE_SHIFT)

#define RW_READ  0x01
#define RW_WRITE 0x02

/**
 * 2-level page table abstraction
 */
struct pte {
	bool valid;
	bool writable;
	unsigned int pfn;
	unsigned int private;	/* May used to backup something ... */
};

struct pte_directory {
	struct pte ptes[NR_PTES_PER_PAGE];
};

struct pagetable {
	struct pte_directory *outer_ptes[NR_PTES_PER_PAGE];
};


/**
 * Simplified PCB
 */
struct process {
	unsigned int pid;

	struct pagetable pagetable;

	struct list_head list;  /* List head to chain processes on the system */
};

#endif
