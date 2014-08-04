/* Copyright (C) 1998-99 Martin Baulig
   This file is part of LibGTop 1.0.

   Contributed by Martin Baulig <martin@home-of-linux.org>, April 1998.

   LibGTop is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License,
   or (at your option) any later version.

   LibGTop is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with LibGTop; see the file COPYING. If not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __cpu_h__
#define __cpu_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define MONITA_MAX_CPU		18

/* Nobody should really be using more than 4 processors.
   Yes we are :)
   Nobody should really be using more than 32 processors.
*/
#define MONITA_NCPU		1024

typedef struct _monita_cpu	monita_cpu;

struct _monita_cpu
{
	uint64_t flags;
	uint64_t total;							// MONITA_CPU_TOTAL
	uint64_t user;							// MONITA_CPU_USER
	uint64_t nice;							// MONITA_CPU_NICE
	uint64_t sys;							// MONITA_CPU_SYS
	uint64_t idle;							// MONITA_CPU_IDLE
	uint64_t iowait;						// MONITA_CPU_IOWAIT
	uint64_t irq;							// MONITA_CPU_IRQ
	uint64_t softirq;						// MONITA_CPU_SOFTIRQ
	uint64_t frequency;						// MONITA_CPU_FREQUENCY
	unsigned nCPU;
	unsigned nRealCPU;
	uint64_t xcpu_total		[MONITA_NCPU];	// MONITA_XCPU_TOTAL
	uint64_t xcpu_user		[MONITA_NCPU];	// MONITA_XCPU_USER
	uint64_t xcpu_nice		[MONITA_NCPU];	// MONITA_XCPU_NICE
	uint64_t xcpu_sys		[MONITA_NCPU];	// MONITA_XCPU_SYS
	uint64_t xcpu_idle		[MONITA_NCPU];	// MONITA_XCPU_IDLE
	uint64_t xcpu_iowait	[MONITA_NCPU];	// MONITA_XCPU_IOWAIT
	uint64_t xcpu_irq		[MONITA_NCPU];	// MONITA_XCPU_IRQ
	uint64_t xcpu_softirq	[MONITA_NCPU];	// MONITA_XCPU_SOFTIRQ
	uint64_t xcpu_flags;	                // MONITA_XCPU_IDLE
};

void getCPUUsage(monita_cpu* buf);

#ifdef __cplusplus
}
#endif
#endif
