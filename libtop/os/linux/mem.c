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

#include "mem.h"
#include "log.h"
#include "helper.h"

static const unsigned long _monita_system_memory =
	(1L << MONITA_MEM_TOTAL)	+	(1L << MONITA_MEM_USED)		+
	(1L << MONITA_MEM_FREE)		+	(1L << MONITA_MEM_SHARED)	+
	(1L << MONITA_MEM_BUFFER)	+	(1L << MONITA_MEM_CACHED)	+
	(1L << MONITA_MEM_USER);

#define FILENAME	"/proc/meminfo"

/**
 * XXX: IMPORTANT - show total, used, free, buffers, cached
 */
void getMemoryUsage(monita_mem* buf)
{
	char buffer [BUFSIZ];

	memset(buf, 0, sizeof* buf);

	fileToBuffer(buffer, sizeof buffer, FILENAME);

	buf->total  = getScaled(buffer, "MemTotal:");
	buf->free   = getScaled(buffer, "MemFree:");
	buf->used   = buf->total - buf->free;
	buf->shared = 0;
	buf->buffer = getScaled(buffer, "Buffers:");
	buf->cached = getScaled(buffer, "Cached:");

	buf->user	= buf->total - buf->free - buf->cached - buf->buffer;
	buf->flags	= _monita_system_memory;
}
