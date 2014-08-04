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

#ifndef __mem_h__
#define __mem_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define MONITA_MEM_TOTAL	0
#define MONITA_MEM_USED		1
#define MONITA_MEM_FREE		2
#define MONITA_MEM_SHARED	3
#define MONITA_MEM_BUFFER	4
#define MONITA_MEM_CACHED	5
#define MONITA_MEM_USER		6
#define MONITA_MEM_LOCKED	7

#define MONITA_MAX_MEM		8

typedef struct _monita_mem	monita_mem;

struct _monita_mem
{
	uint64_t flags;
	uint64_t total;		// MONITA_MEM_TOTAL
	uint64_t used;		// MONITA_MEM_USED
	uint64_t free;		// MONITA_MEM_FREE
	uint64_t shared;	// MONITA_MEM_SHARED
	uint64_t buffer;	// MONITA_MEM_BUFFER
	uint64_t cached;	// MONITA_MEM_CACHED
	uint64_t user;		// MONITA_MEM_USER
	uint64_t locked;	// MONITA_MEM_LOCKED
};

void getMemoryUsage(monita_mem* buf);


#ifdef __cplusplus
}
#endif
#endif
