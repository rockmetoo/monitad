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

#include <fcntl.h>
#include "swap.h"
#include "log.h"
#include "helper.h"
#include "osinfo.h"

#define MEMINFO		"/proc/meminfo"
#define PROC_STAT	"/proc/stat"
#define PROC_VMSTAT	"/proc/vmstat"

/**
 * XXX: IMPORTANt - result will show in Kb
 */
void getSwapUsage(monita_swap* buf)
{
	char buffer [BUFSIZ];
	char* p;

	memset(buf, 0, sizeof (monita_swap));

	fileToBuffer(buffer, sizeof buffer, MEMINFO);

	// XXX: IMPORTANT - Kernel 2.6 with multiple lines
	buf->total	= getScaled(buffer, "SwapTotal:");
	buf->free	= getScaled(buffer, "SwapFree:");
	buf->cached	= getScaled(buffer, "Cached:");
	buf->used	= buf->total - buf->free;

	unsigned long osVersionCode = getLinuxVersionCode();

	if(osVersionCode >= LINUX_VERSION_CODE(2, 6, 0))
	{
		fileToBuffer(buffer, sizeof buffer, PROC_VMSTAT);

		p = strstr(buffer, "\npswpin");

		if(p)
		{
			p = skipToken(p);
			buf->pagein  = strtoull(p, &p, 0);

			p = skipToken(p);
			buf->pageout = strtoull(p, &p, 0);
		}
	}
	else // Linux 2.4
	{
		fileToBuffer(buffer, sizeof buffer, PROC_STAT);

		p = strstr(buffer, "\nswap");

		if(p)
		{
			p = skipToken(p);

			buf->pagein  = strtoull(p, &p, 0);
			buf->pageout = strtoull(p, &p, 0);
		}
	}
}
